import json
import os
import sys

os.environ.setdefault("IDADIR", r"F:\IDA Professional 9.3_headless")

try:
    import idapro
    idapro.enable_console_messages(True)
except Exception:
    pass

from ida_domain import Database
from ida_domain.database import IdaCommandOptions


SERVER_NEEDLES = [
    "PickDefaultSpawnTeam",
    "eventKeyPressed",
    "eventKeyReleased",
    "eventPlayerSpawn",
    "eventPlayerActive",
    "DoLuaThinkFunctions",
    "gamerulesThink",
    "_EntSpawn",
    "_EntCreate",
    "_PlayerChangeTeam",
    "_PlayerInfo",
    "_PlayerIsKeyDown",
    "_PlayerGetShootAng",
    "_PlayerSpectatorStart",
    "_PlayerSpectatorTarget",
    "_PlayerSetChaseCamDistance",
    "_PhysApplyForce",
    "ApplyForceCenter",
    "EnableMotion",
]

CLIENT_NEEDLES = [
    "GModText",
    "GModTextAnimate",
    "GModTextHide",
    "GModRect",
    "GModRectAnimate",
    "GModRectHide",
    "materials/gmod/%s.vmt",
    ".vmt",
    ".vtf",
]

# Addresses from earlier xref passes. They are only used as seed functions;
# string/xref discovery below is still the primary source.
SERVER_SEED_EAS = [
    0x223C8240,  # PickDefaultSpawnTeam bridge caller
    0x223C8790,  # eventKeyPressed/eventKeyReleased bridge caller
    0x22454F50,  # _EntSpawn registration/function area from previous probe
    0x224565C0,  # _PlayerInfo registration/function area
    0x224565E0,  # _PlayerChangeTeam registration/function area
    0x22456900,  # _PlayerIsKeyDown registration/function area
    0x22456920,  # _PlayerSpectatorStart registration/function area
    0x22456940,  # _PlayerSpectatorTarget registration/function area
]

CLIENT_SEED_EAS = [
    0x24191A00,  # GMod material path handling area from previous probe
]


def _hex(ea):
    return None if ea is None else f"0x{int(ea):x}"


def _name(ida_name, ida_funcs, ea):
    name = ida_name.get_name(ea)
    if name:
        return name
    func = ida_funcs.get_func(ea)
    if func:
        return ida_funcs.get_func_name(func.start_ea)
    return f"sub_{int(ea):x}"


def _func_start(ida_funcs, ea):
    func = ida_funcs.get_func(ea)
    return int(func.start_ea) if func else None


def _decompile(ida_hexrays, ida_name, ida_funcs, ea):
    hf = ida_hexrays.hexrays_failure_t()
    cfunc = ida_hexrays.decompile(ea, hf)
    if not cfunc:
        raise RuntimeError(
            f"decompile failed for {_hex(ea)} code={int(hf.code)} "
            f"reason={str(hf.str)} at {_hex(hf.errea)}"
        )
    return {
        "ea": _hex(ea),
        "name": _name(ida_name, ida_funcs, ea),
        "pseudocode": str(cfunc),
    }


def analyze(db_path, mode, out_path):
    opts = IdaCommandOptions(auto_analysis=True, new_database=False)
    needles = CLIENT_NEEDLES if mode == "client" else SERVER_NEEDLES
    seeds = CLIENT_SEED_EAS if mode == "client" else SERVER_SEED_EAS

    result = {
        "database": db_path,
        "mode": mode,
        "hexrays_ok": False,
        "strings": [],
        "functions": [],
        "seed_functions": [],
    }

    with Database.open(db_path, opts, save_on_close=False):
        import ida_auto
        import ida_funcs
        import ida_hexrays
        import ida_name
        import ida_xref
        import idautils

        ida_auto.auto_wait()
        if not ida_hexrays.init_hexrays_plugin():
            raise RuntimeError("Hex-Rays failed to initialize")
        result["hexrays_ok"] = True

        func_reasons = {}

        for ea in seeds:
            start = _func_start(ida_funcs, ea)
            if start is not None:
                result["seed_functions"].append(
                    {"seed": _hex(ea), "function": _hex(start), "name": _name(ida_name, ida_funcs, start)}
                )
                func_reasons.setdefault(start, []).append(f"seed:{_hex(ea)}")

        for s in idautils.Strings():
            text = str(s)
            if not any(n.lower() in text.lower() for n in needles):
                continue

            refs = []
            for xr in idautils.XrefsTo(int(s.ea), 0):
                start = _func_start(ida_funcs, xr.frm)
                refs.append(
                    {
                        "from": _hex(xr.frm),
                        "function": _hex(start),
                        "function_name": _name(ida_name, ida_funcs, start) if start is not None else None,
                    }
                )
                if start is not None:
                    func_reasons.setdefault(start, []).append(f"string:{text}")

            result["strings"].append({"ea": _hex(s.ea), "text": text, "refs": refs[:32]})

        # Include local callers/callees for the discovered functions to expose lifecycle wiring.
        candidate_funcs = sorted(func_reasons)
        for ea in candidate_funcs[:80]:
            try:
                func_entry = _decompile(ida_hexrays, ida_name, ida_funcs, ea)
            except RuntimeError as exc:
                func_entry = {
                    "ea": _hex(ea),
                    "name": _name(ida_name, ida_funcs, ea),
                    "pseudocode": None,
                    "decompile_error": str(exc),
                }
            func_entry["reasons"] = sorted(set(func_reasons[ea]))[:12]
            callees = []
            callers = []

            for item in idautils.FuncItems(ea):
                for xr in idautils.XrefsFrom(item, 0):
                    if xr.iscode:
                        target_start = _func_start(ida_funcs, xr.to)
                        if target_start is not None and target_start != ea:
                            callees.append(
                                {
                                    "at": _hex(item),
                                    "target": _hex(target_start),
                                    "name": _name(ida_name, ida_funcs, target_start),
                                }
                            )

            for xr in idautils.XrefsTo(ea, 0):
                if xr.iscode:
                    caller_start = _func_start(ida_funcs, xr.frm)
                    if caller_start is not None and caller_start != ea:
                        callers.append(
                            {
                                "at": _hex(xr.frm),
                                "caller": _hex(caller_start),
                                "name": _name(ida_name, ida_funcs, caller_start),
                            }
                        )

            func_entry["callees"] = callees[:80]
            func_entry["callers"] = callers[:40]
            result["functions"].append(func_entry)

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)


def main():
    if len(sys.argv) != 4:
        raise SystemExit("usage: ida_gmod_lua_integration_deep.py <server|client> <db.i64> <out.json>")
    analyze(sys.argv[2], sys.argv[1], sys.argv[3])


if __name__ == "__main__":
    main()

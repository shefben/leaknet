import json
import os
import re
import sys

os.environ["IDADIR"] = r"F:\IDA Professional 9.3_headless"

import idapro
from ida_domain import Database
from ida_domain.database import IdaCommandOptions


SERVER_STRINGS = [
    "DoLuaThinkFunctions",
    "gamerulesThink",
    "PickDefaultSpawnTeam",
    "DoEventHook",
    "eventKeyPressed",
    "eventKeyReleased",
    "eventPlayerSpawn",
    "eventPlayerSpawnProp",
    "eventPlayerSpawnRagdoll",
    "eventPropBreak",
    "_EntSpawn",
    "_PlayerInfo",
    "_PlayerChangeTeam",
    "_PlayerIsKeyDown",
    "_PlayerSpectatorStart",
    "_PlayerSpectatorTarget",
    "_PlayerSetChaseCamDistance",
    "_PlayerGetShootAng",
    "_PhysApplyForce",
    "ApplyForceCenter",
    "EnableMotion",
    "_GModText_Send",
    "_GModRect_Send",
]

CLIENT_STRINGS = [
    "GModText",
    "GModTextAnimate",
    "GModTextHide",
    "GModTextHideAll",
    "GModRect",
    "GModRectAnimate",
    "GModRectHide",
    "GModRectHideAll",
    "WQuad",
    "WQuadAnimate",
    "WQuadHide",
    "WQuadHideAll",
    "GModHint",
    "GModToolText",
]


def stringify_pseudocode(cfunc):
    if not cfunc:
        return None
    text = str(cfunc)
    return "\n".join(line.rstrip() for line in text.splitlines())


def decompile(ea):
    import ida_hexrays

    hf = ida_hexrays.hexrays_failure_t()
    try:
        cfunc = ida_hexrays.decompile(ea, hf)
    except Exception as exc:
        return {"ok": False, "error": repr(exc)}
    if not cfunc:
        return {"ok": False, "error": f"code={hf.code} {hf.str}"}
    return {"ok": True, "pseudocode": stringify_pseudocode(cfunc)}


def func_info(ea):
    import ida_funcs
    import ida_name

    f = ida_funcs.get_func(ea)
    if not f:
        return None
    return {
        "start": f"0x{f.start_ea:x}",
        "end": f"0x{f.end_ea:x}",
        "name": ida_name.get_name(f.start_ea),
    }


def collect_strings(targets):
    import ida_bytes
    import ida_funcs
    import ida_name
    import ida_xref
    import idautils

    found = []
    unique_funcs = {}
    for s in idautils.Strings():
        text = str(s)
        if not any(t in text for t in targets):
            continue

        refs = []
        xb = ida_xref.xrefblk_t()
        ok = xb.first_to(s.ea, ida_xref.XREF_ALL)
        while ok:
            f = ida_funcs.get_func(xb.frm)
            info = None
            if f:
                info = func_info(f.start_ea)
                unique_funcs[f.start_ea] = info
            refs.append(
                {
                    "from": f"0x{xb.frm:x}",
                    "iscode": bool(xb.iscode),
                    "function": info,
                    "name_at_from": ida_name.get_name(xb.frm),
                    "disasm": ida_bytes.get_bytes(xb.frm, 16).hex(" ") if xb.iscode else None,
                }
            )
            ok = xb.next_to()

        found.append({"ea": f"0x{s.ea:x}", "text": text, "refs": refs})

    return found, unique_funcs


def parse_subs(pseudocode):
    if not pseudocode:
        return set()
    result = set()
    for match in re.finditer(r"\bsub_([0-9A-Fa-f]{6,})\b", pseudocode):
        result.add(int(match.group(1), 16))
    return result


def collect(binary, targets, mode):
    import ida_auto
    import ida_hexrays

    opts = IdaCommandOptions(auto_analysis=False, new_database=False)
    with Database.open(binary, opts, save_on_close=False) as db:
        ida_auto.auto_wait()
        hexrays_ok = bool(ida_hexrays.init_hexrays_plugin())

        strings, funcs = collect_strings(targets)
        decompiled = {}
        queue = list(funcs.keys())
        seen = set(queue)
        depth = 0
        while queue and depth < 3:
            next_queue = []
            for ea in queue:
                info = func_info(ea) or {"start": f"0x{ea:x}", "name": f"sub_{ea:x}"}
                dec = decompile(ea)
                entry = {"function": info, "decompile": dec}
                decompiled[f"0x{ea:x}"] = entry
                if dec.get("ok"):
                    for sub_ea in parse_subs(dec.get("pseudocode")):
                        if sub_ea not in seen and func_info(sub_ea):
                            seen.add(sub_ea)
                            next_queue.append(sub_ea)
            queue = next_queue
            depth += 1

        return {
            "binary": binary,
            "mode": mode,
            "hexrays_ok": hexrays_ok,
            "metadata": {
                "module": db.metadata.module,
                "base": f"0x{db.metadata.base_address:x}",
                "md5": db.metadata.md5,
                "bitness": db.metadata.bitness,
            },
            "strings": strings,
            "functions": decompiled,
        }


def main():
    idapro.enable_console_messages(True)

    root = os.getcwd()
    server = os.path.join(root, "gmod_9_0_4b", "bin", "server.dll.i64")
    client = os.path.join(root, "gmod_9_0_4b", "bin", "client.dll.i64")
    output = {
        "server": collect(server, SERVER_STRINGS, "server"),
        "client": collect(client, CLIENT_STRINGS, "client"),
    }
    out_path = os.path.join(root, "gmod904_lua_parity_probe.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(output, f, indent=2)
    print(out_path)
    print("server functions", len(output["server"]["functions"]))
    print("client functions", len(output["client"]["functions"]))


if __name__ == "__main__":
    main()

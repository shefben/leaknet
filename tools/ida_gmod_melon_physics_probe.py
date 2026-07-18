import json
import os
import sys

os.environ.setdefault("IDADIR", r"F:\IDA Professional 9.3_headless")

from ida_domain import Database
from ida_domain.database import IdaCommandOptions


NEEDLES = [
    "ApplyForceCenter",
    "ApplyForceOffset",
    "ApplyTorqueCenter",
    "EnableMotion",
    "HasPhysics",
    "_phys",
    "DoLuaThinkFunctions",
    "gamerulesThink",
    "_PlayerIsKeyDown",
    "_PlayerGetShootAng",
    "_PlayerGetShootPos",
    "Wrong syntax used on Phys",
    "Wrong syntax used on PlayerIsKeyDown",
    "Wrong syntax used on PlayerGetShootAng",
]


def main():
    if len(sys.argv) < 3:
        raise SystemExit("usage: ida_gmod_melon_physics_probe.py <db.i64> <out.json>")

    db_path = sys.argv[1]
    out_path = sys.argv[2]
    opts = IdaCommandOptions(auto_analysis=False, new_database=False)
    out = {
        "database": db_path,
        "strings": [],
        "xrefs": [],
        "functions": [],
    }

    with Database.open(db_path, opts, save_on_close=False):
        import ida_auto
        import ida_funcs
        import ida_hexrays
        import ida_name
        import idautils

        ida_auto.auto_wait()
        out["hexrays_ok"] = bool(ida_hexrays.init_hexrays_plugin())

        def name_at(ea):
            return ida_name.get_name(ea) or f"sub_{ea:x}"

        seen_funcs = set()

        def add_func(ea, reason):
            func = ida_funcs.get_func(ea)
            if not func:
                return
            start = int(func.start_ea)
            if start in seen_funcs:
                return
            seen_funcs.add(start)
            hf = ida_hexrays.hexrays_failure_t()
            cfunc = ida_hexrays.decompile(start, hf)
            out["functions"].append({
                "ea": f"0x{start:x}",
                "name": name_at(start),
                "reason": reason,
                "pseudocode": str(cfunc) if cfunc else None,
                "error": None if cfunc else {
                    "code": int(hf.code),
                    "str": str(hf.str),
                    "errea": f"0x{int(hf.errea):x}",
                },
            })

        for s in idautils.Strings():
            text = str(s)
            if not any(n in text for n in NEEDLES):
                continue
            s_ea = int(s.ea)
            out["strings"].append({"ea": f"0x{s_ea:x}", "text": text})
            for xr in idautils.XrefsTo(s_ea):
                frm = int(xr.frm)
                func = ida_funcs.get_func(frm)
                func_ea = int(func.start_ea) if func else None
                out["xrefs"].append({
                    "string": text,
                    "string_ea": f"0x{s_ea:x}",
                    "from": f"0x{frm:x}",
                    "function": f"0x{func_ea:x}" if func_ea is not None else None,
                    "function_name": name_at(func_ea) if func_ea is not None else None,
                })
                if func_ea is not None:
                    add_func(func_ea, f"xref:{text}")

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)


if __name__ == "__main__":
    main()

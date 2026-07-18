import json
import os
import sys

import idapro


def decompile_functions(addresses):
    import ida_auto
    import ida_funcs
    import ida_hexrays
    import ida_name

    ida_auto.auto_wait()
    hexrays_ok = bool(ida_hexrays.init_hexrays_plugin())

    out = {
        "hexrays_ok": hexrays_ok,
        "functions": {},
    }

    for label, ea in addresses.items():
        func = ida_funcs.get_func(ea)
        start_ea = func.start_ea if func else ea
        name = ida_name.get_name(start_ea) or "sub_%X" % start_ea
        entry = {
            "ea": "0x%X" % ea,
            "start": "0x%X" % start_ea,
            "name": name,
        }

        if hexrays_ok:
            hf = ida_hexrays.hexrays_failure_t()
            cfunc = ida_hexrays.decompile(start_ea, hf)
            if cfunc:
                entry["pseudocode"] = str(cfunc)
            else:
                entry["decompile_error"] = {
                    "code": int(hf.code),
                    "reason": str(hf.str),
                    "ea": "0x%X" % int(hf.errea),
                }
        out["functions"][label] = entry

    return out


def main():
    if len(sys.argv) < 3:
        print("usage: gmod_headless_lua_probe.py <server|client> <database-or-binary>")
        return 2

    mode = sys.argv[1]
    db_path = sys.argv[2]

    if not os.path.exists(db_path):
        print("missing input: %s" % db_path)
        return 2

    if mode == "server":
        addresses = {
            "DoLuaThinkFunctions": 0x223C7BE0,
            "_PlayerIsKeyDown_impl": 0x22410620,
            "_PlayerSpectatorStart_impl": 0x22410640,
            "_PlayerSpectatorTarget_impl": 0x224106A0,
            "_PlayerGetShootAng_impl": 0x2240FD50,
            "_phys_ApplyForceCenter_impl": 0x2240E2F0,
        }
    elif mode == "client":
        addresses = {
            "GModRect_message": 0x241AD980,
            "GModRectAnimate_message": 0x241ADCD0,
            "GModRectHide_message": 0x241AE050,
            "GModRect_register": 0x241AE340,
        }
    else:
        print("unknown mode: %s" % mode)
        return 2

    idapro.enable_console_messages(True)
    rc = idapro.open_database(db_path, run_auto_analysis=True, enable_history=False)
    if rc != 0:
        print("open_database failed: %s" % rc)
        return rc

    try:
        result = decompile_functions(addresses)
        result["mode"] = mode
        result["database"] = db_path
        print(json.dumps(result, indent=2))
    finally:
        idapro.close_database(save=False)

    return 0


if __name__ == "__main__":
    sys.exit(main())

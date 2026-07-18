import json
import os

import ida_auto
import ida_funcs
import ida_hexrays
import ida_kernwin
import ida_name
import ida_nalt
import ida_xref
import idautils


def collect_string_xrefs(needles, max_refs_per_string=20):
    results = []
    seen_funcs = set()

    for s in idautils.Strings():
        value = str(s)
        if not any(needle.lower() in value.lower() for needle in needles):
            continue

        refs = []
        xb = ida_xref.xrefblk_t()
        ok = xb.first_to(s.ea, ida_xref.XREF_ALL)
        while ok and len(refs) < max_refs_per_string:
            func = ida_funcs.get_func(xb.frm)
            func_ea = func.start_ea if func else None
            func_name = ida_funcs.get_func_name(func_ea) if func else ""
            refs.append(
                {
                    "from": hex(xb.frm),
                    "function": hex(func_ea) if func_ea is not None else None,
                    "function_name": func_name,
                }
            )
            if func_ea is not None:
                seen_funcs.add(func_ea)
            ok = xb.next_to()

        results.append({"ea": hex(s.ea), "value": value, "refs": refs})

    return results, sorted(seen_funcs)


def decompile_functions(functions, limit=40):
    out = []
    if not ida_hexrays.init_hexrays_plugin():
        return [{"error": "Hex-Rays unavailable"}]

    for ea in functions[:limit]:
        name = ida_funcs.get_func_name(ea)
        try:
            cfunc = ida_hexrays.decompile(ea)
            text = str(cfunc) if cfunc else ""
        except Exception as exc:
            text = "DECOMPILE_ERROR: %s" % exc

        out.append({"ea": hex(ea), "name": name, "pseudocode": text})

    return out


def main():
    ida_auto.auto_wait()

    mode = os.environ.get("GMOD_IDA_MODE", "server")
    if mode == "client":
        needles = [
            "GModRect",
            "GModText",
            "GModTextAnimate",
            "GModRectAnimate",
            "materials/",
            ".vtf",
            ".vmt",
        ]
    else:
        needles = [
            "PickDefaultSpawnTeam",
            "eventKeyPressed",
            "eventKeyReleased",
            "eventPlayerSpawn",
            "_PlayerChangeTeam",
            "_PlayerInfo",
            "_EntSpawn",
            "_PlayerSpectatorStart",
            "_PlayerSpectatorTarget",
            "_PlayerIsKeyDown",
            "_PhysApplyForceCenter",
            "_phys.ApplyForceCenter",
        ]

    strings, funcs = collect_string_xrefs(needles)
    result = {
        "input": ida_nalt.get_root_filename(),
        "mode": mode,
        "strings": strings,
        "functions": decompile_functions(funcs),
    }

    out_path = os.environ.get("GMOD_IDA_OUT", "gmod_ida_probe.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)

    ida_kernwin.qexit(0)


main()

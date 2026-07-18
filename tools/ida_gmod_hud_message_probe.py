import json
import os

os.environ["IDADIR"] = r"F:\IDA Professional 9.3_headless"

import idapro
from ida_domain import Database
from ida_domain.database import IdaCommandOptions


SERVER_NAMES = [
    "usermsg_GModText",
    "usermsg_GModTextAnimate",
    "usermsg_GModRect",
    "usermsg_GModRectAnimate",
]

CLIENT_ADDRS = [
    0x241AC010,  # GModText handler
    0x241ABFF0,  # GModTextAnimate handler
    0x241AC030,  # GModTextHide handler
    0x241AE130,  # GModRect handler
    0x241AE150,  # GModRectHide handler
    0x241AE170,  # GModRectAnimate handler
]


def decompile(ea):
    import ida_hexrays

    hf = ida_hexrays.hexrays_failure_t()
    cfunc = ida_hexrays.decompile(ea, hf)
    if not cfunc:
        return {"ok": False, "error": f"{hf.code}: {hf.str}"}
    return {"ok": True, "pseudocode": str(cfunc)}


def function_name(ea):
    import ida_name

    return ida_name.get_name(ea) or f"sub_{ea:x}"


def lookup_function_by_name(name):
    import ida_name
    import idautils

    for ea in idautils.Functions():
        if ida_name.get_name(ea) == name:
            return ea
    return None


def collect_server(path):
    import ida_auto
    import ida_hexrays

    out = {}
    opts = IdaCommandOptions(auto_analysis=False, new_database=False)
    with Database.open(path, opts, save_on_close=False):
        ida_auto.auto_wait()
        ida_hexrays.init_hexrays_plugin()
        for name in SERVER_NAMES:
            ea = lookup_function_by_name(name)
            out[name] = {"ea": f"0x{ea:x}" if ea else None}
            if ea:
                out[name].update(decompile(ea))
    return out


def collect_client(path):
    import ida_auto
    import ida_hexrays

    out = {}
    opts = IdaCommandOptions(auto_analysis=False, new_database=False)
    with Database.open(path, opts, save_on_close=False):
        ida_auto.auto_wait()
        ida_hexrays.init_hexrays_plugin()
        for ea in CLIENT_ADDRS:
            name = function_name(ea)
            out[f"0x{ea:x}"] = {"name": name}
            out[f"0x{ea:x}"].update(decompile(ea))
    return out


def main():
    idapro.enable_console_messages(True)
    root = os.getcwd()
    out = {
        "server": collect_server(os.path.join(root, "gmod_9_0_4b", "bin", "server.dll.i64")),
        "client": collect_client(os.path.join(root, "gmod_9_0_4b", "bin", "client.dll.i64")),
    }
    out_path = os.path.join(root, "gmod904_hud_message_probe.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    print(out_path)


if __name__ == "__main__":
    main()

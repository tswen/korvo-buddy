from __future__ import annotations

import argparse
import asyncio
import json
import sys

from .events import demo_tick, heartbeat_from_state
from .permissions import handle_permission, parse_permission_line
from .transport import BleakTransport, SerialTransport, StdoutTransport


async def run_loop(transport, demo: bool, interval: float) -> None:
    await transport.connect()
    try:
        while True:
            if demo:
                state = demo_tick()
            else:
                # TODO: 从 Cursor hooks / 日志队列拉取
                state = demo_tick()
            hb = heartbeat_from_state(state)
            await transport.write_line(hb)

            line = await transport.read_line()
            if line:
                perm = parse_permission_line(line)
                if perm:
                    ack = handle_permission(perm)
                    await transport.write_line(ack)
                    print(f"[bridge] permission {perm['decision']} id={perm['id']}", file=sys.stderr)

            await asyncio.sleep(interval)
    finally:
        await transport.disconnect()


def main() -> None:
    parser = argparse.ArgumentParser(description="Korvo 爪爪 Cursor bridge")
    parser.add_argument("--demo", action="store_true", help="模拟心跳序列")
    parser.add_argument("--ble", action="store_true", help="BLE NUS 传输")
    parser.add_argument("--name-prefix", default="Cursor-S31", help="BLE 广播名前缀")
    parser.add_argument("--serial", metavar="PORT", help="串口设备，如 COM7 或 /dev/ttyUSB0")
    parser.add_argument("--interval", type=float, default=1.0, help="心跳间隔秒")
    args = parser.parse_args()

    if args.serial:
        transport = SerialTransport(args.serial)
    elif args.ble:
        transport = BleakTransport(name_prefix=args.name_prefix)
    else:
        transport = StdoutTransport()

    use_demo = args.demo or (not args.ble and not args.serial)
    try:
        asyncio.run(run_loop(transport, demo=use_demo, interval=args.interval))
    except KeyboardInterrupt:
        print("\n[bridge] stopped", file=sys.stderr)


if __name__ == "__main__":
    main()

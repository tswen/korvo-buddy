from __future__ import annotations

import argparse
import asyncio
import sys

from .events import JsonlTail, heartbeat_from_state, next_demo_event
from .permissions import PermissionGate, parse_permission_line
from .session import SessionTracker
from .transport import BleakTransport, SerialTransport, StdoutTransport


async def run_loop(transport, tracker: SessionTracker, source: JsonlTail | None, demo: bool, interval: float,
                   once: int) -> None:
    gate = PermissionGate()
    sent = 0
    await transport.connect()
    try:
        while True:
            if source is not None and not demo:
                event = source.poll()
                state = tracker.apply(event) if event else tracker.snapshot()
            else:
                state = tracker.apply(next_demo_event())
            hb = heartbeat_from_state(state)
            gate.note_heartbeat(hb)
            await transport.write_line(hb)
            sent += 1

            line = await transport.read_line()
            if line:
                perm = parse_permission_line(line)
                if perm:
                    ack = gate.handle(perm)
                    await transport.write_line(ack)
                    print(
                        f"[bridge] permission {perm['decision']} id={perm['id']} -> {ack['detail']}",
                        file=sys.stderr,
                    )

            if once and sent >= once:
                return
            await asyncio.sleep(interval)
    finally:
        await transport.disconnect()
        if source is not None:
            source.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="Korvo 爪爪 Cursor bridge")
    parser.add_argument("--demo", action="store_true", help="循环模拟 idle/running/waiting")
    parser.add_argument("--events", metavar="FILE", help="JSONL 事件文件（Cursor hook 可追加到此）")
    parser.add_argument("--ble", action="store_true", help="BLE NUS 传输")
    parser.add_argument("--name-prefix", default="Cursor-S31", help="BLE 广播名前缀")
    parser.add_argument("--serial", metavar="PORT", help="串口设备，如 COM7 或 /dev/ttyUSB0")
    parser.add_argument("--interval", type=float, default=1.0, help="心跳间隔秒")
    parser.add_argument("--once", type=int, default=0, help="发送 N 帧后退出，0 表示一直运行")
    args = parser.parse_args()

    if args.serial:
        transport = SerialTransport(args.serial)
    elif args.ble:
        transport = BleakTransport(name_prefix=args.name_prefix)
    else:
        transport = StdoutTransport()

    source = JsonlTail(args.events) if args.events else None
    use_demo = args.demo or source is None
    try:
        asyncio.run(
            run_loop(
                transport,
                SessionTracker(),
                source,
                demo=use_demo,
                interval=args.interval,
                once=args.once,
            )
        )
    except KeyboardInterrupt:
        print("\n[bridge] stopped", file=sys.stderr)


if __name__ == "__main__":
    main()

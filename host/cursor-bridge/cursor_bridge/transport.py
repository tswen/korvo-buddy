from __future__ import annotations

import asyncio
import json
import sys
from abc import ABC, abstractmethod

NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
NUS_RX_CHAR = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
NUS_TX_CHAR = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class Transport(ABC):
    @abstractmethod
    async def connect(self) -> None: ...

    @abstractmethod
    async def disconnect(self) -> None: ...

    @abstractmethod
    async def write_line(self, obj: dict) -> None: ...

    @abstractmethod
    async def read_line(self) -> str | None: ...


class StdoutTransport(Transport):
    """开发用：打印 NDJSON 到 stdout。"""

    async def connect(self) -> None:
        return

    async def disconnect(self) -> None:
        return

    async def write_line(self, obj: dict) -> None:
        sys.stdout.write(json.dumps(obj, ensure_ascii=False) + "\n")
        sys.stdout.flush()

    async def read_line(self) -> str | None:
        return None


class BleakTransport(Transport):
    """BLE NUS 写入桩：连接后向 RX 特征写 NDJSON。"""

    def __init__(self, name_prefix: str = "Cursor-S31") -> None:
        self.name_prefix = name_prefix
        self._client = None
        self._rx = None
        self._buffer = ""

    async def connect(self) -> None:
        from bleak import BleakClient, BleakScanner

        device = None
        async with BleakScanner() as scanner:
            for d in scanner.discovered_devices:
                if d.name and d.name.startswith(self.name_prefix):
                    device = d
                    break
        if not device:
            raise RuntimeError(f"未找到 BLE 设备名前缀: {self.name_prefix}")

        self._client = BleakClient(device)
        await self._client.connect()
        await self._client.get_services()
        self._rx = NUS_RX_CHAR

    async def disconnect(self) -> None:
        if self._client and self._client.is_connected:
            await self._client.disconnect()

    async def write_line(self, obj: dict) -> None:
        if not self._client or not self._client.is_connected:
            raise RuntimeError("BLE 未连接")
        payload = (json.dumps(obj, ensure_ascii=False) + "\n").encode("utf-8")
        await self._client.write_gatt_char(self._rx, payload, response=False)

    async def read_line(self) -> str | None:
        # TODO: 订阅 NUS_TX notify，按行分包
        return None


class SerialTransport(Transport):
    def __init__(self, port: str, baud: int = 115200) -> None:
        self.port = port
        self.baud = baud
        self._ser = None

    async def connect(self) -> None:
        import serial

        self._ser = serial.Serial(self.port, self.baud, timeout=0.1)

    async def disconnect(self) -> None:
        if self._ser and self._ser.is_open:
            self._ser.close()

    async def write_line(self, obj: dict) -> None:
        if not self._ser:
            raise RuntimeError("串口未打开")
        line = json.dumps(obj, ensure_ascii=False) + "\n"
        self._ser.write(line.encode("utf-8"))

    async def read_line(self) -> str | None:
        if not self._ser:
            return None
        raw = self._ser.readline()
        if not raw:
            return None
        return raw.decode("utf-8", errors="replace").strip()

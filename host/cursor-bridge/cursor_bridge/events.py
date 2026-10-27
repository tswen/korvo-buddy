from __future__ import annotations

import itertools
import json
import time
from pathlib import Path

from .models import SessionState

_DEMO = itertools.cycle(["idle", "running", "waiting"])


def next_demo_event() -> dict:
    step = next(_DEMO)
    if step == "waiting":
        return {
            "kind": "tool_waiting",
            "id": "prm_demo_001",
            "tool": "run_terminal_cmd",
            "hint": "npm test -- --run",
            "line": "Tool approval required",
        }
    if step == "running":
        return {
            "kind": "running",
            "msg": "正在执行 grep",
            "line": "Running grep in firmware/",
        }
    return {"kind": "idle", "msg": "待命", "line": "agent idle"}


def heartbeat_from_state(state: SessionState) -> dict:
    return state.to_heartbeat(int(time.time()))


class JsonlTail:
    """Read Cursor-like events from a JSONL file, one object per line."""

    def __init__(self, path: str | Path) -> None:
        self._fh = open(path, "r", encoding="utf-8")

    def close(self) -> None:
        self._fh.close()

    def poll(self) -> dict | None:
        while True:
            line = self._fh.readline()
            if not line:
                return None
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            return json.loads(line)

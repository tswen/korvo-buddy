from __future__ import annotations

import itertools
import time
from datetime import datetime

from .models import LogEntry, PromptInfo, SessionState
from .tokens import TokenCounter

_counter = TokenCounter()
_demo_step = itertools.cycle(["idle", "running", "waiting"])


def map_cursor_event_to_state(event: dict) -> SessionState:
    """
    将 Cursor 风格事件 dict 转为 SessionState。
    MVP：接受 {"kind": "...", "tool": "...", "hint": "...", "line": "..."} 等松散字段。
    """
    kind = event.get("kind", "log")
    state = SessionState()
    state.tokens = _counter.tokens_total
    state.tokens_today = _counter.tokens_today
    state.tokens_estimated = _counter.estimated

    line = event.get("line", "")
    if line:
        _counter.add_heuristic(line)
        state.entries.append(
            LogEntry(t=datetime.now().strftime("%H:%M:%S"), line=line[:120])
        )
        state.tokens = _counter.tokens_total
        state.tokens_today = _counter.tokens_today

    if kind == "tool_waiting":
        state.waiting = 1
        state.running = 0
        state.msg = f"等待批准：{event.get('tool', 'tool')}"
        state.prompt = PromptInfo(
            id=event.get("id", "prm_demo"),
            tool=event.get("tool", "unknown"),
            hint=event.get("hint", ""),
        )
    elif kind == "running":
        state.running = 1
        state.waiting = 0
        state.msg = event.get("msg", "Agent 执行中")
    else:
        state.msg = event.get("msg", "待命")

    return state


def demo_tick() -> SessionState:
    step = next(_demo_step)
    if step == "waiting":
        return map_cursor_event_to_state(
            {
                "kind": "tool_waiting",
                "id": "prm_demo_001",
                "tool": "run_terminal_cmd",
                "hint": "npm test -- --run",
                "line": "Tool approval required",
            }
        )
    if step == "running":
        return map_cursor_event_to_state(
            {
                "kind": "running",
                "msg": "正在执行 grep",
                "line": "Running grep in firmware/",
            }
        )
    return map_cursor_event_to_state({"kind": "idle", "msg": "待命"})


def heartbeat_from_state(state: SessionState) -> dict:
    return state.to_heartbeat(int(time.time()))

from __future__ import annotations

from datetime import datetime

from .models import LogEntry, PromptInfo, SessionState
from .tokens import TokenCounter


class SessionTracker:
    """Fold Cursor-like events into one heartbeat snapshot, keeping recent log lines."""

    def __init__(self, counter: TokenCounter | None = None) -> None:
        self.counter = counter or TokenCounter()
        self.total = 1
        self.running = 0
        self.waiting = 0
        self.msg = "待命"
        self.prompt: PromptInfo | None = None
        self.entries: list[LogEntry] = []

    def apply(self, event: dict) -> SessionState:
        kind = event.get("kind", "log")
        line = str(event.get("line", "") or "")
        if line:
            self.counter.add_heuristic(line)
            self.entries.append(LogEntry(t=datetime.now().strftime("%H:%M:%S"), line=line[:120]))
            self.entries = self.entries[-8:]

        if "tokens_today" in event and "tokens" in event:
            self.counter.set_real_counts(int(event["tokens_today"]), int(event["tokens"]))

        if kind == "tool_waiting":
            self.waiting = 1
            self.running = 0
            tool = str(event.get("tool", "tool"))
            self.msg = f"等待批准：{tool}"
            self.prompt = PromptInfo(
                id=str(event.get("id", "prm_demo")),
                tool=tool,
                hint=str(event.get("hint", "")),
            )
        elif kind == "running":
            self.running = 1
            self.waiting = 0
            self.prompt = None
            self.msg = str(event.get("msg", "Agent 执行中"))
        elif kind == "idle":
            self.running = 0
            self.waiting = 0
            self.prompt = None
            self.msg = str(event.get("msg", "待命"))
        elif line and self.waiting == 0 and self.running == 0:
            self.msg = str(event.get("msg", self.msg))

        return self.snapshot()

    def snapshot(self) -> SessionState:
        return SessionState(
            total=self.total,
            running=self.running,
            waiting=self.waiting,
            msg=self.msg,
            entries=list(self.entries),
            tokens=self.counter.tokens_total,
            tokens_today=self.counter.tokens_today,
            tokens_estimated=self.counter.estimated,
            prompt=self.prompt,
        )

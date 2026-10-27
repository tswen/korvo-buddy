from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class LogEntry:
    t: str
    line: str

    def to_dict(self) -> dict[str, str]:
        return {"t": self.t, "line": self.line}


@dataclass
class PromptInfo:
    id: str
    tool: str
    hint: str = ""

    def to_dict(self) -> dict[str, str]:
        d: dict[str, str] = {"id": self.id, "tool": self.tool}
        if self.hint:
            d["hint"] = self.hint
        return d


@dataclass
class SessionState:
    total: int = 1
    running: int = 0
    waiting: int = 0
    msg: str = ""
    entries: list[LogEntry] = field(default_factory=list)
    tokens: int = 0
    tokens_today: int = 0
    tokens_estimated: bool = True
    prompt: PromptInfo | None = None

    def to_heartbeat(self, ts: int) -> dict[str, Any]:
        body: dict[str, Any] = {
            "type": "heartbeat",
            "ts": ts,
            "total": self.total,
            "running": self.running,
            "waiting": self.waiting,
            "msg": self.msg,
            "entries": [e.to_dict() for e in self.entries[-8:]],
            "tokens": self.tokens,
            "tokens_today": self.tokens_today,
            "tokens_estimated": self.tokens_estimated,
        }
        if self.waiting > 0 and self.prompt:
            body["prompt"] = self.prompt.to_dict()
        return body

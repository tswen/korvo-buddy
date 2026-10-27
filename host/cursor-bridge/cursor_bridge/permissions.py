from __future__ import annotations

import json
from typing import Any


def parse_permission_line(line: str) -> dict[str, Any] | None:
    try:
        obj = json.loads(line)
    except json.JSONDecodeError:
        return None
    if obj.get("type") != "permission":
        return None
    if obj.get("decision") not in ("approve", "deny"):
        return None
    if not obj.get("id"):
        return None
    return obj


def handle_permission(perm: dict[str, Any]) -> dict[str, Any]:
    """ACK a permission. Cursor itself is not called; the host log is the MVP sink."""
    decision = perm["decision"]
    detail = "approved" if decision == "approve" else "denied"
    return {"type": "status", "id": perm["id"], "ok": True, "detail": detail}


class PermissionGate:
    """Only the prompt id from the latest waiting heartbeat is actionable."""

    def __init__(self) -> None:
        self.pending_id: str | None = None

    def note_heartbeat(self, heartbeat: dict[str, Any]) -> None:
        prompt = heartbeat.get("prompt")
        if heartbeat.get("waiting", 0) and isinstance(prompt, dict) and prompt.get("id"):
            self.pending_id = str(prompt["id"])
        else:
            self.pending_id = None

    def handle(self, perm: dict[str, Any]) -> dict[str, Any]:
        if self.pending_id is None or perm["id"] != self.pending_id:
            return {"type": "status", "id": perm["id"], "ok": False, "detail": "stale"}
        ack = handle_permission(perm)
        self.pending_id = None
        return ack

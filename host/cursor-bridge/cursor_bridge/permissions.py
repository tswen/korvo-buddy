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


def handle_permission(
    perm: dict[str, Any],
    *,
    forward_to_cursor: bool = False,
) -> dict[str, Any]:
    """
    MVP：记录并返回 status ACK。
    forward_to_cursor=True 时由后续 Hooks/自动化实现真实批准。
    """
    pid = perm["id"]
    decision = perm["decision"]
    ok = True
    detail = "approved" if decision == "approve" else "denied"

    if not forward_to_cursor:
        # 桩：仅 ACK，便于固件联调
        pass

    return {"type": "status", "id": pid, "ok": ok, "detail": detail}

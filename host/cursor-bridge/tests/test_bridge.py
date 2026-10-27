import json
import unittest
from datetime import date, datetime
from pathlib import Path

from cursor_bridge.events import JsonlTail, heartbeat_from_state
from cursor_bridge.permissions import PermissionGate, parse_permission_line
from cursor_bridge.session import SessionTracker
from cursor_bridge.tokens import TokenCounter, local_midnight_epoch


class TokenTests(unittest.TestCase):
    def test_midnight_resets_today_only(self):
        day = {"d": date(2026, 10, 27)}
        counter = TokenCounter(today_fn=lambda: day["d"])
        counter.add_heuristic("abcd")
        self.assertEqual(counter.tokens_today, 1)
        self.assertTrue(counter.estimated)
        day["d"] = date(2026, 10, 28)
        self.assertEqual(counter.tokens_today, 0)
        self.assertEqual(counter.tokens_total, 1)

    def test_real_counts_clear_estimate_flag(self):
        counter = TokenCounter()
        counter.set_real_counts(10, 40)
        self.assertFalse(counter.estimated)
        self.assertEqual(counter.tokens_today, 10)

    def test_local_midnight_is_today(self):
        now = datetime(2026, 10, 27, 19, 4, 0)
        midnight = local_midnight_epoch(now)
        self.assertEqual(datetime.fromtimestamp(midnight).hour, 0)
        self.assertEqual(datetime.fromtimestamp(midnight).date(), now.date())


class SessionTests(unittest.TestCase):
    def test_log_lines_accumulate_and_waiting_prompt(self):
        tracker = SessionTracker(TokenCounter(today_fn=lambda: date(2026, 10, 27)))
        tracker.apply({"kind": "running", "line": "Running grep in firmware/", "msg": "正在执行 grep"})
        state = tracker.apply(
            {
                "kind": "tool_waiting",
                "id": "prm_8f3a2c",
                "tool": "run_terminal_cmd",
                "hint": "npm test",
                "line": "Tool approval required",
            }
        )
        hb = heartbeat_from_state(state)
        self.assertEqual(hb["type"], "heartbeat")
        self.assertEqual(hb["waiting"], 1)
        self.assertEqual(hb["running"], 0)
        self.assertEqual(len(hb["entries"]), 2)
        self.assertEqual(hb["prompt"]["id"], "prm_8f3a2c")
        self.assertTrue(hb["tokens_estimated"])
        self.assertGreater(hb["tokens_today"], 0)
        raw = json.dumps(hb)
        self.assertIn("\n", raw + "\n")

    def test_jsonl_tail(self):
        root = Path(__file__).resolve().parents[1]
        tail = JsonlTail(root / "examples" / "events.jsonl")
        self.addCleanup(tail.close)
        first = tail.poll()
        self.assertEqual(first["kind"], "idle")
        self.assertEqual(tail.poll()["kind"], "running")
        self.assertEqual(tail.poll()["kind"], "tool_waiting")
        self.assertIsNone(tail.poll())


class PermissionTests(unittest.TestCase):
    def test_stale_when_id_does_not_match(self):
        gate = PermissionGate()
        gate.note_heartbeat({"waiting": 1, "prompt": {"id": "prm_8f3a2c", "tool": "run_terminal_cmd"}})
        perm = parse_permission_line('{"type":"permission","id":"other","decision":"approve"}')
        self.assertIsNotNone(perm)
        ack = gate.handle(perm)
        self.assertEqual(ack["detail"], "stale")
        self.assertFalse(ack["ok"])

    def test_approve_then_second_is_stale(self):
        gate = PermissionGate()
        gate.note_heartbeat({"waiting": 1, "prompt": {"id": "prm_8f3a2c"}})
        perm = parse_permission_line('{"type":"permission","id":"prm_8f3a2c","decision":"deny"}')
        ack = gate.handle(perm)
        self.assertEqual(ack["detail"], "denied")
        ack2 = gate.handle(perm)
        self.assertEqual(ack2["detail"], "stale")


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

from datetime import date, datetime
from typing import Callable


class TokenCounter:
    """Local civil-day token counter. Heuristic until Cursor exposes a real count."""

    def __init__(self, today_fn: Callable[[], date] | None = None) -> None:
        self._today_fn = today_fn or date.today
        self._day: date = self._today_fn()
        self._today: int = 0
        self._session: int = 0
        self.estimated: bool = True

    def _roll_midnight(self) -> None:
        today = self._today_fn()
        if today != self._day:
            self._day = today
            self._today = 0

    def add_heuristic(self, text: str) -> None:
        self._roll_midnight()
        # About 4 characters per token. Mark the UI with 约 via estimated=True.
        delta = max(1, len(text) // 4)
        self._today += delta
        self._session += delta

    @property
    def tokens_today(self) -> int:
        self._roll_midnight()
        return self._today

    @property
    def tokens_total(self) -> int:
        return self._session

    def set_real_counts(self, today: int, total: int) -> None:
        self._roll_midnight()
        self._today = today
        self._session = total
        self.estimated = False


def local_midnight_epoch(now: datetime | None = None) -> int:
    now = now or datetime.now()
    midnight = now.replace(hour=0, minute=0, second=0, microsecond=0)
    return int(midnight.timestamp())

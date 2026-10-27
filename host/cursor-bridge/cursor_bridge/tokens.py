from __future__ import annotations

from datetime import date, datetime


class TokenCounter:
    """本地自然日 token 计数；无真实 API 时用启发式估算。"""

    def __init__(self) -> None:
        self._day: date = date.today()
        self._today: int = 0
        self._session: int = 0
        self.estimated: bool = True

    def _roll_midnight(self) -> None:
        today = date.today()
        if today != self._day:
            self._day = today
            self._today = 0

    def add_heuristic(self, text: str) -> None:
        self._roll_midnight()
        # 粗估：约 4 字符 1 token
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


def local_midnight_epoch() -> int:
    now = datetime.now()
    midnight = now.replace(hour=0, minute=0, second=0, microsecond=0)
    return int(midnight.timestamp())

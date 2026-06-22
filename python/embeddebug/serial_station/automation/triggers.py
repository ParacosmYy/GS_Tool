"""触发条件定义与求值。"""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from enum import Enum, auto


class TriggerType(Enum):
    """触发器类型。"""

    VALUE_ABOVE = auto()
    VALUE_BELOW = auto()
    VALUE_EQUALS = auto()
    INTERVAL = auto()
    RX_MATCH = auto()
    TX_MATCH = auto()
    MANUAL = auto()


_VALUE_OPERATORS = {
    TriggerType.VALUE_ABOVE: ">",
    TriggerType.VALUE_BELOW: "<",
    TriggerType.VALUE_EQUALS: "==",
}


@dataclass
class TriggerCondition:
    """单条触发条件。"""

    trigger_type: TriggerType
    channel_name: str = ""
    threshold: float = 0.0
    operator: str = ""
    pattern: str = ""
    interval_ms: int = 0
    debounce_ms: int = 0
    last_fire_ms: int = field(default=-1, init=False, repr=False)

    def __post_init__(self) -> None:
        if not self.operator and self.trigger_type in _VALUE_OPERATORS:
            self.operator = _VALUE_OPERATORS[self.trigger_type]

    def evaluate(
        self,
        channel_values: dict[str, float],
        rx_text: str = "",
        tx_text: str = "",
        elapsed_ms: int = 0,
    ) -> bool:
        """判定本条件是否在当前采样下应触发（含去抖）。"""
        if not self._matches(channel_values, rx_text, tx_text, elapsed_ms):
            return False
        if (
            self.debounce_ms > 0
            and self.last_fire_ms >= 0
            and elapsed_ms - self.last_fire_ms < self.debounce_ms
        ):
            return False
        self.last_fire_ms = elapsed_ms
        return True

    def reset(self) -> None:
        """清除去抖计时。"""
        self.last_fire_ms = -1

    def _matches(
        self,
        channel_values: dict[str, float],
        rx_text: str,
        tx_text: str,
        elapsed_ms: int,
    ) -> bool:
        ttype = self.trigger_type
        if ttype is TriggerType.MANUAL:
            return False
        if ttype is TriggerType.INTERVAL:
            if self.last_fire_ms < 0:
                return True
            return elapsed_ms - self.last_fire_ms >= self.interval_ms
        if ttype is TriggerType.RX_MATCH:
            return bool(self.pattern) and re.search(self.pattern, rx_text or "") is not None
        if ttype is TriggerType.TX_MATCH:
            return bool(self.pattern) and re.search(self.pattern, tx_text or "") is not None
        value = channel_values.get(self.channel_name)
        if value is None:
            return False
        return _compare(float(value), self.operator, self.threshold)

    def to_dict(self) -> dict[str, object]:
        return {
            "trigger_type": self.trigger_type.name,
            "channel_name": self.channel_name,
            "threshold": self.threshold,
            "operator": self.operator,
            "pattern": self.pattern,
            "interval_ms": self.interval_ms,
            "debounce_ms": self.debounce_ms,
        }

    @classmethod
    def from_dict(cls, data: dict[str, object]) -> TriggerCondition:
        return cls(
            trigger_type=TriggerType[str(data["trigger_type"])],  # type: ignore[index]
            channel_name=str(data.get("channel_name", "")),
            threshold=float(data.get("threshold", 0.0)),
            operator=str(data.get("operator", "")),
            pattern=str(data.get("pattern", "")),
            interval_ms=int(data.get("interval_ms", 0)),
            debounce_ms=int(data.get("debounce_ms", 0)),
        )


def _compare(value: float, op: str, threshold: float) -> bool:
    if op == ">":
        return value > threshold
    if op == "<":
        return value < threshold
    if op == ">=":
        return value >= threshold
    if op == "<=":
        return value <= threshold
    if op == "==":
        return abs(value - threshold) <= 1e-9
    return False

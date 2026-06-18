"""自动化规则：触发条件 + 动作序列 + 冷却。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from embeddebug.serial_station.automation.actions import AutomationAction
from embeddebug.serial_station.automation.triggers import TriggerCondition


@dataclass
class AutomationRule:
    """一条自动化规则。"""

    name: str
    trigger: TriggerCondition
    actions: list[AutomationAction] = field(default_factory=list)
    enabled: bool = True
    cooldown_ms: int = 0
    last_fire_ms: int = field(default=-1, init=False, repr=False)

    def should_fire(self, elapsed_ms: int) -> bool:
        """规则是否已过冷却期可再次触发。"""
        if not self.enabled:
            return False
        if self.cooldown_ms <= 0 or self.last_fire_ms < 0:
            return True
        return elapsed_ms - self.last_fire_ms >= self.cooldown_ms

    def mark_fired(self, elapsed_ms: int) -> None:
        """记录本次触发时间。"""
        self.last_fire_ms = elapsed_ms

    def reset(self) -> None:
        """清除冷却与触发器去抖状态。"""
        self.last_fire_ms = -1
        self.trigger.reset()

    def to_dict(self) -> dict[str, Any]:
        return {
            "name": self.name,
            "enabled": self.enabled,
            "cooldown_ms": self.cooldown_ms,
            "trigger": self.trigger.to_dict(),
            "actions": [action.to_dict() for action in self.actions],
        }

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "AutomationRule":
        raw_actions = data.get("actions", [])
        return cls(
            name=str(data["name"]),
            trigger=TriggerCondition.from_dict(data["trigger"]),  # type: ignore[arg-type]
            actions=[AutomationAction.from_dict(item) for item in raw_actions],  # type: ignore[arg-type]
            enabled=bool(data.get("enabled", True)),
            cooldown_ms=int(data.get("cooldown_ms", 0)),
        )

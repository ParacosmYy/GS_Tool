"""脚本自动化引擎（触发器 + 动作）。"""

from __future__ import annotations

from embeddebug.serial_station.automation.actions import (
    ActionType,
    AutomationAction,
)
from embeddebug.serial_station.automation.engine import AutomationEngine
from embeddebug.serial_station.automation.rule import AutomationRule
from embeddebug.serial_station.automation.triggers import (
    TriggerCondition,
    TriggerType,
)

__all__ = [
    "ActionType",
    "AutomationAction",
    "AutomationEngine",
    "AutomationRule",
    "TriggerCondition",
    "TriggerType",
]

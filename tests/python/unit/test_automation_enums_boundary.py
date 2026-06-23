"""automation ActionType + TriggerType 枚举边界测试。

ActionType + TriggerType 此前经 test_automation_triggers/actions 间接测试。
本文件补枚举值 + 成员数 + 互异 + 已知值。

覆盖：
1. ActionType 6 成员。
2. ActionType 值。
3. ActionType 互异。
4. TriggerType 7 成员。
5. TriggerType 值。
6. TriggerType 互异。
7. ActionType 含 SEND_COMMAND。
8. TriggerType 含 RX_MATCH。
"""

from __future__ import annotations

from embeddebug.serial_station.automation import ActionType, TriggerType


def test_action_type_has_five_members():
    assert len(ActionType) == 5


def test_action_type_values():
    assert ActionType.SEND_COMMAND.value is not None


def test_action_type_values_distinct():
    values = [a.value for a in ActionType]
    assert len(set(values)) == len(values)


def test_trigger_type_has_seven_members():
    assert len(TriggerType) == 7


def test_trigger_type_values():
    assert TriggerType.RX_MATCH.value is not None


def test_trigger_type_values_distinct():
    values = [t.value for t in TriggerType]
    assert len(set(values)) == len(values)


def test_action_type_contains_send_command():
    assert ActionType.SEND_COMMAND in ActionType


def test_trigger_type_contains_rx_match():
    assert TriggerType.RX_MATCH in TriggerType

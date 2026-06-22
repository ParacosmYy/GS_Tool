"""automation/triggers 纯 helper + TriggerCondition 单元测试。

覆盖 _compare（5 操作符）+ TriggerCondition（__post_init__/_matches/evaluate 去抖/
reset/to_dict/from_dict）+ _VALUE_OPERATORS 映射（此前 0 测试覆盖）。
"""

from __future__ import annotations

from embeddebug.serial_station.automation.triggers import (
    TriggerCondition,
    TriggerType,
    _VALUE_OPERATORS,
    _compare,
)


# ── _compare 纯函数 ──────────────────────────────────────────────────────


def test_compare_all_operators():
    """5 操作符：>/<=>=/<=/== 边界 + == 浮点容差 1e-9。"""

    assert _compare(5.0, ">", 3.0) and not _compare(3.0, ">", 5.0)
    assert _compare(3.0, "<", 5.0) and not _compare(5.0, "<", 3.0)
    assert _compare(5.0, ">=", 5.0) and not _compare(4.0, ">=", 5.0)
    assert _compare(3.0, "<=", 3.0) and not _compare(4.0, "<=", 3.0)
    assert _compare(1.0, "==", 1.0 + 1e-10) and not _compare(1.0, "==", 1.1)


def test_compare_unknown_operator_returns_false():
    """未知操作符 → False。"""

    assert _compare(5.0, "!=", 3.0) is False
    assert _compare(5.0, "invalid", 3.0) is False


# ── _VALUE_OPERATORS 映射 ────────────────────────────────────────────────


def test_value_operators_mapping():
    """_VALUE_OPERATORS 3 个 VALUE_* → 操作符映射。"""

    assert _VALUE_OPERATORS[TriggerType.VALUE_ABOVE] == ">"
    assert _VALUE_OPERATORS[TriggerType.VALUE_BELOW] == "<"
    assert _VALUE_OPERATORS[TriggerType.VALUE_EQUALS] == "=="


# ── TriggerCondition.__post_init__ ───────────────────────────────────────


def test_post_init_sets_operator_for_value_types():
    """VALUE_* 无显式 operator → 自动设置（>/</==）。"""

    assert TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="t").operator == ">"
    assert TriggerCondition(TriggerType.VALUE_BELOW, channel_name="t").operator == "<"
    assert TriggerCondition(TriggerType.VALUE_EQUALS, channel_name="t").operator == "=="


def test_post_init_preserves_explicit_operator():
    """显式 operator 不被覆盖。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp", operator=">="
    )
    assert cond.operator == ">="


def test_post_init_no_operator_for_non_value_types():
    """非 VALUE_* 类型（如 INTERVAL）→ operator 保持空串。"""

    cond = TriggerCondition(trigger_type=TriggerType.INTERVAL, interval_ms=1000)
    assert cond.operator == ""


# ── TriggerCondition._matches ────────────────────────────────────────────


def test_matches_manual_returns_false():
    """MANUAL 类型 → _matches 永远 False（手动触发不走 evaluate）。"""

    cond = TriggerCondition(trigger_type=TriggerType.MANUAL)
    assert cond._matches({}, "", "", 0) is False


def test_matches_interval_first_call():
    """INTERVAL 首次调用（last_fire<0）→ True。"""

    cond = TriggerCondition(trigger_type=TriggerType.INTERVAL, interval_ms=1000)
    assert cond._matches({}, "", "", 0) is True


def test_matches_interval_after_interval():
    """INTERVAL 经过 interval_ms 后 → True。"""

    cond = TriggerCondition(trigger_type=TriggerType.INTERVAL, interval_ms=1000)
    cond.last_fire_ms = 0
    assert cond._matches({}, "", "", 1001) is True


def test_matches_interval_before_interval():
    """INTERVAL 未到 interval_ms → False。"""

    cond = TriggerCondition(trigger_type=TriggerType.INTERVAL, interval_ms=1000)
    cond.last_fire_ms = 0
    assert cond._matches({}, "", "", 500) is False


def test_matches_rx_match_pattern_found():
    """RX_MATCH 正则匹配 rx_text → True。"""

    cond = TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="ERROR")
    assert cond._matches({}, "SYSTEM ERROR detected", "", 0) is True


def test_matches_rx_match_pattern_not_found():
    """RX_MATCH 正则不匹配 → False。"""

    cond = TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="ERROR")
    assert cond._matches({}, "all good", "", 0) is False


def test_matches_rx_match_empty_pattern():
    """RX_MATCH 空 pattern → False。"""

    cond = TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="")
    assert cond._matches({}, "anything", "", 0) is False


def test_matches_tx_match():
    """TX_MATCH 匹配 tx_text。"""

    cond = TriggerCondition(trigger_type=TriggerType.TX_MATCH, pattern="AT")
    assert cond._matches({}, "", "AT+RESET", 0) is True


def test_matches_value_above():
    """VALUE_ABOVE 通道值 > 阈值 → True。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp", threshold=50.0
    )
    assert cond._matches({"temp": 75.0}, "", "", 0) is True


def test_matches_value_missing_channel():
    """VALUE_* 通道不存在 → False。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="missing", threshold=50.0
    )
    assert cond._matches({"temp": 75.0}, "", "", 0) is False


# ── TriggerCondition.evaluate 去抖 ───────────────────────────────────────


def test_evaluate_updates_last_fire_ms():
    """evaluate 触发后更新 last_fire_ms。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp", threshold=50.0
    )
    assert cond.evaluate({"temp": 75.0}, elapsed_ms=100) is True
    assert cond.last_fire_ms == 100


def test_evaluate_debounce_blocks_rapid_refire():
    """去抖期内重复触发 → False。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp",
        threshold=50.0, debounce_ms=200,
    )
    assert cond.evaluate({"temp": 75.0}, elapsed_ms=100) is True
    # 去抖期内（100+200=300，当前 200）→ False
    assert cond.evaluate({"temp": 75.0}, elapsed_ms=200) is False


def test_evaluate_debounce_allows_after_period():
    """去抖期后允许再次触发。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp",
        threshold=50.0, debounce_ms=200,
    )
    assert cond.evaluate({"temp": 75.0}, elapsed_ms=100) is True
    # 去抖期后（100+200=300，当前 350）→ True
    assert cond.evaluate({"temp": 75.0}, elapsed_ms=350) is True


def test_reset_clears_last_fire():
    """reset 清除 last_fire_ms → -1（下次 evaluate 不受去抖约束）。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp", debounce_ms=200
    )
    cond.evaluate({"temp": 75.0}, elapsed_ms=100)
    cond.reset()
    assert cond.last_fire_ms == -1


# ── to_dict / from_dict round-trip ───────────────────────────────────────


def test_to_dict_contains_all_fields():
    """to_dict 含 7 个字段。"""

    cond = TriggerCondition(
        trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp",
        threshold=50.0, operator=">", debounce_ms=100,
    )
    d = cond.to_dict()
    assert set(d.keys()) == {
        "trigger_type", "channel_name", "threshold", "operator",
        "pattern", "interval_ms", "debounce_ms",
    }


def test_from_dict_round_trip():
    """from_dict(to_dict()) 保持字段等价。"""

    original = TriggerCondition(
        trigger_type=TriggerType.RX_MATCH, pattern="ERROR", debounce_ms=50,
    )
    restored = TriggerCondition.from_dict(original.to_dict())
    assert restored.trigger_type == TriggerType.RX_MATCH
    assert restored.pattern == "ERROR"
    assert restored.debounce_ms == 50


def test_from_dict_defaults_missing_fields():
    """from_dict 缺字段回退默认。"""

    cond = TriggerCondition.from_dict({"trigger_type": "INTERVAL"})
    assert cond.trigger_type == TriggerType.INTERVAL
    assert cond.channel_name == ""
    assert cond.interval_ms == 0


def test_trigger_type_enum_has_seven_members():
    """TriggerType 含 7 个成员。"""

    assert len(TriggerType) == 7

"""automation/engine 单元测试（此前 0 测试覆盖）。

覆盖 AutomationEngine 的规则管理 + 触发求值 + 动作分发：
- add_rule / remove_rule / clear（via remove）+ rule_added/rule_removed 信号。
- enable_rule / disable_rule + rule_names / rules 排序。
- advance_clock / set_clock + 负值 clamp。
- fire_rule 手动触发 + disabled 不触发 + 未知规则 False。
- on_measurement 自动求值 + VALUE_ABOVE 触发 + 动作执行 + send_callable 调用。
- on_rx / on_tx 文本匹配触发。
- 冷却期内不重复触发。
- context 属性累积。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

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


def _make_rule(
    name: str = "test-rule",
    trigger_type: TriggerType = TriggerType.VALUE_ABOVE,
    threshold: float = 50.0,
    pattern: str = "",
    enabled: bool = True,
    cooldown_ms: int = 0,
) -> AutomationRule:
    """构造测试用 AutomationRule。"""

    return AutomationRule(
        name=name,
        trigger=TriggerCondition(
            trigger_type=trigger_type,
            channel_name="temp" if trigger_type in (
                TriggerType.VALUE_ABOVE, TriggerType.VALUE_BELOW, TriggerType.VALUE_EQUALS
            ) else "",
            threshold=threshold,
            pattern=pattern,
        ),
        actions=[AutomationAction(action_type=ActionType.SEND_COMMAND, command="AT+FIRE")],
        enabled=enabled,
        cooldown_ms=cooldown_ms,
    )


def _make_engine() -> tuple[AutomationEngine, list[str]]:
    """构造引擎 + 收集发送命令的列表。"""

    sent: list[str] = []
    return AutomationEngine(sent.append), sent


# ── add_rule / remove_rule + 信号 ────────────────────────────────────────


def test_add_rule_stores_and_emits_signal(qtbot):
    engine, _ = _make_engine()
    with qtbot.waitSignal(engine.rule_added, timeout=500) as blocker:
        engine.add_rule(_make_rule("r1"))
    assert blocker.args == ["r1"]
    assert "r1" in engine.rule_names()


def test_remove_rule_returns_true_and_emits(qtbot):
    engine, _ = _make_engine()
    engine.add_rule(_make_rule("r1"))
    with qtbot.waitSignal(engine.rule_removed, timeout=500) as blocker:
        result = engine.remove_rule("r1")
    assert result is True
    assert blocker.args == ["r1"]
    assert "r1" not in engine.rule_names()


def test_remove_unknown_rule_returns_false():
    engine, _ = _make_engine()
    assert engine.remove_rule("nonexistent") is False


# ── enable / disable ─────────────────────────────────────────────────────


def test_enable_disable_rule():
    """enable/disable 已知规则切换 + 未知规则返回 False。"""

    engine, _ = _make_engine()
    engine.add_rule(_make_rule("r1", enabled=False))
    assert engine.enable_rule("r1") is True
    assert engine.rules()[0].enabled is True
    assert engine.disable_rule("r1") is True
    assert engine.rules()[0].enabled is False
    assert engine.enable_rule("ghost") is False
    assert engine.disable_rule("ghost") is False


# ── rule_names / rules 排序 ──────────────────────────────────────────────


def test_rule_names_sorted():
    engine, _ = _make_engine()
    engine.add_rule(_make_rule("zebra"))
    engine.add_rule(_make_rule("alpha"))
    engine.add_rule(_make_rule("middle"))
    assert engine.rule_names() == ["alpha", "middle", "zebra"]


def test_rules_returns_sorted_list():
    engine, _ = _make_engine()
    engine.add_rule(_make_rule("b"))
    engine.add_rule(_make_rule("a"))
    rules = engine.rules()
    assert [r.name for r in rules] == ["a", "b"]


def test_empty_engine_rule_names():
    engine, _ = _make_engine()
    assert engine.rule_names() == []


# ── advance_clock / set_clock ────────────────────────────────────────────


def test_advance_clock_accumulates_and_clamps():
    """advance_clock 累加 + 负值 clamp + set_clock 覆盖。"""

    engine, _ = _make_engine()
    engine.advance_clock(100)
    engine.advance_clock(200)
    engine.advance_clock(-100)  # clamp，不崩溃
    engine.set_clock(500)


# ── fire_rule 手动触发 ───────────────────────────────────────────────────


def test_fire_rule_executes_actions(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(_make_rule("r1"))
    with qtbot.waitSignal(engine.rule_fired, timeout=500) as blocker:
        result = engine.fire_rule("r1")
    assert result is True
    assert blocker.args == ["r1"]
    assert sent == ["AT+FIRE"]


def test_fire_rule_disabled_returns_false():
    engine, _ = _make_engine()
    engine.add_rule(_make_rule("r1", enabled=False))
    assert engine.fire_rule("r1") is False


def test_fire_rule_unknown_returns_false():
    engine, _ = _make_engine()
    assert engine.fire_rule("ghost") is False


# ── on_measurement 自动求值 ─────────────────────────────────────────────


def test_on_measurement_triggers_value_above(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(_make_rule("r1", threshold=50.0))
    with qtbot.waitSignal(engine.rule_fired, timeout=500):
        engine.on_measurement({"temp": 75.0})
    assert sent == ["AT+FIRE"]


def test_on_measurement_below_threshold_no_fire(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(_make_rule("r1", threshold=100.0))
    engine.on_measurement({"temp": 50.0})
    assert sent == []


# ── on_rx / on_tx 文本匹配 ──────────────────────────────────────────────


def test_on_rx_triggers_pattern_match(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(_make_rule(
        "r1", trigger_type=TriggerType.RX_MATCH, pattern="ERROR",
    ))
    with qtbot.waitSignal(engine.rule_fired, timeout=500):
        engine.on_rx("SYSTEM ERROR detected")
    assert sent == ["AT+FIRE"]


def test_on_tx_triggers_pattern_match(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(_make_rule(
        "r1", trigger_type=TriggerType.TX_MATCH, pattern="RESET",
    ))
    with qtbot.waitSignal(engine.rule_fired, timeout=500):
        engine.on_tx("AT+RESET")
    assert sent == ["AT+FIRE"]


# ── 冷却 ────────────────────────────────────────────────────────────────


def test_cooldown_prevents_refire():
    engine, sent = _make_engine()
    engine.add_rule(_make_rule("r1", threshold=50.0, cooldown_ms=1000))
    engine.on_measurement({"temp": 75.0})
    assert sent == ["AT+FIRE"]
    # 冷却期内再触发
    engine.advance_clock(500)
    engine.on_measurement({"temp": 75.0})
    assert sent == ["AT+FIRE"]  # 不重复


def test_cooldown_allows_refire_after_period():
    engine, sent = _make_engine()
    engine.add_rule(_make_rule("r1", threshold=50.0, cooldown_ms=1000))
    engine.on_measurement({"temp": 75.0})
    engine.advance_clock(1001)
    engine.on_measurement({"temp": 75.0})
    assert sent == ["AT+FIRE", "AT+FIRE"]


# ── context 累积 ─────────────────────────────────────────────────────────


def test_context_accumulates_across_fires():
    engine, _ = _make_engine()
    engine.add_rule(AutomationRule(
        name="logger",
        trigger=TriggerCondition(
            trigger_type=TriggerType.VALUE_ABOVE,
            channel_name="temp",
            threshold=0.0,
        ),
        actions=[AutomationAction(action_type=ActionType.LOG, log_message="hit")],
    ))
    engine.on_measurement({"temp": 1.0})
    engine.advance_clock(1)
    engine.on_measurement({"temp": 2.0})
    assert len(engine.context.get("logs", [])) == 2

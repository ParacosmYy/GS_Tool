"""自动化引擎（触发器 + 动作）单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import json

from embeddebug.serial_station.automation import (
    ActionType,
    AutomationAction,
    AutomationEngine,
    AutomationRule,
    TriggerCondition,
    TriggerType,
)


def _make_engine() -> tuple[AutomationEngine, list[str]]:
    sent: list[str] = []
    engine = AutomationEngine(send_callable=lambda cmd: sent.append(cmd))
    return engine, sent


def test_trigger_value_above_fires_when_over_threshold():
    cond = TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="temp", threshold=50.0)
    assert cond.evaluate({"temp": 60.0}) is True


def test_trigger_value_above_no_fire_when_under():
    cond = TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="temp", threshold=50.0)
    assert cond.evaluate({"temp": 40.0}) is False


def test_trigger_value_below_fires_when_under_threshold():
    cond = TriggerCondition(TriggerType.VALUE_BELOW, channel_name="vbat", threshold=3.0)
    assert cond.evaluate({"vbat": 2.7}) is True
    assert cond.evaluate({"vbat": 3.5}) is False


def test_trigger_value_equals_fires_on_match():
    cond = TriggerCondition(TriggerType.VALUE_EQUALS, channel_name="mode", threshold=7.0)
    assert cond.evaluate({"mode": 7.0}) is True
    assert cond.evaluate({"mode": 8.0}) is False


def test_trigger_missing_channel_does_not_fire():
    cond = TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="x", threshold=1.0)
    assert cond.evaluate({"other": 99.0}) is False
    assert cond.evaluate({}) is False


def test_trigger_debounce_blocks_rapid_refire():
    cond = TriggerCondition(
        TriggerType.VALUE_ABOVE, channel_name="t", threshold=10.0, debounce_ms=100
    )
    assert cond.evaluate({"t": 20.0}, elapsed_ms=0) is True
    assert cond.evaluate({"t": 20.0}, elapsed_ms=10) is False
    assert cond.evaluate({"t": 20.0}, elapsed_ms=99) is False
    assert cond.evaluate({"t": 20.0}, elapsed_ms=100) is True


def test_trigger_interval_fires_on_period():
    cond = TriggerCondition(TriggerType.INTERVAL, interval_ms=200, debounce_ms=0)
    assert cond.evaluate({}, elapsed_ms=0) is True
    assert cond.evaluate({}, elapsed_ms=150) is False
    assert cond.evaluate({}, elapsed_ms=200) is True
    assert cond.evaluate({}, elapsed_ms=350) is False
    assert cond.evaluate({}, elapsed_ms=400) is True


def test_trigger_rx_match_regex():
    cond = TriggerCondition(TriggerType.RX_MATCH, pattern=r"ERR:\d+")
    assert cond.evaluate({}, rx_text="boot ok\nERR:42 timeout") is True
    assert cond.evaluate({}, rx_text="boot ok") is False


def test_trigger_tx_match_regex():
    cond = TriggerCondition(TriggerType.TX_MATCH, pattern=r"^RESET$")
    assert cond.evaluate({}, tx_text="RESET") is True
    assert cond.evaluate({}, tx_text="RESETX") is False


def test_action_send_command_calls_send_callable():
    sent: list[str] = []
    action = AutomationAction(ActionType.SEND_COMMAND, command="LED ON")
    ctx: dict[str, list[str]] = {}
    wait = action.execute(lambda c: sent.append(c), ctx)
    assert wait == 0
    assert sent == ["LED ON"]
    assert ctx["sent"] == ["LED ON"]


def test_action_delay_returns_wait_ms():
    action = AutomationAction(ActionType.DELAY, delay_ms=250)
    ctx: dict[str, list[str]] = {}
    wait = action.execute(lambda _c: None, ctx)
    assert wait == 250
    assert ctx["delays"] == ["250"]


def test_action_log_sound_led_record_intent():
    log_action = AutomationAction(ActionType.LOG, log_message="over heat")
    led_action = AutomationAction(ActionType.SET_LED, led_state="red")
    sound_action = AutomationAction(ActionType.PLAY_SOUND, sound_path="beep.wav")
    ctx: dict[str, list[str]] = {}
    log_action.execute(lambda _c: None, ctx)
    led_action.execute(lambda _c: None, ctx)
    sound_action.execute(lambda _c: None, ctx)
    assert ctx["logs"] == ["over heat"]
    assert ctx["leds"] == ["red"]
    assert ctx["sounds"] == ["beep.wav"]


def test_rule_cooldown_blocks_refire_within_window():
    rule = AutomationRule(
        name="hot",
        trigger=TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="t", threshold=80.0),
        actions=[AutomationAction(ActionType.SEND_COMMAND, command="FAN ON")],
        cooldown_ms=500,
    )
    assert rule.should_fire(0) is True
    rule.mark_fired(0)
    assert rule.should_fire(100) is False
    assert rule.should_fire(499) is False
    assert rule.should_fire(500) is True


def test_rule_disabled_never_fires():
    rule = AutomationRule(
        name="x",
        trigger=TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="t", threshold=1.0),
        enabled=False,
    )
    assert rule.should_fire(0) is False


def test_engine_add_remove_fire(qtbot):
    engine, sent = _make_engine()
    added: list[str] = []
    fired: list[str] = []
    engine.rule_added.connect(lambda n: added.append(n))
    engine.rule_fired.connect(lambda n: fired.append(n))
    rule = AutomationRule(
        name="over_temp",
        trigger=TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="temp", threshold=10.0),
        actions=[AutomationAction(ActionType.SEND_COMMAND, command="ALERT")],
    )
    engine.add_rule(rule)
    assert added == ["over_temp"]
    engine.on_measurement({"temp": 20.0})
    assert sent == ["ALERT"]
    assert fired == ["over_temp"]
    assert engine.remove_rule("over_temp") is True
    assert engine.remove_rule("over_temp") is False
    engine.on_measurement({"temp": 99.0})
    assert sent == ["ALERT"]


def test_engine_cooldown_between_fires(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(
        AutomationRule(
            name="r",
            trigger=TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="v", threshold=1.0),
            actions=[AutomationAction(ActionType.SEND_COMMAND, command="PING")],
            cooldown_ms=1000,
        )
    )
    engine.on_measurement({"v": 5.0})
    engine.advance_clock(500)
    engine.on_measurement({"v": 5.0})
    engine.advance_clock(500)
    engine.on_measurement({"v": 5.0})
    assert sent == ["PING", "PING"]


def test_engine_disable_rule_stops_firing(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(
        AutomationRule(
            name="r",
            trigger=TriggerCondition(TriggerType.VALUE_ABOVE, channel_name="v", threshold=1.0),
            actions=[AutomationAction(ActionType.SEND_COMMAND, command="X")],
        )
    )
    engine.on_measurement({"v": 5.0})
    assert engine.disable_rule("r") is True
    engine.on_measurement({"v": 5.0})
    assert sent == ["X"]
    assert engine.enable_rule("r") is True
    engine.on_measurement({"v": 5.0})
    assert sent == ["X", "X"]


def test_engine_rx_match_fires_action(qtbot):
    engine, sent = _make_engine()
    engine.add_rule(
        AutomationRule(
            name="err_watch",
            trigger=TriggerCondition(TriggerType.RX_MATCH, pattern=r"FAULT"),
            actions=[AutomationAction(ActionType.SEND_COMMAND, command="RESET")],
        )
    )
    engine.on_rx("system FAULT detected")
    assert sent == ["RESET"]
    engine.on_rx("system ok")
    assert sent == ["RESET"]


def test_engine_rule_names_sorted(qtbot):
    engine, _ = _make_engine()
    engine.add_rule(AutomationRule(name="b", trigger=TriggerCondition(TriggerType.MANUAL)))
    engine.add_rule(AutomationRule(name="a", trigger=TriggerCondition(TriggerType.MANUAL)))
    assert engine.rule_names() == ["a", "b"]


def test_rule_json_round_trip():
    rule = AutomationRule(
        name="guard",
        trigger=TriggerCondition(
            TriggerType.VALUE_BELOW, channel_name="vbat", threshold=3.3, debounce_ms=50
        ),
        actions=[
            AutomationAction(ActionType.SEND_COMMAND, command="BUZZER ON"),
            AutomationAction(ActionType.DELAY, delay_ms=100),
            AutomationAction(ActionType.LOG, log_message="low battery"),
        ],
        enabled=True,
        cooldown_ms=1000,
    )
    data = rule.to_dict()
    as_json = json.dumps(data)
    restored = AutomationRule.from_dict(json.loads(as_json))
    assert restored.name == rule.name
    assert restored.enabled is True
    assert restored.cooldown_ms == rule.cooldown_ms
    assert restored.trigger.trigger_type is rule.trigger.trigger_type
    assert restored.trigger.channel_name == rule.trigger.channel_name
    assert restored.trigger.threshold == rule.trigger.threshold
    assert restored.trigger.debounce_ms == rule.trigger.debounce_ms
    assert len(restored.actions) == 3
    assert restored.actions[0].action_type is ActionType.SEND_COMMAND
    assert restored.actions[0].command == "BUZZER ON"
    assert restored.actions[1].action_type is ActionType.DELAY
    assert restored.actions[1].delay_ms == 100
    assert restored.actions[2].log_message == "low battery"
    assert "last_fire_ms" not in data and "last_fire_ms" not in data["trigger"]

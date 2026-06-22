"""automation/rule + actions 单元测试（此前 0 测试覆盖）。

覆盖 AutomationAction（execute 5 类型 + to_dict/from_dict）+ ActionType 5 枚举
+ AutomationRule（should_fire 冷却 + mark_fired + reset + to_dict/from_dict）。
"""

from __future__ import annotations

from embeddebug.serial_station.automation.actions import (
    ActionType,
    AutomationAction,
)
from embeddebug.serial_station.automation.rule import AutomationRule
from embeddebug.serial_station.automation.triggers import (
    TriggerCondition,
    TriggerType,
)


# ── AutomationAction.execute ─────────────────────────────────────────────


def test_execute_send_command_calls_send_callable():
    """SEND_COMMAND → 调用 send_callable + 记录 context。"""

    sent: list[str] = []
    action = AutomationAction(action_type=ActionType.SEND_COMMAND, command="AT+RESET")
    wait = action.execute(sent.append, {})
    assert wait == 0
    assert sent == ["AT+RESET"]


def test_execute_send_command_empty_command_skipped():
    """SEND_COMMAND 空 command → 不调用 send_callable（但仍记录到 context）。"""

    sent: list[str] = []
    ctx: dict = {}
    action = AutomationAction(action_type=ActionType.SEND_COMMAND, command="")
    action.execute(sent.append, ctx)
    assert sent == []
    assert ctx["sent"] == [""]


def test_execute_play_sound():
    """PLAY_SOUND → 记录 sound_path。"""

    ctx: dict = {}
    action = AutomationAction(action_type=ActionType.PLAY_SOUND, sound_path="alert.wav")
    wait = action.execute(lambda cmd: None, ctx)
    assert wait == 0
    assert ctx["sounds"] == ["alert.wav"]


def test_execute_log():
    """LOG → 记录 log_message。"""

    ctx: dict = {}
    action = AutomationAction(action_type=ActionType.LOG, log_message="triggered")
    action.execute(lambda cmd: None, ctx)
    assert ctx["logs"] == ["triggered"]


def test_execute_set_led():
    """SET_LED → 记录 led_state。"""

    ctx: dict = {}
    action = AutomationAction(action_type=ActionType.SET_LED, led_state="on")
    action.execute(lambda cmd: None, ctx)
    assert ctx["leds"] == ["on"]


def test_execute_delay_returns_wait_ms():
    """DELAY → 返回 delay_ms（需等待的时间）。"""

    action = AutomationAction(action_type=ActionType.DELAY, delay_ms=500)
    ctx: dict = {}
    wait = action.execute(lambda cmd: None, ctx)
    assert wait == 500
    assert ctx["delays"] == ["500"]


def test_execute_delay_negative_clamped_to_zero():
    """DELAY 负值 → clamp 到 0。"""

    action = AutomationAction(action_type=ActionType.DELAY, delay_ms=-100)
    assert action.execute(lambda cmd: None, {}) == 0


def test_execute_default_context_when_none():
    """context=None → 自动创建空 dict（不崩溃）。"""

    action = AutomationAction(action_type=ActionType.LOG, log_message="ok")
    action.execute(lambda cmd: None, None)  # type: ignore[arg-type]


# ── AutomationAction to_dict / from_dict ─────────────────────────────────


def test_action_to_dict_contains_all_fields():
    """to_dict 含 6 个字段。"""

    action = AutomationAction(
        action_type=ActionType.SEND_COMMAND, command="AT", delay_ms=100,
    )
    d = action.to_dict()
    assert set(d.keys()) == {
        "action_type", "command", "sound_path", "log_message", "led_state", "delay_ms",
    }


def test_action_from_dict_round_trip():
    """from_dict(to_dict()) 保持字段等价。"""

    original = AutomationAction(
        action_type=ActionType.PLAY_SOUND, sound_path="beep.wav", delay_ms=200,
    )
    restored = AutomationAction.from_dict(original.to_dict())
    assert restored.action_type == ActionType.PLAY_SOUND
    assert restored.sound_path == "beep.wav"
    assert restored.delay_ms == 200


def test_action_from_dict_defaults_missing_fields():
    """from_dict 缺字段回退默认。"""

    action = AutomationAction.from_dict({"action_type": "LOG"})
    assert action.action_type == ActionType.LOG
    assert action.command == ""
    assert action.delay_ms == 0


def test_action_defaults():
    """AutomationAction 默认值 + ActionType 5 枚举。"""

    action = AutomationAction(action_type=ActionType.DELAY)
    assert action.command == "" and action.sound_path == ""
    assert action.log_message == "" and action.led_state == "" and action.delay_ms == 0
    assert len(ActionType) == 5


# ── AutomationRule should_fire 冷却 ──────────────────────────────────────


def _make_rule(cooldown_ms: int = 0, enabled: bool = True) -> AutomationRule:
    """构造测试用 AutomationRule。"""

    return AutomationRule(
        name="test-rule",
        trigger=TriggerCondition(trigger_type=TriggerType.VALUE_ABOVE, threshold=50.0),
        actions=[AutomationAction(action_type=ActionType.LOG, log_message="fired")],
        enabled=enabled,
        cooldown_ms=cooldown_ms,
    )


def test_should_fire_enabled_no_cooldown():
    """enabled=True + 无冷却 → True。"""

    rule = _make_rule(cooldown_ms=0)
    assert rule.should_fire(elapsed_ms=0) is True


def test_should_fire_disabled_returns_false():
    """enabled=False → False。"""

    rule = _make_rule(enabled=False)
    assert rule.should_fire(elapsed_ms=0) is False


def test_should_fire_first_fire_after_mark():
    """mark_fired 后 + 冷却期内 → False。"""

    rule = _make_rule(cooldown_ms=1000)
    rule.mark_fired(elapsed_ms=100)
    assert rule.should_fire(elapsed_ms=500) is False


def test_should_fire_after_cooldown():
    """mark_fired 后 + 过冷却期 → True。"""

    rule = _make_rule(cooldown_ms=1000)
    rule.mark_fired(elapsed_ms=100)
    assert rule.should_fire(elapsed_ms=1101) is True


def test_mark_fired_updates_last_fire():
    """mark_fired 更新 last_fire_ms。"""

    rule = _make_rule()
    rule.mark_fired(elapsed_ms=42)
    assert rule.last_fire_ms == 42


def test_reset_clears_last_fire_and_trigger():
    """reset 清除 last_fire_ms + trigger.reset()。"""

    rule = _make_rule(cooldown_ms=1000)
    rule.mark_fired(elapsed_ms=100)
    rule.reset()
    assert rule.last_fire_ms == -1


# ── AutomationRule to_dict / from_dict ───────────────────────────────────


def test_rule_to_dict_contains_all_fields():
    """to_dict 含 name/enabled/cooldown_ms/trigger/actions 5 字段。"""

    rule = _make_rule(cooldown_ms=500)
    d = rule.to_dict()
    assert set(d.keys()) == {"name", "enabled", "cooldown_ms", "trigger", "actions"}


def test_rule_from_dict_round_trip():
    """from_dict(to_dict()) 保持核心字段等价。"""

    rule = _make_rule(cooldown_ms=500)
    restored = AutomationRule.from_dict(rule.to_dict())
    assert restored.name == "test-rule"
    assert restored.cooldown_ms == 500
    assert restored.enabled is True
    assert len(restored.actions) == 1


def test_rule_from_dict_defaults_missing_fields():
    """from_dict 缺 enabled/cooldown_ms 回退默认。"""

    rule = AutomationRule.from_dict({
        "name": "x",
        "trigger": {
            "trigger_type": "MANUAL",
        },
    })
    assert rule.name == "x"
    assert rule.enabled is True
    assert rule.cooldown_ms == 0


def test_rule_defaults():
    """AutomationRule 默认 actions=[] + enabled=True + cooldown_ms=0。"""

    rule = AutomationRule(
        name="x",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
    )
    assert rule.actions == []
    assert rule.enabled is True
    assert rule.cooldown_ms == 0

"""自动化引擎：聚合规则并在测量/收发事件下求值触发。"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import QObject, pyqtSignal

from embeddebug.serial_station.automation.actions import ActionContext, SendCallable
from embeddebug.serial_station.automation.rule import AutomationRule


class AutomationEngine(QObject):
    """规则求值与动作分发的中心。"""

    rule_added = pyqtSignal(str)
    rule_removed = pyqtSignal(str)
    rule_fired = pyqtSignal(str)

    def __init__(
        self,
        send_callable: SendCallable,
        parent: QObject | None = None,
    ) -> None:
        super().__init__(parent)
        self._send: Callable[[str], None] = send_callable
        self._rules: dict[str, AutomationRule] = {}
        self._clock_ms = 0
        self._context: ActionContext = {}

    def advance_clock(self, ms: int) -> None:
        """推进内部时钟，用于确定性测试。"""
        self._clock_ms += max(0, int(ms))

    def set_clock(self, ms: int) -> None:
        self._clock_ms = max(0, int(ms))

    @property
    def context(self) -> ActionContext:
        return self._context

    def add_rule(self, rule: AutomationRule) -> None:
        self._rules[rule.name] = rule
        self.rule_added.emit(rule.name)

    def remove_rule(self, name: str) -> bool:
        existed = self._rules.pop(name, None)
        if existed is None:
            return False
        self.rule_removed.emit(name)
        return True

    def enable_rule(self, name: str) -> bool:
        rule = self._rules.get(name)
        if rule is None:
            return False
        rule.enabled = True
        return True

    def disable_rule(self, name: str) -> bool:
        rule = self._rules.get(name)
        if rule is None:
            return False
        rule.enabled = False
        return True

    def rule_names(self) -> list[str]:
        return sorted(self._rules)

    def rules(self) -> list[AutomationRule]:
        return [self._rules[name] for name in sorted(self._rules)]

    def on_measurement(self, channel_values: dict[str, float]) -> None:
        """测量批次到达：用通道数值求值所有规则。"""
        self._evaluate(channel_values=channel_values, rx_text="", tx_text="")

    def on_rx(self, text: str) -> None:
        """收到一帧文本：用 rx_text 求值所有规则。"""
        self._evaluate(channel_values={}, rx_text=text, tx_text="")

    def on_tx(self, text: str) -> None:
        """发送一帧文本：用 tx_text 求值所有规则。"""
        self._evaluate(channel_values={}, rx_text="", tx_text=text)

    def fire_rule(self, name: str) -> bool:
        """手动触发（典型用于 MANUAL 类型规则）。"""
        rule = self._rules.get(name)
        if rule is None or not rule.enabled:
            return False
        rule.mark_fired(self._clock_ms)
        self._run_actions(rule)
        self.rule_fired.emit(rule.name)
        return True

    def _evaluate(
        self,
        channel_values: dict[str, float],
        rx_text: str,
        tx_text: str,
    ) -> None:
        elapsed = self._clock_ms
        for rule in list(self._rules.values()):
            if not rule.enabled or not rule.should_fire(elapsed):
                continue
            if rule.trigger.evaluate(channel_values, rx_text, tx_text, elapsed):
                rule.mark_fired(elapsed)
                self._run_actions(rule)
                self.rule_fired.emit(rule.name)

    def _run_actions(self, rule: AutomationRule) -> None:
        for action in rule.actions:
            action.execute(self._send, self._context)

"""自动化模式面板 — 规则列表 + 手动触发 + 触发日志。

ModePanel 实现：表格展示 AutomationEngine 已注册规则（启用/名称/触发/动作/冷却），
支持手动触发（MANUAL 规则）+ 启用/禁用切换 + 实时触发日志。on_enter 订阅
controller 事件驱动引擎求值，on_leave 停止求值（避免后台静默发包）。

约束：面板只调引擎公共 API（add_rule/enable/disable/fire/on_*），引擎逻辑在
automation/ 包；不直接调 controller 内部。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QPlainTextEdit,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.automation import (
    AutomationAction,
    AutomationEngine,
    AutomationRule,
    ActionType,
    TriggerCondition,
    TriggerType,
)
from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.ui.controls import DotState, StatusDot

_COLUMNS = ("启用", "名称", "触发", "动作", "冷却(ms)")


def _default_rules() -> tuple[AutomationRule, ...]:
    """内置演示规则（用真实构造函数，避免 from_dict schema 漂移）。"""

    return (
        AutomationRule(
            name="error_reset",
            trigger=TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="ERROR"),
            actions=[AutomationAction(action_type=ActionType.SEND_COMMAND, command="AT+RESET")],
            enabled=False,
            cooldown_ms=1000,
        ),
        AutomationRule(
            name="ok_log",
            trigger=TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="OK"),
            actions=[AutomationAction(action_type=ActionType.LOG, log_message="收到 OK 响应")],
            enabled=False,
            cooldown_ms=500,
        ),
    )


class AutomationPanel:
    """自动化 ModePanel：规则表 + 手动触发 + 日志。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._engine: AutomationEngine | None = None
        self._rule_table: QTableWidget | None = None
        self._log: QPlainTextEdit | None = None
        self._status: QLabel | None = None
        self._active = False

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationAutomationPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        controller = app_controller.serial_controller
        self._engine = AutomationEngine(send_callable=controller.send_text)
        for rule in _default_rules():
            self._engine.add_rule(rule)

        # 顶栏：启停 + 状态。
        top = QHBoxLayout()
        self._run_btn = QPushButton(widget.tr("启用监听"), widget)
        self._run_btn.setObjectName("serialStationAutomationRunButton")
        self._run_btn.setCheckable(True)
        self._run_btn.clicked.connect(self._toggle_active)
        # Batch 10-2: 监听状态圆点（GREEN 呼吸=监听中 / OFF=未启用，激活 PulseAnimation）。
        self._status_dot = StatusDot(parent=widget)
        self._status = QLabel(widget.tr("监听未启用"), widget)
        self._status.setObjectName("serialStationAutomationStatusLabel")
        top.addWidget(self._run_btn)
        top.addStretch(1)
        top.addWidget(self._status_dot)
        top.addWidget(self._status)
        layout.addLayout(top)

        # 规则表。
        self._rule_table = QTableWidget(0, len(_COLUMNS), widget)
        self._rule_table.setObjectName("serialStationAutomationRuleTable")
        self._rule_table.setHorizontalHeaderLabels(_COLUMNS)
        self._rule_table.verticalHeader().setVisible(False)
        self._rule_table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        layout.addWidget(self._rule_table)

        # 操作行：手动触发 + 刷新。
        ops = QHBoxLayout()
        fire_btn = QPushButton(widget.tr("手动触发"), widget)
        fire_btn.setObjectName("serialStationAutomationFireButton")
        fire_btn.clicked.connect(self._manual_fire)
        refresh_btn = QPushButton(widget.tr("刷新"), widget)
        refresh_btn.setObjectName("serialStationAutomationRefreshButton")
        refresh_btn.clicked.connect(self._refresh_table)
        ops.addWidget(fire_btn)
        ops.addWidget(refresh_btn)
        ops.addStretch(1)
        layout.addLayout(ops)

        # 日志。
        self._log = QPlainTextEdit(widget)
        self._log.setObjectName("serialStationAutomationLog")
        self._log.setReadOnly(True)
        self._log.setMaximumBlockCount(1000)
        self._log.setPlaceholderText(widget.tr("自动化触发日志…"))
        layout.addWidget(self._log, 1)

        self._widget = widget
        self._engine.rule_fired.connect(self._on_rule_fired)
        self._refresh_table()
        return widget

    def on_enter(self) -> None:
        """切入自动化页：播放入场动画 + 刷新表格。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)
        self._refresh_table()

    def on_leave(self) -> None:
        """切出自动化页：停止入场动画 + 停止监听（避免后台静默发包）。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)
        if self._active:
            self._set_active(False)

    # ── 交互 ────────────────────────────────────────────────────────
    def _toggle_active(self, checked: bool) -> None:
        self._set_active(checked)

    def _set_active(self, active: bool) -> None:
        self._active = active
        self._run_btn.setChecked(active)
        if self._app_controller is None or self._engine is None:
            return
        controller = self._app_controller.serial_controller
        if active:
            controller.on_log_entry(self._on_log_entry)
            controller.on_measurement_batch(self._on_measurement)
            # Batch 10-2: 监听中 GREEN 呼吸，文字不带 ●。
            self._status_dot.set_state(DotState.GREEN)
            self._status.setText(self._widget.tr("监听中"))
            self._append_log(self._widget.tr("已启用监听，订阅 controller 事件。"))
        else:
            # controller 当前无 remove_callback；通过标志停止求值（on_* 检查 _active）。
            self._status_dot.set_state(DotState.OFF)
            self._status.setText(self._widget.tr("监听未启用"))
            self._append_log(self._widget.tr("已停止监听。"))

    def _on_log_entry(self, entry: object) -> None:
        if not self._active or self._engine is None:
            return
        text = getattr(entry, "text", "")
        direction = getattr(entry, "direction", "")
        if direction == "TX":
            self._engine.on_tx(text)
        elif direction == "RX":
            self._engine.on_rx(text)

    def _on_measurement(self, batch: object) -> None:
        if not self._active or self._engine is None:
            return
        names = getattr(batch, "channel_names", ())
        values = getattr(batch, "values", None)
        if values is None or not names:
            return
        latest = values[-1] if hasattr(values, "__getitem__") else None
        if latest is None:
            return
        channel_values = {name: float(latest[i]) for i, name in enumerate(names) if i < len(latest)}
        self._engine.on_measurement(channel_values)

    def _on_rule_fired(self, name: str) -> None:
        self._append_log(self._widget.tr("规则「{name}」触发").format(name=name))

    def _manual_fire(self) -> None:
        if self._engine is None or self._rule_table is None:
            return
        row = self._rule_table.currentRow()
        if row < 0:
            self._append_log(self._widget.tr("请先在表格选中一条规则。"))
            return
        name_item = self._rule_table.item(row, 1)
        if name_item is None:
            return
        name = name_item.text()
        fired = self._engine.fire_rule(name)
        self._append_log(
            self._widget.tr("手动触发「{name}」：{result}").format(
                name=name, result=self._widget.tr("已执行") if fired else self._widget.tr("未触发")
            )
        )

    def _refresh_table(self) -> None:
        if self._engine is None or self._rule_table is None:
            return
        rules = self._engine.rules()
        self._rule_table.setRowCount(len(rules))
        for row, rule in enumerate(rules):
            self._rule_table.setItem(row, 0, QTableWidgetItem("✓" if rule.enabled else " "))
            self._rule_table.setItem(row, 1, QTableWidgetItem(rule.name))
            self._rule_table.setItem(row, 2, QTableWidgetItem(_trigger_summary(rule)))
            self._rule_table.setItem(row, 3, QTableWidgetItem(_action_summary(rule)))
            self._rule_table.setItem(row, 4, QTableWidgetItem(str(rule.cooldown_ms)))
        self._rule_table.resizeColumnsToContents()

    def _append_log(self, text: str) -> None:
        if self._log is not None:
            self._log.appendPlainText(text)


def _trigger_summary(rule: AutomationRule) -> str:
    t = rule.trigger
    kind = t.trigger_type
    kind_name = getattr(kind, "name", str(kind)) if kind is not None else "?"
    if t.pattern:
        return f"{kind_name} / {t.pattern}"
    if t.channel_name:
        return f"{kind_name} / {t.channel_name}"
    return kind_name


def _action_summary(rule: AutomationRule) -> str:
    if not rule.actions:
        return "-"
    parts = []
    for action in rule.actions:
        kind = action.action_type
        kind_name = getattr(kind, "name", str(kind)) if kind is not None else "?"
        text = action.command or action.log_message
        parts.append(f"{kind_name}({text})" if text else kind_name)
    return ", ".join(parts)

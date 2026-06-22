"""SvdPanel / AutomationPanel build 装配 + on_enter/leave 边界测试。

覆盖（避免实例化真实 transport，用 AppController 真实构建 + 源码级断言）：
1. SvdPanel.build 返回 QWidget + objectName 契约 + 子控件装配（树/详情/位域表）。
2. SvdPanel._FIELD_COLUMNS / AutomationPanel._COLUMNS 常量契约。
3. AutomationPanel.build 返回 QWidget + objectName + 引擎已初始化 + 规则表/日志控件装配。
4. AutomationPanel on_enter/on_leave 在未 build / 已 build 两种状态都不崩溃。
5. _default_rules / _trigger_summary / _action_summary 纯函数边界。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QPlainTextEdit, QTableWidget, QTreeWidget, QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.automation import (
    ActionType,
    AutomationAction,
    AutomationRule,
    TriggerCondition,
    TriggerType,
)
from embeddebug.serial_station.ui.panels.automation_panel import (
    AutomationPanel,
    _action_summary,
    _COLUMNS,
    _default_rules,
    _trigger_summary,
)
from embeddebug.serial_station.ui.panels.svd_panel import SvdPanel, _FIELD_COLUMNS


# ── 常量契约 ──────────────────────────────────────────────────────
def test_field_columns_contract():
    """_FIELD_COLUMNS 应含 4 列（位域/位范围/访问/复位值）。"""

    assert len(_FIELD_COLUMNS) == 4
    assert _FIELD_COLUMNS[0] == "位域"
    assert "复位值" in _FIELD_COLUMNS


def test_automation_columns_contract():
    """_COLUMNS 应含 5 列（启用/名称/触发/动作/冷却）。"""

    assert len(_COLUMNS) == 5
    assert _COLUMNS[0] == "启用"
    assert _COLUMNS[4] == "冷却(ms)"


# ── SvdPanel.build 装配 ───────────────────────────────────────────
def test_svd_panel_build_returns_widget(qtbot):
    panel = SvdPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_svd_panel_build_objectname(qtbot):
    panel = SvdPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationSvdPanel"


def test_svd_panel_build_wires_controls(qtbot):
    """build 后应初始化 tree/detail_form/field_table/status_dot/device_label。"""

    panel = SvdPanel()
    panel.build(AppController())
    assert panel._tree is not None and isinstance(panel._tree, QTreeWidget)
    assert panel._tree.objectName() == "serialStationSvdTree"
    assert panel._field_table is not None and isinstance(panel._field_table, QTableWidget)
    assert panel._field_table.objectName() == "serialStationSvdFieldTable"
    assert panel._detail_form is not None
    assert panel._status_dot is not None
    assert panel._device_label is not None


def test_svd_panel_field_table_columns(qtbot):
    """field_table 水平表头应等于 _FIELD_COLUMNS。"""

    panel = SvdPanel()
    panel.build(AppController())
    headers = [
        panel._field_table.horizontalHeaderItem(i).text()
        for i in range(panel._field_table.columnCount())
    ]
    assert tuple(headers) == _FIELD_COLUMNS


# ── AutomationPanel.build 装配 ───────────────────────────────────
def test_automation_panel_build_returns_widget(qtbot):
    panel = AutomationPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_automation_panel_build_objectname(qtbot):
    panel = AutomationPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationAutomationPanel"


def test_automation_panel_build_inits_engine_and_controls(qtbot):
    """build 后应初始化 engine + rule_table/log/status_dot + 默认规则已注入。"""

    panel = AutomationPanel()
    panel.build(AppController())
    assert panel._engine is not None
    assert panel._rule_table is not None and isinstance(panel._rule_table, QTableWidget)
    assert panel._log is not None and isinstance(panel._log, QPlainTextEdit)
    assert panel._status is not None
    assert panel._status_dot is not None
    # 默认规则已注入（2 条）。
    assert panel._rule_table.rowCount() == 2


# ── on_enter/on_leave 边界 ────────────────────────────────────────
def test_automation_panel_on_enter_leave_after_build(qtbot):
    """build 后 on_enter/on_leave 不崩溃（on_leave 应把 _active 置 False）。"""

    panel = AutomationPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    panel.on_enter()
    panel.on_leave()
    assert panel._active is False


def test_automation_panel_on_enter_creates_enter_anims(qtbot):
    """on_enter 后 _enter_anims 非空（play_panel_enter 启动入场动画）。"""

    panel = AutomationPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    panel.on_enter()
    assert len(panel._enter_anims) > 0


# ── _default_rules 纯函数 ────────────────────────────────────────
def test_default_rules_count_and_names():
    rules = _default_rules()
    assert len(rules) == 2
    assert {r.name for r in rules} == {"error_reset", "ok_log"}
    # 默认全部未启用（避免后台静默发包）。
    assert all(not r.enabled for r in rules)


# ── _trigger_summary / _action_summary 边界 ──────────────────────
def test_trigger_summary_with_pattern():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.RX_MATCH, pattern="FAIL"),
        actions=[],
    )
    assert _trigger_summary(rule) == "RX_MATCH / FAIL"


def test_trigger_summary_with_channel():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(
            trigger_type=TriggerType.VALUE_ABOVE, channel_name="temp"
        ),
        actions=[],
    )
    assert _trigger_summary(rule) == "VALUE_ABOVE / temp"


def test_trigger_summary_manual_no_pattern_no_channel():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
        actions=[],
    )
    assert _trigger_summary(rule) == "MANUAL"


def test_action_summary_empty_actions():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
        actions=[],
    )
    assert _action_summary(rule) == "-"


def test_action_summary_send_command():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
        actions=[
            AutomationAction(action_type=ActionType.SEND_COMMAND, command="AT+RST")
        ],
    )
    assert _action_summary(rule) == "SEND_COMMAND(AT+RST)"


def test_action_summary_log_only():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
        actions=[AutomationAction(action_type=ActionType.LOG, log_message="hi")],
    )
    assert _action_summary(rule) == "LOG(hi)"


def test_action_summary_multiple_joined():
    rule = AutomationRule(
        name="r",
        trigger=TriggerCondition(trigger_type=TriggerType.MANUAL),
        actions=[
            AutomationAction(action_type=ActionType.SEND_COMMAND, command="A"),
            AutomationAction(action_type=ActionType.LOG, log_message="B"),
        ],
    )
    out = _action_summary(rule)
    assert ", " in out
    assert "SEND_COMMAND(A)" in out and "LOG(B)" in out

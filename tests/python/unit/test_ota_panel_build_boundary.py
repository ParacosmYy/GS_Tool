"""OtaPanel build 装配 + 内部 helper 边界测试。

覆盖（不实例化真实 transport / worker，使用 AppController 真实构建）：
1. _PROTOCOL_OPTIONS 常量契约（4 个协议 + label/kind 对应）。
2. _PlaceholderLineEdit 行为（initial text 来自 toolTip / setText 同步 label+toolTip / objectName）。
3. _OtaSignalBridge 信号契约（progress int,int + finished object）。
4. OtaPanel.build 装配（返回 QWidget + objectName + 控件装配 protocol_combo/file_edit/progress/log/skeleton/status_dot/start_button/bridge）。
5. _refresh_connection_state 未连接态（OFF + 未连接 + start_button disabled）。
6. _on_progress / _on_finished 边界（total=0 分支 / success 与 failure 两路径）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QComboBox, QPlainTextEdit, QProgressBar, QPushButton, QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.ota import OtaProtocolKind, TransferResult
from embeddebug.serial_station.ui.controls import DotState
from embeddebug.serial_station.ui.panels.ota_panel import (
    _OtaSignalBridge,
    _PlaceholderLineEdit,
    _PROTOCOL_OPTIONS,
    OtaPanel,
)


# ── _PROTOCOL_OPTIONS 常量契约 ───────────────────────────────────
def test_protocol_options_has_four_entries():
    assert len(_PROTOCOL_OPTIONS) == 4


def test_protocol_options_labels_and_kinds():
    labels = [label for label, _ in _PROTOCOL_OPTIONS]
    assert labels == ["XMODEM-CRC", "XMODEM", "YMODEM", "YMODEM-g"]
    kinds = [kind for _, kind in _PROTOCOL_OPTIONS]
    assert kinds == [
        OtaProtocolKind.XMODEM_CRC,
        OtaProtocolKind.XMODEM,
        OtaProtocolKind.YMODEM,
        OtaProtocolKind.YMODEM_G,
    ]


def test_protocol_options_kind_values_are_strings():
    """所有 kind.value 应是非空 str（用于日志展示）。"""

    for _label, kind in _PROTOCOL_OPTIONS:
        assert isinstance(kind.value, str) and kind.value


# ── _PlaceholderLineEdit 行为 ────────────────────────────────────
def test_placeholder_line_edit_initial_text_empty(qtbot):
    """占位框初始 toolTip 为空 → text() 为空。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    edit = _PlaceholderLineEdit("ph", parent)
    assert edit.text() == ""


def test_placeholder_line_edit_set_text_syncs(qtbot):
    """setText 应同时更新 label 文本 + toolTip（text() 读 toolTip）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    edit = _PlaceholderLineEdit("ph", parent)
    edit.setText("firmware.bin")
    assert edit.text() == "firmware.bin"
    assert edit._label.text() == "firmware.bin"
    assert edit._label.toolTip() == "firmware.bin"


def test_placeholder_line_edit_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    edit = _PlaceholderLineEdit("ph", parent)
    assert edit._label.objectName() == "serialStationOtaFileLabel"


# ── _OtaSignalBridge 信号契约 ────────────────────────────────────
def test_signal_bridge_has_progress_and_finished_signals():
    """_OtaSignalBridge 应声明 progress + finished 两个信号（类属性存在）。"""

    # pyqtSignal 是类级别的未绑定信号对象。
    assert hasattr(_OtaSignalBridge, "progress")
    assert hasattr(_OtaSignalBridge, "finished")
    # 两个信号应是不同的对象。
    assert _OtaSignalBridge.progress is not _OtaSignalBridge.finished


def test_signal_bridge_emits_progress_and_finished(qtbot):
    """实例 bridge 应能 emit progress(int,int) + finished(object)（信号可连接）。"""

    received_progress = []
    received_finished = []

    bridge = _OtaSignalBridge()
    qtbot.addWidget(bridge)
    bridge.progress.connect(lambda d, t: received_progress.append((d, t)))
    bridge.finished.connect(lambda r: received_finished.append(r))

    bridge.progress.emit(3, 10)
    bridge.finished.emit("done")
    assert received_progress == [(3, 10)]
    assert received_finished == ["done"]


# ── OtaPanel.build 装配 ──────────────────────────────────────────
def test_ota_panel_build_returns_widget(qtbot):
    panel = OtaPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_ota_panel_build_objectname(qtbot):
    panel = OtaPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationOtaPanel"


def test_ota_panel_build_wires_controls(qtbot):
    """build 后应初始化 protocol_combo/file_edit/progress/log/skeleton/bridge 等。"""

    panel = OtaPanel()
    panel.build(AppController())
    assert isinstance(panel._protocol_combo, QComboBox)
    assert panel._protocol_combo.objectName() == "serialStationOtaProtocolCombo"
    assert panel._protocol_combo.count() == 4  # 4 协议
    assert isinstance(panel._file_edit, _PlaceholderLineEdit)
    assert isinstance(panel._progress, QProgressBar)
    assert panel._progress.objectName() == "serialStationOtaProgress"
    assert isinstance(panel._log, QPlainTextEdit)
    assert panel._log.objectName() == "serialStationOtaLog"
    assert isinstance(panel._start_button, QPushButton)
    assert panel._start_button.objectName() == "serialStationOtaStartButton"
    assert panel._status_dot is not None
    assert panel._status_label is not None
    assert panel._transfer_skeleton is not None
    assert panel._bridge is not None


# ── _refresh_connection_state 未连接态 ───────────────────────────
def test_refresh_connection_state_disconnected(qtbot):
    """未连接时：圆点 OFF + 文字未连接 + start_button disabled。"""

    panel = OtaPanel()
    panel.build(AppController())
    panel._refresh_connection_state()
    # 默认 StatusDot 初始为 OFF；未连接刷新后仍 OFF。
    assert panel._status_dot._state == DotState.OFF
    assert panel._start_button.isEnabled() is False


# ── _on_progress 边界 ────────────────────────────────────────────
def test_on_progress_total_zero_no_division(qtbot):
    """total=0 不应除零（不更新 progress value，但追加日志）。"""

    panel = OtaPanel()
    panel.build(AppController())
    initial = panel._progress.value()
    panel._on_progress(0, 0)  # total=0
    # 不崩溃；value 保持（int(0*100/0) 被 total>0 守卫跳过）。
    assert panel._progress.value() == initial


def test_on_progress_updates_bar(qtbot):
    panel = OtaPanel()
    panel.build(AppController())
    panel._on_progress(5, 10)
    assert panel._progress.value() == 50


# ── _on_finished 边界 ────────────────────────────────────────────
def test_on_finished_success(qtbot):
    panel = OtaPanel()
    panel.build(AppController())
    result = TransferResult(
        success=True, blocks_sent=10, blocks_acked=10, retries=1, error=None
    )
    panel._on_finished(result)
    assert panel._progress.value() == 100
    # skeleton 应被 hide。
    assert panel._transfer_skeleton.isHidden()


def test_on_finished_failure(qtbot):
    panel = OtaPanel()
    panel.build(AppController())
    result = TransferResult(
        success=False, blocks_sent=5, blocks_acked=0, retries=0, error="timeout"
    )
    panel._on_finished(result)
    assert panel._transfer_skeleton.isHidden()
    # 日志含错误信息。
    assert "timeout" in panel._log.toPlainText()

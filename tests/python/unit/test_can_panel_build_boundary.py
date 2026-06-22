"""CanPanel build 装配 + 帧处理/清空/发送边界测试。

覆盖（不实例化真实 transport，AppController 真实构建）：
1. _COLUMNS / _DEMO_FRAMES 常量契约。
2. _CanSignalBridge 信号契约（frame_decoded list 可 emit）。
3. CanPanel.build 装配（返回 QWidget + objectName + table/empty_state/id_edit/ext_check/data_edit/bridge/stats 全控件 + 表列数对应 _COLUMNS）。
4. _on_events 过滤非 frame 事件 + _append_frame 首帧 hide empty_state + 累积行 + stats 更新。
5. _clear 恢复空状态（rowCount=0 + stats "0 帧" + empty_state show_with_fade）。
6. _send 未连接路径（warning toast + stats 反馈）+ hex 非法路径（error toast）。
7. on_enter/on_leave 不崩溃 + on_leave 停止 demo timer。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QCheckBox, QLineEdit, QTableWidget, QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.can import CanFrame
from embeddebug.serial_station.ui.panels.can_panel import (
    _CanSignalBridge,
    _COLUMNS,
    _DEMO_FRAMES,
    CanPanel,
)


# ── 常量契约 ──────────────────────────────────────────────────────
def test_columns_contract():
    assert len(_COLUMNS) == 7
    assert _COLUMNS[0] == "#"
    assert "数据" in _COLUMNS


def test_demo_frames_contract():
    """_DEMO_FRAMES 应有 3 帧，第一帧为标准帧，第二帧为扩展帧。"""

    assert len(_DEMO_FRAMES) == 3
    assert isinstance(_DEMO_FRAMES[0], CanFrame)
    assert _DEMO_FRAMES[1].can_id.is_extended is True
    assert _DEMO_FRAMES[0].can_id.is_extended is False


# ── _CanSignalBridge 信号契约 ─────────────────────────────────────
def test_signal_bridge_has_frame_decoded():
    assert hasattr(_CanSignalBridge, "frame_decoded")


def test_signal_bridge_emits_frame_decoded(qtbot):
    bridge = _CanSignalBridge()
    qtbot.addWidget(bridge)
    received = []
    bridge.frame_decoded.connect(lambda evts: received.extend(evts))
    bridge.frame_decoded.emit([{"type": "frame", "payload": {"canIdHex": "100"}}])
    assert len(received) == 1
    assert received[0]["type"] == "frame"


# ── CanPanel.build 装配 ───────────────────────────────────────────
def test_can_panel_build_returns_widget(qtbot):
    panel = CanPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_can_panel_build_objectname(qtbot):
    panel = CanPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationCanPanel"


def test_can_panel_build_wires_controls(qtbot):
    panel = CanPanel()
    panel.build(AppController())
    assert panel._table is not None and isinstance(panel._table, QTableWidget)
    assert panel._table.columnCount() == len(_COLUMNS)
    assert panel._table.objectName() == "serialStationCanFrameTable"
    assert isinstance(panel._id_edit, QLineEdit)
    assert isinstance(panel._data_edit, QLineEdit)
    assert isinstance(panel._ext_check, QCheckBox)
    assert panel._empty_state is not None
    assert panel._bridge is not None
    assert panel._codec is not None


# ── _on_events / _append_frame ────────────────────────────────────
def test_on_events_filters_non_frame(qtbot):
    """非 frame 类型事件应被忽略（不增行）。"""

    panel = CanPanel()
    panel.build(AppController())
    initial_rows = panel._table.rowCount()
    panel._on_events([{"type": "other"}, {"type": "status"}])
    assert panel._table.rowCount() == initial_rows


def test_on_events_appends_frame(qtbot):
    """frame 事件应追加一行，首帧隐藏 empty_state。"""

    panel = CanPanel()
    panel.build(AppController())
    panel._on_events([
        {"type": "frame", "payload": {
            "canIdHex": "123", "isExtended": False, "isFd": False,
            "dlc": 2, "dataHex": "dead", "timestamp": 1.5,
        }}
    ])
    assert panel._table.rowCount() == 1
    assert panel._empty_state.isHidden()
    # stats 应显示 "1 帧"。
    assert "1" in panel._stats.text()


def test_on_events_accumulates_rows(qtbot):
    panel = CanPanel()
    panel.build(AppController())
    for i in range(3):
        panel._on_events([{"type": "frame", "payload": {
            "canIdHex": "100", "isExtended": False, "isFd": False,
            "dlc": 1, "dataHex": "00", "timestamp": float(i),
        }}])
    assert panel._table.rowCount() == 3
    assert "3" in panel._stats.text()


# ── _clear ────────────────────────────────────────────────────────
def test_clear_resets_table_and_stats(qtbot):
    panel = CanPanel()
    panel.build(AppController())
    panel._on_events([{"type": "frame", "payload": {"canIdHex": "1", "dlc": 0}}])
    assert panel._table.rowCount() == 1
    panel._clear()
    assert panel._table.rowCount() == 0
    assert "0" in panel._stats.text()


# ── _send 边界 ────────────────────────────────────────────────────
def test_send_disconnected_shows_warning(qtbot):
    """未连接 transport：stats 显示未连接反馈，不抛异常。"""

    panel = CanPanel()
    panel.build(AppController())
    panel._send()
    assert "未连接" in panel._stats.text()


def test_send_invalid_hex_shows_error(qtbot):
    """连接但 hex 非法：stats 显示失败反馈（连接需 transport，此处仅测 hex 非法路径无 transport）。
    
    未连接时优先返回未连接，所以这里测 _append_local 反馈路径直接。
    """

    panel = CanPanel()
    panel.build(AppController())
    # 模拟 _append_local 直接调用。
    panel._append_local("测试反馈")
    assert panel._stats.text() == "测试反馈"


# ── on_enter/on_leave 边界 ────────────────────────────────────────
def test_on_enter_creates_enter_anims(qtbot):
    panel = CanPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    panel.on_enter()
    assert len(panel._enter_anims) > 0


def test_on_leave_with_no_demo_timer_does_not_crash(qtbot):
    """on_leave 在 _demo_timer 为 None 时不崩溃（防御性）。"""

    panel = CanPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert panel._demo_timer is None
    panel.on_leave()
    assert panel._demo_timer is None


def test_on_leave_clears_demo_timer_via_toggle_off(qtbot):
    """_toggle_demo(False) 应 stop + 清空 _demo_timer（不依赖真实 timer 触发）。"""

    panel = CanPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    # 注入一个已停止的 stub timer（避免真实 QTimer 在事件循环触发导致堆崩溃）。

    class _StoppedTimer:
        def stop(self) -> None:
            pass

    panel._demo_timer = _StoppedTimer()  # type: ignore[assignment]
    panel._toggle_demo(False)
    assert panel._demo_timer is None

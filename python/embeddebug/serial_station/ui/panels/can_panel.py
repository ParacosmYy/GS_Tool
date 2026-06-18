"""CAN 总线模式面板 — 帧列表 + 演示 + 发送。

ModePanel 实现：演示模式用 CanFrameCodec 造 SLCAN 帧再解码复用同渲染管线；
发送区输入 ID/数据编码后 transport.write。帧表展示 ID/Ext/FD/DLC/数据/时间戳。

约束：只调 can/ 引擎公共 API（CanFrame/CanFrameCodec/CanId/DbcDatabase），
不直接访问 transport 内部；不 import rtt/ble/ota。
"""

from __future__ import annotations

from PyQt6.QtCore import QTimer, Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QCheckBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.can import CanFrame, CanFrameCodec, CanId

_COLUMNS = ("#", "ID", "Ext", "FD", "DLC", "数据", "时间")
_DEMO_FRAMES = (
    CanFrame(CanId(0x100), bytes.fromhex("0102"), timestamp=0.0),
    CanFrame(CanId(0x256, is_extended=True), bytes.fromhex("AABBCCDD"), timestamp=0.1),
    CanFrame(CanId(0x123), bytes.fromhex("FF00"), timestamp=0.2),
)


class CanPanel:
    """CAN ModePanel：帧表 + 演示 + 发送。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._codec = CanFrameCodec()
        self._frame_index = 0
        self._demo_timer: QTimer | None = None
        self._bridge: _CanSignalBridge | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationCanPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        # 工具行。
        top = QHBoxLayout()
        demo_btn = QPushButton(widget.tr("演示"), widget)
        demo_btn.setObjectName("serialStationCanDemoButton")
        demo_btn.setCheckable(True)
        demo_btn.clicked.connect(self._toggle_demo)
        clear_btn = QPushButton(widget.tr("清空"), widget)
        clear_btn.setObjectName("serialStationCanClearButton")
        clear_btn.clicked.connect(self._clear)
        self._stats = QLabel(widget.tr("0 帧"), widget)
        self._stats.setObjectName("serialStationCanStatsLabel")
        top.addWidget(demo_btn)
        top.addWidget(clear_btn)
        top.addStretch(1)
        top.addWidget(self._stats)
        layout.addLayout(top)

        # 帧表。
        self._table = QTableWidget(0, len(_COLUMNS), widget)
        self._table.setObjectName("serialStationCanFrameTable")
        self._table.setHorizontalHeaderLabels(_COLUMNS)
        self._table.verticalHeader().setVisible(False)
        self._table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        layout.addWidget(self._table, 1)

        # Batch 9-2: 空帧表占位（EmptyStateWidget，有数据后隐藏）。
        from embeddebug.serial_station.ui.widgets import EmptyStateWidget

        self._empty_state = EmptyStateWidget(
            icon_name="network",
            title=widget.tr("暂无 CAN 帧"),
            description=widget.tr("点击「演示」生成示例帧，或连接 CAN 设备后发送数据"),
            parent=widget,
        )
        layout.addWidget(self._empty_state)

        # 发送行。
        send = QHBoxLayout()
        id_label = QLabel(widget.tr("ID"), widget)
        id_label.setObjectName("serialStationCanFieldLabel")
        self._id_edit = QLineEdit(widget)
        self._id_edit.setObjectName("serialStationCanIdEdit")
        self._id_edit.setPlaceholderText(widget.tr("hex, 如 123"))
        self._ext_check = QCheckBox(widget.tr("扩展"), widget)
        self._ext_check.setObjectName("serialStationCanExtCheckBox")
        data_label = QLabel(widget.tr("数据"), widget)
        data_label.setObjectName("serialStationCanFieldLabel")
        self._data_edit = QLineEdit(widget)
        self._data_edit.setObjectName("serialStationCanDataEdit")
        self._data_edit.setPlaceholderText(widget.tr("hex, 如 DEADBEEF"))
        send_btn = QPushButton(widget.tr("发送"), widget)
        send_btn.setObjectName("serialStationCanSendButton")
        send_btn.clicked.connect(self._send)
        send.addWidget(id_label)
        send.addWidget(self._id_edit)
        send.addWidget(self._ext_check)
        send.addWidget(data_label)
        send.addWidget(self._data_edit, 1)
        send.addWidget(send_btn)
        layout.addLayout(send)

        self._widget = widget
        self._bridge = _CanSignalBridge(widget)
        self._bridge.frame_decoded.connect(self._on_events)
        return widget

    def on_enter(self) -> None:
        """切入 CAN 页：播放入场动画。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)

    def on_leave(self) -> None:
        """切出 CAN 页：停止入场动画 + 停止演示。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)
        if self._demo_timer is not None:
            self._demo_timer.stop()
            self._demo_timer = None

    # ── 交互 ────────────────────────────────────────────────────────
    def _toggle_demo(self, checked: bool) -> None:
        if checked:
            self._demo_timer = QTimer(self._widget)
            self._demo_timer.timeout.connect(self._demo_tick)
            self._demo_timer.start(1000)
        elif self._demo_timer is not None:
            self._demo_timer.stop()
            self._demo_timer = None

    def _demo_tick(self) -> None:
        frame = _DEMO_FRAMES[self._frame_index % len(_DEMO_FRAMES)]
        self._frame_index += 1
        encoded = self._codec.encode(frame)
        events = self._codec.feed(encoded)
        self._bridge.frame_decoded.emit(events)

    def _on_events(self, events: list) -> None:
        for event in events:
            if event.get("type") != "frame":
                continue
            payload = event.get("payload", {})
            self._append_frame(payload)

    def _append_frame(self, payload: dict) -> None:
        if self._table is None:
            return
        # Batch 9-2: 首帧数据到达，隐藏空状态占位。
        if self._table.rowCount() == 0:
            self._empty_state.hide()
        row = self._table.rowCount()
        self._table.insertRow(row)
        self._table.setItem(row, 0, QTableWidgetItem(str(row + 1)))
        self._table.setItem(row, 1, QTableWidgetItem(payload.get("canIdHex", "")))
        self._table.setItem(row, 2, QTableWidgetItem("✓" if payload.get("isExtended") else ""))
        self._table.setItem(row, 3, QTableWidgetItem("✓" if payload.get("isFd") else ""))
        self._table.setItem(row, 4, QTableWidgetItem(str(payload.get("dlc", 0))))
        self._table.setItem(row, 5, QTableWidgetItem(payload.get("dataHex", "")))
        self._table.setItem(row, 6, QTableWidgetItem(f"{payload.get('timestamp', 0):.3f}"))
        self._table.scrollToBottom()
        if self._stats is not None:
            self._stats.setText(self._widget.tr("{n} 帧").format(n=row + 1))

    def _send(self) -> None:
        if self._app_controller is None:
            return
        transport = self._app_controller.active_transport()
        if transport is None:
            self._append_local("未连接串口，无法发送")
            return
        try:
            can_id = CanId(int(self._id_edit.text(), 16), is_extended=self._ext_check.isChecked())
            data = bytes.fromhex(self._data_edit.text())
            frame = CanFrame(can_id, data)
            transport.write(self._codec.encode(frame))
            self._append_local(self._widget.tr("已发送 {id} {data}").format(
                id=can_id.as_hex(), data=data.hex()
            ))
        except (ValueError, OSError) as exc:
            self._append_local(self._widget.tr("发送失败：{err}").format(err=exc))

    def _append_local(self, text: str) -> None:
        # 复用 stats 标签暂存发送反馈（无独立日志区，保持简洁）。
        if self._stats is not None:
            self._stats.setText(text)

    def _clear(self) -> None:
        if self._table is not None:
            self._table.setRowCount(0)
        if self._stats is not None:
            self._stats.setText(self._widget.tr("0 帧"))
        # Batch 9-2: 清空后恢复空状态占位。
        self._empty_state.show()


class _CanSignalBridge(QWidget):
    """跨线程信号桥（demo timer / transport 线程 → 主线程 UI）。"""

    frame_decoded = pyqtSignal(list)

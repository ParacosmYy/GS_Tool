"""RTT 实时模式面板 — 通道选择 + 文本视图 + loopback 演示。

ModePanel 实现：未连接串口时用 ``RttTransportStub`` + QTimer 注入演示文本；
已连接时复用 ``app_controller.active_transport()`` 订阅真实字节流。文本视图
``QPlainTextEdit`` 展示 RTT 通道数据，跨线程经 signal bridge 回主线程。

约束：只调 rtt/ 引擎公共 API（RttSession/RttConfig/RttTransportStub），
不直接访问 transport 内部；不 import can/ble/ota。
"""

from __future__ import annotations

from PyQt6.QtCore import QTimer, Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QPlainTextEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.rtt import RttChannel, RttConfig, RttSession, RttTransportStub
from embeddebug.serial_station.ui.controls import DotState, StatusDot

_DEMO_CHANNELS = (
    RttChannel(name="terminal", buffer_size=1024, mode="up"),
    RttChannel(name="log", buffer_size=512, mode="up"),
)
_DEMO_LINES = (
    b"[terminal] EmbedDebug RTT demo\n",
    b"[log] channel ready\n",
    b"[terminal] hello RTT\n",
    b"[log] tick\n",
)


class RttPanel:
    """RTT ModePanel：通道 + 文本视图 + 启停 + loopback 演示。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._stub: RttTransportStub | None = None
        self._session: RttSession | None = None
        self._demo_timer: QTimer | None = None
        self._demo_index = 0
        self._bridge: _RttSignalBridge | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationRttPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        # 工具行。
        top = QHBoxLayout()
        ch_label = QLabel(widget.tr("通道"), widget)
        ch_label.setObjectName("serialStationRttFieldLabel")
        self._channel_combo = QComboBox(widget)
        self._channel_combo.setObjectName("serialStationRttChannelCombo")
        for ch in _DEMO_CHANNELS:
            self._channel_combo.addItem(ch.name)
        clear_btn = QPushButton(widget.tr("清屏"), widget)
        clear_btn.setObjectName("serialStationRttClearButton")
        clear_btn.clicked.connect(self._clear)
        self._start_btn = QPushButton(widget.tr("启动"), widget)
        self._start_btn.setObjectName("serialStationRttStartButton")
        self._start_btn.setCheckable(True)
        self._start_btn.clicked.connect(self._toggle)
        # Batch 10-2: 运行状态圆点（GREEN=串口 / BLUE=演示 / OFF=停止，呼吸激活 PulseAnimation）。
        self._status_dot = StatusDot(parent=widget)
        self._status = QLabel(widget.tr("未启动"), widget)
        self._status.setObjectName("serialStationRttStatusLabel")
        top.addWidget(ch_label)
        top.addWidget(self._channel_combo)
        top.addWidget(clear_btn)
        top.addStretch(1)
        top.addWidget(self._start_btn)
        top.addWidget(self._status_dot)
        top.addWidget(self._status)
        layout.addLayout(top)

        # 文本视图。
        self._text = QPlainTextEdit(widget)
        self._text.setObjectName("serialStationRttTextView")
        self._text.setReadOnly(True)
        self._text.setMaximumBlockCount(2000)
        self._text.setPlaceholderText(widget.tr("RTT 通道数据…"))
        layout.addWidget(self._text, 1)

        # Batch 9-2: 未启动时空状态占位（收到数据后隐藏）。
        from embeddebug.serial_station.ui.widgets import EmptyStateWidget

        self._empty_state = EmptyStateWidget(
            icon_name="activity",
            title=widget.tr("RTT 未启动"),
            description=widget.tr("点击「启动」开始接收 SEGGER RTT 通道数据"),
            parent=widget,
        )
        layout.addWidget(self._empty_state)

        self._widget = widget
        self._bridge = _RttSignalBridge(widget)
        self._bridge.bytes_received.connect(self._on_bytes)
        self._bridge.error.connect(self._on_error)
        return widget

    def on_enter(self) -> None:
        """切入 RTT 页：播放入场动画（启动由按钮触发）。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)

    def on_leave(self) -> None:
        """切出 RTT 页：停止入场动画 + 停止演示避免后台泄漏。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)
        if self._start_btn is not None and self._start_btn.isChecked():
            self._toggle(False)

    # ── 交互 ────────────────────────────────────────────────────────
    def _toggle(self, checked: bool) -> None:
        if checked:
            self._start()
        else:
            self._stop()

    def _start(self) -> None:
        config = RttConfig(channels=_DEMO_CHANNELS)
        transport = self._app_controller.active_transport() if self._app_controller else None
        if transport is not None:
            self._stub = None
            self._session = RttSession(transport, config)
            # Batch 10-2: 串口运行 GREEN（呼吸），文字不再带 ●。
            self._status_dot.set_state(DotState.GREEN)
            self._status.setText(self._widget.tr("运行中（串口）"))
        else:
            self._stub = RttTransportStub(open=True)
            self._session = RttSession(self._stub, config)
            self._demo_timer = QTimer(self._widget)
            self._demo_timer.timeout.connect(self._demo_tick)
            self._demo_timer.start(800)
            # 演示模式 BLUE（活动呼吸）。
            self._status_dot.set_state(DotState.BLUE)
            self._status.setText(self._widget.tr("运行中（演示）"))
        self._session.on_bytes_received(lambda b: self._bridge.bytes_received.emit(b))
        self._session.on_error(lambda m: self._bridge.error.emit(m))
        self._start_btn.setText(self._widget.tr("停止"))

    def _stop(self) -> None:
        if self._demo_timer is not None:
            self._demo_timer.stop()
            self._demo_timer = None
        if self._session is not None:
            try:
                self._session.close()
            except Exception:
                pass
            self._session = None
        self._stub = None
        if self._start_btn is not None:
            self._start_btn.setText(self._widget.tr("启动"))
        # Batch 10-2: 停止后圆点恢复 OFF（静止）。
        self._status_dot.set_state(DotState.OFF)
        if self._status is not None:
            self._status.setText(self._widget.tr("未启动"))

    def _demo_tick(self) -> None:
        if self._stub is None:
            return
        line = _DEMO_LINES[self._demo_index % len(_DEMO_LINES)]
        self._demo_index += 1
        self._stub.inject_received(line)

    def _on_bytes(self, data: bytes) -> None:
        if self._text is not None:
            self._text.appendPlainText(data.decode("utf-8", errors="replace").rstrip())
        # Batch 9-2: 首次收到数据隐藏空状态。
        if self._empty_state is not None and not self._empty_state.isHidden():
            self._empty_state.hide()

    def _on_error(self, msg: str) -> None:
        if self._text is not None:
            self._text.appendPlainText(f"[err] {msg}")

    def _clear(self) -> None:
        if self._text is not None:
            self._text.clear()
        # Batch 9-2: 清屏后恢复空状态占位；Batch 11: 淡入显示（激活 FadeTransition）。
        if self._empty_state is not None:
            self._empty_state.show_with_fade()


class _RttSignalBridge(QWidget):
    """跨线程信号桥（transport/stub 线程 → 主线程 UI）。"""

    bytes_received = pyqtSignal(bytes)
    error = pyqtSignal(str)

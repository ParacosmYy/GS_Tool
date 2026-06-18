"""OTA 升级模式面板 — 文件选择 + 协议选择 + 进度 + 日志。

通过 AppController.active_transport() 复用已连接串口，用 TransferEngine
驱动 X/YMODEM 传输。传输在后台线程运行，进度/日志通过 Qt signal 回主线程更新。

约束：UI 只发意图 + 展示，协议逻辑在 ota/ 引擎；不直接访问 transport 内部。
"""

from __future__ import annotations

import threading
from pathlib import Path

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QComboBox,
    QFileDialog,
    QFrame,
    QHBoxLayout,
    QLabel,
    QPlainTextEdit,
    QProgressBar,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.ota import (
    OtaProtocolKind,
    SerialTransportAdapter,
    TransferEngine,
    TransferResult,
    make_protocol,
)
from embeddebug.serial_station.ui.controls import DotState, StatusDot


_PROTOCOL_OPTIONS = (
    ("XMODEM-CRC", OtaProtocolKind.XMODEM_CRC),
    ("XMODEM", OtaProtocolKind.XMODEM),
    ("YMODEM", OtaProtocolKind.YMODEM),
    ("YMODEM-g", OtaProtocolKind.YMODEM_G),
)


class OtaPanel:
    """OTA 升级 ModePanel：文件 + 协议 + 进度 + 日志。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._engine: TransferEngine | None = None
        self._worker: threading.Thread | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationOtaPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        # 协议选择行。
        proto_row = QHBoxLayout()
        proto_label = QLabel(widget.tr("协议"), widget)
        proto_label.setObjectName("serialStationOtaFieldLabel")
        self._protocol_combo = QComboBox(widget)
        self._protocol_combo.setObjectName("serialStationOtaProtocolCombo")
        for label, _kind in _PROTOCOL_OPTIONS:
            self._protocol_combo.addItem(label)
        proto_row.addWidget(proto_label)
        proto_row.addWidget(self._protocol_combo, 1)
        layout.addLayout(proto_row)

        # 文件选择行。
        file_row = QHBoxLayout()
        file_label = QLabel(widget.tr("固件"), widget)
        file_label.setObjectName("serialStationOtaFieldLabel")
        self._file_edit = _PlaceholderLineEdit(widget.tr("选择固件文件…"), widget)
        self._file_edit.setObjectName("serialStationOtaFileEdit")
        self._browse_button = QPushButton(widget.tr("选择"), widget)
        self._browse_button.setObjectName("serialStationOtaBrowseButton")
        self._browse_button.clicked.connect(self._browse_file)
        file_row.addWidget(file_label)
        file_row.addWidget(self._file_edit, 1)
        file_row.addWidget(self._browse_button)
        layout.addLayout(file_row)

        # 进度条。
        self._progress = QProgressBar(widget)
        self._progress.setObjectName("serialStationOtaProgress")
        self._progress.setRange(0, 100)
        self._progress.setValue(0)
        layout.addWidget(self._progress)

        # 日志区。
        self._log = QPlainTextEdit(widget)
        self._log.setObjectName("serialStationOtaLog")
        self._log.setReadOnly(True)
        self._log.setPlaceholderText(widget.tr("升级日志…"))
        layout.addWidget(self._log, 1)

        # Batch 9-1: 传输中 skeleton shimmer 占位（叠在日志区，传输开始 show/完成 hide）。
        # 保留 SkeletonBlock 默认 objectName（serialStationSkeletonBlock，已有 QSS 覆盖）。
        from embeddebug.serial_station.ui.widgets.skeleton import SkeletonBlock

        self._transfer_skeleton = SkeletonBlock(widget, rows=4, with_title=True)
        self._transfer_skeleton.hide()
        layout.addWidget(self._transfer_skeleton)

        # 状态 + 开始按钮行。
        action_row = QHBoxLayout()
        # Batch 10-2: 连接状态圆点（呼吸指示，替代 ●/○ 字符；激活 PulseAnimation）。
        self._status_dot = StatusDot(parent=widget)
        action_row.addWidget(self._status_dot)
        self._status_label = QLabel(widget.tr("未连接"), widget)
        self._status_label.setObjectName("serialStationOtaStatusLabel")
        action_row.addWidget(self._status_label)
        action_row.addStretch(1)
        self._start_button = QPushButton(widget.tr("开始升级"), widget)
        self._start_button.setObjectName("serialStationOtaStartButton")
        self._start_button.clicked.connect(self._start_transfer)
        action_row.addWidget(self._start_button)
        layout.addLayout(action_row)

        # signal 桥（worker 线程 → 主线程 UI 更新）。
        self._widget = widget
        self._bridge = _OtaSignalBridge(widget)
        self._bridge.progress.connect(self._on_progress)
        self._bridge.finished.connect(self._on_finished)
        self._refresh_connection_state()
        return widget

    # ── 生命周期 ────────────────────────────────────────────────────
    def on_enter(self) -> None:
        """切入 OTA 模式：播放入场动画 + 刷新连接态。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)
        self._refresh_connection_state()

    def on_leave(self) -> None:
        """切出：停止入场动画（不中止传输，保留后台进行）。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)

    # ── 交互 ────────────────────────────────────────────────────────
    def _browse_file(self) -> None:
        path, _ = QFileDialog.getOpenFileName(
            self._widget, self._widget.tr("选择固件文件"), "", "固件 (*.bin *.hex *.img);;所有文件 (*)"
        )
        if path:
            self._file_edit.setText(path)

    def _refresh_connection_state(self) -> None:
        if self._app_controller is None:
            return
        connected = self._app_controller.is_connected()
        # Batch 10-2: 圆点承担 ●/○ 视觉，文字不再带字符。
        self._status_dot.set_state(DotState.GREEN if connected else DotState.OFF)
        self._status_label.setText(
            self._widget.tr("已连接") if connected else self._widget.tr("未连接")
        )
        self._start_button.setEnabled(connected and bool(self._file_edit.text()))

    def _start_transfer(self) -> None:
        if self._app_controller is None:
            return
        transport = self._app_controller.active_transport()
        if transport is None:
            self._log.appendPlainText(self._widget.tr("错误：未连接串口"))
            return
        path = self._file_edit.text().strip()
        if not path or not Path(path).is_file():
            self._log.appendPlainText(self._widget.tr("错误：请选择有效固件文件"))
            return
        firmware = Path(path).read_bytes()
        kind = _PROTOCOL_OPTIONS[self._protocol_combo.currentIndex()][1]
        protocol = make_protocol(kind, firmware, filename=Path(path).name)
        adapter = SerialTransportAdapter(transport)
        self._engine = TransferEngine(
            protocol, adapter,
            on_progress=lambda done, total: self._bridge.progress.emit(done, total),
        )
        self._start_button.setEnabled(False)
        self._log.appendPlainText(
            self._widget.tr("开始升级：{name} ({bytes} 字节, {proto})").format(
                name=Path(path).name, bytes=len(firmware), proto=kind.value
            )
        )
        # Batch 9-1: 传输中显示 skeleton shimmer 占位。
        self._transfer_skeleton.show()
        self._worker = threading.Thread(
            target=self._run_engine, args=(self._engine,), daemon=True
        )
        self._worker.start()

    def _run_engine(self, engine: TransferEngine) -> None:
        result = engine.run()
        self._bridge.finished.emit(result)

    def _on_progress(self, done: int, total: int) -> None:
        if total > 0:
            self._progress.setValue(int(done * 100 / total))
        self._log.appendPlainText(self._widget.tr("已发送块 {done}/{total}").format(done=done, total=total))

    def _on_finished(self, result: TransferResult) -> None:
        # Batch 9-1: 传输完成隐藏 skeleton。
        self._transfer_skeleton.hide()
        if result.success:
            self._progress.setValue(100)
            self._log.appendPlainText(
                self._widget.tr("升级完成：{acked} 块已确认，{retries} 次重传").format(
                    acked=result.blocks_acked, retries=result.retries
                )
            )
        else:
            self._log.appendPlainText(
                self._widget.tr("升级失败：{error}").format(error=result.error or "未知错误")
            )
        self._start_button.setEnabled(self._app_controller is not None and self._app_controller.is_connected())


class _OtaSignalBridge(QWidget):
    """跨线程信号桥（worker 线程 emit，主线程槽更新 UI）。"""

    progress = pyqtSignal(int, int)
    finished = pyqtSignal(object)


class _PlaceholderLineEdit(QFrame):
    """简单文件路径显示框（只读，模拟 QLineEdit 占位）。"""

    def __init__(self, placeholder: str, parent: QWidget) -> None:
        super().__init__(parent)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(8, 0, 8, 0)
        self._label = QLabel(placeholder, self)
        self._label.setObjectName("serialStationOtaFileLabel")
        layout.addWidget(self._label)

    def text(self) -> str:
        return self._label.toolTip()

    def setText(self, text: str) -> None:
        self._label.setText(text)
        self._label.setToolTip(text)

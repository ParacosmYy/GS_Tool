"""BLE 蓝牙模式面板 — 扫描 + 设备列表 + GATT 树 + 读写 + notify。

ModePanel 实现：用 ``BleTransportStub``（预置 fake 设备）做 D2 演示，UI 全就绪，
等 bleak 接入真实蓝牙时仅替换 transport 工厂。GATT 树展示服务/特征，选中特征
回填读写区，notify 经 codec 解码 + signal bridge 回日志。

约束：只调 ble/ 引擎公共 API（BleTransportStub/BleFrameCodec/BleGattTree），
不 import rtt/can/ota；不 import bleak（未集成）。
"""

from __future__ import annotations

from PyQt6.QtCore import pyqtSignal
from PyQt6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QPushButton,
    QTreeWidget,
    QTreeWidgetItem,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ble import BleFrameCodec, BleTransportStub
from embeddebug.serial_station.ui.controls import DotState, StatusDot

_STUB_ADDRESS = "AA:BB:CC:DD:EE:FF"


class BlePanel:
    """BLE ModePanel：扫描 + GATT 树 + 读写 + notify 日志。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._transport: BleTransportStub | None = None
        self._codec = BleFrameCodec()
        self._bridge: _BleSignalBridge | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationBlePanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        # 扫描行。
        scan = QHBoxLayout()
        scan_label = QLabel(widget.tr("设备"), widget)
        scan_label.setObjectName("serialStationBleFieldLabel")
        self._device_combo = QComboBox(widget)
        self._device_combo.setObjectName("serialStationBleDeviceCombo")
        scan_btn = QPushButton(widget.tr("扫描"), widget)
        scan_btn.setObjectName("serialStationBleScanButton")
        self._scan_btn = scan_btn
        scan_btn.clicked.connect(self._scan)
        self._connect_btn = QPushButton(widget.tr("连接"), widget)
        self._connect_btn.setObjectName("serialStationBleConnectButton")
        self._connect_btn.setCheckable(True)
        self._connect_btn.clicked.connect(self._connect)
        # Batch 10-2: 连接状态圆点（GREEN 呼吸=已连接 / OFF=未连接，激活 PulseAnimation）。
        self._status_dot = StatusDot(parent=widget)
        scan.addWidget(scan_label)
        scan.addWidget(self._device_combo, 1)
        scan.addWidget(scan_btn)
        scan.addWidget(self._connect_btn)
        scan.addWidget(self._status_dot)
        layout.addLayout(scan)

        # GATT 树。
        self._tree = QTreeWidget(widget)
        self._tree.setObjectName("serialStationBleGattTree")
        self._tree.setHeaderLabels((widget.tr("UUID"), widget.tr("属性"), widget.tr("值(hex)"), widget.tr("handle")))
        self._tree.itemSelectionChanged.connect(self._on_tree_select)
        layout.addWidget(self._tree, 1)

        # Batch 9-2: 空 GATT 树占位（扫描连接后填充，初始无设备引导）。
        from embeddebug.serial_station.ui.widgets import EmptyStateWidget

        self._empty_state = EmptyStateWidget(
            icon_name="bluetooth",
            title=widget.tr("未发现 BLE 设备"),
            description=widget.tr("点击「扫描」搜索附近设备，连接后显示 GATT 服务树"),
            parent=widget,
        )
        layout.addWidget(self._empty_state)

        # 读写行。
        ops = QHBoxLayout()
        char_label = QLabel(widget.tr("特征"), widget)
        char_label.setObjectName("serialStationBleFieldLabel")
        self._char_edit = QLineEdit(widget)
        self._char_edit.setObjectName("serialStationBleCharUuidEdit")
        self._char_edit.setReadOnly(True)
        self._payload_edit = QLineEdit(widget)
        self._payload_edit.setObjectName("serialStationBlePayloadEdit")
        self._payload_edit.setPlaceholderText(widget.tr("hex payload"))
        read_btn = QPushButton(widget.tr("读"), widget)
        read_btn.setObjectName("serialStationBleReadButton")
        read_btn.clicked.connect(self._read_char)
        write_btn = QPushButton(widget.tr("写"), widget)
        write_btn.setObjectName("serialStationBleWriteButton")
        write_btn.clicked.connect(self._write_char)
        notify_btn = QPushButton(widget.tr("订阅"), widget)
        notify_btn.setObjectName("serialStationBleNotifyButton")
        notify_btn.setCheckable(True)
        notify_btn.clicked.connect(self._toggle_notify)
        ops.addWidget(char_label)
        ops.addWidget(self._char_edit, 1)
        ops.addWidget(self._payload_edit, 1)
        ops.addWidget(read_btn)
        ops.addWidget(write_btn)
        ops.addWidget(notify_btn)
        layout.addLayout(ops)

        # 日志。
        self._log = QPlainTextEdit(widget)
        self._log.setObjectName("serialStationBleLog")
        self._log.setReadOnly(True)
        self._log.setMaximumBlockCount(1000)
        self._log.setPlaceholderText(widget.tr("BLE 日志…"))
        layout.addWidget(self._log, 1)

        self._widget = widget
        self._bridge = _BleSignalBridge(widget)
        self._bridge.notify_received.connect(self._on_notify)
        self._bridge.error.connect(self._on_error)
        # Batch 22: 给面板输入控件装 focus_ring。
        from embeddebug.serial_station.ui.panels._focus_ring import apply_panel_focus_rings

        apply_panel_focus_rings(widget)
        return widget

    def on_enter(self) -> None:
        """切入 BLE 页：播放入场动画（扫描由按钮触发）。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)

    def on_leave(self) -> None:
        """切出 BLE 页：停止入场动画 + 断开 stub（避免后台回调）。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)
        if self._transport is not None:
            try:
                self._transport.close()
            except Exception:
                pass

    # ── 交互 ────────────────────────────────────────────────────────
    def _scan(self) -> None:
        btn = self._scan_btn
        btn._orig_text = btn.text()
        btn.setEnabled(False)
        btn.setText(self._widget.tr("…"))
        from PyQt6.QtWidgets import QApplication
        QApplication.processEvents()
        self._transport = BleTransportStub()
        device = self._transport.device
        self._device_combo.clear()
        self._device_combo.addItem(f"{device.name} ({device.address})")
        self._log.appendPlainText(self._widget.tr("扫描到 {n} 个设备").format(n=1))
        btn.setEnabled(True)
        btn.setText(btn._orig_text)

    def _connect(self, checked: bool) -> None:
        from embeddebug.serial_station.ui.panels._notify import panel_notify

        if self._transport is None:
            self._log.appendPlainText(self._widget.tr("请先扫描设备。"))
            self._connect_btn.setChecked(False)
            # Batch 14: 未扫描 → warning toast。
            panel_notify(self._widget, "warning", self._widget.tr("未扫描设备"),
                         self._widget.tr("请先点击「扫描」搜索设备"))
            return
        if checked:
            ok = self._transport.connect(_STUB_ADDRESS)
            if ok:
                self._connect_btn.setText(self._widget.tr("断开"))
                self._refresh_tree()
                # Batch 9-2: 连接成功填充树后隐藏空状态。
                self._empty_state.hide()
                # Batch 10-2: 已连接 GREEN（呼吸指示）。
                self._status_dot.set_state(DotState.GREEN)
                self._log.appendPlainText(self._widget.tr("已连接 {addr}").format(addr=_STUB_ADDRESS))
                # Batch 14: 连接成功 → success toast。
                panel_notify(self._widget, "success", self._widget.tr("BLE 已连接"),
                             self._widget.tr("已连接 {addr}").format(addr=_STUB_ADDRESS))
            else:
                self._connect_btn.setChecked(False)
                self._log.appendPlainText(self._widget.tr("连接失败"))
                # Batch 14: 连接失败 → error toast。
                panel_notify(self._widget, "error", self._widget.tr("BLE 连接失败"),
                             self._widget.tr("无法连接到设备"))
        else:
            self._transport.close()
            self._connect_btn.setText(self._widget.tr("连接"))
            self._tree.clear()
            # Batch 9-2: 断开后恢复空状态占位；Batch 11: 淡入显示（激活 FadeTransition）。
            self._empty_state.show_with_fade()
            # Batch 10-2: 断开后圆点恢复 OFF（静止）。
            self._status_dot.set_state(DotState.OFF)
            self._log.appendPlainText(self._widget.tr("已断开"))
            # Batch 14: 断开 → info toast。
            panel_notify(self._widget, "info", self._widget.tr("BLE 已断开"), "")

    def _refresh_tree(self) -> None:
        if self._transport is None:
            return
        self._tree.clear()
        tree = self._transport.tree
        for service in tree.device.services:
            svc_item = QTreeWidgetItem([service.uuid, f"{len(service.characteristics)} 特征", "", ""])
            for char in service.characteristics:
                props = "".join(p[0].upper() for p in sorted(char.properties))
                svc_item.addChild(QTreeWidgetItem([
                    char.uuid, props, char.value.hex(), str(char.handle),
                ]))
            self._tree.addTopLevelItem(svc_item)
        self._tree.expandAll()

    def _on_tree_select(self) -> None:
        item = self._tree.currentItem()
        if item is None or item.parent() is None:
            return
        self._char_edit.setText(item.text(0))

    def _read_char(self) -> None:
        uuid = self._char_edit.text()
        if not uuid or self._transport is None:
            return
        char = self._transport.tree.find_by_uuid(uuid)
        if char is None:
            self._log.appendPlainText(self._widget.tr("未找到特征 {u}").format(u=uuid))
            return
        self._log.appendPlainText(self._widget.tr("读 {u}: {v}").format(u=uuid, v=char.value.hex()))

    def _write_char(self) -> None:
        uuid = self._char_edit.text()
        if not uuid or self._transport is None:
            return
        try:
            payload = bytes.fromhex(self._payload_edit.text())
        except ValueError:
            self._log.appendPlainText(self._widget.tr("payload 非法 hex"))
            return
        char = self._transport.tree.find_by_uuid(uuid)
        if char is None:
            self._log.appendPlainText(self._widget.tr("未找到特征 {u}").format(u=uuid))
            return
        # 通过编码 WRITE 帧写 transport，stub 内部 _handle_request 处理。
        from embeddebug.serial_station.ble.codec import FRAME_WRITE
        encoded = BleFrameCodec.encode_frame(FRAME_WRITE, char.handle, payload)
        written = self._transport.write(encoded)
        self._log.appendPlainText(
            self._widget.tr("写 {u}: {v} ({n} 字节)").format(u=uuid, v=payload.hex(), n=written)
        )

    def _toggle_notify(self, checked: bool) -> None:
        uuid = self._char_edit.text()
        if not uuid or self._transport is None:
            return
        if checked:
            self._transport.on_bytes_received(self._on_bytes)
            ok = self._transport.subscribe(uuid)
            self._log.appendPlainText(self._widget.tr("订阅 {u}: {r}").format(u=uuid, r="✓" if ok else "✗"))
        else:
            self._log.appendPlainText(self._widget.tr("取消订阅 {u}").format(u=uuid))

    def _on_bytes(self, data: bytes) -> None:
        for event in self._codec.feed(data):
            if event.is_notify:
                self._bridge.notify_received.emit(event)

    def _on_notify(self, event: object) -> None:
        handle = getattr(event, "handle", "?")
        value = getattr(event, "value", b"")
        self._log.appendPlainText(self._widget.tr("notify handle={h}: {v}").format(h=handle, v=value.hex()))

    def _on_error(self, msg: str) -> None:
        self._log.appendPlainText(f"[err] {msg}")


class _BleSignalBridge(QWidget):
    """跨线程信号桥（stub 回调 → 主线程 UI）。"""

    notify_received = pyqtSignal(object)
    error = pyqtSignal(str)

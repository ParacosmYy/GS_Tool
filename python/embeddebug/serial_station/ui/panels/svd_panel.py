"""CMSIS SVD 寄存器查看器模式面板 — 加载 .svd → 外设/寄存器/位域树 + 详情。

ModePanel 实现：「加载 SVD 文件」用 QFileDialog 选 .svd，「加载演示」注入内置
样例设备。左侧 QTreeWidget 三级树（外设/寄存器/位域），右侧详情面板展示选中
寄存器的地址/复位/位域表。纯数据浏览，不读写真实 MCU（需 transport/硬件，
后续迭代接入）。

约束：只调 svd/ 引擎公共 API（SvdParser/SvdDevice/SvdRegister 等），不 import
transport；不 import rtt/can/ble。树/详情构建逻辑在 svd_panel_tree.py。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QFileDialog,
    QFormLayout,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QTreeWidget,
    QTreeWidgetItem,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.svd import SvdDevice, SvdRegister
from embeddebug.serial_station.svd.demo import demo_device
from embeddebug.serial_station.svd.parser import SvdParser
from embeddebug.serial_station.ui.controls import DotState, StatusDot
from embeddebug.serial_station.ui.panels.svd_panel_tree import (
    field_table_rows,
    populate_tree,
    register_detail_rows,
)

_FIELD_COLUMNS = ("位域", "位范围", "访问", "复位值")


class SvdPanel:
    """SVD 寄存器查看器 ModePanel。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._device: SvdDevice | None = None
        self._status_dot: StatusDot | None = None
        self._device_label: QLabel | None = None
        self._tree: QTreeWidget | None = None
        self._detail_form: QFormLayout | None = None
        self._field_table: QTableWidget | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationSvdPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        layout.addLayout(self._build_toolbar(widget))
        body = QHBoxLayout()
        body.addWidget(self._build_tree(widget), 1)
        body.addLayout(self._build_detail(widget), 1)
        layout.addLayout(body, 1)

        self._widget = widget
        self._apply_focus_rings(widget)
        return widget

    # ── 构建 ─────────────────────────────────────────────────────────
    def _build_toolbar(self, widget: QWidget) -> QHBoxLayout:
        top = QHBoxLayout()
        load_btn = QPushButton(widget.tr("加载 SVD 文件"), widget)
        load_btn.setObjectName("serialStationSvdLoadButton")
        load_btn.clicked.connect(self._load_file)
        demo_btn = QPushButton(widget.tr("加载演示"), widget)
        demo_btn.setObjectName("serialStationSvdDemoButton")
        demo_btn.clicked.connect(self._load_demo)
        self._status_dot = StatusDot(parent=widget)
        self._device_label = QLabel(widget.tr("未加载设备"), widget)
        self._device_label.setObjectName("serialStationSvdDeviceLabel")
        top.addWidget(load_btn)
        top.addWidget(demo_btn)
        top.addWidget(self._status_dot)
        top.addWidget(self._device_label, 1)
        return top

    def _build_tree(self, widget: QWidget) -> QTreeWidget:
        self._tree = QTreeWidget(widget)
        self._tree.setObjectName("serialStationSvdTree")
        self._tree.setHeaderLabels((widget.tr("名称"), widget.tr("地址/位"), widget.tr("描述")))
        self._tree.itemSelectionChanged.connect(self._on_select)
        return self._tree

    def _build_detail(self, widget: QWidget) -> QVBoxLayout:
        right = QVBoxLayout()
        title = QLabel(widget.tr("寄存器详情"), widget)
        title.setObjectName("serialStationSvdDetailTitle")
        right.addWidget(title)
        self._detail_form = QFormLayout()
        self._detail_form.setLabelAlignment(Qt.AlignmentFlag.AlignRight)
        right.addLayout(self._detail_form)
        field_title = QLabel(widget.tr("位域"), widget)
        field_title.setObjectName("serialStationSvdFieldLabel")
        right.addWidget(field_title)
        self._field_table = QTableWidget(0, len(_FIELD_COLUMNS), widget)
        self._field_table.setObjectName("serialStationSvdFieldTable")
        self._field_table.setHorizontalHeaderLabels(_FIELD_COLUMNS)
        self._field_table.verticalHeader().setVisible(False)
        self._field_table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        right.addWidget(self._field_table, 1)
        return right

    # ── 交互 ─────────────────────────────────────────────────────────
    def _load_demo(self) -> None:
        try:
            self._set_device(demo_device())
        except ValueError as exc:
            self._notify_error(self._widget.tr("加载演示失败：{err}").format(err=exc))

    def _load_file(self) -> None:
        from embeddebug.serial_station.ui.panels._notify import panel_notify

        path, _ = QFileDialog.getOpenFileName(
            self._widget, self._widget.tr("选择 SVD 文件"), "", "SVD 文件 (*.svd *.xml)"
        )
        if not path:
            return
        try:
            device = SvdParser.parse_file(path)
        except (ValueError, OSError) as exc:
            panel_notify(self._widget, "error", self._widget.tr("SVD 解析失败"),
                         self._widget.tr("无法解析文件：{err}").format(err=exc))
            return
        self._set_device(device)

    def _set_device(self, device: SvdDevice) -> None:
        self._device = device
        assert self._tree is not None and self._widget is not None
        populate_tree(self._tree, device, self._widget.tr)
        self._device_label.setText(
            self._widget.tr("{name}（{n} 外设 / {r} 寄存器）").format(
                name=device.name, n=len(device.peripherals), r=device.register_count
            )
        )
        self._status_dot.set_state(DotState.GREEN)

    def _on_select(self) -> None:
        if self._tree is None or self._detail_form is None:
            return
        items = self._tree.selectedItems()
        if not items:
            return
        item = items[0]
        obj = item.data(0, 0x0100)
        self._clear_detail()
        if isinstance(obj, SvdRegister):
            periph = self._parent_peripheral(item)
            if periph is not None:
                self._show_register(obj, periph)

    def _parent_peripheral(self, item: QTreeWidgetItem):
        parent = item.parent()
        if parent is None:
            return item.data(0, 0x0100)
        return parent.data(0, 0x0100)

    def _show_register(self, reg: SvdRegister, periph) -> None:
        assert self._widget is not None
        rows = register_detail_rows(reg, periph, self._widget.tr)
        for label, value in rows:
            lbl = QLabel(label, self._widget)
            lbl.setObjectName("serialStationSvdFieldLabel")
            val = QLabel(value, self._widget)
            val.setObjectName("serialStationSvdDetailValue")
            self._detail_form.addRow(lbl, val)
        field_rows = field_table_rows(reg)
        self._field_table.setRowCount(len(field_rows))
        for r, row in enumerate(field_rows):
            for c, cell in enumerate(row):
                self._field_table.setItem(r, c, QTableWidgetItem(cell))

    def _clear_detail(self) -> None:
        while self._detail_form.rowCount() > 0:
            self._detail_form.removeRow(0)
        self._field_table.setRowCount(0)

    def _notify_error(self, text: str) -> None:
        from embeddebug.serial_station.ui.panels._notify import panel_notify

        panel_notify(self._widget, "error", self._widget.tr("SVD"), text)

    def _apply_focus_rings(self, widget: QWidget) -> None:
        from embeddebug.serial_station.ui.panels._focus_ring import apply_panel_focus_rings

        apply_panel_focus_rings(widget)

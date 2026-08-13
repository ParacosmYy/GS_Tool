"""Connection settings panel builder.

The builder owns only connection-panel composition. Runtime state gates live
in the connection controller and application actions remain on typed ports.
"""

from __future__ import annotations

from enum import StrEnum
from functools import partial

from ...domain.models import (
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
)
from ..action_surface import ActionRailButton, BusyActionButton
from ..connection_bindings import (
    ConnectionShellBindings,
    UartControlBindings,
)
from ..connection_preset_context_surface import ConnectionPresetContextSurface
from ..connection_preset_surface import (
    is_custom_connection_preset,
    refresh_connection_preset_combo,
    update_connection_preset_context,
)
from ..connection_presets import (
    DEFAULT_UART_BAUD_RATE,
    UART_BAUD_RATE_PRESETS,
    ConnectionPreset,
    ConnectionPresetCatalog,
)
from ..connection_status_surface import ConnectionStatusRail
from ..contracts import (
    ConnectionActionCallback,
    ConnectionPresetApplyCallback,
    ConnectionPresetMutationCallback,
)
from ..form_fields import build_labeled_field
from ..property_refresh import refresh_dynamic_property
from ..qt import (
    QCheckBox,
    QComboBox,
    QEvent,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLayout,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
)
from ..responsive_uart_form import ResponsiveUartForm
from ..transport_mode_surface import TransportModeSurface
from ..uart_timing_surface import UartTimingSummarySurface
from .ble_builder import build_ble_panel
from .composition import enum_combo, timeout_combo
from .connection_primitives import (
    configure_bounded_combo,
    field_label,
    section_label,
)
from .network_builder import build_network_panel


class _ConnectionBandMode(StrEnum):
    """Responsive geometry states for the existing connection controls."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW_COMPACT = "narrow_compact"


class _ResponsiveConnectionBand(QFrame):
    """Own the transport/preset shell's responsive geometry without state."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._responsive_items: tuple[QWidget, ...] = ()
        self._layout_mode: _ConnectionBandMode | None = None
        self._reflowing = False

    def register_responsive_items(
        self,
        *,
        transport_caption: QWidget,
        transport_combo: QWidget,
        preset_label: QWidget,
        preset_combo: QWidget,
        connect_button: QWidget,
        save_button: QWidget,
        delete_button: QWidget,
        preset_context: QWidget,
        status_rail: QWidget,
    ) -> None:
        """Register existing controls while preserving their object identity."""

        self._responsive_items = (
            transport_caption,
            transport_combo,
            preset_label,
            preset_combo,
            connect_button,
            save_button,
            delete_button,
            preset_context,
            status_rail,
        )
        self._reflow_layout(force=True)

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)
        self._reflow_layout()

    def changeEvent(self, event: object) -> None:
        super().changeEvent(event)
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self.invalidate_sizing()

    def event(self, event: object) -> bool:
        accepted = super().event(event)
        if event.type() is QEvent.Type.LayoutRequest:
            self.invalidate_sizing()
        return accepted

    def invalidate_sizing(self) -> None:
        """Re-evaluate the existing layout after a sizing-contract change."""

        if self._reflowing or not self._responsive_items:
            return
        layout = self.layout()
        if not isinstance(layout, QGridLayout):
            return
        layout.invalidate()
        layout.activate()
        self.updateGeometry()
        self._reflow_layout()

    def _reflow_layout(self, *, force: bool = False) -> None:
        if self._reflowing or not self._responsive_items:
            return
        layout = self.layout()
        if not isinstance(layout, QGridLayout):
            return
        mode = self._layout_mode_for_width(layout)
        if not force and mode == self._layout_mode:
            return

        self._reflowing = True
        try:
            for widget in self._responsive_items:
                layout.removeWidget(widget)
            for column in range(5):
                layout.setColumnStretch(column, 0)
            if mode is _ConnectionBandMode.REGULAR:
                self._place_regular_layout(layout)
            elif mode is _ConnectionBandMode.COMPACT:
                self._place_compact_layout(layout)
            else:
                self._place_narrow_compact_layout(layout)
            self._layout_mode = mode
            layout.invalidate()
            layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def minimumSizeHint(self) -> object:
        """Expose only the narrow composition as the band's width floor."""

        hint = super().minimumSizeHint()
        layout = self.layout()
        if isinstance(layout, QGridLayout) and self._responsive_items:
            hint.setWidth(
                self._required_width_for_mode(
                    layout,
                    _ConnectionBandMode.NARROW_COMPACT,
                )
            )
        return hint

    def _layout_mode_for_width(self, layout: QGridLayout) -> _ConnectionBandMode:
        """Choose the widest (least compact) existing-control composition that fits."""

        available_width = self.contentsRect().width()
        if available_width >= self._required_width_for_mode(
            layout,
            _ConnectionBandMode.REGULAR,
        ):
            return _ConnectionBandMode.REGULAR
        if available_width >= self._required_width_for_mode(
            layout,
            _ConnectionBandMode.COMPACT,
        ):
            return _ConnectionBandMode.COMPACT
        return _ConnectionBandMode.NARROW_COMPACT

    def _required_width_for_mode(
        self,
        layout: QGridLayout,
        mode: _ConnectionBandMode,
    ) -> int:
        """Derive a mode floor from the controls placed in its logical rows."""

        (
            transport_caption,
            transport_combo,
            preset_label,
            preset_combo,
            connect_button,
            save_button,
            delete_button,
            preset_context,
            status_rail,
        ) = self._responsive_items
        rows = {
            _ConnectionBandMode.REGULAR: (
                (
                    transport_caption,
                    transport_combo,
                    preset_label,
                    preset_combo,
                    connect_button,
                ),
                (preset_context, save_button, delete_button),
                (status_rail,),
            ),
            _ConnectionBandMode.COMPACT: (
                (transport_caption, transport_combo, connect_button),
                (preset_label, preset_combo, save_button, delete_button),
                (preset_context,),
                (status_rail,),
            ),
            _ConnectionBandMode.NARROW_COMPACT: (
                (transport_caption, transport_combo, connect_button),
                (preset_label, preset_combo),
                (save_button, delete_button),
                (preset_context,),
                (status_rail,),
            ),
        }[mode]
        spacing = max(0, layout.horizontalSpacing())
        margins = layout.contentsMargins()
        widest_row = max(
            sum(self._minimum_control_width(widget) for widget in row)
            + spacing * max(0, len(row) - 1)
            for row in rows
        )
        return widest_row + margins.left() + margins.right()

    @staticmethod
    def _minimum_control_width(widget: QWidget) -> int:
        """Read the control's own sizing contract instead of a pixel breakpoint."""

        return max(
            widget.minimumSizeHint().width(),
            widget.sizeHint().width(),
            widget.minimumWidth(),
        )

    def _place_regular_layout(self, layout: QGridLayout) -> None:
        (
            transport_caption,
            transport_combo,
            preset_label,
            preset_combo,
            connect_button,
            save_button,
            delete_button,
            preset_context,
            status_rail,
        ) = self._responsive_items
        layout.addWidget(transport_caption, 0, 0)
        layout.addWidget(transport_combo, 0, 1)
        layout.addWidget(preset_label, 0, 2)
        layout.addWidget(preset_combo, 0, 3)
        layout.addWidget(connect_button, 0, 4)
        layout.addWidget(preset_context, 1, 0, 1, 3)
        layout.addWidget(save_button, 1, 3)
        layout.addWidget(delete_button, 1, 4)
        layout.addWidget(status_rail, 2, 0, 1, 5)
        layout.setColumnStretch(1, 1)
        layout.setColumnStretch(3, 1)

    def _place_compact_layout(self, layout: QGridLayout) -> None:
        (
            transport_caption,
            transport_combo,
            preset_label,
            preset_combo,
            connect_button,
            save_button,
            delete_button,
            preset_context,
            status_rail,
        ) = self._responsive_items
        layout.addWidget(transport_caption, 0, 0)
        layout.addWidget(transport_combo, 0, 1)
        layout.addWidget(connect_button, 0, 2)
        layout.addWidget(preset_label, 1, 0)
        layout.addWidget(preset_combo, 1, 1)
        layout.addWidget(save_button, 1, 2)
        layout.addWidget(delete_button, 1, 3)
        layout.addWidget(preset_context, 2, 0, 1, 4)
        layout.addWidget(status_rail, 3, 0, 1, 4)
        layout.setColumnStretch(1, 1)

    def _place_narrow_compact_layout(self, layout: QGridLayout) -> None:
        """Give selector and persistence actions separate narrow-width rows."""

        (
            transport_caption,
            transport_combo,
            preset_label,
            preset_combo,
            connect_button,
            save_button,
            delete_button,
            preset_context,
            status_rail,
        ) = self._responsive_items
        layout.addWidget(transport_caption, 0, 0)
        layout.addWidget(transport_combo, 0, 1)
        layout.addWidget(connect_button, 0, 2)
        layout.addWidget(preset_label, 1, 0)
        layout.addWidget(preset_combo, 1, 1, 1, 2)
        layout.addWidget(save_button, 2, 0)
        layout.addWidget(delete_button, 2, 1)
        layout.addWidget(preset_context, 3, 0, 1, 3)
        layout.addWidget(status_rail, 4, 0, 1, 3)
        layout.setColumnStretch(1, 1)


def _dispatch_connection_preset(
    window,
    apply_connection_preset: ConnectionPresetApplyCallback,
    index: int,
) -> None:
    preset = window._connection_preset.itemData(index, Qt.ItemDataRole.UserRole)
    refresh_dynamic_property(
        window._connection_preset,
        "customSelected",
        is_custom_connection_preset(preset),
    )
    update_connection_preset_context(
        window._connection_preset,
        preset if isinstance(preset, ConnectionPreset) else None,
        context_surface=getattr(window, "_connection_preset_context", None),
    )
    if isinstance(preset, ConnectionPreset):
        apply_connection_preset(preset)
    window._motion_controller.request_activity(420)


def _refresh_uart_timing_summary(window, *_args: object) -> None:
    """Project the existing UART selectors into one read-only visual summary."""

    try:
        window._uart_timing_summary.set_timing(
            baud_rate=int(window._baud_combo.currentData(Qt.ItemDataRole.UserRole)),
            data_bits=int(window._data_bits.currentData(Qt.ItemDataRole.UserRole)),
            parity=UartParity(window._parity.currentData(Qt.ItemDataRole.UserRole)),
            stop_bits=UartStopBits(window._stop_bits.currentData(Qt.ItemDataRole.UserRole)),
            flow_control=UartFlowControl(
                window._flow_control.currentData(Qt.ItemDataRole.UserRole)
            ),
        )
    except (TypeError, ValueError, KeyError):
        window._uart_timing_summary.set_invalid()


def build_connection_panel(
    window,
    *,
    catalog: ConnectionPresetCatalog,
    apply_connection_preset: ConnectionPresetApplyCallback,
    save_connection_preset: ConnectionPresetMutationCallback,
    delete_connection_preset: ConnectionPresetMutationCallback,
    toggle_connection: ConnectionActionCallback,
) -> QVBoxLayout:
    root = QVBoxLayout()
    root.setContentsMargins(0, 0, 0, 0)
    root.setSpacing(6)

    connection_band = _ResponsiveConnectionBand(window)
    connection_band.setObjectName("connectionControlBand")
    connection_band.setFrameShape(QFrame.Shape.NoFrame)
    connection_band.setProperty("state", "closed")
    connection_band.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Fixed,
    )
    mode_row = QGridLayout(connection_band)
    mode_row.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
    mode_row.setContentsMargins(10, 7, 10, 7)
    mode_row.setSpacing(8)
    transport_caption = QWidget(connection_band)
    transport_caption_layout = QHBoxLayout(transport_caption)
    transport_caption_layout.setContentsMargins(0, 0, 0, 0)
    transport_caption_layout.setSpacing(5)
    transport_label = field_label("传输")
    transport_label.setParent(transport_caption)
    transport_caption_layout.addWidget(transport_label)
    window._transport_mode_surface = TransportModeSurface(transport_caption)
    window._transport_mode_surface.setObjectName("transportModeSurface")
    window._transport_mode_surface.setToolTip(
        "传输模式视觉提示；实际选择请使用旁边的传输下拉框。"
    )
    transport_caption_layout.addWidget(window._transport_mode_surface)
    mode_row.addWidget(transport_caption, 0, 0)
    window._transport_combo = QComboBox()
    window._transport_combo.setObjectName("transportCombo")
    window._transport_combo.setAccessibleName("传输方式")
    window._transport_combo.setEditable(False)
    window._transport_combo.setToolTip(
        "选择 UART、TCP、UDP、BLE 或外部 RTT 传输方式；不会自动连接。"
    )
    window._transport_combo.setAccessibleDescription(
        "选择要使用的传输方式；当前选择只切换对应参数面板，不会自动建立连接。"
    )
    window._transport_combo.addItem("UART", TransportKind.UART)
    window._transport_combo.addItem("TCP Client", TransportKind.TCP_STREAM)
    window._transport_combo.addItem("TCP Server", TransportKind.TCP_SERVER)
    window._transport_combo.addItem("UDP 单播", TransportKind.UDP_DATAGRAM)
    window._transport_combo.addItem("BLE GATT", TransportKind.BLE_GATT)
    window._transport_combo.addItem("J-Link RTT（外部桥接）", TransportKind.RTT)
    window._transport_combo.currentIndexChanged.connect(window._on_transport_changed)
    mode_row.addWidget(window._transport_combo, 0, 1)

    preset_label = field_label("快速配置")
    preset_label.setObjectName("connectionPresetLabel")
    mode_row.addWidget(preset_label, 0, 2)
    window._connection_preset = QComboBox()
    window._connection_preset.setObjectName("connectionPresetCombo")
    window._connection_preset.setAccessibleName("连接快速配置")
    window._connection_preset.setPlaceholderText("选择快速配置")
    configure_bounded_combo(window._connection_preset, minimum_width=180, maximum_width=290)
    window._connection_preset.setToolTip("只填入常用连接选项，不会自动连接或保存密钥。")
    window._connection_preset.setAccessibleDescription(
        "选择一个内置连接快速配置；只填入表单，不会自动连接。"
    )
    window._connection_preset.currentIndexChanged.connect(
        partial(_dispatch_connection_preset, window, apply_connection_preset)
    )
    mode_row.addWidget(window._connection_preset, 0, 3)
    window._connection_preset_context = ConnectionPresetContextSurface(connection_band)
    window._connection_hint = window._connection_preset_context.hint_label
    mode_row.addWidget(window._connection_preset_context, 1, 0, 1, 3)
    window._save_connection_preset_button = ActionRailButton("保存自定义")
    window._save_connection_preset_button.setAccessibleName("保存自定义连接配置")
    window._save_connection_preset_button.setToolTip(
        "从当前连接页表单保存标准化选项；不会保存密钥或自动连接。"
    )
    window._save_connection_preset_button.setAccessibleDescription(
        "从当前连接页表单保存标准化连接选项；不会保存密钥或自动建立连接。"
    )
    window._save_connection_preset_button.clicked.connect(save_connection_preset)
    mode_row.addWidget(window._save_connection_preset_button, 1, 3)
    window._delete_connection_preset_button = ActionRailButton("删除自定义")
    window._delete_connection_preset_button.setObjectName("dangerButton")
    window._delete_connection_preset_button.setAccessibleName("删除当前自定义连接配置")
    window._delete_connection_preset_button.setToolTip("只有选中自定义配置时才可删除。")
    window._delete_connection_preset_button.setAccessibleDescription(
        "删除当前选中的自定义连接配置；只有选中自定义配置时才可用。"
    )
    window._delete_connection_preset_button.setEnabled(False)
    window._delete_connection_preset_button.clicked.connect(delete_connection_preset)
    mode_row.addWidget(window._delete_connection_preset_button, 1, 4)
    window._connection_status_rail = ConnectionStatusRail(connection_band)
    window._connection_status_rail.setObjectName("connectionStatusRail")
    mode_row.addWidget(window._connection_status_rail, 2, 0, 1, 5)
    window._connect_button = BusyActionButton("连接")
    window._connect_button.setAccessibleName("连接或断开当前传输")
    window._connect_button.setToolTip("连接或断开当前传输；不会自动发送数据。")
    window._connect_button.setObjectName("primaryButton")
    window._connect_button.clicked.connect(toggle_connection)
    mode_row.addWidget(window._connect_button, 0, 4)
    connection_band.register_responsive_items(
        transport_caption=transport_caption,
        transport_combo=window._transport_combo,
        preset_label=preset_label,
        preset_combo=window._connection_preset,
        connect_button=window._connect_button,
        save_button=window._save_connection_preset_button,
        delete_button=window._delete_connection_preset_button,
        preset_context=window._connection_preset_context,
        status_rail=window._connection_status_rail,
    )
    window._connection_control_band = connection_band
    window._connection_shell_bindings = ConnectionShellBindings(
        control_band=connection_band,
        transport_mode_surface=window._transport_mode_surface,
        transport_combo=window._transport_combo,
        preset_combo=window._connection_preset,
        preset_context=window._connection_preset_context,
        save_preset_button=window._save_connection_preset_button,
        delete_preset_button=window._delete_connection_preset_button,
        status_rail=window._connection_status_rail,
        connect_button=window._connect_button,
    )
    refresh_connection_preset_combo(
        window._connection_preset,
        catalog,
        context_surface=window._connection_preset_context,
    )
    root.addWidget(connection_band)

    uart_panel = QWidget(window)
    uart_panel.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Fixed,
    )
    uart_layout = QVBoxLayout(uart_panel)
    uart_layout.setContentsMargins(0, 0, 0, 0)
    uart_layout.setSpacing(10)

    window._port_combo = QComboBox()
    window._port_combo.setAccessibleName("UART 端口")
    window._port_combo.setEditable(True)
    port_placeholder = "未发现端口 · 点击刷新或输入 COMx"
    window._port_combo.setPlaceholderText(port_placeholder)
    window._port_combo.lineEdit().setPlaceholderText(port_placeholder)
    configure_bounded_combo(window._port_combo, minimum_width=220, maximum_width=360)
    window._port_combo.setToolTip("选择端口，或输入 COMx 手动连接")
    window._port_combo.setAccessibleDescription(
        "选择已枚举的 UART 端口，或输入 COMx 端口标识；不会自动连接。"
    )
    window._port_combo.editTextChanged.connect(window._on_port_text_changed)

    window._refresh_button = BusyActionButton("刷新端口")
    window._refresh_button.setAccessibleName("刷新 UART 端口")
    window._refresh_button.setToolTip("刷新可用 UART 端口；不会自动连接。")
    window._refresh_button.clicked.connect(window._refresh_ports)

    window._baud_combo = QComboBox()
    window._baud_combo.setAccessibleName("UART 波特率预设")
    window._baud_combo.setEditable(False)
    window._baud_combo.setToolTip("从常用 UART 波特率中选择，无需手动输入数字。")
    window._baud_combo.setAccessibleDescription(
        "从常用 UART 波特率预设中选择；该下拉框不可手动输入数字。"
    )
    for baud_rate in UART_BAUD_RATE_PRESETS:
        window._baud_combo.addItem(str(baud_rate), baud_rate)
    configure_bounded_combo(window._baud_combo, minimum_width=110, maximum_width=150)
    default_baud_index = window._baud_combo.findData(DEFAULT_UART_BAUD_RATE)
    if default_baud_index >= 0:
        window._baud_combo.setCurrentIndex(default_baud_index)

    window._data_bits = enum_combo(
        (5, 6, 7, 8),
        labels=("5 位", "6 位", "7 位", "8 位"),
    )
    window._data_bits.setAccessibleName("UART 数据位")
    window._data_bits.setToolTip("选择 UART 帧的数据位数量。")
    window._data_bits.setAccessibleDescription("选择 UART 帧的数据位数量：5、6、7 或 8 位。")
    window._data_bits.setCurrentIndex(3)
    configure_bounded_combo(window._data_bits, minimum_width=84, maximum_width=110)

    window._parity = enum_combo(
        tuple(UartParity),
        labels=("无校验", "奇校验", "偶校验", "Mark 校验", "Space 校验"),
    )
    window._parity.setAccessibleName("UART 校验")
    window._parity.setToolTip("选择 UART 帧的校验方式。")
    window._parity.setAccessibleDescription(
        "选择 UART 帧校验方式：无校验、奇校验、偶校验、Mark 校验或 Space 校验。"
    )
    configure_bounded_combo(window._parity, minimum_width=84, maximum_width=150)

    window._stop_bits = enum_combo(
        tuple(UartStopBits),
        labels=("1 位", "1.5 位", "2 位"),
    )
    window._stop_bits.setAccessibleName("UART 停止位")
    window._stop_bits.setToolTip("选择 UART 帧的停止位数量。")
    window._stop_bits.setAccessibleDescription("选择 UART 帧停止位数量：1、1.5 或 2 位。")
    configure_bounded_combo(window._stop_bits, minimum_width=84, maximum_width=130)

    window._flow_control = enum_combo(
        tuple(UartFlowControl),
        labels=("无流控", "软件流控 XON/XOFF", "硬件流控 RTS/CTS", "硬件流控 DSR/DTR"),
    )
    window._flow_control.setAccessibleName("UART 流控")
    window._flow_control.setToolTip("选择 UART 的软件或硬件流控方式。")
    window._flow_control.setAccessibleDescription(
        "选择 UART 流控方式：无流控、软件流控 XON/XOFF、硬件流控 RTS/CTS 或 DSR/DTR。"
    )
    configure_bounded_combo(window._flow_control, minimum_width=110, maximum_width=160)
    window._uart_timing_summary = UartTimingSummarySurface(uart_panel)
    for combo in (
        window._baud_combo,
        window._data_bits,
        window._parity,
        window._stop_bits,
        window._flow_control,
    ):
        combo.currentIndexChanged.connect(partial(_refresh_uart_timing_summary, window))
    _refresh_uart_timing_summary(window)
    window._baud_combo.currentIndexChanged.connect(window._on_uart_timing_changed)
    window._data_bits.currentIndexChanged.connect(window._on_uart_timing_changed)
    window._parity.currentIndexChanged.connect(window._on_uart_timing_changed)
    window._stop_bits.currentIndexChanged.connect(window._on_uart_timing_changed)

    window._read_timeout = timeout_combo(0.2, 2.0, minimum=0.001)
    window._read_timeout.setAccessibleName("UART 读超时（秒）")
    window._read_timeout.setToolTip("选择 UART 读取等待的最长时间，单位为秒。")
    window._read_timeout.setAccessibleDescription(
        "选择 UART 读取等待的最长时间，单位为秒；连接后由 UART 配置使用。"
    )

    window._write_timeout = timeout_combo(1.0, 3.0, minimum=0.001)
    window._write_timeout.setAccessibleName("UART 写超时（秒）")
    window._write_timeout.setToolTip("选择 UART 写入等待的最长时间，单位为秒。")
    window._write_timeout.setAccessibleDescription(
        "选择 UART 写入等待的最长时间，单位为秒；连接后由 UART 配置使用。"
    )

    window._inter_byte_timeout = timeout_combo(0.0, 2.0)
    window._inter_byte_timeout.setAccessibleName("UART 字节间超时（秒）")
    window._inter_byte_timeout.setToolTip(
        "选择 UART 字节间等待超时，单位为秒；0 表示不设置字节间超时，连接后由 UART 配置使用。"
    )
    window._inter_byte_timeout.setAccessibleDescription(
        "选择 UART 字节间等待超时，单位为秒；0 表示不设置字节间超时，连接后由 UART 配置使用。"
    )

    window._exclusive = QCheckBox("独占")
    window._exclusive.setAccessibleName("UART 独占模式")
    window._exclusive.setToolTip("连接 UART 时请求独占端口。")
    window._exclusive.setAccessibleDescription("连接 UART 时请求独占端口。")
    window._dtr = QCheckBox("DTR")
    window._dtr.setAccessibleName("UART DTR")
    window._dtr.setToolTip("连接 UART 时设置 DTR 信号。")
    window._dtr.setAccessibleDescription("连接 UART 时设置 DTR 信号。")
    window._dtr.setChecked(True)
    window._rts = QCheckBox("RTS")
    window._rts.setAccessibleName("UART RTS")
    window._rts.setToolTip("连接 UART 时设置 RTS 信号。")
    window._rts.setAccessibleDescription("连接 UART 时设置 RTS 信号。")
    window._rts.setChecked(True)

    port_controls = QWidget()
    port_controls_layout = QHBoxLayout(port_controls)
    port_controls_layout.setContentsMargins(0, 0, 0, 0)
    port_controls_layout.setSpacing(8)
    port_controls_layout.addWidget(window._port_combo, 1)
    port_controls_layout.addWidget(window._refresh_button)

    line_controls = QWidget()
    line_controls_layout = QHBoxLayout(line_controls)
    line_controls_layout.setContentsMargins(0, 0, 0, 0)
    line_controls_layout.setSpacing(12)
    line_controls_layout.addWidget(window._exclusive)
    line_controls_layout.addWidget(window._dtr)
    line_controls_layout.addWidget(window._rts)
    line_controls_layout.addStretch()

    window._uart_form = ResponsiveUartForm(
        port=build_labeled_field("端口", port_controls),
        baud=build_labeled_field("波特率", window._baud_combo),
        data_bits=build_labeled_field("数据位", window._data_bits),
        parity=build_labeled_field("校验", window._parity),
        stop_bits=build_labeled_field("停止位", window._stop_bits),
        flow_control=build_labeled_field("流控", window._flow_control),
        read_timeout=build_labeled_field("读超时", window._read_timeout),
        write_timeout=build_labeled_field("写超时", window._write_timeout),
        inter_byte_timeout=build_labeled_field(
            "字节间超时",
            window._inter_byte_timeout,
        ),
        line_controls=build_labeled_field("线路控制", line_controls),
        parent=uart_panel,
    )
    uart_layout.addWidget(window._uart_form)
    uart_layout.addWidget(window._uart_timing_summary)
    uart_panel.setProperty("role", "surface")
    window._uart_panel = uart_panel
    window._uart_title = section_label("UART 参数")
    root.addWidget(window._uart_title)
    root.addWidget(uart_panel)
    window._uart_bindings = UartControlBindings(
        panel=uart_panel,
        title=window._uart_title,
        port_combo=window._port_combo,
        refresh_button=window._refresh_button,
        baud_combo=window._baud_combo,
        data_bits=window._data_bits,
        parity=window._parity,
        stop_bits=window._stop_bits,
        flow_control=window._flow_control,
        timing_summary=window._uart_timing_summary,
        read_timeout=window._read_timeout,
        write_timeout=window._write_timeout,
        inter_byte_timeout=window._inter_byte_timeout,
        exclusive=window._exclusive,
        dtr=window._dtr,
        rts=window._rts,
    )

    window._network_bindings = build_network_panel(window)
    root.addWidget(window._network_bindings.title)
    root.addWidget(window._network_bindings.panel)

    window._ble_bindings = build_ble_panel(window)
    root.addWidget(window._ble_bindings.title)
    root.addWidget(window._ble_bindings.panel)

    for widget in (
        window._network_host,
        window._network_local_host,
        window._server_allowlist,
    ):
        widget.textChanged.connect(window._on_connection_endpoint_changed)
    for widget in (
        window._network_port,
        window._network_local_port,
    ):
        widget.valueChanged.connect(window._on_connection_endpoint_changed)
    window._server_lan_confirm.toggled.connect(window._on_connection_endpoint_changed)

    return root

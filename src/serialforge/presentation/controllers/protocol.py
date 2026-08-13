"""Protocol/telemetry panel construction and its narrow shell callback contract."""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum

from ...domain.protocol_presets import builtin_protocol_presets
from ...domain.protocols import ChecksumKind, FramingKind
from ..action_surface import ActionRailButton, BusyActionButton
from ..analysis_status_surface import AnalysisStatusLabel
from ..bounded_value_combo import MAX_FRAME_OPTION_VALUES, BoundedIntCombo
from ..component_empty_state import ComponentEmptyStateSurface
from ..contracts import ProtocolPanelCallbacks, StatusSurfaceRegistrar
from ..dataset_curve import DatasetCurveWidget
from ..form_fields import build_field_row, build_labeled_field
from ..observation_viewport import ObservationViewport
from ..pipeline_surface import PipelineSurfaceLabel
from ..protocol_action_hints import PROTOCOL_DERIVED_ACTION_HINTS
from ..protocol_config_context_surface import ProtocolConfigContextSurface
from ..qt import (
    QComboBox,
    QEvent,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLayout,
    QLineEdit,
    QPlainTextEdit,
    QPushButton,
    QSizePolicy,
    Qt,
    QTableWidget,
    QVBoxLayout,
    QWidget,
)
from ..replay_activity_surface import ReplayActivityLabel

PROFILE_LABEL_MIN_WIDTH = 140
DATASET_LABEL_MIN_WIDTH = 160
ANALYSIS_STATUS_MIN_WIDTH = 200

_PROTOCOL_PRESET_LABELS = {
    "raw": "原始字节",
    "line": "文本行（LF/CRLF）",
    "delimiter_aa55": "分隔符 AA 55",
    "length_u8_le": "长度 U8 小端",
    "length_u16_le_crc16": "长度 U16 小端 + CRC16",
    "nmea0183_line": "NMEA 0183 · 行 + XOR",
    "mavlink_stream": "MAVLink v1/v2 · 流",
    "modbus_rtu_timed": "Modbus RTU · 定时",
}


class _ProtocolRowMode(StrEnum):
    """Responsive compositions for one protocol-page action row."""

    REGULAR = "regular"
    COMPACT = "compact"
    NARROW = "narrow"


class _ResponsiveProtocolRow(QWidget):
    """Reflow existing protocol controls without owning their actions."""

    def __init__(
        self,
        widgets: tuple[QWidget, ...],
        *,
        parent: QWidget | None = None,
    ) -> None:
        if not widgets:
            raise ValueError("protocol row requires at least one widget")
        super().__init__(parent)
        self.setObjectName("responsiveProtocolRow")
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._widgets = tuple(widgets)
        self._mode: _ProtocolRowMode | None = None
        self._reflowing = False
        self._layout = QGridLayout(self)
        self._layout.setSizeConstraint(QLayout.SizeConstraint.SetNoConstraint)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setHorizontalSpacing(8)
        self._layout.setVerticalSpacing(8)
        self._relayout(force=True)

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._relayout()

    def changeEvent(self, event: object) -> None:
        super().changeEvent(event)  # type: ignore[arg-type]
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self._invalidate_sizing()

    def event(self, event: object) -> bool:
        accepted = super().event(event)  # type: ignore[arg-type]
        if event.type() is QEvent.Type.LayoutRequest:
            self._invalidate_sizing()
        return accepted

    def minimumSizeHint(self) -> object:
        """Expose only the single-column hard contract as the width floor."""

        hint = super().minimumSizeHint()
        hint.setWidth(self._required_width(_ProtocolRowMode.NARROW))
        return hint

    def sizeHint(self) -> object:
        """Prefer one readable row without making it a hard page floor."""

        hint = super().sizeHint()
        hint.setWidth(
            max(
                hint.width(),
                self._required_width(_ProtocolRowMode.REGULAR, preferred=True),
            )
        )
        return hint

    def _invalidate_sizing(self) -> None:
        if self._reflowing:
            return
        self._layout.invalidate()
        self._layout.activate()
        self.updateGeometry()
        self._relayout()

    def _relayout(self, *, force: bool = False) -> None:
        if self._reflowing:
            return
        mode = self._mode_for_width()
        if not force and mode is self._mode:
            self._apply_column_contract(mode)
            self._layout.invalidate()
            self._layout.activate()
            return

        self._reflowing = True
        try:
            for widget in self._widgets:
                self._layout.removeWidget(widget)
            for row_index, row in enumerate(self._rows_for_mode(mode)):
                for column, widget in enumerate(row):
                    self._layout.addWidget(widget, row_index, column)
            self._apply_column_contract(mode)
            self._mode = mode
            self._layout.invalidate()
            self._layout.activate()
            self.updateGeometry()
        finally:
            self._reflowing = False

    def _mode_for_width(self) -> _ProtocolRowMode:
        available_width = max(0, self.contentsRect().width())
        if available_width >= self._required_width(
            _ProtocolRowMode.REGULAR,
            preferred=True,
        ):
            return _ProtocolRowMode.REGULAR
        if available_width >= self._required_width(_ProtocolRowMode.COMPACT):
            return _ProtocolRowMode.COMPACT
        return _ProtocolRowMode.NARROW

    def _required_width(
        self,
        mode: _ProtocolRowMode,
        *,
        preferred: bool = False,
    ) -> int:
        spacing = max(0, self._layout.horizontalSpacing())
        widest_row = max(
            sum(self._widget_width(widget, preferred=preferred) for widget in row)
            + spacing * max(0, len(row) - 1)
            for row in self._rows_for_mode(mode)
        )
        margins = self._layout.contentsMargins()
        return widest_row + margins.left() + margins.right()

    def _apply_column_contract(self, mode: _ProtocolRowMode) -> None:
        rows = self._rows_for_mode(mode)
        column_widths = [0] * max(len(row) for row in rows)
        for row in rows:
            for column, widget in enumerate(row):
                column_widths[column] = max(
                    column_widths[column],
                    self._widget_width(widget),
                )
        for column in range(len(self._widgets)):
            self._layout.setColumnMinimumWidth(column, 0)
            self._layout.setColumnStretch(column, 0)
        for column, width in enumerate(column_widths):
            self._layout.setColumnMinimumWidth(column, width)
            self._layout.setColumnStretch(column, 1)

    def _rows_for_mode(
        self,
        mode: _ProtocolRowMode,
    ) -> tuple[tuple[QWidget, ...], ...]:
        if mode is _ProtocolRowMode.REGULAR:
            return (self._widgets,)
        if mode is _ProtocolRowMode.COMPACT:
            split = max(1, (len(self._widgets) + 1) // 2)
            return (self._widgets[:split], self._widgets[split:])
        return tuple((widget,) for widget in self._widgets)

    @staticmethod
    def _widget_width(widget: QWidget, *, preferred: bool = False) -> int:
        minimum = max(widget.minimumWidth(), widget.minimumSizeHint().width())
        if not preferred:
            return minimum
        return max(minimum, widget.sizeHint().width())


def _configure_protocol_leaf_view(widget: QWidget) -> None:
    """Let data views yield width to the protocol surface while staying native."""

    widget.setMinimumWidth(0)
    policy = widget.sizePolicy()
    policy.setHorizontalPolicy(QSizePolicy.Policy.Ignored)
    widget.setSizePolicy(policy)


@dataclass(slots=True)
class ProtocolPanelWidgets:
    """Owned widget references exposed to the shell during staged migration."""

    layout: QVBoxLayout
    protocol_preset: QComboBox
    protocol_framing: QComboBox
    protocol_checksum: QComboBox
    protocol_max_frame: BoundedIntCombo
    protocol_apply_button: QPushButton
    protocol_reset_button: QPushButton
    protocol_status: QLabel
    protocol_config_context: QLabel
    pipeline_summary: QLabel
    protocol_delimiter: QLineEdit
    protocol_length_bytes: QComboBox
    protocol_byteorder: QComboBox
    protocol_scope: QLabel
    protocol_timing_hint: QLabel
    protocol_detail_layout: QVBoxLayout
    component_profile_label: QLabel
    load_profile_button: QPushButton
    component_filter: QComboBox
    export_component_button: QPushButton
    component_status: QLabel
    component_table: QTableWidget
    component_empty: ComponentEmptyStateSurface
    component_preview: QPlainTextEdit
    dataset_config_label: QLabel
    load_dataset_button: QPushButton
    export_dataset_button: QPushButton
    dataset_status: QLabel
    dataset_preview: QPlainTextEdit
    dataset_curve_series: QComboBox
    dataset_curve_status: QLabel
    dataset_curve: DatasetCurveWidget
    replay_speed: QComboBox
    replay_start_button: QPushButton
    replay_pause_button: QPushButton
    replay_stop_button: QPushButton
    replay_status: QLabel


def _configure_responsive_combo(
    combo: QComboBox,
    *,
    minimum_width: int,
    maximum_width: int,
) -> None:
    """Keep long option labels from propagating a hard page minimum width."""

    combo.setEditable(False)
    combo.setSizeAdjustPolicy(
        QComboBox.SizeAdjustPolicy.AdjustToMinimumContentsLengthWithIcon
    )
    combo.setMinimumContentsLength(8)
    combo.setMinimumWidth(minimum_width)
    combo.setMaximumWidth(maximum_width)


def _configure_dynamic_label(
    label: QLabel,
    *,
    maximum_width: int,
    minimum_width: int = 0,
) -> None:
    """Allow runtime profile names to yield space back to sibling controls."""

    label.setWordWrap(True)
    label.setMinimumWidth(minimum_width)
    label.setMaximumWidth(maximum_width)
    label.setSizePolicy(
        QSizePolicy.Policy.Ignored,
        QSizePolicy.Policy.Preferred,
    )


def _field_label(text: str) -> QLabel:
    """Create a secondary protocol-form label with theme-aware contrast."""

    label = QLabel(text)
    label.setProperty("role", "muted")
    return label


def _section_title(text: str) -> QLabel:
    """Create a section title that does not absorb a row's spare width."""

    label = QLabel(text)
    label.setProperty("role", "section")
    label.setSizePolicy(QSizePolicy.Policy.Maximum, QSizePolicy.Policy.Fixed)
    return label


def _build_analysis_surface(
    object_name: str,
    accessible_name: str,
) -> tuple[QFrame, QVBoxLayout]:
    """Create a static visual boundary for one derived-data responsibility."""

    surface = QFrame()
    surface.setObjectName(object_name)
    surface.setProperty("role", "surface")
    surface.setFocusPolicy(Qt.FocusPolicy.NoFocus)
    surface.setAccessibleName(accessible_name)
    surface.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Preferred,
    )
    layout = QVBoxLayout(surface)
    layout.setContentsMargins(10, 8, 10, 8)
    layout.setSpacing(10)
    return surface, layout


def build_protocol_panel(
    *,
    callbacks: ProtocolPanelCallbacks,
    register_status_surface: StatusSurfaceRegistrar,
) -> ProtocolPanelWidgets:
    """Build the protocol workspace without importing or calling the ViewModel."""

    root = QVBoxLayout()
    root.setContentsMargins(0, 0, 0, 0)
    root.setSpacing(12)

    protocol_surface, protocol_surface_layout = _build_analysis_surface(
        "protocolConfigSurface",
        "协议解析配置区",
    )
    controls = QVBoxLayout()
    controls.setContentsMargins(0, 0, 0, 0)
    controls.setSpacing(10)
    protocol_title = _section_title("协议解析")
    protocol_header = QHBoxLayout()
    protocol_header.setContentsMargins(0, 0, 0, 0)
    protocol_header.addWidget(protocol_title)
    protocol_header.addStretch(1)
    controls.addLayout(protocol_header)
    protocol_preset = QComboBox()
    protocol_preset.setAccessibleName("协议预设")
    protocol_preset.setToolTip("预设只填入配置编辑区；点击“应用”后才会重置解析状态。")
    protocol_preset.setAccessibleDescription(
        "从有界协议预设中选择配置；预设只填入编辑区，点击应用后才会更新解析状态。"
    )
    protocol_preset.addItem("自定义", None)
    for preset in builtin_protocol_presets():
        protocol_preset.addItem(_PROTOCOL_PRESET_LABELS.get(preset.key, preset.label), preset)
        protocol_preset.setItemData(
            protocol_preset.count() - 1,
            preset.description,
            Qt.ItemDataRole.ToolTipRole,
        )
    _configure_responsive_combo(protocol_preset, minimum_width=140, maximum_width=520)
    protocol_preset.currentIndexChanged.connect(callbacks.on_protocol_preset_changed)
    protocol_primary_fields = [build_labeled_field("预设", protocol_preset)]

    protocol_framing = QComboBox()
    protocol_framing.setAccessibleName("协议帧格式")
    protocol_framing.addItem("原始字节流", FramingKind.RAW)
    protocol_framing.addItem("文本行（LF/CRLF）", FramingKind.LINE)
    protocol_framing.addItem("自定义分隔符", FramingKind.DELIMITER)
    protocol_framing.addItem("长度前缀", FramingKind.LENGTH_PREFIXED)
    protocol_framing.addItem("MAVLink v1/v2 · 流", FramingKind.MAVLINK_STREAM)
    protocol_framing.addItem("Modbus RTU · 定时", FramingKind.MODBUS_RTU_TIMED)
    protocol_framing.setToolTip("选择字节流如何切分为协议帧；下拉选项不可手动输入。")
    protocol_framing.setAccessibleDescription(
        "从有界帧格式中选择字节流切分方式；下拉选项不可手动输入。"
    )
    _configure_responsive_combo(protocol_framing, minimum_width=140, maximum_width=520)
    protocol_framing.currentIndexChanged.connect(callbacks.on_protocol_framing_changed)
    protocol_primary_fields.append(build_labeled_field("帧格式", protocol_framing))
    controls.addLayout(build_field_row(protocol_primary_fields))

    protocol_checksum = QComboBox()
    protocol_checksum.setAccessibleName("协议校验")
    protocol_checksum.addItem("无", ChecksumKind.NONE)
    protocol_checksum.addItem("XOR-8（异或）", ChecksumKind.XOR8)
    protocol_checksum.addItem("CRC16 / Modbus", ChecksumKind.CRC16_MODBUS)
    protocol_checksum.addItem("CRC32 / IEEE", ChecksumKind.CRC32)
    protocol_checksum.addItem("NMEA 0183 XOR（*HH）", ChecksumKind.NMEA0183)
    protocol_checksum.setToolTip("选择协议帧的附加校验方式；当前帧格式不支持时会自动禁用。")
    protocol_checksum.setAccessibleDescription(
        "从有界校验方式中选择协议帧的附加校验；当前帧格式不支持时会自动禁用。"
    )
    _configure_responsive_combo(protocol_checksum, minimum_width=140, maximum_width=520)
    protocol_limits_fields = [build_labeled_field("校验", protocol_checksum)]

    protocol_max_frame = BoundedIntCombo(
        1,
        65_536,
        values=MAX_FRAME_OPTION_VALUES,
        suffix=" B",
    )
    protocol_max_frame.setAccessibleName("协议最大帧长度")
    protocol_max_frame.setValue(4_096)
    protocol_max_frame.setMinimumWidth(108)
    protocol_max_frame.setMaximumWidth(132)
    protocol_max_frame.setToolTip("限制单帧最大字节数；MAVLink/Modbus 等格式可能使用固定上限。")
    protocol_max_frame.setAccessibleDescription(
        "选择单帧最大字节数；某些专用帧格式会根据协议固定上限并禁用此控件。"
    )
    protocol_limits_fields.append(build_labeled_field("最大帧", protocol_max_frame))
    controls.addLayout(build_field_row(protocol_limits_fields))

    protocol_apply_button = ActionRailButton("应用")
    protocol_apply_button.setAccessibleName("应用协议配置")
    protocol_apply_button.setObjectName("primaryButton")
    protocol_apply_button.setToolTip("应用编辑区配置，并清空当前协议/组件派生状态。")
    protocol_apply_button.setAccessibleDescription("应用协议编辑区配置，并清空当前协议与组件派生状态。")
    protocol_apply_button.clicked.connect(callbacks.on_apply_protocol_config)
    protocol_reset_button = ActionRailButton("重置解析")
    protocol_reset_button.setAccessibleName("重置协议解析状态")
    protocol_reset_button.setToolTip("清空当前协议、组件和 Dataset 派生状态，不影响原始终端记录。")
    protocol_reset_button.setAccessibleDescription(
        "清空当前协议、组件和 Dataset 派生状态，不影响原始终端、记录或历史文件。"
    )
    protocol_reset_button.clicked.connect(callbacks.on_reset_protocol)
    action_row = QHBoxLayout()
    action_row.setContentsMargins(0, 0, 0, 0)
    action_row.setSpacing(8)
    action_row.addWidget(protocol_apply_button)
    action_row.addWidget(protocol_reset_button)
    action_row.addStretch(1)
    controls.addLayout(action_row)
    protocol_status = AnalysisStatusLabel("原始字节 · 等待接收")
    register_status_surface.register(
        "protocol",
        protocol_status,
        object_name="protocolStatus",
        state="waiting",
    )
    protocol_status.setProperty("role", "status")
    protocol_status.setTextFormat(Qt.TextFormat.PlainText)
    protocol_status.setAccessibleName("协议统计状态")
    _configure_dynamic_label(
        protocol_status,
        minimum_width=ANALYSIS_STATUS_MIN_WIDTH,
        maximum_width=1_000,
    )
    controls.addWidget(protocol_status)
    protocol_surface_layout.addLayout(controls)

    protocol_config_context = ProtocolConfigContextSurface()
    protocol_config_context.setMinimumWidth(0)
    protocol_config_context.setWordWrap(True)
    protocol_context_policy = protocol_config_context.sizePolicy()
    protocol_context_policy.setHorizontalPolicy(QSizePolicy.Policy.Ignored)
    protocol_config_context.setSizePolicy(protocol_context_policy)
    protocol_surface_layout.addWidget(protocol_config_context)

    pipeline_summary = PipelineSurfaceLabel()
    pipeline_summary.setObjectName("pipelineSummary")
    pipeline_summary.setProperty("role", "subtle")
    pipeline_summary.setProperty("source", "live")
    pipeline_summary.setProperty("state", "idle")
    pipeline_summary.setAccessibleName("解析流水线状态")
    pipeline_summary.setWordWrap(True)
    protocol_surface_layout.addWidget(pipeline_summary)

    detail = QVBoxLayout()
    detail.setContentsMargins(0, 0, 0, 0)
    protocol_delimiter = QLineEdit("0A")
    protocol_delimiter.setAccessibleName("协议分隔符 Hex")
    protocol_delimiter.setPlaceholderText("例如：0A 或 AA 55")
    protocol_delimiter.setToolTip("仅“自定义分隔符”帧格式使用；输入十六进制字节，空格可选。")
    protocol_delimiter.setAccessibleDescription(
        "输入自定义分隔符的十六进制字节，例如 0A 或 AA 55；其他帧格式下不可编辑。"
    )
    protocol_length_bytes = QComboBox()
    protocol_length_bytes.setAccessibleName("长度前缀字节数")
    protocol_length_bytes.addItem("1 字节", 1)
    protocol_length_bytes.addItem("2 字节", 2)
    protocol_length_bytes.addItem("4 字节", 4)
    protocol_length_bytes.setToolTip("仅“长度前缀”帧格式使用；选择长度字段占用的字节数。")
    protocol_length_bytes.setAccessibleDescription(
        "选择长度前缀字段占用的字节数；仅长度前缀帧格式使用。"
    )
    protocol_byteorder = QComboBox()
    protocol_byteorder.setAccessibleName("长度前缀字节序")
    protocol_byteorder.addItem("小端序", "little")
    protocol_byteorder.addItem("大端序", "big")
    protocol_byteorder.setToolTip("仅“长度前缀”帧格式使用；选择长度字段的字节序。")
    protocol_byteorder.setAccessibleDescription(
        "选择长度前缀字段的字节序；仅长度前缀帧格式使用。"
    )
    detail.addLayout(
        build_field_row(
            (
                build_labeled_field("分隔符 Hex", protocol_delimiter),
                build_labeled_field("长度字节", protocol_length_bytes),
                build_labeled_field("字节序", protocol_byteorder),
            )
        )
    )
    protocol_checksum.currentIndexChanged.connect(callbacks.on_mark_protocol_editor_dirty)
    protocol_max_frame.currentIndexChanged.connect(callbacks.on_mark_protocol_editor_dirty)
    protocol_delimiter.textChanged.connect(callbacks.on_mark_protocol_editor_dirty)
    protocol_length_bytes.currentIndexChanged.connect(callbacks.on_mark_protocol_editor_dirty)
    protocol_byteorder.currentIndexChanged.connect(callbacks.on_mark_protocol_editor_dirty)
    protocol_scope = QLabel(
        "组件解析仅处理 UART/TCP Client 接收；Modbus RTU/MAVLink codec 需 UART 已分帧 packet。"
    )
    protocol_scope.setProperty("role", "muted")
    protocol_scope.setWordWrap(True)
    detail.addWidget(protocol_scope)
    protocol_timing_hint = QLabel()
    protocol_timing_hint.setProperty("role", "muted")
    protocol_timing_hint.setWordWrap(True)
    detail.addWidget(protocol_timing_hint)
    protocol_surface_layout.addLayout(detail)
    root.addWidget(protocol_surface)

    component_surface, component_surface_layout = _build_analysis_surface(
        "componentSurface",
        "组件遥测区",
    )
    component_controls = QVBoxLayout()
    component_controls.setContentsMargins(0, 0, 0, 0)
    component_controls.setSpacing(8)
    profile_title = _section_title("组件遥测")
    component_profile_label = QLabel("原始帧")
    component_profile_label.setProperty("role", "muted")
    component_profile_label.setTextFormat(Qt.TextFormat.PlainText)
    component_profile_label.setAccessibleName("当前组件 Profile")
    _configure_dynamic_label(
        component_profile_label,
        minimum_width=PROFILE_LABEL_MIN_WIDTH,
        maximum_width=280,
    )
    load_profile_button = ActionRailButton("加载组件配置 / Codec")
    load_profile_button.setAccessibleName("加载组件 Profile 或 Codec")
    load_profile_button.setToolTip(PROTOCOL_DERIVED_ACTION_HINTS.load_profile)
    load_profile_button.setAccessibleDescription(PROTOCOL_DERIVED_ACTION_HINTS.load_profile)
    load_profile_button.setMaximumWidth(220)
    load_profile_button.clicked.connect(callbacks.on_load_component_codec)
    component_filter = QComboBox()
    component_filter.setAccessibleName("组件帧过滤器")
    component_filter.addItem("全部", None)
    component_filter.addItem("有效", "valid")
    component_filter.addItem("错误", "error")
    _configure_responsive_combo(component_filter, minimum_width=132, maximum_width=180)
    component_filter.currentIndexChanged.connect(callbacks.on_component_filter_changed)
    component_filter_field = build_labeled_field("过滤", component_filter)
    export_component_button = ActionRailButton("导出 CSV")
    export_component_button.setAccessibleName("导出组件 CSV")
    export_component_button.setToolTip(PROTOCOL_DERIVED_ACTION_HINTS.export_component)
    export_component_button.setAccessibleDescription(PROTOCOL_DERIVED_ACTION_HINTS.export_component)
    export_component_button.setMaximumWidth(136)
    export_component_button.clicked.connect(callbacks.on_export_component_csv)
    component_header = _ResponsiveProtocolRow(
        (
            profile_title,
            component_profile_label,
            load_profile_button,
        ),
        parent=component_surface,
    )
    component_controls.addWidget(component_header)
    component_actions = QHBoxLayout()
    component_actions.setContentsMargins(0, 0, 0, 0)
    component_actions.setSpacing(10)
    component_actions.addWidget(component_filter_field)
    component_actions.addStretch(1)
    component_actions.addWidget(export_component_button)
    component_controls.addLayout(component_actions)
    component_status = AnalysisStatusLabel("字段 0 · 行 0")
    register_status_surface.register(
        "component",
        component_status,
        object_name="componentStatus",
        state="waiting",
    )
    component_status.setProperty("role", "status")
    component_status.setTextFormat(Qt.TextFormat.PlainText)
    component_status.setAccessibleName("组件帧统计状态")
    _configure_dynamic_label(
        component_status,
        minimum_width=ANALYSIS_STATUS_MIN_WIDTH,
        maximum_width=440,
    )
    component_controls.addWidget(component_status)
    component_surface_layout.addLayout(component_controls)

    component_table = QTableWidget(0, 6)
    component_table.setHorizontalHeaderLabels(("时间", "序号", "状态", "来源", "字段", "原始 Hex"))
    component_table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
    component_table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
    component_table.setMaximumHeight(170)
    component_table.setVisible(False)
    component_table.setAlternatingRowColors(True)
    component_table.setAccessibleName("组件帧表")
    component_table.setAccessibleDescription(
        "只读显示最近的组件帧；过滤器可切换全部、有效或错误行。"
    )
    component_table.verticalHeader().setVisible(False)
    component_header = component_table.horizontalHeader()
    component_header.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
    component_header.setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
    component_header.setSectionResizeMode(2, QHeaderView.ResizeMode.ResizeToContents)
    component_header.setSectionResizeMode(3, QHeaderView.ResizeMode.ResizeToContents)
    component_header.setSectionResizeMode(4, QHeaderView.ResizeMode.Interactive)
    component_header.setSectionResizeMode(5, QHeaderView.ResizeMode.Stretch)
    _configure_protocol_leaf_view(component_table)
    component_surface_layout.addWidget(component_table)
    component_empty = ComponentEmptyStateSurface()
    component_empty.setText(
        "暂无组件帧 · 接收 UART/TCP Client RX 或选择历史回放；加载组件配置 / Codec 可解析字段。"
    )
    component_empty.setAccessibleDescription(component_empty.text())
    component_empty.setToolTip(component_empty.text())
    component_empty.load_requested.connect(callbacks.on_load_component_codec)
    component_surface_layout.addWidget(component_empty)

    component_preview = ObservationViewport(scope="component")
    component_preview.setObjectName("componentPreview")
    component_preview.setReadOnly(True)
    component_preview.setAccessibleName("组件帧预览")
    component_preview.setPlaceholderText(
        "尚无可显示的协议帧 · 接收 UART/TCP Client RX 后会在这里显示。"
    )
    component_preview.setMaximumBlockCount(120)
    component_preview.setMaximumHeight(110)
    _configure_protocol_leaf_view(component_preview)
    # The semantic empty-state surface owns the waiting view.  Keep the raw
    # preview collapsed until frames exist so an empty page never renders two
    # competing placeholders with a large blank editor between them.
    component_preview.setVisible(False)
    component_surface_layout.addWidget(component_preview)
    root.addWidget(component_surface)

    dataset_surface, dataset_surface_layout = _build_analysis_surface(
        "datasetSurface",
        "Dataset 与数据曲线区",
    )
    dataset_controls = QVBoxLayout()
    dataset_controls.setContentsMargins(0, 0, 0, 0)
    dataset_controls.setSpacing(8)
    dataset_title = _section_title("Dataset 遥测")
    dataset_config_label = QLabel("未启用 · 0 个序列")
    dataset_config_label.setProperty("role", "muted")
    dataset_config_label.setTextFormat(Qt.TextFormat.PlainText)
    dataset_config_label.setAccessibleName("当前 Dataset 配置")
    _configure_dynamic_label(
        dataset_config_label,
        minimum_width=DATASET_LABEL_MIN_WIDTH,
        maximum_width=300,
    )
    load_dataset_button = ActionRailButton("加载 Dataset")
    load_dataset_button.setAccessibleName("加载 Dataset 配置")
    load_dataset_button.setToolTip(PROTOCOL_DERIVED_ACTION_HINTS.load_dataset)
    load_dataset_button.setAccessibleDescription(PROTOCOL_DERIVED_ACTION_HINTS.load_dataset)
    load_dataset_button.setMaximumWidth(160)
    load_dataset_button.clicked.connect(callbacks.on_load_dataset_config)
    export_dataset_button = ActionRailButton("导出 Dataset CSV")
    export_dataset_button.setAccessibleName("导出 Dataset CSV")
    export_dataset_button.setToolTip(PROTOCOL_DERIVED_ACTION_HINTS.export_dataset)
    export_dataset_button.setAccessibleDescription(PROTOCOL_DERIVED_ACTION_HINTS.export_dataset)
    export_dataset_button.setMaximumWidth(190)
    export_dataset_button.clicked.connect(callbacks.on_export_dataset_csv)
    dataset_header = QHBoxLayout()
    dataset_header.setContentsMargins(0, 0, 0, 0)
    dataset_header.setSpacing(10)
    dataset_header.addWidget(dataset_title)
    dataset_header.addWidget(dataset_config_label, 1)
    dataset_controls.addLayout(dataset_header)
    dataset_actions = _ResponsiveProtocolRow(
        (
            _field_label("Dataset 操作"),
            load_dataset_button,
            export_dataset_button,
        ),
        parent=dataset_surface,
    )
    dataset_controls.addWidget(dataset_actions)
    dataset_status = AnalysisStatusLabel("样本 0 · 等待组件帧")
    register_status_surface.register(
        "dataset",
        dataset_status,
        object_name="datasetStatus",
        state="waiting",
    )
    dataset_status.setProperty("role", "status")
    dataset_status.setTextFormat(Qt.TextFormat.PlainText)
    dataset_status.setAccessibleName("Dataset 统计状态")
    _configure_dynamic_label(
        dataset_status,
        minimum_width=ANALYSIS_STATUS_MIN_WIDTH,
        maximum_width=420,
    )
    dataset_controls.addWidget(dataset_status)
    dataset_surface_layout.addLayout(dataset_controls)

    dataset_preview = ObservationViewport(scope="dataset")
    dataset_preview.setObjectName("datasetPreview")
    dataset_preview.setReadOnly(True)
    dataset_preview.setAccessibleName("Dataset 预览")
    dataset_preview.setPlaceholderText(
        "尚无 Dataset 样本 · 加载配置并接收有效组件帧后会在这里显示。"
    )
    dataset_preview.setMaximumBlockCount(120)
    dataset_preview.setMaximumHeight(100)
    _configure_protocol_leaf_view(dataset_preview)
    # Dataset status and the load action are sufficient guidance before the
    # first sample; expand the observation surface only when it has content.
    dataset_preview.setVisible(False)
    dataset_surface_layout.addWidget(dataset_preview)

    curve_controls = QVBoxLayout()
    curve_controls.setContentsMargins(0, 0, 0, 0)
    curve_controls.setSpacing(8)
    curve_title = _section_title("数据曲线")
    curve_header = QHBoxLayout()
    curve_header.setContentsMargins(0, 0, 0, 0)
    curve_header.addWidget(curve_title)
    curve_header.addStretch(1)
    curve_controls.addLayout(curve_header)
    dataset_curve_series = QComboBox()
    dataset_curve_series.setAccessibleName("Dataset 曲线序列")
    dataset_curve_series.addItem("选择数值序列", None)
    dataset_curve_series.setToolTip("选择要绘制的数值序列；曲线最多保留 512 个采样点。")
    dataset_curve_series.setAccessibleDescription(
        "从已加载 Dataset 的数值序列中选择曲线来源；曲线最多保留 512 个采样点。"
    )
    dataset_curve_series.currentIndexChanged.connect(callbacks.on_curve_series_changed)
    _configure_responsive_combo(dataset_curve_series, minimum_width=160, maximum_width=520)
    dataset_curve_status = AnalysisStatusLabel("仅绘制有限数值 · 最多 512 点")
    register_status_surface.register(
        "curve",
        dataset_curve_status,
        object_name="datasetCurveStatus",
        state="waiting",
    )
    dataset_curve_status.setProperty("role", "status")
    dataset_curve_status.setTextFormat(Qt.TextFormat.PlainText)
    dataset_curve_status.setAccessibleName("Dataset 曲线统计状态")
    _configure_dynamic_label(
        dataset_curve_status,
        minimum_width=ANALYSIS_STATUS_MIN_WIDTH,
        maximum_width=420,
    )
    curve_fields = QHBoxLayout()
    curve_fields.setContentsMargins(0, 0, 0, 0)
    curve_fields.setSpacing(12)
    curve_fields.addWidget(build_labeled_field("数值序列", dataset_curve_series), 1)
    curve_fields.addWidget(dataset_curve_status, 1)
    curve_controls.addLayout(curve_fields)
    dataset_surface_layout.addLayout(curve_controls)
    dataset_curve = DatasetCurveWidget()
    dataset_surface_layout.addWidget(dataset_curve)
    root.addWidget(dataset_surface)

    replay_surface, replay_surface_layout = _build_analysis_surface(
        "replaySurface",
        "原始记录回放区",
    )
    replay_controls = QVBoxLayout()
    replay_controls.setContentsMargins(0, 0, 0, 0)
    replay_controls.setSpacing(8)
    replay_title = _section_title("原始记录回放")
    replay_header = QHBoxLayout()
    replay_header.setContentsMargins(0, 0, 0, 0)
    replay_header.addWidget(replay_title)
    replay_header.addStretch(1)
    replay_controls.addLayout(replay_header)
    replay_speed = QComboBox()
    replay_speed.setAccessibleName("历史回放速度")
    for label, value in (("0.1×", 0.1), ("0.5×", 0.5), ("1×", 1.0), ("2×", 2.0), ("8×", 8.0)):
        replay_speed.addItem(label, value)
    replay_speed.setCurrentIndex(2)
    replay_speed.setToolTip("选择历史记录回放速度；不会改变记录内容或连接设备。")
    replay_speed.setAccessibleDescription("选择历史记录回放速度，不会连接设备或修改历史文件。")
    _configure_responsive_combo(replay_speed, minimum_width=84, maximum_width=110)
    replay_speed_row = QHBoxLayout()
    replay_speed_row.setContentsMargins(0, 0, 0, 0)
    replay_speed_row.setSpacing(8)
    replay_speed_row.addWidget(_field_label("回放速度"))
    replay_speed_row.addWidget(replay_speed)
    replay_speed_row.addStretch(1)
    replay_controls.addLayout(replay_speed_row)
    replay_start_button = ActionRailButton("选择并回放")
    replay_start_button.setAccessibleName("选择并开始历史回放")
    replay_start_button.setToolTip("选择一个 JSONL 原始记录并开始回放；不会连接设备。")
    replay_start_button.clicked.connect(callbacks.on_start_replay)
    replay_pause_button = BusyActionButton("暂停")
    replay_pause_button.setAccessibleName("暂停或继续历史回放")
    replay_pause_button.setToolTip("暂停或继续当前历史回放；只读取历史记录。")
    replay_pause_button.clicked.connect(callbacks.on_toggle_replay_pause)
    replay_stop_button = ActionRailButton("停止")
    replay_stop_button.setAccessibleName("停止历史回放")
    replay_stop_button.setToolTip("停止当前历史回放；不会修改历史文件。")
    replay_stop_button.clicked.connect(callbacks.on_stop_replay)
    replay_actions = QHBoxLayout()
    replay_actions.setContentsMargins(0, 0, 0, 0)
    replay_actions.setSpacing(8)
    replay_actions.addWidget(_field_label("回放操作"))
    replay_actions.addWidget(replay_start_button)
    replay_actions.addWidget(replay_pause_button)
    replay_actions.addWidget(replay_stop_button)
    replay_actions.addStretch(1)
    replay_controls.addLayout(replay_actions)
    replay_status = ReplayActivityLabel()
    replay_status.setText("未加载 · 仅 RX · 不连接设备")
    register_status_surface.register(
        "replay",
        replay_status,
        object_name="replayStatus",
        state="idle",
    )
    replay_status.setProperty("role", "status")
    replay_status.setTextFormat(Qt.TextFormat.PlainText)
    replay_status.setAccessibleName("历史回放状态")
    _configure_dynamic_label(replay_status, maximum_width=520)
    replay_controls.addWidget(replay_status)
    replay_surface_layout.addLayout(replay_controls)
    root.addWidget(replay_surface)

    return ProtocolPanelWidgets(
        layout=root,
        protocol_preset=protocol_preset,
        protocol_framing=protocol_framing,
        protocol_checksum=protocol_checksum,
        protocol_max_frame=protocol_max_frame,
        protocol_apply_button=protocol_apply_button,
        protocol_reset_button=protocol_reset_button,
        protocol_status=protocol_status,
        protocol_config_context=protocol_config_context,
        pipeline_summary=pipeline_summary,
        protocol_delimiter=protocol_delimiter,
        protocol_length_bytes=protocol_length_bytes,
        protocol_byteorder=protocol_byteorder,
        protocol_scope=protocol_scope,
        protocol_timing_hint=protocol_timing_hint,
        protocol_detail_layout=detail,
        component_profile_label=component_profile_label,
        load_profile_button=load_profile_button,
        component_filter=component_filter,
        export_component_button=export_component_button,
        component_status=component_status,
        component_table=component_table,
        component_empty=component_empty,
        component_preview=component_preview,
        dataset_config_label=dataset_config_label,
        load_dataset_button=load_dataset_button,
        export_dataset_button=export_dataset_button,
        dataset_status=dataset_status,
        dataset_preview=dataset_preview,
        dataset_curve_series=dataset_curve_series,
        dataset_curve_status=dataset_curve_status,
        dataset_curve=dataset_curve,
        replay_speed=replay_speed,
        replay_start_button=replay_start_button,
        replay_pause_button=replay_pause_button,
        replay_stop_button=replay_stop_button,
        replay_status=replay_status,
    )


__all__ = ["ProtocolPanelWidgets", "build_protocol_panel"]

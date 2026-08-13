"""Semantic two-row builder for the live observation controls."""

from __future__ import annotations

from ...domain.models import CommandMode
from ..action_surface import ActionRailButton, BusyActionButton
from ..data_activity_surface import DataActivitySurface
from ..qt import (
    QCheckBox,
    QComboBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
)


class _ResponsiveObservationBand(QFrame):
    """Own live-observation geometry while keeping activity state external."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._compact: bool | None = None
        self._registered = False
        self._controls_row: QHBoxLayout | None = None
        self._activity_row: QHBoxLayout | None = None
        self._controls: tuple[QWidget, ...] = ()
        self._activity_controls: tuple[QWidget, ...] = ()

    def register_layout(
        self,
        *,
        controls_row: QHBoxLayout,
        activity_row: QHBoxLayout,
        controls: tuple[QWidget, ...],
        activity_controls: tuple[QWidget, ...],
    ) -> None:
        """Register existing rows and controls without creating a second state owner."""

        self._controls_row = controls_row
        self._activity_row = activity_row
        self._controls = controls
        self._activity_controls = activity_controls
        self._registered = True
        self._reflow(force=True)

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)
        self._reflow()

    def _reflow(self, *, force: bool = False) -> None:
        """Reflow."""
        if not self._registered:
            return
        compact = self._uses_compact_spacing()
        if not force and compact == self._compact:
            return
        spacing = 8 if compact else 10
        if self._controls_row is not None:
            self._controls_row.setSpacing(spacing)
        if self._activity_row is not None:
            self._activity_row.setSpacing(spacing)
        for widget in self._controls + self._activity_controls:
            self._set_intrinsic_policy(widget)
        self._compact = compact

    def _uses_compact_spacing(self) -> bool:
        """Derive the compact boundary from the rows' comfortable widths."""

        width = self.contentsRect().width()
        rows = (self._controls_row, self._activity_row)
        required = max(
            self._row_width(row, widgets)
            for row, widgets in zip(rows, (self._controls, self._activity_controls), strict=True)
            if row is not None
        )
        return width < required + 3 * 10

    @staticmethod
    def _row_width(row: QHBoxLayout, widgets: tuple[QWidget, ...]) -> int:
        """Row width."""
        spacing = max(0, row.spacing())
        margins = row.contentsMargins()
        intrinsic = sum(
            max(
                widget.minimumSizeHint().width(),
                widget.sizeHint().width(),
                widget.minimumWidth(),
            )
            for widget in widgets
        )
        return intrinsic + spacing * max(0, len(widgets) - 1) + margins.left() + margins.right()

    @staticmethod
    def _set_intrinsic_policy(widget: QWidget) -> None:
        """Set intrinsic policy."""
        policy = widget.sizePolicy()
        if isinstance(widget, (QPushButton, QComboBox, QCheckBox)):
            policy.setHorizontalPolicy(QSizePolicy.Policy.Fixed)
        else:
            policy.setHorizontalPolicy(QSizePolicy.Policy.Preferred)
        widget.setSizePolicy(policy)


def _field_label(text: str) -> QLabel:
    """Create a compact secondary label for one terminal toolbar field."""

    label = QLabel(text)
    label.setProperty("role", "muted")
    return label


def build_terminal_toolbar(window) -> QFrame:
    """Build live observation controls in two readable semantic rows."""

    band = _ResponsiveObservationBand(window)
    band.setObjectName("liveObservationBand")
    band.setFrameShape(QFrame.Shape.NoFrame)
    band.setProperty("role", "stationBand")
    band.setProperty("source", "live")
    band.setProperty("state", "idle")
    layout = QVBoxLayout(band)
    layout.setContentsMargins(12, 8, 12, 8)
    layout.setSpacing(8)

    controls_row = QHBoxLayout()
    controls_row.setContentsMargins(0, 0, 0, 0)
    controls_row.setSpacing(10)
    terminal_title = QLabel("实时观测")
    terminal_title.setProperty("role", "section")
    terminal_title.setProperty("scope", "station")
    terminal_title.setAccessibleName("实时观测工作区")
    terminal_title.setMinimumWidth(96)
    terminal_title.setMaximumWidth(132)
    terminal_title.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
    controls_row.addWidget(terminal_title)
    display_label = _field_label("显示")
    controls_row.addWidget(display_label)
    window._display_mode = QComboBox()
    window._display_mode.setAccessibleName("终端显示模式")
    window._display_mode.setEditable(False)
    window._display_mode.setToolTip("选择终端预览显示为文本或 Hex；不会改变原始接收字节。")
    window._display_mode.setAccessibleDescription(
        "选择终端预览显示格式：文本或十六进制；只改变展示，不改变原始接收数据。"
    )
    window._display_mode.addItem("文本", CommandMode.TEXT)
    window._display_mode.addItem("Hex", CommandMode.HEX)
    window._display_mode.currentIndexChanged.connect(window._rerender_preview)
    window._display_mode.setMinimumWidth(88)
    window._display_mode.setMaximumWidth(128)
    controls_row.addWidget(window._display_mode)

    window._pause_check = QCheckBox("暂停显示")
    window._pause_check.setAccessibleName("暂停终端显示")
    window._pause_check.setAccessibleDescription("仅暂停终端预览显示，不暂停接收或原始记录。")
    window._pause_check.setToolTip("只暂停终端显示；接收与记录继续进行。")
    window._pause_check.toggled.connect(window._view_model.set_preview_paused)
    window._pause_check.toggled.connect(lambda _checked=False: window._update_data_activity())
    controls_row.addWidget(window._pause_check)
    window._paused_label = QLabel()
    window._paused_label.setObjectName("pausedState")
    window._paused_label.setProperty("role", "status")
    window._paused_label.setProperty("state", "idle")
    window._paused_label.setAccessibleName("终端暂停状态")
    window._paused_label.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Preferred)
    window._paused_label.setMinimumWidth(90)
    window._paused_label.setMaximumWidth(190)
    controls_row.addWidget(window._paused_label, 1)
    layout.addLayout(controls_row)

    activity_row = QHBoxLayout()
    activity_row.setContentsMargins(0, 0, 0, 0)
    activity_row.setSpacing(10)
    clear_button = ActionRailButton("清空预览")
    clear_button.setObjectName("dangerButton")
    clear_button.setAccessibleName("清空终端预览")
    clear_button.setToolTip("清空当前终端显示预览；不会停止接收或原始记录。")
    clear_button.setAccessibleDescription(
        "清空当前终端显示预览；不会停止接收、原始记录或改变连接。"
    )
    clear_button.clicked.connect(window._view_model.clear_preview)
    activity_row.addWidget(clear_button)
    window._clear_terminal_button = clear_button

    window._record_button = BusyActionButton("开始原始记录")
    window._record_button.setAccessibleName("原始记录控制")
    window._record_button.clicked.connect(window._toggle_recording)
    activity_row.addWidget(window._record_button)
    window._record_label = QLabel("未记录")
    window._record_label.setObjectName("recordState")
    window._record_label.setProperty("role", "status")
    window._record_label.setProperty("state", "idle")
    window._record_label.setAccessibleName("记录状态")
    window._record_label.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Preferred)
    window._record_label.setMinimumWidth(90)
    window._record_label.setMaximumWidth(210)
    activity_row.addWidget(window._record_label)
    window._data_activity_label = DataActivitySurface()
    window._data_activity_label.setObjectName("dataActivity")
    window._data_activity_label.setProperty("role", "status")
    window._data_activity_label.setProperty("state", "idle")
    window._data_activity_label.setAccessibleName("接收数据活动")
    window._data_activity_label.setToolTip(
        "显示最近一批接收数据和当前有界预览窗口；暂停显示不会停止接收。"
    )
    window._data_activity_label.setSizePolicy(
        QSizePolicy.Policy.Ignored,
        QSizePolicy.Policy.Preferred,
    )
    window._data_activity_label.setMinimumWidth(160)
    window._data_activity_label.setMaximumWidth(340)
    window._data_activity_label.setAlignment(
        Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter
    )
    activity_row.addWidget(window._data_activity_label, 1)
    layout.addLayout(activity_row)
    band.register_layout(
        controls_row=controls_row,
        activity_row=activity_row,
        controls=(
            terminal_title,
            display_label,
            window._display_mode,
            window._pause_check,
            window._paused_label,
        ),
        activity_controls=(
            clear_button,
            window._record_button,
            window._record_label,
            window._data_activity_label,
        ),
    )
    window._live_observation_band = band
    return band


__all__ = ["build_terminal_toolbar"]

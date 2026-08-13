"""Application header, workspace navigation, and motion controller.

The workspace controller owns shell-level navigation and animation lifecycle.
Feature panels remain composed by the root and do not depend on this module.
"""

from __future__ import annotations

from functools import partial

from shiboken6 import isValid

from ..bounded_text_label import BoundedTextLabel
from ..brand_mark_surface import BrandMarkSurface
from ..chrome_bindings import HeaderChromeBindings, header_chrome_bindings_for
from ..embedded_extension_panel import build_extension_panel
from ..property_refresh import refresh_dynamic_property
from ..qt import (
    QCheckBox,
    QComboBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QSizePolicy,
    Qt,
    QTabWidget,
    QTimer,
    QToolButton,
    QVBoxLayout,
    QWidget,
)
from ..responsive_header_controls import ResponsiveHeaderControls
from ..responsive_scroll_area import ShortPageVerticalRhythm
from ..theme import THEME_OPTIONS, reduced_motion_requested, theme_spec_for_widget
from ..theme_palette_surface import ThemePaletteSwatch
from ..theme_picker_icons import refresh_theme_picker_icons
from ..widgets import SignalFieldWidget, StatusIndicator
from ..workspace_bindings import WorkspaceShellBindings
from ..workspace_context_surface import WorkspaceContextLabel
from ..workspace_focus_transition import set_workspace_focus_mode
from ..workspace_route_surface import WorkspaceRouteSurface
from ..workspace_scroll_hint import WorkspaceScrollHint
from ..workspace_tab_icons import refresh_workspace_tab_icons
from ..workspace_tab_surface import AnimatedWorkspaceTabBar
from .command_workspace_builder import build_command_workspace
from .composition import build_protocol_panel_for_window, scroll_page
from .connection_builder import build_connection_panel
from .connection_presets import (
    apply_connection_preset,
    delete_custom_connection_preset,
    save_custom_connection_preset,
)
from .connection_runtime import toggle_connection
from .lifecycle import (
    on_motion_frame,
    on_motion_pause_toggled,
    on_motion_toggled,
    on_theme_changed,
)
from .workspace_runtime import (
    animate_workspace_transition,
    on_workspace_tab_changed,
    request_workspace_activity,
    set_connection_focus_override,
)

_COMPACT_HEADER_WIDTH = 1_120
_OVERVIEW_WORKSPACE_MIN_HEIGHT = 150
_OVERVIEW_WORKSPACE_MAX_HEIGHT = 220


def _sync_header_density(header: QFrame) -> None:
    """Keep header chrome legible while yielding vertical space at narrow widths."""

    window = header.window()
    bindings = header_chrome_bindings_for(window)
    if bindings is None:
        return
    width = header.width() or int(window.width())
    compact = width < _COMPACT_HEADER_WIDTH
    refresh_dynamic_property(header, "density", "compact" if compact else "regular")
    bindings.signal_field.setVisible(not compact)
    bindings.theme_palette_swatch.setVisible(not compact)

    header_layout = header.layout()
    if isinstance(header_layout, QVBoxLayout):
        vertical_margin = 6 if compact else 8
        header_layout.setContentsMargins(12, vertical_margin, 12, vertical_margin)
        header_layout.setSpacing(4 if compact else 6)

    brand_row = getattr(header, "_brand_row", None)
    if isinstance(brand_row, QHBoxLayout):
        brand_row.setSpacing(7 if compact else 9)
    brand_layout = getattr(header, "_brand_layout", None)
    if isinstance(brand_layout, QVBoxLayout):
        brand_layout.setSpacing(0 if compact else 1)

    cluster_layouts = (
        (getattr(header, "_status_layout", None), 6 if compact else 7),
        (getattr(header, "_motion_layout", None), 7 if compact else 8),
        (getattr(header, "_theme_layout", None), 6 if compact else 7),
    )
    for cluster_layout, spacing in cluster_layouts:
        if isinstance(cluster_layout, QHBoxLayout):
            vertical_margin = 3 if compact else 5
            cluster_layout.setContentsMargins(7, vertical_margin, 7, vertical_margin)
            cluster_layout.setSpacing(spacing)

    state_caption = getattr(header, "_state_caption", None)
    if isinstance(state_caption, QLabel):
        # The adjacent indicator and state value already express this label
        # at compact width; keep the caption in the accessible status cluster
        # contract while removing one redundant visual token from the row.
        state_caption.setVisible(not compact)

    controls_owner = getattr(header, "_controls_owner", None)
    if isinstance(controls_owner, ResponsiveHeaderControls):
        controls_owner.set_density(compact=compact)


class _AdaptiveHeader(QFrame):
    """Keep compact header policy local to the header's geometry owner."""

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)
        _sync_header_density(self)


def _overview_workspace_minimum_height(window: QWidget) -> int:
    """Reserve the largest safe overview viewport without squeezing siblings."""

    root = getattr(window, "_app_root", None)
    layout = root.layout() if isinstance(root, QWidget) else None
    if layout is None:
        return _OVERVIEW_WORKSPACE_MIN_HEIGHT

    shell = getattr(window, "_workspace_bindings", None)
    shell = getattr(shell, "shell", None)
    occupied = layout.contentsMargins().top() + layout.contentsMargins().bottom()
    occupied += max(0, layout.spacing()) * max(0, layout.count() - 1)
    for index in range(layout.count()):
        widget = layout.itemAt(index).widget()
        if not isinstance(widget, QWidget) or widget is shell or not widget.isVisible():
            continue
        occupied += max(widget.minimumHeight(), widget.minimumSizeHint().height())

    available_height = root.height() - occupied
    if available_height <= 0:
        return 0
    return min(_OVERVIEW_WORKSPACE_MAX_HEIGHT, available_height)


class _ResponsiveWorkspaceShell(QFrame):
    """Own only the overview shell's height floor; pages keep their scroll owner."""

    def __init__(self, window: QWidget, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._window = window
        self._overview_sync_pending = False
        self._overview_sync_round = 0

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)
        self.sync_overview_minimum_height()
        self._queue_overview_minimum_sync()

    def _queue_overview_minimum_sync(self) -> None:
        """Recheck after the root layout has consumed the current resize."""

        if self._overview_sync_pending or bool(
            getattr(self._window, "_workspace_focus_mode", False)
        ):
            return
        self._overview_sync_pending = True
        self._overview_sync_round = 0
        QTimer.singleShot(0, self._run_queued_overview_sync)

    def _run_queued_overview_sync(self) -> None:
        """Apply one coalesced post-layout overview correction."""

        if not isValid(self) or not isValid(self._window):
            return
        if bool(getattr(self._window, "_workspace_focus_mode", False)):
            self._overview_sync_pending = False
            return
        root = getattr(self._window, "_app_root", None)
        layout = root.layout() if isinstance(root, QWidget) else None
        if layout is not None:
            layout.activate()
        self.sync_overview_minimum_height()
        if self._overview_sync_round == 0:
            self._overview_sync_round = 1
            QTimer.singleShot(0, self._run_queued_overview_sync)
            return
        self._overview_sync_round = 0
        self._overview_sync_pending = False

    def sync_overview_minimum_height(self) -> None:
        """Reapply the overview floor after a temporary focus layout restores."""

        if bool(getattr(self._window, "_workspace_focus_mode", False)):
            return
        minimum_height = _overview_workspace_minimum_height(self._window)
        if self.minimumHeight() != minimum_height:
            self.setMinimumHeight(minimum_height)


def build_app_header(window) -> QWidget:
    """Build the stable visual anchor for connection state and motion."""

    header = _AdaptiveHeader(window)
    header.setObjectName("appHeader")
    window._app_header = header
    layout = QVBoxLayout(header)
    layout.setContentsMargins(12, 8, 12, 8)
    layout.setSpacing(6)

    brand_row = QHBoxLayout()
    brand_row.setContentsMargins(0, 0, 0, 0)
    brand_row.setSpacing(9)

    window._brand_mark = BrandMarkSurface(header)
    window._brand_mark.setObjectName("brandMark")
    brand_row.addWidget(window._brand_mark)

    brand = QVBoxLayout()
    brand.setSpacing(1)
    header._brand_row = brand_row
    header._brand_layout = brand
    title = QLabel("SERIALFORGE")
    title.setObjectName("brandTitle")
    brand.addWidget(title)
    subtitle = QLabel("嵌入式调试工具站 · 观测 / 解析 / 回放")
    subtitle.setObjectName("brandSubtitle")
    subtitle.setProperty("role", "muted")
    brand.addWidget(subtitle)
    brand_row.addLayout(brand)
    window._signal_field = SignalFieldWidget(header)
    window._motion_controller.frame_changed.connect(partial(on_motion_frame, window))
    brand_row.addWidget(window._signal_field)
    brand_row.addStretch(1)
    layout.addLayout(brand_row)

    controls_row = QHBoxLayout()
    header._controls_row = controls_row
    controls_row.setContentsMargins(0, 0, 0, 0)
    controls_row.setSpacing(8)

    status_cluster = QFrame()
    status_cluster.setObjectName("statusCluster")
    status_cluster.setProperty("role", "headerCluster")
    status_cluster.setProperty("state", "closed")
    status_cluster.setAccessibleName("连接状态摘要")
    status_cluster.setAccessibleDescription("显示当前连接上下文、数据来源和连接状态。")
    status_layout = QHBoxLayout(status_cluster)
    status_layout.setContentsMargins(8, 5, 8, 5)
    status_layout.setSpacing(7)
    header._status_layout = status_layout

    window._context_label = BoundedTextLabel("选择连接方式", status_cluster)
    window._context_label.setObjectName("connectionContext")
    window._context_label.setProperty("role", "context")
    window._context_label.setAccessibleName("当前连接上下文")
    status_layout.addWidget(window._context_label)
    window._source_badge = BoundedTextLabel("实时 · 未连接", status_cluster)
    window._source_badge.setObjectName("sourceBadge")
    window._source_badge.setProperty("source", "live")
    window._source_badge.setAccessibleName("当前数据来源")
    status_layout.addWidget(window._source_badge)
    state_caption = QLabel("连接状态")
    state_caption.setProperty("role", "muted")
    header._state_caption = state_caption
    status_layout.addWidget(state_caption)
    window._state_indicator = StatusIndicator(status_cluster)
    status_layout.addWidget(window._state_indicator)
    window._state_label = BoundedTextLabel("未连接", status_cluster)
    window._state_label.setObjectName("stateValue")
    window._state_label.setAccessibleName("当前连接状态")
    status_layout.addWidget(window._state_label)
    window._status_cluster = status_cluster
    motion_controls = QFrame()
    motion_controls.setObjectName("motionControls")
    motion_controls.setProperty("role", "headerCluster")
    motion_controls.setAccessibleName("动效控制")
    motion_controls.setAccessibleDescription("控制装饰性信号场和状态脉冲，不影响连接和数据处理。")
    motion_layout = QHBoxLayout(motion_controls)
    motion_layout.setContentsMargins(8, 5, 8, 5)
    motion_layout.setSpacing(8)
    header._motion_layout = motion_layout
    window._motion_check = QCheckBox("低动效")
    window._motion_check.setToolTip("关闭装饰性信号场和状态脉冲；不影响连接、接收、记录或回放。")
    window._motion_check.setAccessibleName("低动效")
    window._motion_check.setAccessibleDescription(
        "启用后仅停用装饰性信号场和状态脉冲，业务状态和数据展示保持不变。"
    )
    forced_reduced_motion = reduced_motion_requested()
    window._motion_check.blockSignals(True)
    window._motion_check.setChecked(
        window._presentation_preferences.reduced_motion or forced_reduced_motion
    )
    window._motion_check.blockSignals(False)
    if forced_reduced_motion:
        window._motion_check.setEnabled(False)
        window._motion_check.setToolTip(
            "SERIALFORGE_REDUCED_MOTION 已强制启用低动效；移除环境变量后可在此处修改。"
        )
    window._motion_check.toggled.connect(partial(on_motion_toggled, window))
    motion_layout.addWidget(window._motion_check)
    window._motion_pause_check = QCheckBox("暂停动效")
    window._motion_pause_check.setToolTip(
        "暂停装饰性信号场和状态脉冲；不影响连接、接收、记录或回放。"
    )
    window._motion_pause_check.setAccessibleName("暂停动效")
    window._motion_pause_check.setAccessibleDescription(
        "独立暂停装饰性信号场和状态脉冲，不会暂停终端滚屏或历史回放。"
    )
    window._motion_pause_check.blockSignals(True)
    window._motion_pause_check.setChecked(window._presentation_preferences.motion_paused)
    window._motion_pause_check.blockSignals(False)
    window._motion_pause_check.toggled.connect(partial(on_motion_pause_toggled, window))
    motion_layout.addWidget(window._motion_pause_check)
    # Hydrate both switches explicitly; blocked signals above intentionally
    # prevent startup preferences from being mistaken for user actions.
    window._motion_controller.set_motion_enabled(not window._motion_check.isChecked())
    window._motion_controller.set_paused(window._motion_pause_check.isChecked())
    window._motion_controls = motion_controls
    theme_controls = QFrame()
    theme_controls.setObjectName("themeControls")
    theme_controls.setProperty("role", "headerCluster")
    theme_controls.setAccessibleName("主题切换")
    theme_controls.setAccessibleDescription("选择界面配色主题，连接和数据状态不会被改变。")
    theme_layout = QHBoxLayout(theme_controls)
    theme_layout.setContentsMargins(8, 5, 8, 5)
    theme_layout.setSpacing(7)
    header._theme_layout = theme_layout
    theme_label = QLabel("主题")
    theme_label.setProperty("role", "muted")
    theme_layout.addWidget(theme_label)
    window._theme_combo = QComboBox(theme_controls)
    window._theme_combo.setObjectName("themePicker")
    window._theme_combo.setAccessibleName("界面主题")
    window._theme_combo.setToolTip("切换二次元配色主题，不影响链路与数据。")
    for theme in THEME_OPTIONS:
        window._theme_combo.addItem(theme.label, theme.key)
        window._theme_combo.setItemData(
            window._theme_combo.count() - 1,
            theme.description,
            Qt.ItemDataRole.ToolTipRole,
        )
    refresh_theme_picker_icons(window._theme_combo, THEME_OPTIONS)
    default_theme_index = window._theme_combo.findData(
        window._presentation_preferences.theme_key
    )
    if default_theme_index >= 0:
        window._theme_combo.setCurrentIndex(default_theme_index)
    window._theme_combo.currentIndexChanged.connect(partial(on_theme_changed, window))
    theme_layout.addWidget(window._theme_combo)
    window._theme_palette_swatch = ThemePaletteSwatch(theme_controls)
    window._theme_palette_swatch.setObjectName("themePaletteSwatch")
    theme_layout.addWidget(window._theme_palette_swatch)
    window._theme_controls = theme_controls
    controls_owner = ResponsiveHeaderControls(
        status_cluster=status_cluster,
        motion_controls=motion_controls,
        theme_controls=theme_controls,
        parent=header,
    )
    header._controls_owner = controls_owner
    controls_row.addWidget(controls_owner)
    window._header_chrome_bindings = HeaderChromeBindings(
        status_cluster=status_cluster,
        context_label=window._context_label,
        source_badge=window._source_badge,
        state_indicator=window._state_indicator,
        state_label=window._state_label,
        motion_controls=motion_controls,
        motion_check=window._motion_check,
        motion_pause_check=window._motion_pause_check,
        theme_controls=theme_controls,
        theme_combo=window._theme_combo,
        theme_palette_swatch=window._theme_palette_swatch,
        brand_mark=window._brand_mark,
        signal_field=window._signal_field,
    )
    layout.addLayout(controls_row)
    _sync_header_density(header)
    on_theme_changed(window)
    return header


def _toggle_workspace_focus(window, enabled: bool) -> None:
    """Treat an explicit return-to-overview click as onboarding opt-out."""

    if not enabled:
        window._connection_onboarding = False
    set_connection_focus_override(window, enabled)
    set_workspace_focus_mode(window, enabled)


def build_workspace_tabs(window) -> WorkspaceShellBindings:
    """Keep secondary configuration accessible without crowding the terminal."""

    workspace_shell = _ResponsiveWorkspaceShell(window, window)
    workspace_shell.setObjectName("workspaceShell")
    workspace_shell.setProperty("mode", "overview")
    workspace_shell.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Preferred,
    )
    shell_layout = QVBoxLayout(workspace_shell)
    shell_layout.setContentsMargins(0, 0, 0, 0)
    shell_layout.setSpacing(2)

    tabs = QTabWidget(workspace_shell)
    tabs.setObjectName("workspaceTabs")
    tabs.setAccessibleName("工作区页面")
    tabs.setAccessibleDescription("在链路连接、协议遥测、命令管理和扩展工具站之间切换。")
    tabs.setDocumentMode(True)
    tabs.setMovable(False)
    tabs.setUsesScrollButtons(True)
    tab_bar = AnimatedWorkspaceTabBar(tabs)
    tabs.setTabBar(tab_bar)
    tab_bar.setUsesScrollButtons(True)
    # The root layout may be compressed at the supported 980x680 minimum.
    # Let the tab viewport yield to that bounded height; each page already
    # owns a vertical QScrollArea, so the route strip cannot be overlapped by
    # a tab widget that still insists on its preferred 220px minimum.
    tabs.setMinimumHeight(0)
    tabs.setMaximumHeight(350)
    shell_layout.addWidget(tabs)

    route_strip = QFrame(workspace_shell)
    route_strip.setObjectName("workspaceRouteStrip")
    route_strip.setFixedHeight(31)
    route_layout = QHBoxLayout(route_strip)
    route_layout.setContentsMargins(0, 3, 6, 0)
    route_layout.setSpacing(0)
    focus_button = QToolButton(route_strip)
    focus_button.setObjectName("workspaceFocusButton")
    focus_button.setCheckable(True)
    focus_button.setText("专注设置")
    focus_button.setAccessibleName("工作区视图模式")
    focus_button.setAccessibleDescription(
        "展开配置工作区，临时收起实时观测、终端和发送区；不会断开连接或停止记录。"
    )
    focus_button.setToolTip(
        "展开配置工作区，临时收起实时观测、终端和发送区；不会断开连接或停止记录。"
    )
    focus_button.setMinimumWidth(88)
    focus_button.setMaximumWidth(108)
    focus_button.toggled.connect(partial(_toggle_workspace_focus, window))
    route_layout.addWidget(focus_button)
    context_label = WorkspaceContextLabel(route_strip)
    route_layout.addWidget(context_label)
    scroll_hint = WorkspaceScrollHint(route_strip)
    route_layout.addWidget(scroll_hint)
    route_layout.addStretch(1)
    route = WorkspaceRouteSurface(route_strip)
    route_layout.addWidget(route)
    shell_layout.addWidget(route_strip)

    connection_panel = build_connection_panel(
        window,
        catalog=window._connection_preset_catalog,
        apply_connection_preset=partial(apply_connection_preset, window),
        save_connection_preset=partial(save_custom_connection_preset, window),
        delete_connection_preset=partial(delete_custom_connection_preset, window),
        toggle_connection=partial(toggle_connection, window),
    )
    tabs.addTab(scroll_page(window, connection_panel, "connectionPage"), "链路 / 连接")
    protocol_page = scroll_page(
        window,
        build_protocol_panel_for_window(window),
        "protocolPage",
    )
    tabs.addTab(protocol_page, "协议 / 遥测")
    tabs.addTab(
        scroll_page(
            window,
            build_command_workspace(window),
            "commandPage",
            vertical_rhythm=ShortPageVerticalRhythm.TOP,
        ),
        "命令管理",
    )
    extension_panel = build_extension_panel()
    window._extension_station_overview = extension_panel.station_overview
    tabs.addTab(
        scroll_page(window, extension_panel.layout, "extensionPage"),
        "扩展 / 工具站",
    )
    tab_bar.set_workspace_tab_contract(
        (
            ("链路 / 连接", "链路"),
            ("协议 / 遥测", "协议"),
            ("命令管理", "命令"),
            ("扩展 / 工具站", "扩展"),
        )
    )
    refresh_workspace_tab_icons(tabs, theme_spec_for_widget(window))
    bindings = WorkspaceShellBindings(
        shell=workspace_shell,
        tabs=tabs,
        tab_bar=tab_bar,
        route=route,
        context_label=context_label,
        scroll_hint=scroll_hint,
        focus_button=focus_button,
    )
    window._workspace_bindings = bindings
    window._protocol_tab_index = tabs.indexOf(protocol_page)
    tabs.currentChanged.connect(partial(on_workspace_tab_changed, window))
    tabs.currentChanged.connect(partial(animate_workspace_transition, window))
    tabs.currentChanged.connect(partial(request_workspace_activity, window))
    on_workspace_tab_changed(window, tabs.currentIndex())
    return bindings

"""qss_sections 核心域分区生成器单元测试。

补强 test_qss_sections_layout / panels / widgets 未覆盖的 3 个 section 模块：
- qss_sections_core（global/window/labels/inputs/combos 5 函数）
- qss_sections_waveform.waveform_section（波形预览 + 李萨如 + 条形图）
- qss_sections_controls.controls_section（LED/滑块/仪表盘/Dashboard/微交互控件）

契约：objectName 覆盖 + 按钮三态（铁律 17）+ palette/tokens 引用（铁律 15）+ build_qss 集成。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.qss_sections_controls import controls_section
from embeddebug.serial_station.ui.theme.qss_sections_core import (
    combos_section,
    global_section,
    inputs_section,
    labels_section,
    window_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_waveform import waveform_section

# ── global_section / window_section / labels_section ─────────────────────


def test_global_section_sets_window_background_and_text():
    """全局 QWidget 用 BG_WINDOW 底 + TEXT_PRIMARY 主文本。"""

    qss = global_section()
    assert P.BG_WINDOW in qss
    assert P.TEXT_PRIMARY in qss
    assert ":disabled" in qss
    assert P.TEXT_DISABLED in qss


def test_global_section_tooltip_uses_raised_palette():
    """全局 tooltip：深底圆角 + BORDER_STRONG 边框。"""

    qss = global_section()
    assert "QToolTip" in qss
    assert P.BG_PANEL_RAISED in qss
    assert P.BORDER_STRONG in qss


def test_window_section_covers_root_and_title():
    """window_section 覆盖 QMainWindow + serialStationPyRoot + serialStationPyTitle。"""

    qss = window_section()
    assert "QMainWindow" in qss
    assert "#serialStationPyRoot" in qss
    assert "#serialStationPyTitle" in qss
    assert P.BG_APP in qss


def test_labels_section_default_label_uses_text_secondary():
    """默认 QLabel 用 TEXT_SECONDARY 次文本色。"""

    qss = labels_section()
    assert P.TEXT_SECONDARY in qss


def test_labels_section_log_stats_uses_mono_font():
    """日志统计标签用等宽字体 + TEXT_MUTED。"""

    qss = labels_section()
    assert "#serialStationLogStatsLabel" in qss
    assert T.FONT_FAMILY_MONO in qss
    assert P.TEXT_MUTED in qss


# ── inputs_section ───────────────────────────────────────────────────────


_INPUT_NAMES = (
    "serialStationSendEdit", "serialStationInjectEdit", "serialStationLogSearchEdit",
    "serialStationTcpHostEdit", "serialStationUdpPortEdit",
)


def test_inputs_section_covers_all_input_objectnames():
    qss = inputs_section()
    for name in _INPUT_NAMES:
        assert f"#{name}" in qss, f"inputs_section missing objectName: {name}"


def test_inputs_section_has_focus_hover_disabled_states():
    """QLineEdit 必须有 :focus/:hover/:disabled 状态。"""

    qss = inputs_section()
    assert "QLineEdit:hover" in qss
    assert "QLineEdit:focus" in qss
    assert "QLineEdit:disabled" in qss
    assert P.BG_INPUT in qss


def test_inputs_section_focus_uses_focus_border_palette():
    """输入框聚焦：强调青边框 + BG_INPUT_FOCUS 底。"""

    qss = inputs_section()
    assert P.BORDER_FOCUS in qss
    assert P.BG_INPUT_FOCUS in qss


# ── combos_section ───────────────────────────────────────────────────────


_COMBO_NAMES = (
    "serialStationProtocolCombo", "serialStationPortCombo", "serialStationBaudCombo",
    "serialStationLogFilterCombo",
)


def test_combos_section_covers_all_combo_objectnames():
    qss = combos_section()
    for name in _COMBO_NAMES:
        assert f"#{name}" in qss, f"combos_section missing objectName: {name}"


def test_combos_section_has_focus_hover_disabled_states():
    qss = combos_section()
    assert "QComboBox:hover" in qss
    assert "QComboBox:focus" in qss
    assert "QComboBox:disabled" in qss


def test_combos_section_dropdown_uses_css_triangle():
    """下拉箭头用 CSS border-triangle（不依赖文件路径，PyInstaller onedir 兼容）。"""

    qss = combos_section()
    assert "QComboBox::down-arrow" in qss
    assert "border-left" in qss
    assert "border-right" in qss
    assert "border-top" in qss
    assert "image: none" in qss


# ── waveform_section ────────────────────────────────────────────────────


_WAVEFORM_OBJECT_NAMES = (
    "serialStationWaveformPanel", "serialStationWaveformPlot", "serialStationWaveformLegend",
    "serialStationLissajousPanel", "serialStationBarChartPlot",
)


def test_waveform_section_covers_all_objectnames():
    qss = waveform_section()
    for name in _WAVEFORM_OBJECT_NAMES:
        assert f"#{name}" in qss, f"waveform_section missing objectName: {name}"


def test_waveform_section_plots_use_terminal_background():
    """3 个绘图区用 TERM_BACKGROUND（深底，与日志区协调）。"""

    qss = waveform_section()
    assert P.TERM_BACKGROUND in qss


def test_waveform_section_loading_overlay_uses_overlay_palette():
    """波形加载态覆盖层用 BG_OVERLAY 模态遮罩。"""

    qss = waveform_section()
    assert "#serialStationWaveformLoadingOverlay" in qss
    assert P.BG_OVERLAY in qss


# ── controls_section ────────────────────────────────────────────────────


def test_controls_section_covers_led_slider_dashboard_objects():
    """controls_section 覆盖 LED/滑块/仪表盘/数值显示/Dashboard 关键 objectName。"""

    qss = controls_section()
    key_names = (
        "serialStationStatusLed", "serialStationStatusDot", "serialStationCommandSlider",
        "serialStationGauge", "serialStationValueDisplay", "serialStationDashboardCanvas",
        "serialStationDashboardTabs", "serialStationWidgetPalette",
    )
    for name in key_names:
        assert f"#{name}" in qss, f"controls_section missing objectName: {name}"


def test_controls_section_configurable_button_has_three_states():
    """可配置按钮必须有三态 — 铁律 17。"""

    qss = controls_section()
    assert "#serialStationConfigurableButton:hover" in qss
    assert "#serialStationConfigurableButton:pressed" in qss
    assert "#serialStationConfigurableButton:disabled" in qss


def test_controls_section_dashboard_action_buttons_have_three_states():
    """Dashboard 5 个动作按钮（AddTab/Clear/Save/Load/Grid）都有三态。"""

    qss = controls_section()
    for btn in (
        "serialStationDashboardAddTabButton", "serialStationDashboardClearButton",
        "serialStationDashboardSaveButton", "serialStationDashboardLoadButton",
        "serialStationDashboardGridButton",
    ):
        for state in (":hover", ":pressed", ":disabled"):
            assert f"#{btn}{state}" in qss, f"{btn} missing {state}"


def test_controls_section_palette_button_has_dragging_state():
    """拖拽中源按钮高亮 [dragging=true] 选择器（Batch 34）。"""

    qss = controls_section()
    assert '#serialStationPaletteButton[dragging="true"]' in qss
    assert P.ACCENT_SOFT in qss


def test_controls_section_self_drawn_widgets_transparent():
    """微交互控件自绘，QSS 仅透明契约（Chip/Segmented/Drawer/Badge/StatusBar 等）。"""

    qss = controls_section()
    for name in (
        "serialStationChip", "serialStationSegmentedControl", "serialStationInfoBanner",
        "serialStationProgressRing", "serialStationRichTooltip", "serialStationDrawer",
        "serialStationBadge", "serialStationStatusBar", "serialStationToggleSwitch",
        "serialStationDivider",
    ):
        assert f"#{name}" in qss, f"controls_section missing self-drawn widget: {name}"
    assert "transparent" in qss


# build_qss 集成：core/waveform/controls 必须挂入主输出（与 layout/panels/widgets 契约一致）。


def test_build_qss_integrates_all_three_sections():
    qss = build_qss()
    # core
    assert "#serialStationPyRoot" in qss
    assert "#serialStationSendEdit" in qss
    assert "#serialStationProtocolCombo" in qss
    # waveform
    assert "#serialStationWaveformPanel" in qss
    assert "#serialStationLissajousPanel" in qss
    # controls
    assert "#serialStationDashboardCanvas" in qss
    assert "#serialStationConfigurableButton" in qss

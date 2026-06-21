"""QSS 分区生成器 — 控件组件库（LED/滑块/按钮/仪表盘/数值显示）。

对齐 VOFA+ 控件目录的样式。颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def controls_section() -> str:
    return f"""/* === Control Widgets (LED / Slider / Button / Gauge / Value) === */
/* 状态 LED：自绘圆形，QSS 仅控制容器背景。 */
QWidget#serialStationStatusLed {{
    background-color: transparent;
}}
/* Batch 10-1: 状态圆点（自绘 + PulseAnimation 呼吸），QSS 仅契约占位。 */
QWidget#serialStationStatusDot {{
    background-color: transparent;
}}
/* 命令滑块容器与子控件 */
QWidget#serialStationCommandSlider {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QLabel#serialStationSliderLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
QLabel#serialStationSliderValue {{
    color: {P.ACCENT};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
QSlider#serialStationSliderTrack::groove:horizontal {{
    background: {P.BG_INPUT};
    height: 6px;
    border-radius: 3px;
}}
QSlider#serialStationSliderTrack::sub-page:horizontal {{
    background: {P.ACCENT};
    border-radius: 3px;
}}
QSlider#serialStationSliderTrack::handle:horizontal {{
    background: {P.TEXT_PRIMARY};
    width: 14px;
    height: 14px;
    margin: -5px 0;
    border-radius: 7px;
}}
/* Batch 7-4: 滑块跟手气泡（拖拽时显示 value，跟随 handle） */
QLabel#serialStationSliderBubble {{
    background-color: {P.ACCENT};
    color: {P.TEXT_ON_ACCENT};
    border: none;
    border-radius: 8px;
    padding: 2px 6px;
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_XS};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}
/* Batch 7-5: RippleButton（ripple 自绘，QSS 背景透明让涟漪可见） */
QPushButton#serialStationRippleButton {{
    background-color: transparent;
    border: none;
}}
/* 可配置按钮 */
QPushButton#serialStationConfigurableButton {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.ACCENT_BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
}}
QPushButton#serialStationConfigurableButton:hover {{
    background-color: {P.ACCENT_SOFT};
    border-color: {P.ACCENT};
}}
QPushButton#serialStationConfigurableButton:pressed {{
    background-color: {P.BG_SELECTION};
}}
QPushButton#serialStationConfigurableButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
/* 仪表盘：自绘，QSS 仅控制背景。 */
QWidget#serialStationGauge {{
    background-color: transparent;
}}
/* 数值显示 */
QWidget#serialStationValueDisplay {{
    background-color: transparent;
}}
QLabel#serialStationValueLabel {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
}}
QLabel#serialStationValueNumber {{
    color: {P.TEXT_PRIMARY};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}
QLabel#serialStationValueTrend {{
    background-color: transparent;
}}
/* === Dashboard（画布/控件库/标签页） === */
QFrame#serialStationDashboardCanvas {{
    background-color: {P.BG_APP};
    border: 1px dashed {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
QTabWidget#serialStationDashboardTabs::pane {{
    background-color: {P.BG_APP};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QTabWidget#serialStationDashboardTabs::tab-bar {{
    alignment: left;
}}
QTabBar#serialStationDashboardTabs {{
    background: transparent;
}}
QFrame#serialStationWidgetPalette {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
QLabel#serialStationPaletteTitle {{
    color: {P.TEXT_PRIMARY};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    font-size: {T.FONT_BASE};
}}
QPushButton#serialStationPaletteButton {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_SECONDARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_SM};
    text-align: left;
}}
QPushButton#serialStationPaletteButton:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-color: {P.ACCENT_BORDER};
    color: {P.TEXT_PRIMARY};
}}
/* Batch 34: 拖拽中源按钮高亮（accent 边框 + 半透明填充）。 */
QPushButton#serialStationPaletteButton[dragging="true"] {{
    background-color: {P.ACCENT_SOFT};
    border: {T.BORDER_THIN} solid {P.ACCENT};
    color: {P.TEXT_PRIMARY};
}}
/* Batch 17: DashboardPanel 顶栏控件 + 主体容器 */
QWidget#serialStationDashboardPanel {{
    background-color: transparent;
}}
QFrame#serialStationDashboardPalette {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
QPushButton#serialStationDashboardAddTabButton,
QPushButton#serialStationDashboardClearButton,
QPushButton#serialStationDashboardSaveButton,
QPushButton#serialStationDashboardLoadButton,
QPushButton#serialStationDashboardGridButton {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.ACCENT_BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_SM};
}}
QPushButton#serialStationDashboardAddTabButton:hover,
QPushButton#serialStationDashboardClearButton:hover,
QPushButton#serialStationDashboardSaveButton:hover,
QPushButton#serialStationDashboardLoadButton:hover,
QPushButton#serialStationDashboardGridButton:hover {{
    background-color: {P.ACCENT_SOFT};
    border-color: {P.ACCENT};
}}
QPushButton#serialStationDashboardAddTabButton:pressed,
QPushButton#serialStationDashboardClearButton:pressed,
QPushButton#serialStationDashboardSaveButton:pressed,
QPushButton#serialStationDashboardLoadButton:pressed,
QPushButton#serialStationDashboardGridButton:pressed {{
    background-color: {P.BG_SELECTION};
    border-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationDashboardAddTabButton:disabled,
QPushButton#serialStationDashboardClearButton:disabled,
QPushButton#serialStationDashboardSaveButton:disabled,
QPushButton#serialStationDashboardLoadButton:disabled,
QPushButton#serialStationDashboardGridButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
QLabel#serialStationDashboardStatusLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
/* Batch 40: 新增微交互控件。控件均自绘主体（paintEvent），QSS 仅契约占位
   「background-color: transparent」，避免 QWidget 默认底破坏色相；具体配色
   由 palette + paintEvent 完成，集中管理（05-ui-standard §颜色集中管理）。 */
QWidget#serialStationChip,
QWidget#serialStationSegmentedControl,
QWidget#serialStationInfoBanner,
QWidget#serialStationProgressRing,
QWidget#serialStationRichTooltip {{
    background-color: transparent;
}}
/* Batch 41: 新增控件/工具面板。控件均自绘主体或自带 QSS，此处仅契约占位。 */
QWidget#serialStationDrawer,
QWidget#serialStationDrawerPanel,
QWidget#serialStationBadge,
QWidget#serialStationCrcCalculator,
QWidget#serialStationCrcCalculatorInput,
QWidget#serialStationCrcCalculatorInputEdit,
QWidget#serialStationCrcCalculatorConfig,
QWidget#serialStationCrcCalculatorResult,
QLineEdit#serialStationCrcResultEdit {{
    background-color: transparent;
}}
/* Batch 42: 新增 3 个工具面板。均自带本地 QSS，此处仅契约占位满足覆盖率测试。 */
QWidget#serialStationTimestampConverter,
QWidget#serialStationTimestampConverterConfig,
QWidget#serialStationTimestampConverterEpochEdit,
QWidget#serialStationTimestampConverterInput,
QWidget#serialStationTimestampConverterNowButton,
QWidget#serialStationTimestampConverterResult,
QLineEdit#serialStationTimestampConverterResultEdit,
QWidget#serialStationHexViewer,
QWidget#serialStationHexViewerCount,
QWidget#serialStationHexViewerDump,
QWidget#serialStationHexViewerFormatBtn,
QWidget#serialStationHexViewerInput,
QWidget#serialStationHexViewerInputEdit,
QWidget#serialStationHexViewerStatus,
QWidget#serialStationHexViewerView,
QWidget#serialStationByteFreqAnalyzer,
QWidget#serialStationByteFreqAnalyzerAnalyzeBtn,
QWidget#serialStationByteFreqAnalyzerClearBtn,
QWidget#serialStationByteFreqAnalyzerInput,
QWidget#serialStationByteFreqAnalyzerInputEdit,
QWidget#serialStationByteFreqAnalyzerResult,
QWidget#serialStationByteFreqAnalyzerStatsLabel,
QWidget#serialStationByteFreqCanvas {{
    background-color: transparent;
}}
/* Batch 47: StatusBar 组件。自绘背景，QSS 仅契约占位。 */
QWidget#serialStationStatusBar {{
    background-color: transparent;
}}
/* Batch 47: KeyboardShortcut 徽章。自带样式，QSS 仅契约占位。 */
QLabel#serialStationKeyHint {{
    background-color: transparent;
}}
/* Batch 48: ToggleSwitch。自绘，QSS 仅契约占位。 */
QWidget#serialStationToggleSwitch {{
    background-color: transparent;
}}
/* Batch 48: Divider 分隔线。自绘，QSS 仅契约占位。 */
QWidget#serialStationDivider {{
    background-color: transparent;
}}
QLabel#serialStationDividerLabel {{
    background-color: transparent;
}}"""

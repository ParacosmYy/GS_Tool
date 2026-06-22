"""qss_sections 各域面板分区生成器单元测试。

补强 test_qss_sections_layout 未覆盖的 7 个 section 函数（无 Qt 依赖，纯字符串）：
- qss_sections_app.nav_rail_section / ota_section（NavRail + OTA 面板）
- qss_sections_domain.domain_panels_section（RTT/CAN/BLE/Automation/Settings/SVD）
- qss_sections_tools.tools_section（CRC/HexViewer/ByteFrequency/Timestamp 工具）
- qss_sections_empty.empty_state_section（空状态 + 骨架屏 + 加载态覆盖层）
- qss_sections_toast.toast_section（Toast 卡片 + 容器）
- qss_sections_log_options.log_options_section（日志工具条场景实例）

验证维度（对齐 test_qss_sections_layout 既有契约）：
1. 各 objectName 都有对应选择器（防 objectName 漂移导致 QSS 失效）。
2. 主操作按钮有三态（hover/pressed/disabled）— 铁律 17。
3. 引用 palette/tokens 而非硬编码颜色（铁律 15）。
4. build_qss 已接入对应 section（防分区漏挂主输出）。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.qss_sections_app import (
    nav_rail_section,
    ota_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_domain import domain_panels_section
from embeddebug.serial_station.ui.theme.qss_sections_empty import empty_state_section
from embeddebug.serial_station.ui.theme.qss_sections_log_options import (
    log_options_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_toast import toast_section
from embeddebug.serial_station.ui.theme.qss_sections_tools import tools_section

# ── NavRail（qss_sections_app.nav_rail_section） ─────────────────────────

_NAV_RAIL_OBJECT_NAMES = (
    "serialStationNavRail",
    "serialStationNavBrand",
    "serialStationNavIndicator",
)


def test_nav_rail_section_covers_all_objectnames():
    qss = nav_rail_section()
    for name in _NAV_RAIL_OBJECT_NAMES:
        assert f"#{name}" in qss, f"nav_rail_section missing objectName: {name}"


def test_nav_rail_section_uses_brand_gradient_palette():
    """品牌徽标 ED 应引用青→蓝品牌渐变 + 反白字。"""

    qss = nav_rail_section()
    assert P.ACCENT_GRADIENT_FROM in qss
    assert P.ACCENT_GRADIENT_TO in qss
    assert P.TEXT_INVERTED in qss
    assert "qlineargradient" in qss


def test_nav_rail_section_checked_state_uses_accent_indicator():
    """导航按钮激活态：强调青软底 + 左指示条。"""

    qss = nav_rail_section()
    assert ":checked" in qss
    assert P.ACCENT_SOFT in qss
    assert P.ACCENT in qss


def test_nav_rail_section_indicator_transparent():
    """NavIndicator 自绘胶囊指示条 — QSS 仅契约占位（透明背景）。"""

    qss = nav_rail_section()
    assert "#serialStationNavIndicator" in qss
    assert "transparent" in qss


# ── OTA（qss_sections_app.ota_section） ──────────────────────────────────

_OTA_OBJECT_NAMES = (
    "serialStationOtaPanel",
    "serialStationOtaFieldLabel",
    "serialStationOtaProtocolCombo",
    "serialStationOtaFileEdit",
    "serialStationOtaFileLabel",
    "serialStationOtaProgress",
    "serialStationOtaLog",
    "serialStationOtaStatusLabel",
    "serialStationOtaBrowseButton",
    "serialStationOtaStartButton",
)


def test_ota_section_covers_all_objectnames():
    qss = ota_section()
    for name in _OTA_OBJECT_NAMES:
        assert f"#{name}" in qss, f"ota_section missing objectName: {name}"


def test_ota_section_start_button_has_three_states():
    """主操作按钮（Start）必须有 hover/pressed/disabled 三态 — 铁律 17。"""

    qss = ota_section()
    assert "#serialStationOtaStartButton" in qss
    assert "#serialStationOtaStartButton:hover" in qss
    assert "#serialStationOtaStartButton:pressed" in qss
    assert "#serialStationOtaStartButton:disabled" in qss


def test_ota_section_browse_button_has_three_states():
    """次操作按钮（Browse）必须有 hover/pressed/disabled 三态。"""

    qss = ota_section()
    assert "#serialStationOtaBrowseButton:hover" in qss
    assert "#serialStationOtaBrowseButton:pressed" in qss
    assert "#serialStationOtaBrowseButton:disabled" in qss


def test_ota_section_progress_chunk_uses_accent():
    """进度条填充块用强调青（视觉进度反馈）。"""

    qss = ota_section()
    assert "#serialStationOtaProgress::chunk" in qss
    assert P.ACCENT in qss


def test_ota_section_log_uses_terminal_palette():
    """OTA 日志区终端风：深底 + 系统日志色 + 等宽字体。"""

    qss = ota_section()
    assert P.TERM_BACKGROUND in qss
    assert P.TERM_SYSTEM in qss
    assert T.FONT_FAMILY_MONO in qss


# ── Domain Panels（qss_sections_domain.domain_panels_section） ───────────

_DOMAIN_PANEL_OBJECT_NAMES = (
    "serialStationRttPanel",
    "serialStationCanPanel",
    "serialStationBlePanel",
    "serialStationAutomationPanel",
    "serialStationSettingsPanel",
    "serialStationSvdPanel",
)
_DOMAIN_BUTTON_OBJECT_NAMES = (
    "serialStationRttStartButton",
    "serialStationCanSendButton",
    "serialStationBleConnectButton",
    "serialStationAutomationRunButton",
    "serialStationSettingsApplyButton",
)


def test_domain_panels_section_covers_all_panel_roots():
    qss = domain_panels_section()
    for name in _DOMAIN_PANEL_OBJECT_NAMES:
        assert f"#{name}" in qss, f"domain_panels_section missing panel root: {name}"


def test_domain_panels_section_all_action_buttons_have_three_states():
    """5 个域主操作按钮（启停/连接/发送/应用）都必须有三态 — 铁律 17。"""

    qss = domain_panels_section()
    for btn in _DOMAIN_BUTTON_OBJECT_NAMES:
        assert f"#{btn}" in qss, f"domain_panels_section missing button: {btn}"
        assert f"#{btn}:hover" in qss, f"{btn} missing :hover"
        assert f"#{btn}:pressed" in qss, f"{btn} missing :pressed"
        assert f"#{btn}:disabled" in qss, f"{btn} missing :disabled"


def test_domain_panels_section_panels_use_app_background():
    """域面板根统一深色应用底（与 NavRail 协调）。"""

    qss = domain_panels_section()
    assert P.BG_APP in qss


def test_domain_panels_section_text_views_use_terminal_palette():
    """域日志/文本视图终端风：深底 + 系统日志色。"""

    qss = domain_panels_section()
    assert P.TERM_BACKGROUND in qss
    assert P.TERM_SYSTEM in qss


def test_domain_panels_section_field_labels_use_text_secondary():
    """域字段标签（通道/设备/ID/数据等）用次文本色 + 小字号。"""

    qss = domain_panels_section()
    assert P.TEXT_SECONDARY in qss
    assert T.FONT_SM in qss


# ── Tools（qss_sections_tools.tools_section） ────────────────────────────

_TOOLS_GROUPS = ("Crc", "ByteFreq", "HexViewer", "Timestamp")
_TOOLS_SAMPLE_NAMES = (
    "serialStationCrcCopyButton",
    "serialStationCrcWidthCombo",
    "serialStationByteFreqModeHex",
    "serialStationHexViewerModeAscii",
    "serialStationTimestampFmtMs",
    "serialStationTimestampTzCombo",
)


def test_tools_section_covers_sample_objectnames():
    """tools_section 生成所有 4 个工具面板的关键 objectName 选择器。"""

    qss = tools_section()
    for name in _TOOLS_SAMPLE_NAMES:
        assert f"#{name}" in qss, f"tools_section missing objectName: {name}"


def test_tools_section_covers_multiple_widget_types():
    """tools_section 为每个 objectName 生成 6 种控件类型选择器（契约占位）。"""

    qss = tools_section()
    # 每个 name 应有 QWidget/QLabel/QRadioButton/QComboBox/QLineEdit/QCheckBox/QPushButton 7 行。
    for widget_type in ("QWidget", "QLabel", "QRadioButton", "QComboBox",
                        "QLineEdit", "QCheckBox", "QPushButton"):
        assert widget_type in qss, f"tools_section missing widget type: {widget_type}"


def test_tools_section_returns_nonempty_contract():
    """tools_section 至少含所有 4 个工具组（CRC/ByteFreq/HexViewer/Timestamp）。"""

    qss = tools_section()
    for group in _TOOLS_GROUPS:
        assert group in qss, f"tools_section missing tool group marker: {group}"


# ── Empty State（qss_sections_empty.empty_state_section） ────────────────

_EMPTY_STATE_OBJECT_NAMES = (
    "serialStationEmptyState",
    "serialStationEmptyStateIcon",
    "serialStationEmptyStateTitle",
    "serialStationEmptyStateDescription",
    "serialStationEmptyStateCta",
    "serialStationSkeleton",
    "serialStationSkeletonBlock",
    "serialStationLogLoadingOverlay",
    "serialStationLogLoadingSkeleton",
    "serialStationLogLoadingLabel",
    "serialStationButtonLoadingRing",
)


def test_empty_state_section_covers_all_objectnames():
    qss = empty_state_section()
    for name in _EMPTY_STATE_OBJECT_NAMES:
        assert f"#{name}" in qss, f"empty_state_section missing objectName: {name}"


def test_empty_state_section_cta_has_three_states():
    """空状态 CTA 按钮必须有三态 — 铁律 17。"""

    qss = empty_state_section()
    assert "#serialStationEmptyStateCta:hover" in qss
    assert "#serialStationEmptyStateCta:pressed" in qss
    assert "#serialStationEmptyStateCta:disabled" in qss


def test_empty_state_section_loading_overlay_uses_overlay_palette():
    """日志加载态覆盖层用 BG_OVERLAY 模态遮罩色。"""

    qss = empty_state_section()
    assert "#serialStationLogLoadingOverlay" in qss
    assert P.BG_OVERLAY in qss


def test_empty_state_section_title_uses_text_primary():
    """空状态标题用主文本色 + 大字号（拉开视觉层级）。"""

    qss = empty_state_section()
    assert P.TEXT_PRIMARY in qss
    assert T.FONT_LG in qss


# ── Toast（qss_sections_toast.toast_section） ────────────────────────────

_TOAST_OBJECT_NAMES = (
    "serialStationToast",
    "serialStationToastAccent",
    "serialStationToastGlyph",
    "serialStationToastTitle",
    "serialStationToastMessage",
    "serialStationToastCloseButton",
    "serialStationToastContainer",
    "serialStationToastPlaceholder",
)


def test_toast_section_covers_all_objectnames():
    qss = toast_section()
    for name in _TOAST_OBJECT_NAMES:
        assert f"#{name}" in qss, f"toast_section missing objectName: {name}"


def test_toast_section_card_uses_raised_panel_palette():
    """Toast 卡片底用 BG_PANEL_RAISED（浮起视觉），边框用 BORDER_STRONG。"""

    qss = toast_section()
    assert P.BG_PANEL_RAISED in qss
    assert P.BORDER_STRONG in qss


def test_toast_section_title_uses_semibold_weight():
    """Toast 标题用 SEMIBOLD 字重（视觉强调）。"""

    qss = toast_section()
    assert T.FONT_WEIGHT_SEMIBOLD in qss


def test_toast_section_close_button_has_hover():
    """关闭按钮 hover 提亮到 TEXT_PRIMARY（可达性反馈）。"""

    qss = toast_section()
    assert "#serialStationToastCloseButton:hover" in qss
    assert P.TEXT_PRIMARY in qss


# ── Log Options（qss_sections_log_options.log_options_section） ──────────

_LOG_OPTIONS_OBJECT_NAMES = (
    "serialStationLogConnectionBadge",
    "serialStationActiveFilterChip",
    "serialStationAutoScrollToggle",
    "serialStationLogViewModeSegmented",
    "serialStationLogInfoBanner",
    "serialStationHistoryDrawer",
    "serialStationHistoryPlaceholder",
    "serialStationAutoScrollLabel",
    "serialStationHistoryButton",
)


def test_log_options_section_covers_all_objectnames():
    qss = log_options_section()
    for name in _LOG_OPTIONS_OBJECT_NAMES:
        assert f"#{name}" in qss, f"log_options_section missing objectName: {name}"


def test_log_options_section_history_button_has_three_states():
    """历史按钮必须有三态 — 铁律 17。"""

    qss = log_options_section()
    assert "#serialStationHistoryButton:hover" in qss
    assert "#serialStationHistoryButton:pressed" in qss
    assert "#serialStationHistoryButton:disabled" in qss


def test_log_options_section_self_drawn_widgets_transparent():
    """Badge/Chip/ToggleSwitch/Segmented/InfoBanner/Drawer 是自绘控件，QSS 仅透明契约。"""

    qss = log_options_section()
    self_drawn = (
        "serialStationLogConnectionBadge",
        "serialStationActiveFilterChip",
        "serialStationAutoScrollToggle",
        "serialStationLogViewModeSegmented",
        "serialStationLogInfoBanner",
        "serialStationHistoryDrawer",
    )
    for name in self_drawn:
        assert f"#{name}" in qss
    # 至少有一处 transparent（自绘契约占位）。
    assert "transparent" in qss


def test_log_options_section_history_button_hover_uses_accent_soft():
    """历史按钮 hover 转强调青软底 + 强调青文字。"""

    qss = log_options_section()
    assert P.ACCENT_SOFT in qss
    assert P.ACCENT_HOVER in qss


# ── build_qss 集成（所有 7 个 section 必须挂入主输出） ───────────────────


def test_build_qss_integrates_nav_rail_and_ota_sections():
    qss = build_qss()
    assert "#serialStationNavRail" in qss
    assert "#serialStationOtaPanel" in qss


def test_build_qss_integrates_domain_panels_section():
    qss = build_qss()
    assert "#serialStationRttPanel" in qss
    assert "#serialStationBlePanel" in qss
    assert "#serialStationSettingsPanel" in qss


def test_build_qss_integrates_tools_section():
    qss = build_qss()
    # tools_section 生成所有工具 objectName，验证关键样本。
    assert "#serialStationCrcCopyButton" in qss
    assert "#serialStationTimestampTzCombo" in qss


def test_build_qss_integrates_empty_state_section():
    qss = build_qss()
    assert "#serialStationEmptyState" in qss
    assert "#serialStationSkeleton" in qss


def test_build_qss_integrates_toast_section():
    qss = build_qss()
    assert "#serialStationToast" in qss
    assert "#serialStationToastContainer" in qss


def test_build_qss_integrates_log_options_section():
    qss = build_qss()
    assert "#serialStationHistoryButton" in qss
    assert "#serialStationAutoScrollToggle" in qss

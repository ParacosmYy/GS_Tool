"""qss_sections 面板域分区生成器单元测试（NavRail / OTA / Domain）。

补强 test_qss_sections_layout 未覆盖的 3 个 section 函数（无 Qt 依赖，纯字符串）：
- qss_sections_app.nav_rail_section（NavRail 导航栏 + 品牌徽标 + 指示条契约）
- qss_sections_app.ota_section（OTA 升级面板：文件/协议/进度/日志/按钮）
- qss_sections_domain.domain_panels_section（RTT/CAN/BLE/Automation/Settings/SVD）

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


# ── build_qss 集成（NavRail / OTA / Domain 必须挂入主输出） ──────────────


def test_build_qss_integrates_nav_rail_and_ota_sections():
    qss = build_qss()
    assert "#serialStationNavRail" in qss
    assert "#serialStationOtaPanel" in qss


def test_build_qss_integrates_domain_panels_section():
    qss = build_qss()
    assert "#serialStationRttPanel" in qss
    assert "#serialStationBlePanel" in qss
    assert "#serialStationSettingsPanel" in qss

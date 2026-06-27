"""QSS widgets 分区聚合测试。

覆盖按钮、日志视图、状态、滚动条、快捷键，以及 tools/empty/toast/log-options
组件分区。原先拆成 widgets / widgets_states 两个文件，这里按域合并。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.qss_sections_empty import empty_state_section
from embeddebug.serial_station.ui.theme.qss_sections_log_options import (
    log_options_section,
)
from embeddebug.serial_station.ui.theme.qss_sections_toast import toast_section
from embeddebug.serial_station.ui.theme.qss_sections_tools import tools_section
from embeddebug.serial_station.ui.theme.qss_sections_widgets import (
    buttons_section,
    log_view_section,
    plaintext_section,
    scrollbar_section,
    shortcut_section,
    status_section,
)

_CONNECT_BUTTONS = (
    "serialStationConnectButton",
    "serialStationConnectSerialButton",
    "serialStationConnectTcpButton",
    "serialStationConnectUdpButton",
)
_TOOL_BUTTONS = (
    "serialStationRefreshPortsButton",
    "serialStationExportLogButton",
    "serialStationReplayLogButton",
    "serialStationSaveProfileButton",
    "serialStationLoadProfileButton",
    "serialStationClearButton",
    "serialStationInjectButton",
    "serialStationSendButton",
)


def test_buttons_section_default_button_states_and_focus():
    qss = buttons_section()

    for selector in (
        "QPushButton:hover",
        "QPushButton:pressed",
        "QPushButton:disabled",
        "QPushButton:focus",
    ):
        assert selector in qss
    assert P.BG_PANEL in qss
    assert P.ACCENT_BORDER in qss


@pytest.mark.parametrize("name", _CONNECT_BUTTONS)
def test_buttons_section_connect_buttons_have_brand_palette_and_states(name):
    qss = buttons_section()

    assert f"#{name}" in qss
    for state in (":hover", ":pressed", ":disabled"):
        assert f"#{name}{state}" in qss
    assert P.ACCENT_GRADIENT in qss
    assert P.TEXT_ON_ACCENT in qss


def test_buttons_section_disconnect_button_uses_error_palette_and_states():
    qss = buttons_section()

    assert "#serialStationDisconnectButton" in qss
    assert P.ERROR_SOFT in qss
    assert P.ERROR in qss
    for state in (":hover", ":pressed", ":disabled"):
        assert f"#serialStationDisconnectButton{state}" in qss


@pytest.mark.parametrize("name", _TOOL_BUTTONS)
def test_buttons_section_tool_buttons_have_three_states(name):
    qss = buttons_section()

    for state in (":hover", ":pressed", ":disabled"):
        assert f"#{name}{state}" in qss


def test_buttons_section_send_inject_use_terminal_tx_blue():
    qss = buttons_section()

    assert P.TERM_TX in qss
    assert "#serialStationSendButton" in qss
    assert "#serialStationInjectButton" in qss


@pytest.mark.parametrize(
    ("factory", "required"),
    [
        (log_view_section, ("#serialStationLogView", P.TERM_BACKGROUND, T.FONT_FAMILY_MONO)),
        (status_section, ("#serialStationStatusLabel", P.WARNING, T.RADIUS_PILL)),
        (status_section, ("#serialStationProfileLabel", P.TEXT_MUTED)),
        (scrollbar_section, ("QScrollBar:vertical", "QScrollBar::handle:horizontal:hover")),
        (plaintext_section, ("QPlainTextEdit", P.BG_INPUT, T.RADIUS_MD)),
        (shortcut_section, ("#serialStationSendShortcut", "#serialStationCommandPaletteShortcut")),
        (tools_section, ("#serialStationCrcCopyButton", "#serialStationTimestampTzCombo")),
        (empty_state_section, ("#serialStationEmptyState", "#serialStationEmptyStateCta:hover")),
        (toast_section, ("#serialStationToast", "#serialStationToastCloseButton:hover")),
        (log_options_section, ("#serialStationHistoryButton", "#serialStationHistoryButton:disabled")),
    ],
)
def test_widget_sections_keep_core_contracts(factory, required):
    qss = factory()

    for token in required:
        assert token in qss


@pytest.mark.parametrize(
    ("factory", "tokens"),
    [
        (log_view_section, (P.TERM_SYSTEM, P.ACCENT_SOFT)),
        (scrollbar_section, (P.SCROLLBAR, P.SCROLLBAR_HOVER, "height: 0", "width: 0")),
        (tools_section, ("QWidget", "QPushButton", "Crc", "Timestamp")),
        (empty_state_section, (P.BG_OVERLAY, P.TEXT_PRIMARY, T.FONT_LG)),
        (toast_section, (P.BG_PANEL_RAISED, P.BORDER_STRONG, T.FONT_WEIGHT_SEMIBOLD)),
        (log_options_section, ("transparent", P.ACCENT_SOFT, P.ACCENT_HOVER)),
    ],
)
def test_widget_sections_keep_palette_and_state_details(factory, tokens):
    qss = factory()

    for token in tokens:
        assert token in qss


def test_build_qss_integrates_all_widget_sections():
    qss = build_qss()

    for selector in (
        "#serialStationConnectButton",
        "#serialStationDisconnectButton",
        "#serialStationLogView",
        "#serialStationStatusLabel",
        "QScrollBar:vertical",
        "#serialStationSendShortcut",
        "#serialStationCrcCopyButton",
        "#serialStationEmptyState",
        "#serialStationToast",
        "#serialStationHistoryButton",
    ):
        assert selector in qss

"""qss_sections_widgets 交互控件分区单元测试。

补强 test_qss_sections_layout / panels / core 未覆盖的 6 个 section 函数：
- buttons_section（默认按钮 + 4 连接按钮渐变 + 断开危险红 + 8 工具按钮 + 发送/注入终端蓝）
- log_view_section（日志视图终端风）
- status_section（状态标签警告药丸 + Profile 标签）
- scrollbar_section（垂直/水平滚动条胶囊）
- plaintext_section（QPlainTextEdit 兜底样式）
- shortcut_section（QShortcut objectName 契约占位）

契约：objectName 覆盖 + 按钮三态（铁律 17）+ palette/tokens 引用（铁律 15）+ build_qss 集成。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.qss_sections_widgets import (
    buttons_section,
    log_view_section,
    plaintext_section,
    scrollbar_section,
    shortcut_section,
    status_section,
)

# ── buttons_section ─────────────────────────────────────────────────────

_CONNECT_BUTTONS = (
    "serialStationConnectButton", "serialStationConnectSerialButton",
    "serialStationConnectTcpButton", "serialStationConnectUdpButton",
)
_TOOL_BUTTONS = (
    "serialStationRefreshPortsButton", "serialStationExportLogButton",
    "serialStationReplayLogButton", "serialStationSaveProfileButton",
    "serialStationLoadProfileButton", "serialStationClearButton",
    "serialStationInjectButton", "serialStationSendButton",
)


def test_buttons_section_default_button_has_three_states():
    """默认 QPushButton 必须有 :hover/:pressed/:disabled 三态 — 铁律 17。"""

    qss = buttons_section()
    assert "QPushButton:hover" in qss
    assert "QPushButton:pressed" in qss
    assert "QPushButton:disabled" in qss
    assert P.BG_PANEL in qss


def test_buttons_section_default_button_has_focus_state():
    """默认按钮 :focus 用 ACCENT_BORDER 边框（键盘可达性）。"""

    qss = buttons_section()
    assert "QPushButton:focus" in qss
    assert P.ACCENT_BORDER in qss


def test_buttons_section_connect_buttons_use_brand_gradient():
    """4 个连接按钮用 ACCENT_GRADIENT 品牌渐变 + TEXT_ON_ACCENT。"""

    qss = buttons_section()
    for name in _CONNECT_BUTTONS:
        assert f"#{name}" in qss, f"buttons_section missing connect button: {name}"
    assert P.ACCENT_GRADIENT in qss
    assert P.TEXT_ON_ACCENT in qss


def test_buttons_section_connect_buttons_have_three_states():
    """4 个连接按钮都有 :hover/:pressed/:disabled 三态。"""

    qss = buttons_section()
    for name in _CONNECT_BUTTONS:
        for state in (":hover", ":pressed", ":disabled"):
            assert f"#{name}{state}" in qss, f"{name} missing {state}"


def test_buttons_section_disconnect_button_uses_error_palette():
    """断开按钮用 ERROR 危险红 + ERROR_SOFT 底，有三态。"""

    qss = buttons_section()
    assert "#serialStationDisconnectButton" in qss
    assert P.ERROR_SOFT in qss
    assert P.ERROR in qss
    for state in (":hover", ":pressed", ":disabled"):
        assert f"#serialStationDisconnectButton{state}" in qss


def test_buttons_section_tool_buttons_have_three_states():
    """8 个次要工具按钮都有 :hover/:pressed/:disabled。"""

    qss = buttons_section()
    for name in _TOOL_BUTTONS:
        for state in (":hover", ":pressed", ":disabled"):
            assert f"#{name}{state}" in qss, f"{name} missing {state}"


def test_buttons_section_send_inject_use_terminal_tx_blue():
    """发送/注入按钮用 TERM_TX 终端蓝（区分 TX 方向）。"""

    qss = buttons_section()
    assert P.TERM_TX in qss
    assert "#serialStationSendButton" in qss
    assert "#serialStationInjectButton" in qss


# ── log_view_section ────────────────────────────────────────────────────


def test_log_view_section_uses_terminal_palette():
    """日志视图终端风：TERM_BACKGROUND + TERM_SYSTEM + 等宽字体。"""

    qss = log_view_section()
    assert "#serialStationLogView" in qss
    assert P.TERM_BACKGROUND in qss
    assert P.TERM_SYSTEM in qss
    assert T.FONT_FAMILY_MONO in qss


def test_log_view_section_focus_uses_accent_border():
    """日志视图聚焦用 ACCENT_BORDER 边框。"""

    qss = log_view_section()
    assert "#serialStationLogView:focus" in qss
    assert P.ACCENT_BORDER in qss


def test_log_view_section_selection_uses_accent_soft():
    """日志视图选中文字用 ACCENT_SOFT 底。"""

    qss = log_view_section()
    assert P.ACCENT_SOFT in qss


# ── status_section ──────────────────────────────────────────────────────


def test_status_section_status_label_uses_warning_palette():
    """状态标签用 WARNING 警告药丸 + WARNING_BORDER + WARNING_SOFT 底。"""

    qss = status_section()
    assert "#serialStationStatusLabel" in qss
    assert P.WARNING in qss
    assert P.WARNING_BORDER in qss
    assert P.WARNING_SOFT in qss
    assert T.RADIUS_PILL in qss


def test_status_section_profile_label_uses_muted_text():
    """Profile 标签用 TEXT_MUTED 弱文本色。"""

    qss = status_section()
    assert "#serialStationProfileLabel" in qss
    assert P.TEXT_MUTED in qss


# ── scrollbar_section ───────────────────────────────────────────────────


def test_scrollbar_section_vertical_uses_scrollbar_palette():
    """垂直滚动条 handle 用 SCROLLBAR 色 + hover 提亮到 SCROLLBAR_HOVER。"""

    qss = scrollbar_section()
    assert "QScrollBar:vertical" in qss
    assert "QScrollBar::handle:vertical" in qss
    assert P.SCROLLBAR in qss
    assert "QScrollBar::handle:vertical:hover" in qss
    assert P.SCROLLBAR_HOVER in qss


def test_scrollbar_section_horizontal_mirror_vertical():
    """水平滚动条镜像垂直规格（8px 胶囊 + hover）。"""

    qss = scrollbar_section()
    assert "QScrollBar:horizontal" in qss
    assert "QScrollBar::handle:horizontal" in qss
    assert "QScrollBar::handle:horizontal:hover" in qss


def test_scrollbar_section_add_sub_line_hidden():
    """滚动条 add-line/sub-line 隐藏（height/width=0，无步进按钮）。"""

    qss = scrollbar_section()
    assert "QScrollBar::add-line:vertical" in qss
    assert "QScrollBar::sub-line:vertical" in qss
    assert "height: 0" in qss
    assert "width: 0" in qss


# ── plaintext_section ───────────────────────────────────────────────────


def test_plaintext_section_uses_input_palette():
    """QPlainTextEdit 兜底用 BG_INPUT + BORDER_STRONG + RADIUS_MD。"""

    qss = plaintext_section()
    assert "QPlainTextEdit" in qss
    assert P.BG_INPUT in qss
    assert P.BORDER_STRONG in qss
    assert T.RADIUS_MD in qss


# ── shortcut_section ────────────────────────────────────────────────────


_SHORTCUT_NAMES = (
    "serialStationSendShortcut", "serialStationClearShortcut",
    "serialStationRefreshPortsShortcut", "serialStationCommandPaletteShortcut",
)


def test_shortcut_section_covers_all_objectnames():
    """shortcut_section 覆盖 4 个 QShortcut objectName 契约占位。"""

    qss = shortcut_section()
    for name in _SHORTCUT_NAMES:
        assert f"#{name}" in qss, f"shortcut_section missing objectName: {name}"


# ── build_qss 集成（6 个 section 必须挂入主输出） ────────────────────────


def test_build_qss_integrates_all_widgets_sections():
    qss = build_qss()
    # buttons
    assert "#serialStationConnectButton" in qss
    assert "#serialStationDisconnectButton" in qss
    # log_view
    assert "#serialStationLogView" in qss
    # status
    assert "#serialStationStatusLabel" in qss
    # scrollbar
    assert "QScrollBar:vertical" in qss
    # shortcut
    assert "#serialStationSendShortcut" in qss

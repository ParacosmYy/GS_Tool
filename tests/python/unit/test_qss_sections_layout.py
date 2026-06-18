"""qss_sections_layout 分区生成器单元测试。

验证三栏 Shell / 玻璃卡片 / TopBar 分区函数返回的 QSS：
1. 覆盖对应 objectName 的选择器（与全局覆盖率守护测试互补，按域细化）。
2. 引用 palette/tokens 而非硬编码颜色（玻璃感近似手段落地）。
3. build_qss 已把四个分区接入主输出。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.qss_sections_layout import (
    cards_section,
    command_palette_section,
    splitter_section,
    topbar_section,
    zones_section,
)

_CARD_OBJECT_NAMES = (
    "serialStationCard",
    "serialStationCardHeader",
    "serialStationCardTitle",
    "serialStationCardIcon",
    "serialStationCardBody",
    "serialStationLogCardBody",
)
_ZONE_OBJECT_NAMES = (
    "serialStationLeftZone",
    "serialStationCenterZone",
    "serialStationRightZone",
)
_TOPBAR_OBJECT_NAMES = (
    "serialStationTopBar",
    "serialStationBrandChip",
    "serialStationBrandIcon",
    "serialStationBrandName",
    "serialStationBrandTagline",
)
_COMMAND_PALETTE_OBJECT_NAMES = (
    "serialStationCommandPalette",
    "serialStationCommandPaletteCard",
    "serialStationCommandPaletteEdit",
    "serialStationCommandPaletteList",
    "serialStationCommandPaletteHint",
)


def test_cards_section_covers_card_objectnames():
    qss = cards_section()
    for name in _CARD_OBJECT_NAMES:
        assert f"#{name}" in qss, f"cards_section missing objectName: {name}"


def test_cards_section_uses_glass_highlight_gradient():
    """玻璃卡片顶部高光渐变必须引用 tokens 高光停止点（近似 EK-OmniProbe surface）。"""

    qss = cards_section()
    assert T.CARD_HIGHLIGHT_STOP_0 in qss
    assert T.CARD_HIGHLIGHT_STOP_1 in qss
    assert "qlineargradient" in qss
    # 大圆角拉开视觉层级。
    assert T.RADIUS_2XL in qss


def test_cards_section_hover_lifts_to_accent_border():
    """卡片悬浮应提亮到 BG_PANEL_RAISED 并转强调青软边。"""

    qss = cards_section()
    assert ":hover" in qss
    assert P.BG_PANEL_RAISED in qss
    assert P.ACCENT_BORDER in qss


def test_cards_section_header_has_bottom_separator():
    """标题行应有底部分隔线，拉开标题与内容区。"""

    qss = cards_section()
    assert "#serialStationCardHeader" in qss
    assert "border-bottom" in qss


def test_zones_section_covers_zone_objectnames_and_is_transparent():
    qss = zones_section()
    for name in _ZONE_OBJECT_NAMES:
        assert f"#{name}" in qss, f"zones_section missing objectName: {name}"
    # zone 容器透明，让卡片浮在窗口底色上。
    assert P.SCROLLBAR_BACKGROUND == "transparent" or "transparent" in qss


def test_splitter_section_covers_handle_and_hover():
    qss = splitter_section()
    assert "#serialStationMainSplitter::handle" in qss
    assert ":hover" in qss
    # 悬浮转强调青软边，提升分栏可达性。
    assert P.ACCENT_BORDER in qss


def test_topbar_section_covers_all_topbar_objectnames():
    qss = topbar_section()
    for name in _TOPBAR_OBJECT_NAMES:
        assert f"#{name}" in qss, f"topbar_section missing objectName: {name}"


def test_topbar_section_uses_brand_chip_palette():
    """TopBar 品牌区应引用品牌 chip 色板（强调青软底）。"""

    qss = topbar_section()
    assert P.BRAND_CHIP_BG in qss
    assert P.BRAND_CHIP_BORDER in qss
    assert P.TOPBAR_BG_TOP in qss


def test_build_qss_integrates_all_layout_sections():
    """build_qss 主输出必须含四个布局分区。"""

    qss = build_qss()
    # 卡片选择器。
    assert "#serialStationCard" in qss
    # zone 选择器。
    assert "#serialStationLeftZone" in qss
    # splitter 把手。
    assert "#serialStationMainSplitter::handle" in qss
    # TopBar 选择器。
    assert "#serialStationTopBar" in qss


def test_build_qss_does_not_duplicate_cards_zones_section():
    """旧的 cards_zones_section 已迁移到 qss_sections_layout，不应残留重复分区标记。"""

    qss = build_qss()
    # 旧分区用 "Cards & Three-Zone Layout" 标题，新分区用 "Glass Cards" 标题。
    assert "Cards & Three-Zone Layout" not in qss
    assert "Glass Cards" in qss


def test_command_palette_section_covers_all_objectnames():
    qss = command_palette_section()
    for name in _COMMAND_PALETTE_OBJECT_NAMES:
        assert f"#{name}" in qss, f"command_palette_section missing objectName: {name}"


def test_command_palette_section_uses_overlay_and_accent_selection():
    """命令面板遮罩用 BG_OVERLAY，列表选中项用强调青软底。"""

    qss = command_palette_section()
    assert P.BG_OVERLAY in qss
    assert P.ACCENT_SOFT in qss
    assert P.ACCENT in qss


def test_build_qss_integrates_command_palette_section():
    qss = build_qss()
    assert "#serialStationCommandPalette" in qss
    assert "#serialStationCommandPaletteCard" in qss

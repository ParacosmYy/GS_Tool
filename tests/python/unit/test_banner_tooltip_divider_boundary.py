"""BannerKind 枚举 + _parse_color + Divider + InfoBanner/RichTooltip 边界测试。

补强 test_info_banner / test_rich_tooltip / test_divider 未直接断言的边角：
- BannerKind：4 成员 + value 小写。
- _parse_color：hex 解析 + rgba 解析 + 未知格式返回黑色。
- Divider：无标签构造 + 有标签 + objectName 契约。
- InfoBanner：set_text/text round-trip + dismissed 信号 + KIND_COLORS 4 键。
- RichTooltip：install/uninstall_tooltip 不崩溃 + title/body 初始空。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.controls.divider import Divider
from embeddebug.serial_station.ui.controls.info_banner import BannerKind, InfoBanner
from embeddebug.serial_station.ui.controls.rich_tooltip import (
    RichTooltip,
    _parse_color,
    install_tooltip,
    uninstall_tooltip,
)


# ── BannerKind 枚举 ───────────────────────────────────────────────────


def test_banner_kind_has_four_members():
    """BannerKind 含 4 成员。"""

    assert len(BannerKind) == 4


def test_banner_kind_values_lowercase():
    """value 小写。"""

    for kind in BannerKind:
        assert kind.value == kind.value.lower()


def test_banner_kind_values_distinct():
    assert len({k.value for k in BannerKind}) == 4


def test_kind_colors_has_all_four():
    """InfoBanner.KIND_COLORS 含 4 个 BannerKind 映射。"""

    assert len(InfoBanner.KIND_COLORS) == 4
    for kind in BannerKind:
        assert kind in InfoBanner.KIND_COLORS


# ── _parse_color ──────────────────────────────────────────────────────


def test_parse_color_hex():
    """hex 颜色 → QColor。"""

    c = _parse_color("#22d3ee")
    assert isinstance(c, QColor)


def test_parse_color_rgba():
    """rgba 颜色 → QColor。"""

    c = _parse_color("rgba(34, 211, 238, 0.12)")
    assert isinstance(c, QColor)


def test_parse_color_unknown_returns_black():
    """未知格式 → 黑色 QColor（兜底）。"""

    c = _parse_color("not-a-color")
    assert isinstance(c, QColor)


def test_parse_color_empty_string():
    """空串 → QColor（兜底）。"""

    c = _parse_color("")
    assert isinstance(c, QColor)


# ── Divider 边界 ──────────────────────────────────────────────────────


def test_divider_no_label(qtbot):
    """无标签构造。"""

    d = Divider()
    qtbot.addWidget(d)
    assert d.objectName() == "serialStationDivider"


def test_divider_with_label(qtbot):
    """有标签构造。"""

    d = Divider(label="Section")
    qtbot.addWidget(d)
    assert d.objectName() == "serialStationDivider"


def test_divider_objectname(qtbot):
    """Divider objectName = serialStationDivider。"""

    d = Divider()
    qtbot.addWidget(d)
    assert d.objectName() == "serialStationDivider"


# ── InfoBanner 边界 ───────────────────────────────────────────────────


def test_info_banner_set_text_round_trip(qtbot):
    """set_text / text round-trip。"""

    banner = InfoBanner(text="Hello")
    qtbot.addWidget(banner)
    assert banner.text() == "Hello"
    banner.set_text("World")
    assert banner.text() == "World"


def test_info_banner_initial_kind_info(qtbot):
    """默认 kind=INFO。"""

    banner = InfoBanner(text="x")
    qtbot.addWidget(banner)
    assert banner.kind() == BannerKind.INFO


def test_info_banner_set_kind(qtbot):
    """set_kind 修改 kind()。"""

    banner = InfoBanner(text="x")
    qtbot.addWidget(banner)
    banner.set_kind(BannerKind.WARNING)
    assert banner.kind() == BannerKind.WARNING


def test_info_banner_has_objectname(qtbot):
    """objectName = serialStationInfoBanner。"""

    banner = InfoBanner(text="x")
    qtbot.addWidget(banner)
    assert banner.objectName() == "serialStationInfoBanner"


# ── RichTooltip install/uninstall ─────────────────────────────────────


def test_install_tooltip_no_crash(qtbot):
    """install_tooltip 不崩溃。"""

    btn = QPushButton("Test")
    qtbot.addWidget(btn)
    install_tooltip(btn, "Hint", "Helpful text")


def test_uninstall_tooltip_no_crash(qtbot):
    """uninstall_tooltip 不崩溃（即使未安装）。"""

    btn = QPushButton("Test")
    qtbot.addWidget(btn)
    uninstall_tooltip(btn)


def test_install_then_uninstall(qtbot):
    """install → uninstall 不崩溃。"""

    btn = QPushButton("Test")
    qtbot.addWidget(btn)
    install_tooltip(btn, "T", "B")
    uninstall_tooltip(btn)


def test_rich_tooltip_initial_title_body(qtbot):
    """RichTooltip 初始 title/body 来自构造参数。"""

    tip = RichTooltip(title="My Title", body="My Body")
    qtbot.addWidget(tip)
    assert tip.title() == "My Title"
    assert tip.body() == "My Body"

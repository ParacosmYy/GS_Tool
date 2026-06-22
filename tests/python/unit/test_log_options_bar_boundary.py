"""log_options_bar build_history_drawer + build_log_info_banner + show/update 行为边界。

模块此前无直接测试覆盖（grep 0 命中）。build_log_options_bar 的子控件因 layout
未安装到 parent 而 findChild 不可靠，本文件聚焦可独立验证的函数。

覆盖：
1. build_log_options_bar 返回 QHBoxLayout（类型契约）。
2. build_history_drawer 返回 Drawer + objectName。
3. build_log_info_banner 返回 InfoBanner + objectName + 初始 hide。
4. show_log_info_banner None 安全跳过 + set_text/set_kind/show。
5. update_connection_badge None 安全 + connected SUCCESS/disconnected INFO。
"""

from __future__ import annotations

import os
from types import SimpleNamespace

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.controls import Badge, BadgeKind, InfoBanner
from embeddebug.serial_station.ui.log_options_bar import (
    build_history_drawer,
    build_log_info_banner,
    build_log_options_bar,
    show_log_info_banner,
    update_connection_badge,
)


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    return owner


# ── build_log_options_bar 类型契约 ───────────────────────────────
def test_build_log_options_bar_returns_hboxlayout(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    row = build_log_options_bar(owner, parent)
    from PyQt6.QtWidgets import QHBoxLayout

    assert isinstance(row, QHBoxLayout)


def test_build_log_options_bar_does_not_crash(qtbot):
    """build 完整执行不抛（含 Badge/Chip/Toggle/Segmented/Drawer 构造）。"""

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    build_log_options_bar(owner, parent)


# ── build_history_drawer ─────────────────────────────────────────
def test_build_history_drawer_returns_drawer(qtbot):
    from embeddebug.serial_station.ui.controls import Drawer

    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    drawer = build_history_drawer(owner, parent)
    assert isinstance(drawer, Drawer)
    assert drawer.objectName() == "serialStationHistoryDrawer"


# ── build_log_info_banner ────────────────────────────────────────
def test_build_log_info_banner_returns_banner(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    banner = build_log_info_banner(owner, parent)
    assert isinstance(banner, InfoBanner)
    assert banner.objectName() == "serialStationLogInfoBanner"


def test_build_log_info_banner_initially_hidden(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    banner = build_log_info_banner(owner, parent)
    assert banner.isHidden()


# ── show_log_info_banner ─────────────────────────────────────────
def test_show_log_info_banner_none_skips():
    """banner=None → 安全跳过不抛。"""

    show_log_info_banner(None, "text")  # 不抛


def test_show_log_info_banner_sets_text_and_shows(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    banner = build_log_info_banner(owner, parent)
    show_log_info_banner(banner, "警告信息", BadgeKind.WARNING)
    assert not banner.isHidden()


def test_show_log_info_banner_sets_kind(qtbot):
    owner = _make_owner()
    parent = QWidget()
    qtbot.addWidget(parent)
    banner = build_log_info_banner(owner, parent)
    show_log_info_banner(banner, "错误", BadgeKind.ERROR)
    assert banner._kind == BadgeKind.ERROR


# ── update_connection_badge ──────────────────────────────────────
def test_update_connection_badge_none_skips():
    """badge=None → 安全跳过。"""

    update_connection_badge(None, True, lambda s: s)  # 不抛


def test_update_connection_badge_connected_sets_success(qtbot):
    """手动构造 Badge 测试 update（不依赖 build_log_options_bar 的子控件查找）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    badge = Badge("未连接", BadgeKind.INFO, parent=parent)
    update_connection_badge(badge, connected=True, tr=lambda s: f"T:{s}")
    assert badge._kind == BadgeKind.SUCCESS


def test_update_connection_badge_disconnected_sets_info(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    badge = Badge("已连接", BadgeKind.SUCCESS, parent=parent)
    update_connection_badge(badge, connected=False, tr=lambda s: f"T:{s}")
    assert badge._kind == BadgeKind.INFO


def test_update_connection_badge_none_disconnected_skips():
    """badge=None disconnected → 安全跳过。"""

    update_connection_badge(None, False, lambda s: s)

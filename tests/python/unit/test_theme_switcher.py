"""B11 UI 精致度 v2 测试：微交互 + 主题切换 + 浅色色板。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui import micro_interactions
from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.theme_switcher import (
    AVAILABLE_THEMES,
    THEME_DARK,
    THEME_LIGHT,
    ThemeSwitcher,
    apply_theme_by_name,
    build_light_qss,
)


# ── 微交互 ────────────────────────────────────────────────────────
def test_install_hover_lift_returns_effect(qtbot):
    widget = QPushButton("X")
    qtbot.addWidget(widget)
    effect = micro_interactions.install_hover_lift(widget)
    assert effect is not None
    assert effect.blurRadius() == micro_interactions.SHADOW_BLUR_NORMAL


def test_install_focus_ring_does_not_crash(qtbot):
    widget = QLineEdit()
    qtbot.addWidget(widget)
    micro_interactions.install_focus_ring(widget)
    # 安装后 widget 仍可正常使用。
    widget.setText("hello")
    assert widget.text() == "hello"


def test_hover_lift_constants():
    assert micro_interactions.LIFT_PIXELS > 0
    assert micro_interactions.SHADOW_BLUR_HOVER > micro_interactions.SHADOW_BLUR_NORMAL
    assert micro_interactions.ANIM_DURATION > 0


# ── 浅色色板 ──────────────────────────────────────────────────────
def test_light_palette_has_all_dark_keys():
    dark_keys = set(dark.all_tokens())
    light_keys = set(light.all_tokens())
    assert dark_keys == light_keys


def test_light_palette_differs_from_dark():
    assert dark.BG_WINDOW != light.BG_WINDOW
    assert dark.TEXT_PRIMARY != light.TEXT_PRIMARY
    assert dark.BG_PANEL != light.BG_PANEL


def test_light_palette_is_actually_light():
    """浅色背景应比深色背景"更亮"（hex 数值更大）。"""

    assert int(light.BG_WINDOW[1:3], 16) > int(dark.BG_WINDOW[1:3], 16)


def test_light_palette_accent_is_consistent_brand():
    """强调色在浅色下应仍是青系（与深色同色系）。"""

    assert light.ACCENT.startswith("#0")


# ── 主题切换 ──────────────────────────────────────────────────────
def test_available_themes_contains_both():
    assert THEME_DARK in AVAILABLE_THEMES
    assert THEME_LIGHT in AVAILABLE_THEMES


def test_build_light_qss_replaces_dark_colors(qapp):
    dark_qss = _build_dark()
    light_qss = build_light_qss()
    # 浅色 QSS 不应再含深色窗口底色（冲突值由最后一个 key 决定目标）。
    assert dark.BG_WINDOW not in light_qss
    # 应含某个浅色 palette 值（证明替换生效）。
    assert light.TEXT_PRIMARY in light_qss
    assert len(light_qss) > 0


def _build_dark() -> str:
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    return build_qss()


def test_theme_switcher_default_is_dark(qapp):
    switcher = ThemeSwitcher(qapp)
    assert switcher.current_theme == THEME_DARK
    assert switcher.is_dark() is True


def test_theme_switcher_toggle(qapp):
    switcher = ThemeSwitcher(qapp)
    switcher.apply_dark()
    assert switcher.current_theme == THEME_DARK
    result = switcher.toggle()
    assert result == THEME_LIGHT
    assert switcher.is_dark() is False
    # 切换后 app 应有非空 styleSheet。
    assert qapp.styleSheet()
    # 再切回深色。
    result = switcher.toggle()
    assert result == THEME_DARK
    assert switcher.is_dark() is True


def test_theme_switcher_apply_light(qapp):
    switcher = ThemeSwitcher(qapp)
    switcher.apply_light()
    assert switcher.current_theme == THEME_LIGHT
    # 浅色 QSS 应含浅色文本色（深色已替换）。
    assert light.TEXT_PRIMARY in qapp.styleSheet()
    assert dark.TEXT_PRIMARY not in qapp.styleSheet()


def test_theme_switcher_apply_dark(qapp):
    switcher = ThemeSwitcher(qapp)
    switcher.apply_light()
    switcher.apply_dark()
    assert switcher.current_theme == THEME_DARK


def test_theme_switcher_light_cached(qapp):
    switcher = ThemeSwitcher(qapp)
    switcher.apply_light()
    cached = switcher._light_qss
    assert cached is not None
    switcher.apply_light()
    # 第二次应复用缓存（同一对象）。
    assert switcher._light_qss is cached


def test_apply_theme_by_name_dark(qapp):
    result = apply_theme_by_name(qapp, THEME_DARK)
    assert result == THEME_DARK


def test_apply_theme_by_name_light(qapp):
    result = apply_theme_by_name(qapp, THEME_LIGHT)
    assert result == THEME_LIGHT


def test_apply_theme_by_name_unknown_falls_back_to_dark(qapp):
    result = apply_theme_by_name(qapp, "unknown")
    assert result == THEME_DARK

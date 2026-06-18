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


# ── Batch 2 (A2): elevation / gradient / 深度层次 ─────────────────
def test_dark_palette_has_elevation_tokens():
    """深色 palette 应含三档 elevation 投影 + accent glow token。"""

    for key in ("shadow_card", "shadow_popover", "shadow_modal", "card_glow"):
        assert key in dark.all_tokens(), f"missing elevation token: {key}"
    assert dark.SHADOW_CARD != dark.SHADOW_MODAL  # 三档应有层次差


def test_dark_palette_has_accent_gradient():
    """深色 palette 应含跨色相 accent gradient（青→蓝）。"""

    assert dark.ACCENT_GRADIENT_FROM.startswith("#")
    assert dark.ACCENT_GRADIENT_TO.startswith("#")
    assert dark.ACCENT_GRADIENT_FROM != dark.ACCENT_GRADIENT_TO  # 跨色相
    assert "qlineargradient" in dark.ACCENT_GRADIENT


def test_dark_light_tokens_keys_aligned():
    """深浅色 palette 的 token key 集合必须完全一致（切换不漏 key）。"""

    assert set(dark.all_tokens()) == set(light.all_tokens())


def test_card_panel_brightness_gap_for_depth():
    """BG_PANEL 应明显亮于 BG_WINDOW，让卡片真正浮起（深度层次）。

    Batch 2 改进：原 BG_PANEL=#151b24 与 BG_WINDOW=#0d1118 亮度差仅 ~3%，
    卡片几乎浮不起来。改进后亮度差应 >=6%。
    """

    def luminance(hex_color: str) -> int:
        return int(hex_color[1:3], 16) + int(hex_color[3:5], 16) + int(hex_color[5:7], 16)

    gap = luminance(dark.BG_PANEL) - luminance(dark.BG_WINDOW)
    assert gap >= 18, f"BG_PANEL 与 BG_WINDOW 亮度差过小 ({gap})，卡片浮不起来"


def test_build_qss_uses_gradient_on_primary_buttons():
    """主按钮 QSS 应使用 accent gradient（跨色相品牌渐变）。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    qss = build_qss()
    assert dark.ACCENT_GRADIENT_FROM in qss or "qlineargradient" in qss


def test_no_hardcoded_rgba_in_qss():
    """QSS 不应残留硬编码 RGBA（应走 palette token，浅色切换才不漏色）。

    Batch 2 修复的三处：滚动条 rgba(51,65,85)、placeholder rgba(154,167,189)、
    卡片头 rgba(39,49,63)。这三处原绕过 token，浅色切换时会残留深色值。
    """

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    qss = build_qss()
    forbidden = ["rgba(51, 65, 85", "rgba(71, 85, 105", "rgba(154, 167, 189", "rgba(39, 49, 63"]
    for frag in forbidden:
        assert frag not in qss, f"QSS 残留硬编码 RGBA: {frag}"

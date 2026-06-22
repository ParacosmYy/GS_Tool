"""主题切换器 + windowOpacity 过渡动画 + 浅色色板 + 微交互测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QPropertyAnimation, QSequentialAnimationGroup
from PyQt6.QtWidgets import QApplication, QLineEdit, QPushButton

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
from embeddebug.serial_station.ui.theme.theme_transition import (
    DIP_MS,
    OPACITY_DIP,
    RISE_MS,
    ThemeTransition,
    transition_theme,
)

# ── 微交互 ──
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
    widget.setText("hello")
    assert widget.text() == "hello"

def test_hover_lift_constants():
    assert micro_interactions.LIFT_PIXELS > 0
    assert micro_interactions.SHADOW_BLUR_HOVER > micro_interactions.SHADOW_BLUR_NORMAL
    assert micro_interactions.ANIM_DURATION > 0

# ── 浅色色板 ──
def test_light_palette_has_all_dark_keys():
    assert set(dark.all_tokens()) == set(light.all_tokens())

def test_light_palette_differs_from_dark():
    assert dark.BG_WINDOW != light.BG_WINDOW
    assert dark.TEXT_PRIMARY != light.TEXT_PRIMARY
    assert dark.BG_PANEL != light.BG_PANEL

def test_light_palette_is_actually_light():
    assert int(light.BG_WINDOW[1:3], 16) > int(dark.BG_WINDOW[1:3], 16)

def test_light_palette_accent_is_consistent_brand():
    assert light.ACCENT.startswith("#0")

# ── 主题切换 ──
def test_available_themes_contains_both():
    assert THEME_DARK in AVAILABLE_THEMES
    assert THEME_LIGHT in AVAILABLE_THEMES

def test_build_light_qss_replaces_dark_colors(qapp):
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss
    build_qss()
    light_qss = build_light_qss()
    assert dark.BG_WINDOW not in light_qss
    assert light.TEXT_PRIMARY in light_qss
    assert len(light_qss) > 0

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
    assert qapp.styleSheet()
    result = switcher.toggle()
    assert result == THEME_DARK
    assert switcher.is_dark() is True

def test_theme_switcher_apply_light(qapp):
    switcher = ThemeSwitcher(qapp)
    switcher.apply_light()
    assert switcher.current_theme == THEME_LIGHT
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
    cached = switcher._qss_cache.get((THEME_LIGHT, "cyan"))
    assert cached is not None
    switcher.apply_light()
    assert switcher._qss_cache.get((THEME_LIGHT, "cyan")) is cached

def test_apply_theme_by_name_dark(qapp):
    assert apply_theme_by_name(qapp, THEME_DARK) == THEME_DARK

def test_apply_theme_by_name_light(qapp):
    assert apply_theme_by_name(qapp, THEME_LIGHT) == THEME_LIGHT

def test_apply_theme_by_name_unknown_falls_back_to_dark(qapp):
    assert apply_theme_by_name(qapp, "unknown") == THEME_DARK

# ── elevation / gradient / 深度层次 ──
def test_dark_palette_has_elevation_tokens():
    for key in ("shadow_card", "shadow_popover", "shadow_modal", "card_glow"):
        assert key in dark.all_tokens(), f"missing elevation token: {key}"
    assert dark.SHADOW_CARD != dark.SHADOW_MODAL

def test_dark_palette_has_accent_gradient():
    assert dark.ACCENT_GRADIENT_FROM.startswith("#")
    assert dark.ACCENT_GRADIENT_TO.startswith("#")
    assert dark.ACCENT_GRADIENT_FROM != dark.ACCENT_GRADIENT_TO
    assert "qlineargradient" in dark.ACCENT_GRADIENT

def test_dark_light_tokens_keys_aligned():
    assert set(dark.all_tokens()) == set(light.all_tokens())

def test_card_panel_brightness_gap_for_depth():
    def luminance(hex_color: str) -> int:
        return int(hex_color[1:3], 16) + int(hex_color[3:5], 16) + int(hex_color[5:7], 16)
    gap = luminance(dark.BG_PANEL) - luminance(dark.BG_WINDOW)
    assert gap >= 18, f"BG_PANEL 与 BG_WINDOW 亮度差过小 ({gap})"

def test_build_qss_uses_gradient_on_primary_buttons():
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss
    qss = build_qss()
    assert dark.ACCENT_GRADIENT_FROM in qss or "qlineargradient" in qss

def test_no_hardcoded_rgba_in_qss():
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss
    qss = build_qss()
    forbidden = ["rgba(51, 65, 85", "rgba(71, 85, 105", "rgba(154, 167, 189", "rgba(39, 49, 63"]
    for frag in forbidden:
        assert frag not in qss, f"QSS 残留硬编码 RGBA: {frag}"

# ── ThemeTransition 过渡动画 ──
def test_opacity_dip_and_durations_in_expected_range():
    assert 0.3 < OPACITY_DIP < 0.9
    assert 50 <= DIP_MS <= 300
    assert 50 <= RISE_MS <= 400
    assert DIP_MS + RISE_MS <= 600

def test_run_creates_window_opacity_animation_group(qtbot):
    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()
    called = {"n": 0}
    def apply_fn() -> None:
        called["n"] += 1
    group = ThemeTransition.run(app, apply_fn)
    try:
        assert group is not None
        assert isinstance(group, QSequentialAnimationGroup)
        assert group.animationCount() == 2
        for i in range(group.animationCount()):
            child = group.animationAt(i)
            assert isinstance(child, QPropertyAnimation)
            assert child.propertyName() == b"windowOpacity"
        dip = group.animationAt(0)
        rise = group.animationAt(1)
        assert dip.startValue() == 1.0
        assert dip.endValue() == OPACITY_DIP
        assert rise.startValue() == OPACITY_DIP
        assert rise.endValue() == 1.0
    finally:
        if group is not None:
            group.stop()
        ThemeTransition._active.clear()

def test_apply_fn_called_on_dip_finished(qtbot):
    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()
    called = {"n": 0}
    def apply_fn() -> None:
        called["n"] += 1
    group = ThemeTransition.run(app, apply_fn, dip_ms=1, rise_ms=1)
    assert group is not None
    try:
        qtbot.waitUntil(lambda: group.state() == QAbstractAnimation.State.Stopped, timeout=2000)
        qtbot.wait(50)
    finally:
        group.stop()
        ThemeTransition._active.clear()
    assert called["n"] == 1

def test_run_returns_none_when_already_running(qtbot):
    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()
    group1 = ThemeTransition.run(app, lambda: None, dip_ms=500, rise_ms=500)
    assert group1 is not None
    try:
        assert ThemeTransition.is_running() is True
        group2 = ThemeTransition.run(app, lambda: None)
        assert group2 is None
    finally:
        group1.stop()
        ThemeTransition._active.clear()
    assert ThemeTransition.is_running() is False

def test_transition_theme_falls_back_to_sync_on_reentry(qtbot):
    app = QApplication.instance()
    assert app is not None
    ThemeTransition._active.clear()
    blocker = ThemeTransition.run(app, lambda: None, dip_ms=500, rise_ms=500)
    assert blocker is not None
    try:
        called = {"n": 0}
        def apply_fn() -> None:
            called["n"] += 1
        started = transition_theme(app, apply_fn)
        assert started is False
        assert called["n"] == 1
    finally:
        blocker.stop()
        ThemeTransition._active.clear()


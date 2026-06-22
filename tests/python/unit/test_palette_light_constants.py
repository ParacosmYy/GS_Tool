"""palette_light 浅色主题常量契约单元测试。

补强 test_theme_switching.py 未直接断言的边角：
- 核心常量值契约（BG/TEXT/ACCENT/状态色）。
- 浅色主题亮度 > 深色（BG_WINDOW/TEXT_PRIMARY 反转）。
- ACCENT 以 #0 开头（品牌青深色变体）。
- all_tokens() 返回 dict[str, str] + 键集与 palette 对齐。
- 状态色（SUCCESS/WARNING/ERROR/TERM_*）存在且非空。
- 渐变 + soft/border rgba 格式。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light


def _hex_brightness(hex_color: str) -> int:
    """计算 hex 颜色亮度（R+G+B 总和，越大越亮）。"""

    h = hex_color.lstrip("#")
    return int(h[0:2], 16) + int(h[2:4], 16) + int(h[4:6], 16)


# ── 核心常量值契约 ───────────────────────────────────────────────────────


def test_bg_window_is_light():
    """浅色主题 BG_WINDOW 是浅色（高亮度）。"""

    assert light.BG_WINDOW == "#f5f6f8"


def test_bg_app_and_panel_are_white():
    """BG_APP 和 BG_PANEL 是白色（浅色卡片底）。"""

    assert light.BG_APP == "#ffffff"
    assert light.BG_PANEL == "#ffffff"


def test_text_primary_is_dark():
    """浅色主题文字是深色（深底白字反转）。"""

    assert light.TEXT_PRIMARY == "#1e293b"


def test_accent_starts_with_dark_cyan():
    """浅色 ACCENT 以 #0 开头（深色变体青，对比度更高）。"""

    assert light.ACCENT.startswith("#0")


# ── 浅色 > 深色 亮度关系 ────────────────────────────────────────────────


def test_light_bg_brighter_than_dark():
    """浅色 BG_WINDOW 亮度 > 深色（反转语义）。"""

    assert _hex_brightness(light.BG_WINDOW) > _hex_brightness(dark.BG_WINDOW)


def test_light_text_darker_than_dark():
    """浅色 TEXT_PRIMARY 亮度 < 深色（反转语义）。"""

    assert _hex_brightness(light.TEXT_PRIMARY) < _hex_brightness(dark.TEXT_PRIMARY)


def test_light_panel_brighter_than_dark():
    """浅色 BG_PANEL 亮度 > 深色。"""

    assert _hex_brightness(light.BG_PANEL) > _hex_brightness(dark.BG_PANEL)


# ── 状态色存在且非空 ─────────────────────────────────────────────────────


def test_status_colors_exist():
    """SUCCESS/WARNING/ERROR 存在且非空 hex。"""

    for name in ("SUCCESS", "WARNING", "ERROR", "ERROR_HOVER", "WARNING_HOVER"):
        value = getattr(light, name)
        assert value.startswith("#"), f"{name}={value} not hex"
        assert len(value) >= 7  # #RRGGBB 最小


def test_terminal_colors_exist():
    """TERM_TX/TERM_RX/TERM_BACKGROUND/TERM_SYSTEM 存在。"""

    for name in ("TERM_TX", "TERM_RX", "TERM_BACKGROUND", "TERM_SYSTEM"):
        value = getattr(light, name)
        assert value  # 非空


# ── ACCENT 系列 ─────────────────────────────────────────────────────────


def test_accent_hover_lighter_than_pressed():
    """ACCENT_HOVER 亮度 > ACCENT_PRESSED（hover 提亮）。"""

    assert _hex_brightness(light.ACCENT_HOVER) > _hex_brightness(light.ACCENT_PRESSED)


def test_accent_soft_is_rgba():
    """ACCENT_SOFT 是 rgba() 格式。"""

    assert light.ACCENT_SOFT.startswith("rgba(")


def test_accent_border_is_rgba():
    """ACCENT_BORDER 是 rgba() 格式。"""

    assert light.ACCENT_BORDER.startswith("rgba(")


def test_accent_gradient_contains_from_and_to():
    """ACCENT_GRADIENT 含 from/to 两个停止点。"""

    assert light.ACCENT_GRADIENT_FROM in light.ACCENT_GRADIENT
    assert light.ACCENT_GRADIENT_TO in light.ACCENT_GRADIENT


def test_accent_gradient_from_to_distinct():
    """渐变起点 ≠ 终点（跨色相）。"""

    assert light.ACCENT_GRADIENT_FROM != light.ACCENT_GRADIENT_TO


# ── all_tokens() 契约 ───────────────────────────────────────────────────


def test_all_tokens_returns_dict():
    """all_tokens() 返回 dict[str, str]。"""

    tokens = light.all_tokens()
    assert isinstance(tokens, dict)
    assert len(tokens) > 0
    for key, value in tokens.items():
        assert isinstance(key, str)
        assert isinstance(value, str)


def test_all_tokens_keys_align_with_dark():
    """浅色 all_tokens 键集 == 深色（对齐契约）。"""

    dark_tokens = dark.all_tokens()
    light_tokens = light.all_tokens()
    assert set(dark_tokens.keys()) == set(light_tokens.keys())


def test_all_tokens_contains_core_keys():
    """all_tokens 含 bg_window/bg_panel/text_primary/accent 核心键。"""

    tokens = light.all_tokens()
    for key in ("bg_window", "bg_panel", "text_primary", "accent"):
        assert key in tokens, f"missing key: {key}"


# ── BORDER / BG_INPUT 系列 ───────────────────────────────────────────────


def test_border_constants_exist():
    """BORDER/BORDER_STRONG/BORDER_FOCUS 存在。"""

    for name in ("BORDER", "BORDER_STRONG", "BORDER_FOCUS"):
        assert getattr(light, name)


def test_bg_input_exists():
    """BG_INPUT 存在（输入框底色）。"""

    assert light.BG_INPUT


def test_bg_disabled_exists():
    """BG_DISABLED 存在（禁用控件底）。"""

    assert light.BG_DISABLED


# ── 滚动条/品牌色 ───────────────────────────────────────────────────────


def test_scrollbar_constants_exist():
    """SCROLLBAR/SCROLLBAR_HOVER 存在。"""

    assert light.SCROLLBAR
    assert light.SCROLLBAR_HOVER


def test_brand_chip_constants_exist():
    """BRAND_CHIP_BG/BRAND_CHIP_BORDER 存在（TopBar 品牌 chip）。"""

    assert light.BRAND_CHIP_BG
    assert light.BRAND_CHIP_BORDER


def test_text_inverted_exists():
    """TEXT_INVERTED 存在（accent 按钮上的反白文字，浅色主题 accent 深所以文字也深）。"""

    assert light.TEXT_INVERTED

"""accents _soft + AccentTones.as_recolor_map + AnimationTokens 常量契约。

补强 test_theme_extras/test_animations_* 未直接断言：_soft rgba 格式 + as_recolor_map 7 键
+ tones_for dark/light + AnimationTokens 时长递增/曲线/ELEVATION/SCALE/抖动/阴影。
"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme.accents import (
    ACCENTS,
    AccentTones,
    DEFAULT_ACCENT_ID,
    _soft,
    get_accent_by_id,
)


# ── _soft 私有 helper ────────────────────────────────────────────────────


def test_soft_formats_rgba_with_two_decimal_alpha():
    """_soft 返回 rgba(r,g,b,a.xx) 格式（alpha 2 位小数）。"""

    assert _soft((34, 211, 238), 0.12) == "rgba(34, 211, 238, 0.12)"


def test_soft_alpha_zero():
    """alpha=0 → 0.00。"""

    assert _soft((255, 0, 0), 0.0) == "rgba(255, 0, 0, 0.00)"


def test_soft_alpha_one():
    """alpha=1 → 1.00。"""

    assert _soft((0, 0, 255), 1.0) == "rgba(0, 0, 255, 1.00)"


def test_soft_truncates_long_alpha():
    """alpha 3 位小数截断到 2 位（:.2f 格式化）。"""

    assert _soft((1, 2, 3), 0.123) == "rgba(1, 2, 3, 0.12)"


def test_soft_preserves_rgb_values():
    """RGB 值原样填入（不变换）。"""

    result = _soft((100, 200, 50), 0.5)
    assert "100" in result and "200" in result and "50" in result


# ── AccentTones.as_recolor_map ───────────────────────────────────────────


def _make_tones() -> AccentTones:
    """构造测试用 AccentTones。"""

    return AccentTones(
        base="#22d3ee", hover="#67e8f9", pressed="#0891b2",
        soft="rgba(34,211,238,0.12)", border="rgba(34,211,238,0.35)",
        gradient_from="#22d3ee", gradient_to="#3b82f6",
    )


def test_as_recolor_map_has_seven_keys():
    """as_recolor_map 含 7 个语义键（base/hover/pressed/soft/border/gradient_from/to）。"""

    tones = _make_tones()
    mapping = tones.as_recolor_map()
    assert set(mapping.keys()) == {
        "base", "hover", "pressed", "soft", "border", "gradient_from", "gradient_to",
    }


def test_as_recolor_map_values_match_tones_fields():
    """as_recolor_map 值与 AccentTones 字段一致。"""

    tones = _make_tones()
    mapping = tones.as_recolor_map()
    assert mapping["base"] == tones.base
    assert mapping["hover"] == tones.hover
    assert mapping["pressed"] == tones.pressed
    assert mapping["soft"] == tones.soft
    assert mapping["border"] == tones.border
    assert mapping["gradient_from"] == tones.gradient_from
    assert mapping["gradient_to"] == tones.gradient_to


def test_accent_tones_is_frozen():
    """AccentTones 是 frozen dataclass。"""

    tones = _make_tones()
    import pytest
    with pytest.raises((AttributeError, Exception)):
        tones.base = "#ff0000"  # type: ignore[misc]


# ── AccentVariant.tones_for ──────────────────────────────────────────────


def test_tones_for_dark_returns_dark():
    """tones_for(is_light=False) → dark 色调。"""

    variant = get_accent_by_id(DEFAULT_ACCENT_ID)
    assert variant.tones_for(is_light=False) is variant.dark


def test_tones_for_light_returns_light():
    """tones_for(is_light=True) → light 色调。"""

    variant = get_accent_by_id(DEFAULT_ACCENT_ID)
    assert variant.tones_for(is_light=True) is variant.light


def test_all_accents_have_distinct_dark_light_tones():
    """每个 accent 的 dark 与 light 色调不同（base 色不同）。"""

    for variant in ACCENTS:
        assert variant.dark.base != variant.light.base, f"{variant.id} dark==light base"


# ── AnimationTokens 时长 5 档递增 ────────────────────────────────────────


def test_duration_five_levels_ascending():
    """5 档时长递增：INSTANT < FAST < NORMAL < SLOW < SLOWER。"""

    assert AnimationTokens.DURATION_INSTANT < AnimationTokens.DURATION_FAST
    assert AnimationTokens.DURATION_FAST < AnimationTokens.DURATION_NORMAL
    assert AnimationTokens.DURATION_NORMAL < AnimationTokens.DURATION_SLOW
    assert AnimationTokens.DURATION_SLOW < AnimationTokens.DURATION_SLOWER


def test_duration_container_between_normal_and_slow():
    """CONTAINER 在 NORMAL 和 SLOW 之间（Material 3 emphasized 300ms）。"""

    assert AnimationTokens.DURATION_NORMAL < AnimationTokens.DURATION_CONTAINER
    assert AnimationTokens.DURATION_CONTAINER < AnimationTokens.DURATION_SLOW


def test_duration_values_positive():
    """所有时长常量为正整数。"""

    durations = [
        AnimationTokens.DURATION_INSTANT, AnimationTokens.DURATION_FAST,
        AnimationTokens.DURATION_NORMAL, AnimationTokens.DURATION_CONTAINER,
        AnimationTokens.DURATION_SLOW, AnimationTokens.DURATION_SLOWER,
        AnimationTokens.DURATION_PROGRESS, AnimationTokens.DURATION_FLYOUT,
        AnimationTokens.DURATION_DRAWER, AnimationTokens.DURATION_SCROLL,
    ]
    for d in durations:
        assert d > 0


# ── AnimationTokens 缓动曲线 ────────────────────────────────────────────


def test_easing_curves_are_qeasingcurve_types():
    """缓动曲线常量是 QEasingCurve.Type 枚举 + 互异。"""

    T = AnimationTokens
    assert QEasingCurve.Type.OutCubic == T.EASE_OUT
    assert QEasingCurve.Type.InCubic == T.EASE_IN
    assert QEasingCurve.Type.InOutCubic == T.EASE_IN_OUT
    assert QEasingCurve.Type.Linear == T.LINEAR
    curves = {T.EASE_OUT, T.EASE_IN, T.EASE_IN_OUT, T.LINEAR, T.EASE_OUT_BACK, T.EASE_OUT_BOUNCE}
    assert len(curves) >= 5


# ── AnimationTokens ELEVATION L0-L5 ──────────────────────────────────────


def test_elevation_six_levels():
    """ELEVATION_L0-L5 是 6 个三元组 (blur, offset_y, alpha)。"""

    levels = [
        AnimationTokens.ELEVATION_L0, AnimationTokens.ELEVATION_L1,
        AnimationTokens.ELEVATION_L2, AnimationTokens.ELEVATION_L3,
        AnimationTokens.ELEVATION_L4, AnimationTokens.ELEVATION_L5,
    ]
    assert len(levels) == 6
    for level in levels:
        assert len(level) == 3  # (blur, offset_y, alpha)


def test_elevation_l0_is_flat():
    """L0 = (0, 0, 0) 无阴影。"""

    assert AnimationTokens.ELEVATION_L0 == (0, 0, 0)


def test_elevation_alpha_increases_with_level():
    """alpha 随层级递增（更深阴影）。"""

    alphas = [
        AnimationTokens.ELEVATION_L0[2], AnimationTokens.ELEVATION_L1[2],
        AnimationTokens.ELEVATION_L2[2], AnimationTokens.ELEVATION_L3[2],
        AnimationTokens.ELEVATION_L4[2], AnimationTokens.ELEVATION_L5[2],
    ]
    for i in range(len(alphas) - 1):
        assert alphas[i] <= alphas[i + 1]


# ── AnimationTokens SCALE 比例 ───────────────────────────────────────────


def test_scale_constants():
    """SCALE 常量值契约。"""

    T = AnimationTokens
    assert T.SCALE_PRESSED == 0.96
    assert T.SCALE_POP_IN == 0.6
    assert T.SCALE_BOUNCE == 1.08
    assert T.SCALE_HOVER == 1.03
    assert T.SCALE_NORMAL == 1.0
    assert T.SCALE_PRESSED < T.SCALE_NORMAL  # 按压缩小
    assert T.SCALE_BOUNCE > T.SCALE_NORMAL   # 弹跳放大


# ── AnimationTokens 抖动/阴影/折叠 ──────────────────────────────────────


def test_shake_and_collapsed_constants():
    """抖动/折叠/阴影常量契约。"""

    T = AnimationTokens
    assert T.SHAKE_AMPLITUDE == 8
    assert T.SHAKE_COUNT == 3
    assert T.COLLAPSED_HEIGHT == 0
    assert T.SHADOW_BLUR_HOVER > T.SHADOW_BLUR_NORMAL
    assert len(T.SHADOW_COLOR_RGB) == 4
    assert T.SHADOW_COLOR_RGB[3] == 120


# ── AnimationTokens 关键帧/stagger ───────────────────────────────────────


def test_keyframe_and_stagger_in_range():
    """关键帧位置在 (0,1) + stagger 为正。"""

    T = AnimationTokens
    assert 0 < T.KEYFRAME_PREVIEW < 1
    assert 0 < T.KEYFRAME_HERALED < 1
    assert T.STAGGER_STEP_MS > 0

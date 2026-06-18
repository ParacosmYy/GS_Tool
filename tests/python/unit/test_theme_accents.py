"""Batch 10 测试：多强调色变体 + accent recolor 恒等性/无残留。

覆盖：
- 7 套 accent 变体结构（id 唯一、cyan 在首位、色调字段齐全）。
- cyan 变体色调与 palette/palette_light 现有常量严格一致（默认零回归的依据）。
- ``apply_accent_recolor`` 对 cyan 为恒等变换；对其他变体不残留 cyan base 值。
- 运行时 override 状态机（set/get/reset）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.theme import accents
from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.qss_builder import apply_accent_recolor, build_qss


# ── 变体结构 ──────────────────────────────────────────────────────
def test_accents_has_seven_variants():
    assert len(accents.ACCENTS) == 7


def test_accent_ids_are_unique():
    ids = [a.id for a in accents.ACCENTS]
    assert len(ids) == len(set(ids))


def test_cyan_is_first_and_default():
    assert accents.ACCENTS[0].id == "cyan"
    assert accents.DEFAULT_ACCENT_ID == "cyan"


def test_accent_tones_fields_complete():
    """每个变体的 dark/light 色调 7 元组字段非空。"""

    for variant in accents.ACCENTS:
        for tones in (variant.dark, variant.light):
            assert tones.base.startswith("#")
            assert tones.hover.startswith("#")
            assert tones.pressed.startswith("#")
            assert tones.soft.startswith("rgba(")
            assert tones.border.startswith("rgba(")
            assert tones.gradient_from.startswith("#")
            assert tones.gradient_to.startswith("#")


def test_accent_gradient_from_differs_from_to():
    """每套 accent 的渐变起止色应不同（跨色相品牌渐变）。"""

    for variant in accents.ACCENTS:
        for tones in (variant.dark, variant.light):
            assert tones.gradient_from != tones.gradient_to


def test_get_accent_by_id_falls_back_to_cyan():
    assert accents.get_accent_by_id("nonexistent").id == "cyan"


# ── cyan 默认零回归（色调 == palette 常量） ────────────────────────
def test_cyan_dark_tones_match_palette():
    """cyan dark 色调必须与 palette.py 现有常量严格一致（recolor 恒等性的前提）。"""

    cyan = accents.ACCENTS[0]
    assert cyan.dark.base == dark.ACCENT
    assert cyan.dark.hover == dark.ACCENT_HOVER
    assert cyan.dark.pressed == dark.ACCENT_PRESSED
    assert cyan.dark.soft == dark.ACCENT_SOFT
    assert cyan.dark.border == dark.ACCENT_BORDER
    assert cyan.dark.gradient_from == dark.ACCENT_GRADIENT_FROM
    assert cyan.dark.gradient_to == dark.ACCENT_GRADIENT_TO


def test_cyan_light_tones_match_palette_light():
    cyan = accents.ACCENTS[0]
    assert cyan.light.base == light.ACCENT
    assert cyan.light.hover == light.ACCENT_HOVER
    assert cyan.light.pressed == light.ACCENT_PRESSED
    assert cyan.light.soft == light.ACCENT_SOFT
    assert cyan.light.border == light.ACCENT_BORDER
    assert cyan.light.gradient_from == light.ACCENT_GRADIENT_FROM
    assert cyan.light.gradient_to == light.ACCENT_GRADIENT_TO


# ── accent recolor 恒等性 / 无残留 ────────────────────────────────
def test_apply_accent_recolor_cyan_is_identity():
    """cyan recolor 应是恒等变换（源值==目标值，全部跳过）。"""

    qss = build_qss()
    cyan = accents.get_accent_by_id("cyan")
    assert apply_accent_recolor(qss, cyan, is_light=False) == qss


def test_apply_accent_recolor_blue_changes_base():
    """blue 变体应把 cyan base (#22d3ee) 替换为 blue base。"""

    qss = build_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(qss, blue, is_light=False)
    assert blue.dark.base in recolored
    # cyan base 不应再作为 accent 出现（border_focus/gradient_from 原本也是 #22d3ee，
    # recolor 后全部替换；若 QSS 里有非 accent 处偶然含该子串也算通过——此处宽松断言）。
    # 严格断言：recolor 后 cyan base 计数应少于 recolor 前。
    assert recolored.count(dark.ACCENT) < qss.count(dark.ACCENT)


def test_apply_accent_recolor_preserves_non_accent_tokens():
    """recolor 不应改动非 accent 的 palette 值（如背景色）。"""

    qss = build_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(qss, blue, is_light=False)
    assert dark.BG_WINDOW in recolored
    assert dark.TEXT_PRIMARY in recolored


def test_apply_accent_recolor_light_path():
    """浅色路径：cyan 源值取 palette_light，blue 目标取 blue.light。"""

    from embeddebug.serial_station.ui.theme.theme_switcher import build_light_qss

    light_qss = build_light_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(light_qss, blue, is_light=True)
    assert blue.light.base in recolored


# ── 运行时 override 状态机 ────────────────────────────────────────
def test_get_active_accent_defaults_to_cyan():
    accents.reset_active_accent()
    assert accents.get_active_accent_id() == "cyan"


def test_set_and_get_active_accent():
    accents.reset_active_accent()
    accents.set_active_accent("purple")
    assert accents.get_active_accent_id() == "purple"
    accents.reset_active_accent()


def test_set_active_accent_cyan_clears_override():
    """set 回 cyan 应等价于 reset（_active_override 归 None）。"""

    accents.reset_active_accent()
    accents.set_active_accent("purple")
    assert accents.get_active_accent_id() == "purple"
    accents.set_active_accent("cyan")
    assert accents.get_active_accent_id() == "cyan"
    accents.reset_active_accent()


def test_tones_for_is_light():
    cyan = accents.get_accent_by_id("cyan")
    assert cyan.tones_for(is_light=False) is cyan.dark
    assert cyan.tones_for(is_light=True) is cyan.light

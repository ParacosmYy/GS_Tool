"""theme_serializer 纯 helper 边界单元测试。

补强 test_theme_quality.py 未直接断言的边角：
- _is_exportable_string：str/non-str/下划线前缀/空串/dunder。
- _CONST_NAME_RE：合法常量名/dunder/camelCase/小写/数字开头。
- _HEX_COLOR_RE：3/4/6/8 位 hex + 大小写 + 非法。
- _RGBA_COLOR_RE：rgb/rgba + int alpha + float alpha + 缺参数。
- validate_color_string：非 str/空白/3 位 hex/8 位 hex/大写 hex/rgb int alpha/rgba float alpha/transparent。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.theme_serializer import (
    _CONST_NAME_RE,
    _HEX_COLOR_RE,
    _RGBA_COLOR_RE,
    _is_exportable_string,
    validate_color_string,
)


# ── _is_exportable_string ────────────────────────────────────────────────


def test_is_exportable_plain_string():
    """普通 str → True。"""

    assert _is_exportable_string("#22d3ee") is True
    assert _is_exportable_string("hello") is True


def test_is_exportable_non_string():
    """非 str（int/None/list）→ False。"""

    assert _is_exportable_string(42) is False
    assert _is_exportable_string(None) is False
    assert _is_exportable_string(["a"]) is False


def test_is_exportable_underscore_prefix():
    """下划线前缀 str → False（过滤私有成员）。"""

    assert _is_exportable_string("_private") is False
    assert _is_exportable_string("_HEX_COLOR_RE") is False


def test_is_exportable_empty_string():
    """空串 → True（isinstance 通过 + 不以 _ 开头）。"""

    assert _is_exportable_string("") is True


def test_is_exportable_dunder_value():
    """dunder 值（如 __doc__ 的内容）→ True（值本身不以 _ 开头时）。"""

    # __doc__ 的值是普通文本，不以 _ 开头 → True（名称过滤由 _CONST_NAME_RE 负责）
    assert _is_exportable_string("This is a docstring") is True


# ── _CONST_NAME_RE ───────────────────────────────────────────────────────


def test_const_name_valid_uppercase():
    """合法大写常量名 → 匹配。"""

    assert _CONST_NAME_RE.match("ACCENT") is not None
    assert _CONST_NAME_RE.match("BG_WINDOW") is not None
    assert _CONST_NAME_RE.match("TEXT_PRIMARY") is not None


def test_const_name_with_digits():
    """含数字的常量名 → 匹配。"""

    assert _CONST_NAME_RE.match("COLOR_2") is not None
    assert _CONST_NAME_RE.match("RADIUS_2XL") is not None


def test_const_name_rejects_dunder():
    """dunder（__name__）→ 不匹配。"""

    assert _CONST_NAME_RE.match("__name__") is None
    assert _CONST_NAME_RE.match("__doc__") is None


def test_const_name_rejects_lowercase():
    """小写开头 → 不匹配。"""

    assert _CONST_NAME_RE.match("accent") is None
    assert _CONST_NAME_RE.match("myVar") is None


def test_const_name_rejects_camelcase():
    """camelCase → 不匹配。"""

    assert _CONST_NAME_RE.match("AccentColor") is None


def test_const_name_rejects_digit_start():
    """数字开头 → 不匹配。"""

    assert _CONST_NAME_RE.match("2XL") is None


# ── _HEX_COLOR_RE ────────────────────────────────────────────────────────


def test_hex_color_3_digit():
    """#RGB 3 位 → 匹配。"""

    assert _HEX_COLOR_RE.match("#abc") is not None


def test_hex_color_4_digit():
    """#RGBA 4 位 → 匹配。"""

    assert _HEX_COLOR_RE.match("#abcd") is not None


def test_hex_color_6_digit():
    """#RRGGBB 6 位 → 匹配。"""

    assert _HEX_COLOR_RE.match("#22d3ee") is not None


def test_hex_color_8_digit():
    """#RRGGBBAA 8 位 → 匹配。"""

    assert _HEX_COLOR_RE.match("#22d3eeff") is not None


def test_hex_color_uppercase():
    """大写 hex → 匹配。"""

    assert _HEX_COLOR_RE.match("#ABCDEF") is not None


def test_hex_color_rejects_5_digit():
    """5 位 hex → 不匹配（不在 3/4/6/8 中）。"""

    assert _HEX_COLOR_RE.match("#abcde") is None


def test_hex_color_rejects_no_hash():
    """无 # 前缀 → 不匹配。"""

    assert _HEX_COLOR_RE.match("22d3ee") is None


# ── _RGBA_COLOR_RE ───────────────────────────────────────────────────────


def test_rgba_color_basic():
    """rgba(r,g,b,a) → 匹配。"""

    assert _RGBA_COLOR_RE.match("rgba(34,211,238,0.5)") is not None


def test_rgb_color_no_alpha():
    """rgb(r,g,b) 无 alpha → 匹配。"""

    assert _RGBA_COLOR_RE.match("rgb(34,211,238)") is not None


def test_rgba_int_alpha():
    """rgba 整数 alpha（0-255）→ 匹配。"""

    assert _RGBA_COLOR_RE.match("rgba(34,211,238,128)") is not None


def test_rgba_with_spaces():
    """rgba 含空格 → 匹配。"""

    assert _RGBA_COLOR_RE.match("rgba(34, 211, 238, 0.5)") is not None


def test_rgba_rejects_missing_paren():
    """缺右括号 → 不匹配。"""

    assert _RGBA_COLOR_RE.match("rgba(34,211,238") is None


# ── validate_color_string 边界 ───────────────────────────────────────────


def test_validate_color_3_hex():
    """#abc 3 位 hex → True。"""

    assert validate_color_string("#abc") is True


def test_validate_color_8_hex():
    """#22d3eeff 8 位 hex（含 alpha）→ True。"""

    assert validate_color_string("#22d3eeff") is True


def test_validate_color_uppercase_hex():
    """大写 hex → True。"""

    assert validate_color_string("#ABCDEF") is True


def test_validate_color_rgb_int_alpha():
    """rgb(r,g,b) 无 alpha → True。"""

    assert validate_color_string("rgb(34,211,238)") is True


def test_validate_color_rgba_float_alpha():
    """rgba(r,g,b,0.5) float alpha → True。"""

    assert validate_color_string("rgba(34,211,238,0.5)") is True


def test_validate_color_transparent():
    """transparent 是合法 CSS 命名色 → True。"""

    assert validate_color_string("transparent") is True


def test_validate_color_whitespace_stripped():
    """前后空白被 strip 后验证。"""

    assert validate_color_string("  #22d3ee  ") is True


def test_validate_color_non_string():
    """非 str 输入 → False。"""

    assert validate_color_string(None) is False  # type: ignore[arg-type]
    assert validate_color_string(42) is False  # type: ignore[arg-type]


def test_validate_color_whitespace_only():
    """纯空白串 → False。"""

    assert validate_color_string("   ") is False


def test_validate_color_invalid_hex_too_short():
    """#ab 2 位 hex → False。"""

    assert validate_color_string("#ab") is False


def test_validate_color_garbage():
    """明显非法字符串 → False。"""

    assert validate_color_string("not-a-color-at-all") is False

"""theme_switcher 纯 helper 边界单元测试。

补强 test_theme_switching.py 未直接断言的边角：
- _dark_to_light_map：键值结构 + dark≠light 才入映射 + 值非空。
- build_themed_qss：dark 返回深色 QSS / light 返回浅色 / accent override 不崩溃。
- default_accent_id：返回 cyan。
- build_light_qss：非空 + 含浅色 TEXT_PRIMARY + 不含深色 BG_WINDOW（已被替换）。
- THEME_DARK/LIGHT/AVAILABLE_THEMES 常量契约。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.theme_switcher import (
    AVAILABLE_THEMES,
    THEME_DARK,
    THEME_LIGHT,
    _dark_to_light_map,
    build_light_qss,
    build_themed_qss,
    default_accent_id,
)


# ── 常量契约 ─────────────────────────────────────────────────────────────


def test_theme_dark_constant():
    """THEME_DARK = serial_station_dark。"""

    assert THEME_DARK == "serial_station_dark"


def test_theme_light_constant():
    """THEME_LIGHT = serial_station_light。"""

    assert THEME_LIGHT == "serial_station_light"


def test_available_themes_has_two():
    """AVAILABLE_THEMES 含 dark + light 2 个主题。"""

    assert len(AVAILABLE_THEMES) == 2
    assert THEME_DARK in AVAILABLE_THEMES
    assert THEME_LIGHT in AVAILABLE_THEMES


def test_default_accent_id_returns_cyan():
    """default_accent_id() 返回 cyan（DEFAULT_ACCENT_ID）。"""

    assert default_accent_id() == "cyan"


# ── _dark_to_light_map ───────────────────────────────────────────────────


def test_dark_to_light_map_returns_dict():
    """_dark_to_light_map 返回非空 dict。"""

    mapping = _dark_to_light_map()
    assert isinstance(mapping, dict)
    assert len(mapping) > 0


def test_dark_to_light_map_only_includes_differing_values():
    """映射只含 dark≠light 的项（相同值不入映射）。"""

    mapping = _dark_to_light_map()
    for dark_val, light_val in mapping.items():
        assert dark_val != light_val, f"same value in mapping: {dark_val}"


def test_dark_to_light_map_values_are_light_tokens():
    """映射值来自 light_tokens（不是随意字符串）。"""

    mapping = _dark_to_light_map()
    light_values = set(light.all_tokens().values())
    for light_val in mapping.values():
        assert light_val in light_values


def test_dark_to_light_map_keys_are_dark_tokens():
    """映射键来自 dark_tokens。"""

    mapping = _dark_to_light_map()
    dark_values = set(dark.all_tokens().values())
    for dark_val in mapping:
        assert dark_val in dark_values


def test_dark_to_light_map_contains_bg_window():
    """映射含 dark.BG_WINDOW（深浅差异最大的核心色）。"""

    mapping = _dark_to_light_map()
    assert dark.BG_WINDOW in mapping
    assert mapping[dark.BG_WINDOW] != dark.BG_WINDOW  # 映射到不同值


# ── build_light_qss ──────────────────────────────────────────────────────


def test_build_light_qss_non_empty():
    """build_light_qss 返回非空字符串。"""

    qss = build_light_qss()
    assert isinstance(qss, str)
    assert len(qss) > 100  # 有实质内容


def test_build_light_qss_contains_light_text_primary():
    """浅色 QSS 含 light.TEXT_PRIMARY（替换后的浅色文字色）。"""

    qss = build_light_qss()
    assert light.TEXT_PRIMARY in qss


def test_build_light_qss_does_not_contain_dark_bg_window():
    """浅色 QSS 不含 dark.BG_WINDOW（深色背景已被替换）。"""

    qss = build_light_qss()
    assert dark.BG_WINDOW not in qss


# ── build_themed_qss ─────────────────────────────────────────────────────


def test_build_themed_qss_dark_returns_non_empty():
    """build_themed_qss(THEME_DARK) 返回非空深色 QSS。"""

    qss = build_themed_qss(THEME_DARK)
    assert isinstance(qss, str)
    assert len(qss) > 100


def test_build_themed_qss_light_returns_non_empty():
    """build_themed_qss(THEME_LIGHT) 返回非空浅色 QSS。"""

    qss = build_themed_qss(THEME_LIGHT)
    assert isinstance(qss, str)
    assert len(qss) > 100


def test_build_themed_qss_dark_and_light_differ():
    """dark 和 light QSS 不同。"""

    dark_qss = build_themed_qss(THEME_DARK)
    light_qss = build_themed_qss(THEME_LIGHT)
    assert dark_qss != light_qss


def test_build_themed_qss_with_accent_does_not_crash():
    """build_themed_qss 带 accent_id 不崩溃。"""

    qss = build_themed_qss(THEME_DARK, accent_id="blue")
    assert isinstance(qss, str)
    assert len(qss) > 0


def test_build_themed_qss_dark_contains_dark_bg():
    """深色 QSS 含 dark.BG_WINDOW。"""

    qss = build_themed_qss(THEME_DARK)
    assert dark.BG_WINDOW in qss


def test_build_themed_qss_light_does_not_contain_dark_bg():
    """浅色 QSS 不含 dark.BG_WINDOW（已被替换）。"""

    qss = build_themed_qss(THEME_LIGHT)
    assert dark.BG_WINDOW not in qss

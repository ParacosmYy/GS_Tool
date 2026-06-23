"""settings_service DEFAULT_* 常量 + UserSettings 字段边界测试。

DEFAULT_* 常量 + UserSettings dataclass 字段此前无直接测试。
本文件覆盖常量值 + dataclass 字段 + update 未知字段。

覆盖：
1. DEFAULT_THEME = 'serial_station_dark'。
2. DEFAULT_ACCENT = 'cyan'。
3. DEFAULT_FONT_POINT = 13。
4. DEFAULT_BAUDRATE = 115200。
5. DEFAULT_ANIMATION_ENABLED = True。
6. UserSettings 全字段默认值。
7. UserSettings 可变（非 frozen）。
8. update 未知字段 raises AttributeError。
9. UserSettings theme 字段赋值。
10. DEFAULT_DATA_DIR = ''。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.services.settings_service import (
    DEFAULT_ACCENT,
    DEFAULT_ANIMATION_ENABLED,
    DEFAULT_BAUDRATE,
    DEFAULT_DATA_DIR,
    DEFAULT_FONT_POINT,
    DEFAULT_THEME,
    SettingsManager,
    UserSettings,
)


# ── DEFAULT_* 常量 ───────────────────────────────────────────────
def test_default_theme():
    assert DEFAULT_THEME == "serial_station_dark"


def test_default_accent():
    assert DEFAULT_ACCENT == "cyan"


def test_default_font_point():
    assert DEFAULT_FONT_POINT == 13


def test_default_baudrate():
    assert DEFAULT_BAUDRATE == 115200


def test_default_animation_enabled():
    assert DEFAULT_ANIMATION_ENABLED is True


def test_default_data_dir_empty():
    assert DEFAULT_DATA_DIR == ""


# ── UserSettings dataclass ───────────────────────────────────────
def test_user_settings_all_defaults():
    s = UserSettings()
    assert s.theme == DEFAULT_THEME
    assert s.accent == DEFAULT_ACCENT
    assert s.font_point == DEFAULT_FONT_POINT
    assert s.data_dir == DEFAULT_DATA_DIR
    assert s.default_baudrate == DEFAULT_BAUDRATE
    assert s.animation_enabled is DEFAULT_ANIMATION_ENABLED


def test_user_settings_mutable():
    """UserSettings 非 frozen（可变）。"""

    s = UserSettings()
    s.theme = "custom"
    assert s.theme == "custom"


def test_user_settings_custom_values():
    s = UserSettings(theme="light", accent="rose", font_point=16)
    assert s.theme == "light"
    assert s.accent == "rose"
    assert s.font_point == 16


def test_user_settings_baudrate_custom():
    s = UserSettings(default_baudrate=9600)
    assert s.default_baudrate == 9600


# ── update 未知字段 ──────────────────────────────────────────────
def test_update_unknown_key_raises():
    SettingsManager.reset_singleton()
    mgr = SettingsManager()
    mgr.reset()
    with pytest.raises(AttributeError):
        mgr.update(nonexistent_field=42)
    SettingsManager.reset_singleton()

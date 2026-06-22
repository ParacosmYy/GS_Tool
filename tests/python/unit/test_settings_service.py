"""``SettingsManager`` 用户偏好持久化单元测试。

覆盖：默认值加载、update() 写入与缓存同步、reset() 回默认、单例、reload、
QSettings 异常容错、theme_store 镜像、类型容错、包导出。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

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


@pytest.fixture(autouse=True)
def _isolated_settings():
    """每个测试前后清掉单例缓存与 QSettings 中所有键，避免互相污染。"""

    SettingsManager.reset_singleton()
    SettingsManager().reset()
    SettingsManager.reset_singleton()
    yield
    SettingsManager.reset_singleton()
    SettingsManager().reset()
    SettingsManager.reset_singleton()


# ── 默认值 ──────────────────────────────────────────────────────────
def test_defaults_on_fresh_load():
    s = SettingsManager().get()
    assert s.theme == DEFAULT_THEME
    assert s.accent == DEFAULT_ACCENT
    assert s.font_point == DEFAULT_FONT_POINT
    assert s.data_dir == DEFAULT_DATA_DIR
    assert s.default_baudrate == DEFAULT_BAUDRATE
    assert s.animation_enabled is DEFAULT_ANIMATION_ENABLED


def test_defaults_dataclass_matches_manager_defaults():
    d = UserSettings()
    assert d.theme == DEFAULT_THEME
    assert d.accent == DEFAULT_ACCENT
    assert d.font_point == DEFAULT_FONT_POINT
    assert d.default_baudrate == DEFAULT_BAUDRATE


# ── update() ────────────────────────────────────────────────────────
def test_update_single_field_persists_and_caches():
    mgr = SettingsManager()
    mgr.update(theme="serial_station_light")
    assert mgr.get().theme == "serial_station_light"


def test_update_multiple_fields():
    mgr = SettingsManager()
    mgr.update(
        font_point=18,
        default_baudrate=9600,
        animation_enabled=False,
        data_dir="/tmp/data",
    )
    s = mgr.get()
    assert s.font_point == 18
    assert s.default_baudrate == 9600
    assert s.animation_enabled is False
    assert s.data_dir == "/tmp/data"


def test_update_persists_across_singleton_reset():
    SettingsManager().update(font_point=20)
    SettingsManager.reset_singleton()
    assert SettingsManager().get().font_point == 20


def test_update_returns_cache():
    mgr = SettingsManager()
    assert mgr.update(theme="serial_station_light") is mgr.get()


def test_update_unknown_key_raises_attribute_error():
    with pytest.raises(AttributeError):
        SettingsManager().update(nonexistent_field=42)


# ── reset() ─────────────────────────────────────────────────────────
def test_reset_restores_all_defaults():
    mgr = SettingsManager()
    mgr.update(
        theme="serial_station_light",
        accent="purple",
        font_point=20,
        data_dir="/x",
        default_baudrate=9600,
        animation_enabled=False,
    )
    s = mgr.reset()
    assert s.theme == DEFAULT_THEME
    assert s.accent == DEFAULT_ACCENT
    assert s.font_point == DEFAULT_FONT_POINT
    assert s.data_dir == DEFAULT_DATA_DIR
    assert s.default_baudrate == DEFAULT_BAUDRATE
    assert s.animation_enabled is DEFAULT_ANIMATION_ENABLED


def test_reset_clears_persistence():
    SettingsManager().update(font_point=22)
    SettingsManager().reset()
    SettingsManager.reset_singleton()
    assert SettingsManager().get().font_point == DEFAULT_FONT_POINT


# ── 单例 ────────────────────────────────────────────────────────────
def test_instance_returns_same_object():
    assert SettingsManager.instance() is SettingsManager.instance()


def test_reset_singleton_then_new_instance_differs():
    a = SettingsManager.instance()
    SettingsManager.reset_singleton()
    b = SettingsManager.instance()
    assert a is not b


# ── reload() ────────────────────────────────────────────────────────
def test_reload_picks_up_external_writes():
    """通过另一实例写入后，reload() 应能读到外部直接 setValue 的新值。"""

    mgr = SettingsManager()
    mgr._settings.setValue("font_point", 11)
    mgr._settings.sync()
    assert mgr.get().font_point == DEFAULT_FONT_POINT  # 缓存未变
    mgr.reload()
    assert mgr.get().font_point == 11


# ── 异常容错 ────────────────────────────────────────────────────────
def test_load_falls_back_when_qsettings_value_raises(monkeypatch):
    SettingsManager.reset_singleton()
    mgr = SettingsManager()

    def boom(*args, **kwargs):
        raise RuntimeError("simulated registry failure")

    monkeypatch.setattr(mgr._settings, "value", boom)
    result = mgr._load()
    assert isinstance(result, UserSettings)
    assert result.theme == DEFAULT_THEME


def test_update_does_not_raise_when_qsettings_fails(monkeypatch):
    mgr = SettingsManager()

    def boom(*args, **kwargs):
        raise RuntimeError("disk full")

    monkeypatch.setattr(mgr._settings, "setValue", boom)
    mgr.update(font_point=15)
    assert mgr.get().font_point == 15


def test_reset_does_not_raise_when_qsettings_fails(monkeypatch):
    mgr = SettingsManager()

    def boom(*args, **kwargs):
        raise RuntimeError("permission denied")

    monkeypatch.setattr(mgr._settings, "remove", boom)
    mgr.reset()
    assert mgr.get().theme == DEFAULT_THEME


# ── theme_store 镜像 ───────────────────────────────────────────────
def test_update_theme_mirrors_to_theme_store():
    from embeddebug.serial_station.ui.theme import theme_store

    SettingsManager().update(theme="serial_station_light", accent="purple")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_light"
    assert prefs["accent"] == "purple"


def test_reset_mirrors_to_theme_store():
    from embeddebug.serial_station.ui.theme import theme_store

    mgr = SettingsManager()
    mgr.update(theme="serial_station_light")
    mgr.reset()
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == DEFAULT_THEME
    assert prefs["accent"] == DEFAULT_ACCENT


# ── 类型容错 ────────────────────────────────────────────────────────
def test_read_bool_accepts_string_true():
    mgr = SettingsManager()
    mgr._settings.setValue("animation_enabled", "false")
    mgr._settings.sync()
    mgr.reload()
    assert mgr.get().animation_enabled is False


def test_read_int_with_bad_value_falls_back(monkeypatch):
    mgr = SettingsManager()

    def bad_value(*args, **kwargs):
        return "not-an-int"

    monkeypatch.setattr(mgr._settings, "value", bad_value)
    result = mgr._load()
    assert result.font_point == DEFAULT_FONT_POINT
    assert result.default_baudrate == DEFAULT_BAUDRATE


# ── exports ─────────────────────────────────────────────────────────
def test_services_init_exports_settings_manager():
    from embeddebug.serial_station.services import SettingsManager as SM
    from embeddebug.serial_station.services import UserSettings as US

    assert SM is SettingsManager
    assert US is UserSettings

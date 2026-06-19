"""主题偏好持久化测试（theme_store）—— theme + accent 统一落盘。

覆盖：
- theme save/load 往返；缺失/损坏/非法字段回退默认深色。
- 统一 JSON：save_theme 不丢 accent，save_accent 不丢 theme（合并写）。
- load_theme_prefs 同时返回 theme + accent 默认。
- 旧 accent.json 迁移回退（新文件不存在时继承旧 accent）。
- ThemeSwitcher.apply_dark/apply_light 落盘 theme 选择。
- 启动路径模拟：set theme+accent → 重置 → restore → 一致。
"""

from __future__ import annotations

import json
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.theme import theme_store


def _isolate(monkeypatch, tmp_path):
    prefs_target = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy_target = tmp_path / "embeddebug" / "accent.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs_target)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_target)
    return prefs_target, legacy_target


# ── theme id save/load ───────────────────────────────────────────
def test_theme_save_load_roundtrip(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.save_theme_id("serial_station_light") is True
    assert theme_store.load_theme_id() == "serial_station_light"


def test_theme_load_missing_defaults_to_dark(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.load_theme_id() == "serial_station_dark"


def test_theme_load_corrupt_defaults(monkeypatch, tmp_path):
    prefs, _ = _isolate(monkeypatch, tmp_path)
    prefs.parent.mkdir(parents=True, exist_ok=True)
    prefs.write_text("{ broken", encoding="utf-8")
    assert theme_store.load_theme_id() == "serial_station_dark"


def test_theme_load_empty_field_defaults(monkeypatch, tmp_path):
    prefs, _ = _isolate(monkeypatch, tmp_path)
    prefs.parent.mkdir(parents=True, exist_ok=True)
    prefs.write_text(json.dumps({"theme": ""}), encoding="utf-8")
    assert theme_store.load_theme_id() == "serial_station_dark"


def test_theme_save_custom_default_param(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.load_theme_id(default="serial_station_light") == "serial_station_light"


# ── 统一 JSON：theme + accent 不互丢 ──────────────────────────────
def test_save_theme_preserves_accent(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    theme_store.save_accent_id("purple")
    theme_store.save_theme_id("serial_station_light")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_light"
    assert prefs["accent"] == "purple"  # accent 不丢


def test_save_accent_preserves_theme(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    theme_store.save_theme_id("serial_station_light")
    theme_store.save_accent_id("green")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_light"  # theme 不丢
    assert prefs["accent"] == "green"


def test_save_theme_prefs_both_at_once(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.save_theme_prefs(theme="serial_station_light", accent="amber") is True
    prefs = theme_store.load_theme_prefs()
    assert prefs == {"theme": "serial_station_light", "accent": "amber"}


def test_load_theme_prefs_defaults_both(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    prefs = theme_store.load_theme_prefs()
    assert prefs == {"theme": "serial_station_dark", "accent": "cyan"}


def test_theme_save_is_atomic(monkeypatch, tmp_path):
    prefs, _ = _isolate(monkeypatch, tmp_path)
    theme_store.save_theme_id("serial_station_light")
    assert not (prefs.with_suffix(prefs.suffix + ".tmp")).exists()


# ── 旧 accent.json 迁移回退 ──────────────────────────────────────
def test_legacy_accent_json_migrated_on_first_read(monkeypatch, tmp_path):
    """新文件不存在但旧 accent.json 存在时，accent 继承、theme 走默认。"""

    _, legacy = _isolate(monkeypatch, tmp_path)
    legacy.parent.mkdir(parents=True, exist_ok=True)
    legacy.write_text(json.dumps({"accent": "rose"}), encoding="utf-8")
    prefs = theme_store.load_theme_prefs()
    assert prefs["accent"] == "rose"
    assert prefs["theme"] == "serial_station_dark"


def test_legacy_accent_ignored_when_new_prefs_exists(monkeypatch, tmp_path):
    """新 theme_prefs.json 存在时，旧 accent.json 不再回退读取。"""

    prefs, legacy = _isolate(monkeypatch, tmp_path)
    prefs.parent.mkdir(parents=True, exist_ok=True)
    prefs.write_text(json.dumps({"theme": "serial_station_light", "accent": "teal"}), encoding="utf-8")
    legacy.parent.mkdir(parents=True, exist_ok=True)
    legacy.write_text(json.dumps({"accent": "rose"}), encoding="utf-8")  # 应被忽略
    assert theme_store.load_theme_prefs()["accent"] == "teal"


# ── ThemeSwitcher 落盘集成 ────────────────────────────────────────
def test_switcher_apply_light_persists_theme(monkeypatch, tmp_path, qapp):
    _isolate(monkeypatch, tmp_path)
    from embeddebug.serial_station.ui.theme.theme_switcher import (
        THEME_LIGHT,
        ThemeSwitcher,
    )

    switcher = ThemeSwitcher(qapp)
    switcher.apply_light()
    assert theme_store.load_theme_id() == THEME_LIGHT


def test_switcher_apply_dark_persists_theme(monkeypatch, tmp_path, qapp):
    _isolate(monkeypatch, tmp_path)
    # 先存浅色，再切深色，验证落盘更新。
    theme_store.save_theme_id("serial_station_light")
    from embeddebug.serial_station.ui.theme.theme_switcher import THEME_DARK, ThemeSwitcher

    switcher = ThemeSwitcher(qapp)
    switcher.apply_dark()
    assert theme_store.load_theme_id() == THEME_DARK


# ── 启动路径模拟：theme + accent 同时恢复 ─────────────────────────
def test_startup_restore_both_theme_and_accent(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    from embeddebug.serial_station.ui.theme import accents

    # 模拟用户上次选了 浅色 + 紫色
    theme_store.save_theme_prefs(theme="serial_station_light", accent="purple")
    # 模拟重启：重置运行时状态
    accents.reset_active_accent()
    # 启动期恢复
    persisted_theme = theme_store.load_theme_id()
    restored_accent = accents.restore_active_accent()
    assert persisted_theme == "serial_station_light"
    assert restored_accent == "purple"
    accents.reset_active_accent()

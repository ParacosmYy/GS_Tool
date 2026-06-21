"""ThemeManager + theme_store 单元测试 — 色板/QSS/应用/资源解析 + 偏好持久化。"""
from __future__ import annotations
import json
import os
import re
from pathlib import Path
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import pytest
from embeddebug.serial_station.ui.theme import (
    BUILTIN_THEMES, DEFAULT_THEME, ThemeManager, apply_theme,
    current_theme_name, palette_tokens, size_tokens,
)
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import theme_store
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
REPO_ROOT = Path(__file__).resolve().parents[3]
UI_DIR = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"
EXPECTED_PALETTE_KEYS = (
    "bg_window", "bg_panel", "text_primary", "accent", "success",
    "warning", "error", "term_rx", "term_tx", "border", "scrollbar",
)
EXPECTED_SIZE_KEYS = (
    "radius_md", "spacing_md", "padding_md",
    "border_thin", "font_base", "font_family_mono",
)
SMOKE_CRITICAL_OBJECTNAMES = (
    "serialStationConnectButton", "serialStationSendEdit",
    "serialStationCommandHistoryCombo", "serialStationSendButton",
    "serialStationInjectEdit", "serialStationInjectButton",
    "serialStationClearButton", "serialStationLogView", "serialStationStatusLabel",
)
def _extract_serial_station_objectnames() -> set[str]:
    pattern = re.compile(r'setObjectName\(\s*["\']([A-Za-z0-9_]+)["\']\s*\)')
    names: set[str] = set()
    for path in UI_DIR.rglob("*.py"):
        if "theme" in path.parts:
            continue
        text = path.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            name = match.group(1)
            if name.startswith("serialStation"):
                names.add(name)
    return names
def _isolate(monkeypatch, tmp_path):
    prefs_target = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy_target = tmp_path / "embeddebug" / "accent.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs_target)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_target)
    return prefs_target, legacy_target
def test_palette_tokens_contain_all_expected_keys():
    tokens = palette_tokens()
    for key in EXPECTED_PALETTE_KEYS:
        assert key in tokens, f"palette missing token: {key}"
    assert all(isinstance(v, str) and v for v in tokens.values())
def test_size_tokens_contain_all_expected_keys():
    tokens = size_tokens()
    for key in EXPECTED_SIZE_KEYS:
        assert key in tokens, f"size token missing: {key}"
    assert all(isinstance(v, str) and v for v in tokens.values())
def test_palette_constants_match_modern_dark_industrial_palette():
    assert P.BG_WINDOW == "#0d1118"
    assert P.ACCENT == "#22d3ee"
    assert P.SUCCESS == "#22c55e"
    assert P.WARNING == "#f59e0b"
    assert P.ERROR == "#ef4444"
    assert P.TERM_TX == "#38bdf8"
    assert P.TERM_RX == "#22c55e"
def test_build_qss_returns_non_empty_industrial_stylesheet():
    qss = build_qss()
    assert isinstance(qss, str)
    assert len(qss) > 1000
    assert qss.endswith("\n")
@pytest.mark.parametrize("object_name", SMOKE_CRITICAL_OBJECTNAMES)
def test_build_qss_covers_smoke_critical_objectnames(object_name):
    qss = build_qss()
    assert f"#{object_name}" in qss, f"QSS missing critical objectName: {object_name}"
def test_build_qss_embeds_three_button_states():
    qss = build_qss()
    for state in (":hover", ":pressed", ":disabled", ":focus"):
        assert state in qss, f"QSS missing button state: {state}"
def test_build_qss_embeds_industrial_accent_color():
    qss = build_qss()
    assert P.ACCENT in qss
    assert P.TERM_BACKGROUND in qss
def test_theme_manager_is_singleton():
    assert ThemeManager() is ThemeManager()
def test_theme_manager_reset_clears_current_theme():
    manager = ThemeManager()
    manager._current_theme = "stale"
    manager.reset()
    assert manager.current_theme is None
def test_apply_theme_applies_qss_to_application(qapp):
    manager = ThemeManager()
    manager.reset()
    qss = manager.apply_theme(qapp, DEFAULT_THEME)
    assert qss
    assert manager.current_theme == DEFAULT_THEME
    assert "#serialStationConnectButton" in qapp.styleSheet()
def test_apply_theme_convenience_function_updates_current_name(qapp):
    ThemeManager().reset()
    qss = apply_theme(qapp, DEFAULT_THEME)
    assert qss
    assert current_theme_name() == DEFAULT_THEME
def test_builtin_themes_contains_default():
    assert DEFAULT_THEME in BUILTIN_THEMES
def test_load_qss_prefers_external_file_when_present():
    external = REPO_ROOT / "resources" / "themes" / f"{DEFAULT_THEME}.qss"
    if not external.is_file():
        pytest.skip(f"external theme file not present: {external}")
    loaded = ThemeManager().load_qss(DEFAULT_THEME)
    assert "serialStationConnectButton" in loaded
def test_resolve_resource_path_finds_themes_directory():
    resolved = ThemeManager.resolve_resource_path(Path("resources") / "themes")
    assert resolved.is_dir()
    assert (resolved / f"{DEFAULT_THEME}.qss").is_file()
def test_resolve_resource_path_finds_existing_resource_file():
    resolved = ThemeManager.resolve_resource_path(
        Path("resources") / "themes" / f"{DEFAULT_THEME}.qss")
    assert resolved.is_file()
def test_resolve_resource_path_returns_candidate_for_missing_resource():
    resolved = ThemeManager.resolve_resource_path(
        Path("resources") / "nonexistent_xyz.qss")
    assert not resolved.exists()
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
def test_save_theme_preserves_accent(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    theme_store.save_accent_id("purple")
    theme_store.save_theme_id("serial_station_light")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_light"
    assert prefs["accent"] == "purple"
def test_save_accent_preserves_theme(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    theme_store.save_theme_id("serial_station_light")
    theme_store.save_accent_id("green")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_light"
    assert prefs["accent"] == "green"
def test_save_theme_prefs_both_at_once(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.save_theme_prefs(theme="serial_station_light", accent="amber") is True
    assert theme_store.load_theme_prefs() == {"theme": "serial_station_light", "accent": "amber"}
def test_load_theme_prefs_defaults_both(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert theme_store.load_theme_prefs() == {"theme": "serial_station_dark", "accent": "cyan"}
def test_theme_save_is_atomic(monkeypatch, tmp_path):
    prefs, _ = _isolate(monkeypatch, tmp_path)
    theme_store.save_theme_id("serial_station_light")
    assert not (prefs.with_suffix(prefs.suffix + ".tmp")).exists()
def test_legacy_accent_json_migrated_on_first_read(monkeypatch, tmp_path):
    _, legacy = _isolate(monkeypatch, tmp_path)
    legacy.parent.mkdir(parents=True, exist_ok=True)
    legacy.write_text(json.dumps({"accent": "rose"}), encoding="utf-8")
    prefs = theme_store.load_theme_prefs()
    assert prefs["accent"] == "rose"
    assert prefs["theme"] == "serial_station_dark"
def test_legacy_accent_ignored_when_new_prefs_exists(monkeypatch, tmp_path):
    prefs, legacy = _isolate(monkeypatch, tmp_path)
    prefs.parent.mkdir(parents=True, exist_ok=True)
    prefs.write_text(json.dumps({"theme": "serial_station_light", "accent": "teal"}), encoding="utf-8")
    legacy.parent.mkdir(parents=True, exist_ok=True)
    legacy.write_text(json.dumps({"accent": "rose"}), encoding="utf-8")
    assert theme_store.load_theme_prefs()["accent"] == "teal"
def test_switcher_apply_light_persists_theme(monkeypatch, tmp_path, qapp):
    _isolate(monkeypatch, tmp_path)
    from embeddebug.serial_station.ui.theme.theme_switcher import THEME_LIGHT, ThemeSwitcher
    ThemeSwitcher(qapp).apply_light()
    assert theme_store.load_theme_id() == THEME_LIGHT
def test_switcher_apply_dark_persists_theme(monkeypatch, tmp_path, qapp):
    _isolate(monkeypatch, tmp_path)
    theme_store.save_theme_id("serial_station_light")
    from embeddebug.serial_station.ui.theme.theme_switcher import THEME_DARK, ThemeSwitcher
    ThemeSwitcher(qapp).apply_dark()
    assert theme_store.load_theme_id() == THEME_DARK
def test_startup_restore_both_theme_and_accent(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    from embeddebug.serial_station.ui.theme import accents
    theme_store.save_theme_prefs(theme="serial_station_light", accent="purple")
    accents.reset_active_accent()
    persisted_theme = theme_store.load_theme_id()
    restored_accent = accents.restore_active_accent()
    assert persisted_theme == "serial_station_light"
    assert restored_accent == "purple"
    accents.reset_active_accent()

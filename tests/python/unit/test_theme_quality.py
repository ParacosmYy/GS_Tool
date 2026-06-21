"""theme_serializer JSON 导出/导入 + QSS 覆盖率守护 + settings_panel 实时预览测试。"""
from __future__ import annotations
import inspect
import os
import re
from pathlib import Path
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import pytest
from embeddebug.serial_station.ui.theme import palette
from embeddebug.serial_station.ui.theme.qss_builder import build_qss
from embeddebug.serial_station.ui.theme.theme_serializer import (
    ThemeSnapshot, apply_overrides, deserialize_to_dict, export_current_theme,
    from_json, load_from_file, save_to_file, serialize_palette, to_json,
    validate_color_string, validate_snapshot,
)
REPO_ROOT = Path(__file__).resolve().parents[3]
UI_DIR = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"
STYLE_EXEMPT_SUFFIXES = ("Shortcut", "CursorX", "CursorY")
_OBJECTNAME_PATTERN = re.compile(r'setObjectName\(\s*["\']([A-Za-z0-9_]+)["\']\s*\)')
def _extract_serial_station_objectnames() -> set[str]:
    names: set[str] = set()
    for path in UI_DIR.rglob("*.py"):
        if "theme" in path.parts:
            continue
        text = path.read_text(encoding="utf-8")
        for match in _OBJECTNAME_PATTERN.finditer(text):
            name = match.group(1)
            if name.startswith("serialStation"):
                names.add(name)
    return names
def _is_style_exempt(name: str) -> bool:
    return any(name.endswith(suffix) for suffix in STYLE_EXEMPT_SUFFIXES)
def _make_settings_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel
    panel = SettingsPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel
def _demo_snapshot() -> ThemeSnapshot:
    return ThemeSnapshot(
        name="demo", version="1.0",
        colors={"ACCENT": "#22d3ee", "BG": "#0d1118"},
        metadata={"source": "test", "generated_at": "2025-01-01T00:00:00Z"},
    )
def test_serialize_palette_extracts_constants():
    assert len(serialize_palette(palette).colors) >= 30
def test_serialize_palette_includes_known_keys():
    snapshot = serialize_palette(palette)
    for key in ("BG_PANEL", "ACCENT", "TEXT_PRIMARY", "ERROR"):
        assert key in snapshot.colors, f"missing expected key {key!r}"
def test_serialize_palette_excludes_non_string_members():
    snapshot = serialize_palette(palette)
    assert "WAVE_CURVES" not in snapshot.colors
    assert "all_tokens" not in snapshot.colors
def test_serialize_palette_metadata_fields():
    snapshot = serialize_palette(palette)
    assert snapshot.name == palette.__name__
    assert snapshot.version == "1.0"
    assert "source" in snapshot.metadata
    assert "generated_at" in snapshot.metadata
def test_to_json_roundtrip():
    original = _demo_snapshot()
    assert from_json(to_json(original)) == original
def test_to_json_indent_option():
    assert "\n    " in to_json(_demo_snapshot(), indent=4)
def test_from_json_missing_keys_raises():
    with pytest.raises(ValueError):
        from_json("{}")
def test_from_json_partial_keys_raises():
    with pytest.raises(ValueError):
        from_json('{"name": "x", "version": "1.0"}')
def test_from_json_malformed_raises():
    with pytest.raises(ValueError):
        from_json("not json")
def test_validate_color_hex_6_valid():
    assert validate_color_string("#22d3ee") is True
def test_validate_color_rgba_valid():
    assert validate_color_string("rgba(34,211,238,0.5)") is True
def test_validate_color_invalid():
    assert validate_color_string("not-a-color") is False
def test_validate_color_empty_string_invalid():
    assert validate_color_string("") is False
def test_validate_color_qss_named_color():
    assert validate_color_string("red") is True
def test_validate_snapshot_returns_warnings_for_invalid():
    snapshot = ThemeSnapshot(
        name="bad", version="1.0",
        colors={"GOOD": "#22d3ee", "BAD": "not-a-color"}, metadata={})
    warnings = validate_snapshot(snapshot)
    assert len(warnings) == 1
    assert "BAD" in warnings[0]
def test_validate_snapshot_clean_when_all_valid():
    assert validate_snapshot(_demo_snapshot()) == []
def test_apply_overrides_changes_existing():
    original = palette.ACCENT
    try:
        changed = apply_overrides(palette, {"ACCENT": "#ff0000"})
        assert changed == ["ACCENT"]
        assert palette.ACCENT == "#ff0000"
    finally:
        palette.ACCENT = original
    assert palette.ACCENT == original
def test_apply_overrides_skips_unknown():
    changed = apply_overrides(palette, {"NONEXISTENT_ATTR": "x"})
    assert changed == []
    assert not hasattr(palette, "NONEXISTENT_ATTR")
def test_apply_overrides_returns_only_changed_names():
    original = palette.BG_PANEL
    try:
        changed = apply_overrides(palette, {"BG_PANEL": "#111111", "MISSING_KEY": "#222222"})
        assert changed == ["BG_PANEL"]
    finally:
        palette.BG_PANEL = original
def test_deserialize_to_dict_returns_independent_copy():
    snapshot = _demo_snapshot()
    as_dict = deserialize_to_dict(snapshot)
    assert as_dict == snapshot.colors
    as_dict["ACCENT"] = "#changed"
    assert snapshot.colors["ACCENT"] == "#22d3ee"
def test_save_load_file_roundtrip(tmp_path):
    original = serialize_palette(palette)
    target = tmp_path / "sub" / "theme.json"
    save_to_file(original, target)
    assert target.exists()
    assert load_from_file(target) == original
def test_save_to_file_creates_parent_dirs(tmp_path):
    target = tmp_path / "deep" / "nested" / "dir" / "theme.json"
    save_to_file(_demo_snapshot(), target)
    assert target.exists()
def test_load_from_file_missing_raises(tmp_path):
    with pytest.raises(FileNotFoundError):
        load_from_file(tmp_path / "nope.json")
def test_export_current_theme_name():
    snapshot = export_current_theme("myname")
    assert snapshot.name == "myname"
    assert len(snapshot.colors) >= 30
    assert snapshot.version == "1.0"
def test_export_current_theme_default_name():
    assert export_current_theme().name == "current"
def test_ui_source_contains_serial_station_objectnames():
    assert _extract_serial_station_objectnames(), "expected serialStation* objectNames in ui/ source"
def test_build_qss_covers_all_styled_serial_station_objectnames():
    qss = build_qss()
    names = _extract_serial_station_objectnames()
    missing = sorted(n for n in names if not _is_style_exempt(n) and f"#{n}" not in qss)
    assert not missing, f"QSS missing {len(missing)} objectName(s): {missing}"
def test_build_qss_documents_exempt_objectnames():
    qss = build_qss()
    names = _extract_serial_station_objectnames()
    exempt = sorted(n for n in names if _is_style_exempt(n))
    for name in exempt:
        assert f"#{name}" in qss, f"exempt objectName {name} must still have a contract placeholder"
def test_settings_panel_wires_preview_on_combo_change():
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel
    src = inspect.getsource(SettingsPanel._build_theme_tab)
    assert "currentIndexChanged" in src
    assert "_preview_theme" in src
def test_settings_panel_has_preview_method():
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel
    assert hasattr(SettingsPanel, "_preview_theme")
    assert callable(SettingsPanel._preview_theme)
def test_preview_applies_theme_and_updates_status(qtbot, monkeypatch):
    apply_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: apply_calls.append(name))
    panel = _make_settings_panel(qtbot)
    panel._theme_combo.setCurrentIndex(1)
    assert len(apply_calls) >= 1
    assert "预览" in panel._theme_status.text()
def test_preview_failure_silent(qtbot, monkeypatch):
    def _boom(*a, **k):
        raise RuntimeError("boom")
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name", _boom)
    panel = _make_settings_panel(qtbot)
    panel._theme_status.setText("before")
    panel._preview_theme(1)
    assert panel._theme_status.text() == "before"
def test_preview_does_not_notify_but_apply_does(qtbot, monkeypatch):
    notify_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels._notify.panel_notify",
        lambda *a, **k: notify_calls.append(a))
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: None)
    panel = _make_settings_panel(qtbot)
    before = len(notify_calls)
    panel._preview_theme(1)
    assert len(notify_calls) == before
    panel._apply_theme()
    assert len(notify_calls) > before
def test_combo_change_triggers_preview(qtbot, monkeypatch):
    apply_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: apply_calls.append(name))
    panel = _make_settings_panel(qtbot)
    panel._theme_combo.setCurrentIndex(1)
    assert any(call is not None for call in apply_calls)

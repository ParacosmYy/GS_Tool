"""Unit tests for ``theme_serializer`` — JSON palette export/import.

Kernel tests exercise the pure-Python serialization / validation logic without
a Qt runtime; the QColor-named-color test exercises the lazy Qt import path.
Palette mutations are restored in ``finally`` blocks to keep the module
pristine across the suite ( mirrors the teardown contract documented in the
serializer's module docstring).
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest  # noqa: E402

from embeddebug.serial_station.ui.theme import palette  # noqa: E402
from embeddebug.serial_station.ui.theme.theme_serializer import (  # noqa: E402
    ThemeSnapshot,
    apply_overrides,
    deserialize_to_dict,
    export_current_theme,
    from_json,
    load_from_file,
    save_to_file,
    serialize_palette,
    to_json,
    validate_color_string,
    validate_snapshot,
)


# ── fixtures / helpers ─────────────────────────────────────────────
def _demo_snapshot() -> ThemeSnapshot:
    """Deterministic snapshot for round-trip / equality assertions."""

    return ThemeSnapshot(
        name="demo",
        version="1.0",
        colors={"ACCENT": "#22d3ee", "BG": "#0d1118"},
        metadata={"source": "test", "generated_at": "2025-01-01T00:00:00Z"},
    )


# ── A) serialize_palette ───────────────────────────────────────────
def test_serialize_palette_extracts_constants():
    """palette exposes >=30 uppercase str constants (we have 63)."""

    snapshot = serialize_palette(palette)
    assert len(snapshot.colors) >= 30


def test_serialize_palette_includes_known_keys():
    snapshot = serialize_palette(palette)
    for key in ("BG_PANEL", "ACCENT", "TEXT_PRIMARY", "ERROR"):
        assert key in snapshot.colors, f"missing expected key {key!r}"


def test_serialize_palette_excludes_non_string_members():
    """WAVE_CURVES (tuple) and all_tokens (fn) must not leak into colors."""

    snapshot = serialize_palette(palette)
    assert "WAVE_CURVES" not in snapshot.colors
    assert "all_tokens" not in snapshot.colors


def test_serialize_palette_metadata_fields():
    snapshot = serialize_palette(palette)
    assert snapshot.name == palette.__name__
    assert snapshot.version == "1.0"
    assert "source" in snapshot.metadata
    assert "generated_at" in snapshot.metadata


# ── B) JSON round-trip ────────────────────────────────────────────
def test_to_json_roundtrip():
    original = _demo_snapshot()
    rebuilt = from_json(to_json(original))
    assert rebuilt == original


def test_to_json_indent_option():
    text = to_json(_demo_snapshot(), indent=4)
    assert "\n    " in text  # 4-space indent marker present


def test_from_json_missing_keys_raises():
    with pytest.raises(ValueError):
        from_json("{}")


def test_from_json_partial_keys_raises():
    with pytest.raises(ValueError):
        from_json('{"name": "x", "version": "1.0"}')


def test_from_json_malformed_raises():
    with pytest.raises(ValueError):
        from_json("not json")


# ── C) color validation ───────────────────────────────────────────
def test_validate_color_hex_6_valid():
    assert validate_color_string("#22d3ee") is True


def test_validate_color_rgba_valid():
    assert validate_color_string("rgba(34,211,238,0.5)") is True


def test_validate_color_invalid():
    assert validate_color_string("not-a-color") is False


def test_validate_color_empty_string_invalid():
    assert validate_color_string("") is False


def test_validate_color_qss_named_color():
    """CSS named color 'red' is recognized via QColor (Qt path)."""

    assert validate_color_string("red") is True


def test_validate_snapshot_returns_warnings_for_invalid():
    snapshot = ThemeSnapshot(
        name="bad",
        version="1.0",
        colors={"GOOD": "#22d3ee", "BAD": "not-a-color"},
        metadata={},
    )
    warnings = validate_snapshot(snapshot)
    assert len(warnings) == 1
    assert "BAD" in warnings[0]


def test_validate_snapshot_clean_when_all_valid():
    assert validate_snapshot(_demo_snapshot()) == []


# ── D) apply_overrides ────────────────────────────────────────────
def test_apply_overrides_changes_existing():
    original = palette.ACCENT
    try:
        changed = apply_overrides(palette, {"ACCENT": "#ff0000"})
        assert changed == ["ACCENT"]
        assert palette.ACCENT == "#ff0000"
    finally:
        palette.ACCENT = original
    assert palette.ACCENT == original  # teardown verified


def test_apply_overrides_skips_unknown():
    changed = apply_overrides(palette, {"NONEXISTENT_ATTR": "x"})
    assert changed == []
    assert not hasattr(palette, "NONEXISTENT_ATTR")


def test_apply_overrides_returns_only_changed_names():
    original = palette.BG_PANEL
    try:
        overrides = {"BG_PANEL": "#111111", "MISSING_KEY": "#222222"}
        changed = apply_overrides(palette, overrides)
        assert changed == ["BG_PANEL"]
    finally:
        palette.BG_PANEL = original


def test_deserialize_to_dict_returns_independent_copy():
    snapshot = _demo_snapshot()
    as_dict = deserialize_to_dict(snapshot)
    assert as_dict == snapshot.colors
    as_dict["ACCENT"] = "#changed"
    assert snapshot.colors["ACCENT"] == "#22d3ee"  # snapshot untouched


# ── E) file I/O ───────────────────────────────────────────────────
def test_save_load_file_roundtrip(tmp_path):
    original = serialize_palette(palette)
    target = tmp_path / "sub" / "theme.json"
    save_to_file(original, target)
    assert target.exists()
    rebuilt = load_from_file(target)
    assert rebuilt == original


def test_save_to_file_creates_parent_dirs(tmp_path):
    target = tmp_path / "deep" / "nested" / "dir" / "theme.json"
    save_to_file(_demo_snapshot(), target)
    assert target.exists()


def test_load_from_file_missing_raises(tmp_path):
    with pytest.raises(FileNotFoundError):
        load_from_file(tmp_path / "nope.json")


# ── F) convenience entrypoints ────────────────────────────────────
def test_export_current_theme_name():
    snapshot = export_current_theme("myname")
    assert snapshot.name == "myname"
    assert len(snapshot.colors) >= 30
    assert snapshot.version == "1.0"


def test_export_current_theme_default_name():
    assert export_current_theme().name == "current"

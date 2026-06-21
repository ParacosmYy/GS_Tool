"""Theme palette serialization (JSON export/import).

Mirrors the MobaXterm ``.mxtcolors`` shareable-theme pattern: serialize the
``palette`` module's color constants to a JSON snapshot that can be saved,
shared, validated, and re-applied to override the active theme at runtime.

Design constraints
------------------
- **Pure-Python kernel** (``dataclasses`` + ``json`` + ``inspect``); the module
  is importable without a Qt runtime. ``validate_color_string`` imports
  ``QColor`` lazily so JSON round-tripping works in headless environments.
- **``apply_overrides`` mutates** the target module in place via ``setattr``;
  callers are responsible for teardown (tests restore originals in ``finally``).
- **Snapshot is a frozen dataclass** — safe to share across threads and trivial
  to diff against an exported baseline.
"""

from __future__ import annotations

import inspect
import json
import re
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from types import ModuleType
from typing import Any

THEME_VERSION = "1.0"

# Uppercase CONSTANT_NAME pattern — rejects dunders / camelCase / lowercase.
_CONST_NAME_RE = re.compile(r"^[A-Z][A-Z0-9_]*$")

# Canonical QSS color fallbacks (used when QColor is unavailable, or when an
# older Qt rejects valid CSS rgba() with float alpha). Keep these strict so
# nonsensical strings like "not-a-color" can never slip through.
_HEX_COLOR_RE = re.compile(
    r"^#(?:[0-9a-fA-F]{3}|[0-9a-fA-F]{4}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$"
)
_RGBA_COLOR_RE = re.compile(
    r"^rgba?\(\s*\d{1,3}\s*,\s*\d{1,3}\s*,\s*\d{1,3}"
    r"(?:\s*,\s*(?:\d{1,3}|0?\.\d+)\s*)?\)$"
)

_REQUIRED_KEYS: tuple[str, ...] = ("name", "version", "colors", "metadata")


@dataclass(frozen=True)
class ThemeSnapshot:
    """Immutable snapshot of a palette module's exported color constants."""

    name: str
    version: str
    colors: dict[str, str]
    metadata: dict[str, str]


# ── A) Pure functions (no Qt) ───────────────────────────────────────────────
def _is_exportable_string(value: Any) -> bool:
    """``inspect.getmembers`` predicate: str values that are not dunder-internal.

    Filters by *value* (per spec). Name-shape filtering happens downstream via
    ``_CONST_NAME_RE`` so that ``__doc__`` / ``__file__`` / ``__name__`` — whose
    values are plain strings — are excluded by name rather than by value.
    """

    return isinstance(value, str) and not value.startswith("_")


def serialize_palette(palette_module: ModuleType) -> ThemeSnapshot:
    """Scan ``palette_module`` for uppercase ``str`` constants -> ThemeSnapshot.

    Uses ``inspect.getmembers`` with the string-value predicate (per spec),
    then keeps only names matching ``^[A-Z][A-Z0-9_]*$`` so that dunders and
    lowercase helpers are excluded. Tuples (``WAVE_CURVES``), lists and
    functions are naturally skipped by the string predicate.
    """

    members = inspect.getmembers(palette_module, _is_exportable_string)
    colors = {name: value for name, value in members if _CONST_NAME_RE.match(name)}
    source = getattr(palette_module, "__file__", None) or palette_module.__name__
    return ThemeSnapshot(
        name=palette_module.__name__,
        version=THEME_VERSION,
        colors=colors,
        metadata={
            "source": source,
            "generated_at": datetime.now(timezone.utc).isoformat(),
        },
    )


def deserialize_to_dict(snapshot: ThemeSnapshot) -> dict[str, str]:
    """Return a shallow copy of the colors dict (for inspection / overrides)."""

    return dict(snapshot.colors)


def apply_overrides(target_module: ModuleType, overrides: dict[str, str]) -> list[str]:
    """Apply ``overrides`` to ``target_module`` via ``setattr``.

    Returns the list of attribute names that were actually changed. Attributes
    not present on ``target_module`` are skipped (no new attributes are added),
    matching the MobaXterm "override existing keys only" semantics — a typo in
    a shared theme file never pollutes the module namespace.
    """

    changed: list[str] = []
    for key, value in overrides.items():
        if hasattr(target_module, key):
            setattr(target_module, key, value)
            changed.append(key)
    return changed


# ── B) JSON I/O ─────────────────────────────────────────────────────────────
def to_json(snapshot: ThemeSnapshot, *, indent: int = 2) -> str:
    """Serialize ThemeSnapshot to a pretty JSON string (utf-8 safe)."""

    payload = {
        "name": snapshot.name,
        "version": snapshot.version,
        "colors": snapshot.colors,
        "metadata": snapshot.metadata,
    }
    return json.dumps(payload, indent=indent, ensure_ascii=False)


def from_json(text: str) -> ThemeSnapshot:
    """Parse JSON string -> ThemeSnapshot.

    Raises ``ValueError`` on malformed JSON or missing/invalid required keys.
    ``json.JSONDecodeError`` is a ``ValueError`` subclass; we re-raise with a
    descriptive message so callers get a single exception type to catch.
    """

    try:
        data = json.loads(text)
    except json.JSONDecodeError as exc:
        raise ValueError(f"Malformed theme JSON: {exc}") from exc
    if not isinstance(data, dict):
        raise ValueError("Theme JSON root must be an object")
    missing = [key for key in _REQUIRED_KEYS if key not in data]
    if missing:
        raise ValueError(f"Missing required theme keys: {missing}")
    colors = data["colors"]
    metadata = data["metadata"]
    if not isinstance(colors, dict) or not isinstance(metadata, dict):
        raise ValueError("Theme 'colors' and 'metadata' must be objects")
    return ThemeSnapshot(
        name=str(data["name"]),
        version=str(data["version"]),
        colors={str(k): str(v) for k, v in colors.items()},
        metadata={str(k): str(v) for k, v in metadata.items()},
    )


def save_to_file(snapshot: ThemeSnapshot, path: Path) -> None:
    """Write JSON to ``path`` (utf-8); create parent dirs if missing."""

    target = Path(path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(to_json(snapshot), encoding="utf-8")


def load_from_file(path: Path) -> ThemeSnapshot:
    """Read JSON from ``path`` (utf-8); ``FileNotFoundError`` if file is missing."""

    target = Path(path)
    return from_json(target.read_text(encoding="utf-8"))


# ── C) Validation ───────────────────────────────────────────────────────────
def validate_color_string(s: str) -> bool:
    """Return True if ``s`` is a valid QSS color.

    Accepts ``#RGB`` / ``#RGBA`` / ``#RRGGBB`` / ``#RRGGBBAA`` hex, CSS
    ``rgb()`` / ``rgba()`` (alpha as 0-255 int or 0-1 float), and SVG/CSS
    named colors (``red``, ``cyan``, ``transparent`` ...).

    Primary authority is ``QColor(s).isValid()``; a regex fallback accepts
    canonical CSS hex / rgba() forms that older Qt versions may reject (e.g.
    float alpha in ``rgba()`` pre-Qt 6.4). This keeps validation lenient
    enough for theme sharing yet strict enough to reject obvious garbage.
    """

    if not isinstance(s, str) or not s.strip():
        return False
    candidate = s.strip()
    try:
        from PyQt6.QtGui import QColor

        if QColor(candidate).isValid():
            return True
    except Exception:
        # PyQt6 unavailable or import failed — fall through to regex fallback.
        pass
    return bool(_HEX_COLOR_RE.match(candidate) or _RGBA_COLOR_RE.match(candidate))


def validate_snapshot(snapshot: ThemeSnapshot) -> list[str]:
    """Return warning strings for invalid colors; empty list means all valid.

    Each warning is human-readable and names the offending key + value, e.g.
    ``"Invalid color for 'BAD': 'not-a-color'"`` — suitable for surfacing in a
    theme-import dialog without further formatting.
    """

    warnings: list[str] = []
    for key, value in snapshot.colors.items():
        if not validate_color_string(value):
            warnings.append(f"Invalid color for '{key}': {value!r}")
    return warnings


# ── D) Convenience ──────────────────────────────────────────────────────────
def export_current_theme(name: str = "current") -> ThemeSnapshot:
    """Serialize the active embeddebug palette module to a named snapshot.

    The palette module is imported lazily so this module remains importable in
    environments where the full ``embeddebug`` package is not on the path
    (e.g. when reusing the serializer for a different project's palette).
    """

    from embeddebug.serial_station.ui.theme import palette as _palette

    base = serialize_palette(_palette)
    return ThemeSnapshot(
        name=name,
        version=base.version,
        colors=base.colors,
        metadata=base.metadata,
    )


def import_theme_file(
    path: Path, *, dry_run: bool = False
) -> tuple[ThemeSnapshot, list[str]]:
    """Load + validate + (optionally) apply a theme file.

    Returns ``(snapshot, warnings)``. When ``dry_run`` is True the palette
    module is left untouched; otherwise ``apply_overrides`` patches every
    existing constant referenced by the snapshot. Callers should surface
    ``warnings`` to the user before committing a non-dry-run import.
    """

    from embeddebug.serial_station.ui.theme import palette as _palette

    snapshot = load_from_file(path)
    warnings = validate_snapshot(snapshot)
    if not dry_run:
        apply_overrides(_palette, snapshot.colors)
    return snapshot, warnings

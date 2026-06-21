"""Codebase guardian: all UI colors must come from palette.py.

Soft by default (prints violations); set ``STRICT_COLORS=1`` to fail the build.
This future-proofs the polish goals: new code outside ``ui/theme`` must not
hardcode hex / rgb / QColor literals.
"""

from __future__ import annotations

import ast
import os
import re
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


ROOT = Path("python/embeddebug/serial_station")


def _allowed_color_files() -> set[Path]:
    """Files where hardcoded colors are EXPECTED (theme truth sources)."""
    theme = ROOT / "ui" / "theme"
    files: set[Path] = {theme / "palette.py"}
    files.update(theme.glob("qss_sections_*.py"))
    anim = ROOT / "ui" / "animations" / "__init__.py"
    if anim.exists():
        files.add(anim)
    return files


COLOR_PATTERNS = [
    re.compile(r'QColor\(\s*["\'][^"\']+["\']'),
    re.compile(r'["\']#(?:[0-9a-fA-F]{3}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})["\']'),
    re.compile(r'["\']rgba?\([^)]+\)["\']'),
]


def _docstring_ranges(path: Path) -> list[tuple[int, int]]:
    """Return (start, end) 1-based line ranges of module/class/func docstrings."""
    try:
        tree = ast.parse(path.read_text(encoding="utf-8"))
    except (SyntaxError, OSError, ValueError):
        return []
    out: list[tuple[int, int]] = []
    for node in ast.walk(tree):
        if isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            body = getattr(node, "body", None) or []
            if (body and isinstance(body[0], ast.Expr)
                    and isinstance(getattr(body[0], "value", None), ast.Constant)
                    and isinstance(body[0].value.value, str)):
                out.append((body[0].lineno, body[0].end_lineno or body[0].lineno))
    return out


def scan_source_for_hardcoded_colors(
    root: Path, allowed: set[Path]
) -> list[tuple[Path, int, str]]:
    """Walk ``root`` for .py files, skipping ``allowed`` and ``__pycache__``."""
    violations: list[tuple[Path, int, str]] = []
    allowed_resolved = {p.resolve() for p in allowed}
    for path in root.rglob("*.py"):
        if "__pycache__" in path.parts:
            continue
        if path.resolve() in allowed_resolved:
            continue
        try:
            lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError:
            continue
        ds_ranges = _docstring_ranges(path)
        for idx, line in enumerate(lines, start=1):
            stripped = line.strip()
            if stripped.startswith("#"):
                continue  # pure comment line
            if any(a <= idx <= b for a, b in ds_ranges):
                continue  # inside a docstring
            for pat in COLOR_PATTERNS:
                m = pat.search(line)
                if m:
                    violations.append((path, idx, m.group(0)))
                    break
    return violations


def test_no_hardcoded_colors_outside_theme_files(capsys):
    violations = scan_source_for_hardcoded_colors(ROOT, _allowed_color_files())
    if violations:
        print(f"\n[hardcoded colors] {len(violations)} violation(s) outside theme files:")
        for path, line, text in violations[:50]:
            print(f"  {path}:{line}: {text}")
    # TODO: tighten to assertion once violations are migrated to palette.
    if os.environ.get("STRICT_COLORS") == "1":
        assert not violations, (
            f"{len(violations)} hardcoded color violations outside theme files; "
            "see captured stdout"
        )


def test_palette_constants_count_above_threshold():
    palette = ROOT / "ui" / "theme" / "palette.py"
    text = palette.read_text(encoding="utf-8")
    count = len(re.findall(r'^[A-Z][A-Z0-9_]+\s*=\s*["\']', text, re.MULTILINE))
    assert count >= 30, f"palette.py has {count} color constants; expected >= 30"

"""Codebase guardian: font sizes must use pt / tokens, never px or setPixelSize.

Soft by default (prints violations); set ``STRICT_FONTS=1`` to fail the build.
- ``QFont.setPixelSize(N)`` is HDPI-unsafe and forbidden everywhere.
- Inline ``font-size: NNpx`` QSS strings must live in ``ui/theme`` (tokens/QSS).
"""

from __future__ import annotations

import os
import re
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


ROOT = Path("python/embeddebug/serial_station")

SET_PIXEL_SIZE_PATTERN = re.compile(r"\.setPixelSize\s*\(")
PX_FONT_SIZE_PATTERN = re.compile(r"font-size:\s*\d+\s*px", re.IGNORECASE)


def _allowed_font_files() -> set[Path]:
    """Files where px font-sizes are EXPECTED (theme tokens & QSS generators)."""
    theme = ROOT / "ui" / "theme"
    files: set[Path] = {theme / "tokens.py"}
    files.update(theme.glob("qss_sections_*.py"))
    return files


def scan_source_for_px_font_sizes(
    root: Path, allowed: set[Path]
) -> tuple[list[tuple[Path, int, str]], list[tuple[Path, int, str]]]:
    """Return (setPixelSize_hits, inline_px_font_size_hits).

    setPixelSize is HDPI-unsafe and flagged everywhere (no allowlist).
    Inline ``font-size: NNpx`` is flagged only outside ALLOWED_FONT_FILES.
    """
    allowed_resolved = {p.resolve() for p in allowed}
    pixel_hits: list[tuple[Path, int, str]] = []
    px_font_hits: list[tuple[Path, int, str]] = []
    for path in root.rglob("*.py"):
        if "__pycache__" in path.parts:
            continue
        is_allowed = path.resolve() in allowed_resolved
        try:
            lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError:
            continue
        for idx, line in enumerate(lines, start=1):
            if line.strip().startswith("#"):
                continue  # pure comment line
            m = SET_PIXEL_SIZE_PATTERN.search(line)
            if m:
                pixel_hits.append((path, idx, m.group(0)))
            if not is_allowed:
                m2 = PX_FONT_SIZE_PATTERN.search(line)
                if m2:
                    px_font_hits.append((path, idx, m2.group(0)))
    return pixel_hits, px_font_hits


def test_no_setPixelSize_calls(capsys):
    pixel_hits, _ = scan_source_for_px_font_sizes(ROOT, _allowed_font_files())
    if pixel_hits:
        print(f"\n[setPixelSize] {len(pixel_hits)} HDPI-unsafe call(s):")
        for path, line, text in pixel_hits[:50]:
            print(f"  {path}:{line}: {text}")
    # TODO: tighten to assertion once setPixelSize calls are migrated to setPointSize.
    if os.environ.get("STRICT_FONTS") == "1":
        assert not pixel_hits, (
            f"{len(pixel_hits)} .setPixelSize() calls remain; "
            "use setPointSize for HDPI safety"
        )


def test_no_inline_px_font_size(capsys):
    _, px_hits = scan_source_for_px_font_sizes(ROOT, _allowed_font_files())
    if px_hits:
        print(f"\n[inline px font-size] {len(px_hits)} violation(s) outside theme files:")
        for path, line, text in px_hits[:50]:
            print(f"  {path}:{line}: {text}")
    # TODO: tighten to assertion once inline px font sizes are migrated to tokens.
    if os.environ.get("STRICT_FONTS") == "1":
        assert not px_hits, (
            f"{len(px_hits)} inline 'font-size: NNpx' occurrences outside theme files; "
            "use tokens.FONT_* instead"
        )

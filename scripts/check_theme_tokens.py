"""Static audit for semantic theme coverage and no-white fallback regressions."""

from __future__ import annotations

import re
import sys
from dataclasses import fields
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = PROJECT_ROOT / "src"
if str(SOURCE_ROOT) not in sys.path:
    sys.path.insert(0, str(SOURCE_ROOT))

from serialforge.presentation.theme_stylesheet_runtime import (  # noqa: E402
    render_theme_stylesheet,
)
from serialforge.presentation.theme_tokens import THEME_OPTIONS, ThemeSpec  # noqa: E402

REQUIRED_TOKENS = {
    "success",
    "success_surface",
    "success_border",
    "info_surface",
    "info_border",
    "warning_surface",
    "warning_border",
    "error_surface",
    "error_border",
    "history_surface",
    "history_border",
    "neutral_surface",
    "neutral_border",
    "interactive_hover",
    "interactive_pressed",
    "disabled_surface",
    "disabled_border",
    "disabled_text",
    "focus",
    "selection_background",
    "selection_text",
    "on_accent",
}

REQUIRED_SELECTORS = (
    "QFrame#connectionControlBand",
    "QComboBox QAbstractItemView",
    "QComboBox QAbstractItemView::item:selected",
    'QComboBox#connectionPresetCombo[customSelected="true"]',
    'QFrame[surfaceRole="comboPopup"]',
    "QFileDialog QTreeView",
    "QFileDialog QListView",
    "QFileDialog QHeaderView::section",
    "QFileDialog QSidebar",
    "QFileDialog QDialogButtonBox QPushButton",
    'QMessageBox[surfaceRole="confirmation"]',
    "QPushButton:disabled",
    "QTabWidget#workspaceTabs QTabBar QToolButton",
    "QTabWidget#workspaceTabs QTabBar::tab:selected",
    "QScrollArea#settingsScroll > QWidget#settingsViewport",
    "QScrollBar::handle:horizontal:hover",
    "QAbstractScrollArea::corner",
    'QLabel#protocolStatus[state="error"]',
    'QLabel#commandBatchStatus[state="failed"]',
)

WHITE_BACKGROUND = re.compile(r"background\s*:\s*(?:white|#fff(?:fff)?)(?:\s*;|\s|$)", re.I)
HEX_LITERAL = re.compile(r"#[0-9a-fA-F]{3,8}")


def _fail(message: str) -> None:
    raise SystemExit(f"theme token audit failed: {message}")


def main() -> int:
    theme_fields = {field.name for field in fields(ThemeSpec)}
    missing = REQUIRED_TOKENS - theme_fields
    if missing:
        _fail(f"ThemeSpec missing semantic tokens: {', '.join(sorted(missing))}")

    variant_sources = (
        SOURCE_ROOT / "serialforge/presentation/theme_variant_shell.py",
        SOURCE_ROOT / "serialforge/presentation/theme_variant_controls.py",
    )
    variant_text = "\n".join(path.read_text(encoding="utf-8") for path in variant_sources)
    missing_renderers = sorted(
        token for token in REQUIRED_TOKENS if f"theme.{token}" not in variant_text
    )
    if missing_renderers:
        _fail(f"theme override does not render tokens: {', '.join(missing_renderers)}")

    for theme in THEME_OPTIONS:
        stylesheet = render_theme_stylesheet(theme)
        if any(marker in stylesheet for marker in ("{theme.", "{{", "}}")):
            _fail(f"unresolved stylesheet marker for {theme.key}")
        if WHITE_BACKGROUND.search(stylesheet):
            _fail(f"white background fallback for {theme.key}")
        missing_selectors = [
            selector for selector in REQUIRED_SELECTORS if selector not in stylesheet
        ]
        if missing_selectors:
            _fail(f"{theme.key} missing selectors: {', '.join(missing_selectors)}")

    stable_qss = "\n".join(
        (
            (SOURCE_ROOT / "serialforge/presentation/theme_stylesheet_base.py").read_text(
                encoding="utf-8"
            ),
            # The stable controls template is intentionally audited as a separate
            # file so future changes cannot hide a new legacy color in a large blob.
            (SOURCE_ROOT / "serialforge/presentation/theme_stylesheet_controls.py").read_text(
                encoding="utf-8"
            ),
        )
    )
    legacy_literal_count = len(HEX_LITERAL.findall(stable_qss))
    print(
        "theme token audit: pass "
        f"({len(THEME_OPTIONS)} themes, {len(REQUIRED_TOKENS)} semantic tokens, "
        f"{len(REQUIRED_SELECTORS)} selectors, legacy_qss_literals={legacy_literal_count})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

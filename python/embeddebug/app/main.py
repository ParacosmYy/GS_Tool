"""Minimal PyQt application entry for the Python migration lane."""

from __future__ import annotations

import argparse
import os
import sys
from collections.abc import Sequence

from PyQt6.QtCore import QCoreApplication, QEvent
from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.main_window import SerialStationMainWindow


def create_application(argv: Sequence[str] | None = None) -> QApplication:
    """Create or reuse the process QApplication."""

    existing = QApplication.instance()
    if existing is not None:
        return existing

    app = QApplication(list(argv) if argv is not None else list(sys.argv))
    app.setApplicationName("EmbedDebugPy")
    app.setApplicationDisplayName("EmbedDebug PyQt")
    return app


def build_main_window() -> SerialStationMainWindow:
    """Build the top-level Python/PyQt migration window."""

    return SerialStationMainWindow()


def main(argv: Sequence[str] | None = None) -> int:
    """Run the Python/PyQt migration entry point."""

    parser = argparse.ArgumentParser(prog="start-embeddebug")
    parser.add_argument(
        "--smoke",
        action="store_true",
        help="Create the window and process events without entering the event loop.",
    )
    args = parser.parse_args(list(argv) if argv is not None else None)

    if args.smoke:
        os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

    app = create_application(["start-embeddebug"] if args.smoke else None)
    window = build_main_window()
    window.show()
    app.processEvents()

    if args.smoke:
        window.close()
        app.processEvents()
        window.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        app.processEvents()
        return 0

    return int(app.exec())


if __name__ == "__main__":
    raise SystemExit(main())

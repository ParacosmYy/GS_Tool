"""Module entry point for ``python -m serialforge`` and PyInstaller."""

from __future__ import annotations

from serialforge.presentation.main import main

if __name__ == "__main__":
    raise SystemExit(main())

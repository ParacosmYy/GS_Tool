"""Canonical root Python entry for the Windows personal experience launcher.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Delegate the root Python command to the Windows runtime composition root.
Module: Root delivery / Windows launcher boundary
"""

from __future__ import annotations

import runpy
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent
WINDOWS_ROOT = PROJECT_ROOT / "windows"
WINDOWS_ENTRYPOINT = WINDOWS_ROOT / "run.py"


def main() -> None:
    """Run the Windows entrypoint without duplicating server composition logic."""

    if not WINDOWS_ENTRYPOINT.is_file():
        raise FileNotFoundError(f"Windows runtime entrypoint is missing: {WINDOWS_ENTRYPOINT}")
    windows_path = str(WINDOWS_ROOT)
    if windows_path not in sys.path:
        sys.path.insert(0, windows_path)
    runpy.run_path(str(WINDOWS_ENTRYPOINT), run_name="__main__")


if __name__ == "__main__":
    main()

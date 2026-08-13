"""Filesystem-independent path identity helpers shared by application layers."""

import os
from pathlib import Path


def path_key(path: Path) -> str:
    """Return a stable comparison key for an already-normalized absolute path."""
    return os.path.normcase(os.path.normpath(str(path)))

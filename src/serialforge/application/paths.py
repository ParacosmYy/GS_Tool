"""Bounded Windows-local application paths and diagnostic logging."""

from __future__ import annotations

import logging
import os
from dataclasses import dataclass
from logging.handlers import RotatingFileHandler
from pathlib import Path

MAX_APPLICATION_LOG_BYTES = 4 * 1024 * 1024
APPLICATION_LOG_BACKUPS = 2


@dataclass(frozen=True, slots=True)
class ApplicationPaths:
    """Explicit config/log boundaries used by the desktop composition root."""

    config_dir: Path
    log_dir: Path

    @classmethod
    def from_environment(cls) -> ApplicationPaths:
        """From environment."""
        local_app_data = os.environ.get("LOCALAPPDATA", "").strip()
        if local_app_data:
            root = Path(local_app_data)
        else:
            root = Path.home() / "AppData" / "Local"
        serialforge_root = root / "SerialForge"
        return cls(
            config_dir=serialforge_root / "config",
            log_dir=serialforge_root / "logs",
        )

    @property
    def default_record_path(self) -> Path:
        """Return the bounded raw-record location suggested by the UI."""

        return self.log_dir / "uart-record.jsonl"

    def ensure_directories(self) -> None:
        """Create only the app-owned local directories, never a workspace folder."""

        self.config_dir.mkdir(parents=True, exist_ok=True)
        self.log_dir.mkdir(parents=True, exist_ok=True)


def configure_logging(paths: ApplicationPaths) -> None:
    """Install one bounded diagnostic log file below LOCALAPPDATA."""

    logger = logging.getLogger("serialforge")
    if any(getattr(handler, "_serialforge_handler", False) for handler in logger.handlers):
        return
    try:
        paths.log_dir.mkdir(parents=True, exist_ok=True)
        handler = RotatingFileHandler(
            paths.log_dir / "serialforge.log",
            maxBytes=MAX_APPLICATION_LOG_BYTES,
            backupCount=APPLICATION_LOG_BACKUPS,
            encoding="utf-8",
        )
    except OSError:
        logging.getLogger(__name__).warning(
            "SerialForge file logging is unavailable",
            exc_info=True,
        )
        return

    handler._serialforge_handler = True  # type: ignore[attr-defined]
    handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(name)s: %(message)s"))
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    logger.propagate = False

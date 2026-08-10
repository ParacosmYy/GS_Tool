"""Environment-backed application configuration.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Centralize deployment defaults and prevent route modules from reading
         environment variables ad hoc.
Module: Configuration boundary
"""

from __future__ import annotations

import os
import secrets
from pathlib import Path
from typing import Any

from dotenv import load_dotenv

from . import db


load_dotenv()

_PLACEHOLDER_SECRET = "replace-with-a-long-random-secret"
_SHARED_MODES = frozenset({"production", "shared", "lan"})


def allowed_base_urls() -> list[str]:
    """Read and normalize the explicit provider allowlist."""

    raw = os.getenv("TOKEN_TRACKER_ALLOWED_BASE_URLS", "")
    return [item.strip().rstrip("/") for item in raw.split(",") if item.strip()]


def build_settings(database: str | Path) -> dict[str, Any]:
    """Return Flask settings with safe local defaults and explicit boundaries."""

    runtime_mode = os.getenv("TOKEN_TRACKER_RUNTIME_MODE", "local").strip().casefold() or "local"
    return {
        "SECRET_KEY": resolve_secret_key(runtime_mode),
        "DATABASE": str(database),
        "RUNTIME_MODE": runtime_mode,
        "TOKEN_TRACKER_HOST": os.getenv("TOKEN_TRACKER_HOST", "127.0.0.1"),
        "TOKEN_TRACKER_PORT": os.getenv("TOKEN_TRACKER_PORT", "5000"),
        "PROXY_TIMEOUT": int(os.getenv("TOKEN_TRACKER_PROXY_TIMEOUT", "60")),
        "PROXY_MAX_RESPONSE_BYTES": int(os.getenv("TOKEN_TRACKER_PROXY_MAX_RESPONSE_BYTES", str(2 * 1024 * 1024))),
        "ALLOWED_BASE_URLS": allowed_base_urls(),
        "ALLOW_HTTP_PROXY": os.getenv("TOKEN_TRACKER_ALLOW_HTTP_PROXY", "0") == "1",
        "MAX_CONTENT_LENGTH": 256 * 1024,
        "SESSION_COOKIE_HTTPONLY": True,
        "SESSION_COOKIE_SAMESITE": "Lax",
        "SESSION_COOKIE_SECURE": os.getenv("TOKEN_TRACKER_SECURE_COOKIE", "0") == "1",
    }


def resolve_secret_key(runtime_mode: str = "local") -> str:
    """Resolve a session key and reject unsafe shared-deployment defaults."""

    configured = os.getenv("TOKEN_TRACKER_SECRET_KEY", "").strip()
    usable = bool(configured and configured != _PLACEHOLDER_SECRET and len(configured) >= 32)
    if usable:
        return configured
    if runtime_mode in _SHARED_MODES:
        raise RuntimeError(
            "TOKEN_TRACKER_SECRET_KEY must be a non-placeholder value of at least 32 characters "
            "before shared or production mode can start"
        )
    return secrets.token_hex(32)


def resolve_database(path: str | os.PathLike[str] | None = None) -> Path:
    """Resolve a database path through the infrastructure-owned helper."""

    return db.get_db_path(path)

"""Non-destructive deployment configuration checks.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Fail early on unsafe shared/production configuration without side effects.
Module: Delivery / deployment verification boundary
"""

from __future__ import annotations

from pathlib import Path
from typing import Any
from urllib.parse import urlparse


class DeploymentCheckError(RuntimeError):
    """Raised when required deployment configuration is unsafe or incomplete."""


def validate(settings: dict[str, Any], require_https: bool = False) -> list[str]:
    """Return actionable errors without exposing secret values or changing state."""

    errors: list[str] = []
    runtime_mode = str(settings.get("RUNTIME_MODE", "local")).casefold()
    if runtime_mode in {"shared", "production", "lan"}:
        secret = str(settings.get("SECRET_KEY", ""))
        if len(secret) < 32:
            errors.append("Session Secret 长度不足 32 个字符")
        if not bool(settings.get("SESSION_COOKIE_SECURE")) and require_https:
            errors.append("HTTPS 生产预检要求 TOKEN_TRACKER_SECURE_COOKIE=1")

    allowed_urls = settings.get("ALLOWED_BASE_URLS", [])
    if require_https and not allowed_urls:
        errors.append("HTTPS 生产预检要求配置 TOKEN_TRACKER_ALLOWED_BASE_URLS")
    for raw_url in allowed_urls:
        parsed = urlparse(str(raw_url))
        if require_https and parsed.scheme != "https":
            errors.append("HTTPS 生产预检不允许 HTTP provider allowlist")
            break

    database = Path(str(settings.get("DATABASE", ""))).resolve()
    static_root = Path(__file__).resolve().parent / "static"
    try:
        database.relative_to(static_root)
    except ValueError:
        pass
    else:
        errors.append("数据库不能位于网站 static 目录内")
    return errors


def assert_valid(settings: dict[str, Any], require_https: bool = False) -> None:
    """Raise one safe error containing no secret or database contents."""

    errors = validate(settings, require_https=require_https)
    if errors:
        raise DeploymentCheckError("；".join(errors))

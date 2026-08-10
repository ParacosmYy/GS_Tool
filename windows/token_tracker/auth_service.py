"""Account credential application use cases.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Centralize username/password validation, registration, and verification.
Module: Application / credential boundary
"""

from __future__ import annotations

import re
import sqlite3
from typing import Any

from werkzeug.security import check_password_hash, generate_password_hash

from . import db


MIN_PASSWORD_LENGTH = 8
MAX_USERNAME_LENGTH = 32
_USERNAME_PATTERN = re.compile(r"[\w.@+-]{3,32}\Z", flags=re.UNICODE)


class AuthValidationError(ValueError):
    """Raised when account credentials fail the public input contract."""


class AuthConflictError(ValueError):
    """Raised when registration collides with an existing username."""


def authenticate_user(username: Any, password: Any, path: str) -> dict[str, Any] | None:
    """Verify credentials with one generic failure result for callers."""

    username_value = str(username or "").strip()
    if not _USERNAME_PATTERN.fullmatch(username_value) or not isinstance(password, str):
        return None
    user = db.find_user(username_value, path)
    if user is None:
        return None
    try:
        valid_password = check_password_hash(user["password_hash"], password)
    except (TypeError, ValueError):
        valid_password = False
    return user if valid_password else None


def register_user(username: Any, password: Any, path: str) -> dict[str, Any]:
    """Validate and create one account without returning its password hash."""

    username_value = normalize_username(username)
    password_value = normalize_password(password)
    if db.find_user(username_value, path):
        raise AuthConflictError("该用户名已存在，请换一个。")
    try:
        created_user = db.create_user(
            username_value,
            generate_password_hash(password_value, method="scrypt"),
            path,
        )
    except sqlite3.IntegrityError as exc:
        raise AuthConflictError("该用户名已存在，请换一个。") from exc
    return _public_user(created_user)


def normalize_username(value: Any) -> str:
    """Return a contract-valid username without accepting arbitrary text."""

    username = str(value or "").strip()
    if not _USERNAME_PATTERN.fullmatch(username):
        raise AuthValidationError("用户名需为 3-32 个字母、数字或 . _ @ + -。")
    return username


def normalize_password(value: Any) -> str:
    """Return a password value suitable for hashing without logging it."""

    if not isinstance(value, str) or len(value) < MIN_PASSWORD_LENGTH:
        raise AuthValidationError(f"密码至少需要 {MIN_PASSWORD_LENGTH} 个字符。")
    return value


def _public_user(user: dict[str, Any]) -> dict[str, Any]:
    """Project account fields without password or token material."""

    return {
        "id": int(user["id"]),
        "username": user["username"],
        "role": user.get("role", "user"),
        "created_at": user.get("created_at"),
    }

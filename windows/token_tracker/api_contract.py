"""HTTP response helpers for the version-one JSON contract.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep error shape, machine codes, and request correlation consistent.
"""

from __future__ import annotations

from typing import Any

from flask import g, jsonify


def error_response(
    code: str,
    message: str,
    status: int,
    details: dict[str, Any] | None = None,
) -> tuple[Any, int]:
    """Return a stable error envelope without exposing implementation details."""

    error: dict[str, Any] = {"code": code, "message": message}
    if details:
        error["details"] = details
    payload = {
        "error": error,
        "request_id": getattr(g, "request_id", "unknown"),
    }
    return jsonify(payload), status

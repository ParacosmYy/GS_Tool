"""Privacy-safe HTTP access logging boundary.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Emit useful request diagnostics without copying credentials, cookies,
         query values, bodies, user agents, or raw client addresses to logs.
Module: Presentation / observability boundary
"""

from __future__ import annotations

import logging
import re
import time
from logging import Logger
from typing import Any

from flask import Request, Response


_HTTP_REQUEST_LINE = re.compile(r'("[A-Z]+\s+)([^\s"]+)(\s+HTTP/\d(?:\.\d)?")')


class QueryRedactingFilter(logging.Filter):
    """Remove query strings from Werkzeug's built-in access log line."""

    def filter(self, record: logging.LogRecord) -> bool:
        """Keep the server log useful while preventing URL query leakage."""

        message = record.getMessage()
        redacted = _HTTP_REQUEST_LINE.sub(_redact_request_target, message)
        if redacted != message:
            record.msg = redacted
            record.args = ()
        return True


def install_server_log_filters() -> None:
    """Install the query filter once for the local Flask server logger."""

    logger = logging.getLogger("werkzeug")
    if not any(isinstance(item, QueryRedactingFilter) for item in logger.filters):
        logger.addFilter(QueryRedactingFilter())


def _redact_request_target(match: re.Match[str]) -> str:
    """Return the request line with everything after the path delimiter removed."""

    target = match.group(2).split("?", 1)[0]
    return f"{match.group(1)}{target}{match.group(3)}"


def start_timer() -> float:
    """Return a monotonic start point for request duration measurement."""

    return time.perf_counter()


def record_request(
    logger: Logger,
    request: Request,
    response: Response,
    request_id: str,
    started_at: float | None,
) -> None:
    """Write one bounded access event without sensitive request material."""

    path = _safe_path(request.path)
    duration_ms = _duration_ms(started_at)
    logger.info(
        "access method=%s path=%s status=%s duration_ms=%s request_id=%s",
        request.method,
        path,
        response.status_code,
        duration_ms,
        _safe_text(request_id),
    )


def _duration_ms(started_at: float | None) -> int:
    """Convert a monotonic timer to a non-negative integer for log fields."""

    if started_at is None:
        return 0
    return max(0, int((time.perf_counter() - started_at) * 1000))


def _safe_path(path: Any) -> str:
    """Keep log lines single-line and bounded while omitting query strings."""

    return _safe_text(path, limit=256)


def _safe_text(value: Any, limit: int = 128) -> str:
    """Normalize a diagnostic value without allowing multiline log injection."""

    text = str(value or "unknown")
    return text.replace("\r", "").replace("\n", "")[:limit]

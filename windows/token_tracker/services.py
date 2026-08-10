"""Validation and application-level operations.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep usage-record use cases independent from HTTP and templates.
"""

from __future__ import annotations

from typing import Any

from . import csv_export, db


class UsageValidationError(ValueError):
    """Raised when a usage record is invalid."""


class UsageExportTooLargeError(RuntimeError):
    """Raised when a personal CSV exceeds the shared export safety boundary."""


MAX_USAGE_PAGE_SIZE = 200
USAGE_EXPORT_MAX_ROWS = csv_export.MAX_EXPORT_ROWS
USAGE_EXPORT_MAX_BYTES = csv_export.MAX_EXPORT_BYTES


def non_negative_int(value: Any, field_name: str) -> int:
    """Parse one token count and reject booleans, malformed values, and negatives."""

    if isinstance(value, bool):
        raise UsageValidationError(f"{field_name} must be a non-negative integer")
    try:
        parsed = int(str(value).strip())
    except (TypeError, ValueError) as exc:
        raise UsageValidationError(f"{field_name} must be a non-negative integer") from exc
    if parsed < 0:
        raise UsageValidationError(f"{field_name} must be a non-negative integer")
    return parsed


def add_usage(
    user_id: int,
    model: Any,
    input_tokens: Any,
    output_tokens: Any,
    timestamp: str | None = None,
    note: Any = "",
    source: str = "manual",
    path: str | None = None,
    idempotency_key: Any = None,
) -> dict[str, Any]:
    """Validate and insert usage while retaining the original service API."""

    record, _ = add_usage_result(
        user_id=user_id,
        model=model,
        input_tokens=input_tokens,
        output_tokens=output_tokens,
        timestamp=timestamp,
        note=note,
        source=source,
        idempotency_key=idempotency_key,
        path=path,
    )
    return record


def add_usage_result(
    user_id: int,
    model: Any,
    input_tokens: Any,
    output_tokens: Any,
    timestamp: str | None = None,
    note: Any = "",
    source: str = "manual",
    path: str | None = None,
    idempotency_key: Any = None,
) -> tuple[dict[str, Any], bool]:
    """Validate usage and return ``(record, replayed)`` for API writes."""

    model_name = str(model or "").strip()
    if not model_name:
        raise UsageValidationError("model is required")
    if len(model_name) > 200:
        raise UsageValidationError("model must be 200 characters or fewer")
    note_text = str(note or "").strip()
    if len(note_text) > 1000:
        raise UsageValidationError("note must be 1000 characters or fewer")
    source_name = str(source or "manual").strip()[:40] or "manual"
    key_text = str(idempotency_key or "").strip() or None
    if key_text and len(key_text) > db.MAX_IDEMPOTENCY_KEY_LENGTH:
        raise UsageValidationError(
            f"idempotency_key must be {db.MAX_IDEMPOTENCY_KEY_LENGTH} characters or fewer"
        )
    try:
        normalized_timestamp = db.normalize_timestamp(timestamp)
    except ValueError as exc:
        raise UsageValidationError(str(exc)) from exc
    return db.insert_record_result(
        user_id=user_id,
        model=model_name,
        input_tokens=non_negative_int(input_tokens, "input_tokens"),
        output_tokens=non_negative_int(output_tokens, "output_tokens"),
        timestamp=normalized_timestamp,
        note=note_text,
        source=source_name,
        idempotency_key=key_text,
        path=path,
    )


def query_range(
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
) -> tuple[str | None, str | None]:
    """Translate a user-facing period or date range into SQL boundaries."""

    try:
        return db.get_period_bounds(period, date_from, date_to)
    except (TypeError, ValueError) as exc:
        raise UsageValidationError(f"invalid date range: {exc}") from exc


def usage_summary(
    user_id: int,
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
    path: str | None = None,
) -> dict[str, Any]:
    """Build one user's summary through the application boundary.

    The controller supplies request values, but this function owns range
    normalization and the user-scoped read-model composition. It never returns
    data for a different user id.
    """

    period_value, start, end = _summary_bounds(period, date_from, date_to)
    payload = db.summary_bundle(user_id, start, end, path)
    payload.update({"period": period_value, "from": start, "to": end})
    return payload


def usage_records_page(
    user_id: int,
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
    limit: Any = 50,
    offset: Any = 0,
    path: str | None = None,
) -> dict[str, Any]:
    """Return a bounded personal record page with stable range metadata."""

    period_value, start, end = _summary_bounds(period, date_from, date_to)
    safe_limit = _page_number(limit, "limit", maximum=MAX_USAGE_PAGE_SIZE, minimum=1)
    safe_offset = _page_number(offset, "offset", maximum=None, minimum=0)
    page = db.recent_records_page(user_id, safe_limit, safe_offset, start, end, path)
    return {
        "period": period_value,
        "from": start,
        "to": end,
        "records": page["items"],
        "pagination": page["pagination"],
    }


def export_usage_csv(
    user_id: int,
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
    path: str | None = None,
) -> bytes:
    """Export only the authenticated user's bounded usage projection."""

    _, start, end = _summary_bounds(period, date_from, date_to)
    try:
        return db.export_csv(user_id, start, end, path)
    except csv_export.ExportTooLargeError as exc:
        raise UsageExportTooLargeError("export exceeds the usage safety bound") from exc


def _summary_bounds(
    period: str = "day",
    date_from: str | None = None,
    date_to: str | None = None,
) -> tuple[str, str | None, str | None]:
    period_value = str(period or "day")
    start, end = query_range(period_value, date_from, date_to)
    return period_value, start, end


def _page_number(value: Any, field_name: str, maximum: int | None, minimum: int) -> int:
    """Normalize pagination once so controllers cannot diverge on limits."""

    try:
        number = int(str(value).strip())
    except (TypeError, ValueError) as exc:
        raise UsageValidationError(f"{field_name} must be an integer") from exc
    if number < minimum or (maximum is not None and number > maximum):
        upper = f" and no greater than {maximum}" if maximum is not None else ""
        raise UsageValidationError(f"{field_name} must be at least {minimum}{upper}")
    return number

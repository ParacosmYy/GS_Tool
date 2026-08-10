"""Administrator application use cases and audit orchestration.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep admin authorization outcomes, read models, exports, and audits together.
Module: Application / administrator boundary
"""

from __future__ import annotations

from typing import Any

from . import admin_data, csv_export, events


VALID_EXPORT_KINDS = frozenset({"usage", "events", "logs"})
EXPORT_MAX_ROWS = csv_export.MAX_EXPORT_ROWS
EXPORT_MAX_BYTES = csv_export.MAX_EXPORT_BYTES


class AdminApplicationError(ValueError):
    """Raised when an administrator use-case input is not supported."""


class AdminExportTooLargeError(AdminApplicationError):
    """Raised when a bounded administrator export cannot be completed."""


def record_page_view(actor_user_id: int, request_id: str, path: str) -> None:
    """Record an authenticated administrator page visit."""

    events.insert_audit_event(
        actor_user_id, "admin.page.view", "page", "admin", request_id, path
    )


def get_overview(actor_user_id: int, request_id: str, path: str) -> dict[str, Any]:
    """Audit and return the bounded team overview read model."""

    events.insert_audit_event(actor_user_id, "admin.overview.view", "overview", "", request_id, path)
    return admin_data.overview(path)


def list_users(
    actor_user_id: int,
    request_id: str,
    path: str,
    limit: Any = 50,
    offset: Any = 0,
) -> dict[str, Any]:
    """Audit and return the bounded member aggregate projection."""

    events.insert_audit_event(actor_user_id, "admin.users.list", "user", "", request_id, path)
    return admin_data.list_users(path, limit, offset)


def get_user_activity(
    actor_user_id: int,
    target_user_id: int,
    request_id: str,
    path: str,
    limit: Any = 100,
) -> dict[str, Any] | None:
    """Return one member's bounded activity and audit successful access only."""

    activity = admin_data.user_activity(target_user_id, path, limit)
    if activity["user"] is None:
        return None
    events.insert_audit_event(
        actor_user_id,
        "admin.user.activity.view",
        "user",
        str(target_user_id),
        request_id,
        path,
        target_user_id,
    )
    return activity


def export_csv(actor_user_id: int, kind: str, request_id: str, path: str) -> bytes:
    """Validate, export, and audit one fixed administrator data class."""

    export_kind = str(kind or "usage").strip().lower()
    if export_kind not in VALID_EXPORT_KINDS:
        raise AdminApplicationError("kind must be usage, events, or logs")
    try:
        csv_bytes = admin_data.export_csv(export_kind, path)
    except csv_export.ExportTooLargeError as exc:
        events.insert_audit_event(
            actor_user_id,
            "admin.export.rejected",
            "export",
            export_kind,
            request_id,
            path,
            metadata={"kind": export_kind, "reason": "size_limit"},
        )
        raise AdminExportTooLargeError("export exceeds the administrator safety bound") from exc
    events.insert_audit_event(
        actor_user_id,
        "admin.export",
        export_kind,
        "",
        request_id,
        path,
        metadata={"kind": export_kind},
    )
    return csv_bytes

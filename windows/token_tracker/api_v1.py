"""Versioned JSON API for web, Android, and future collection clients.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Expose stable cross-client authentication, activity, and admin routes.
Module: API v1 controller boundary
"""

from __future__ import annotations

from functools import wraps
from typing import Any, Callable

from flask import Blueprint, Response, current_app, g, jsonify, request

from . import admin_service, auth_service, events, mobile_auth, provider_service, rate_limit
from .api_contract import error_response
from .providers import ProviderNetworkError, ProviderResponseTooLarge
from .services import UsageValidationError, add_usage_result, usage_records_page, usage_summary


api_v1 = Blueprint("api_v1", __name__, url_prefix="/api/v1")


def _database() -> str:
    return str(current_app.config["DATABASE"])


def _provider_config() -> provider_service.ProviderRuntimeConfig:
    """Project only the provider limits needed by the application service."""

    return provider_service.runtime_config(current_app.config)


def _payload() -> dict[str, Any] | None:
    value = request.get_json(silent=True)
    return value if isinstance(value, dict) else None


def _required_user(view: Callable[..., Any]) -> Callable[..., Any]:
    @wraps(view)
    def wrapped(*args: Any, **kwargs: Any) -> Any:
        if g.user is None:
            return error_response("AUTH_REQUIRED", "请先登录", 401)
        return view(*args, **kwargs)

    return wrapped


def _required_admin(view: Callable[..., Any]) -> Callable[..., Any]:
    @_required_user
    @wraps(view)
    def wrapped(*args: Any, **kwargs: Any) -> Any:
        if g.user.get("role") != "admin":
            return error_response("ADMIN_REQUIRED", "需要管理员权限", 403)
        return view(*args, **kwargs)

    return wrapped


def _unpack_command(payload: dict[str, Any], expected_command: str) -> tuple[dict[str, Any], str | None]:
    """Validate the optional command envelope without dispatching arbitrary names."""

    if "command" not in payload and "protocol_version" not in payload:
        return payload, request.headers.get("Idempotency-Key")
    try:
        version = int(payload.get("protocol_version"))
    except (TypeError, ValueError) as exc:
        raise events.EventValidationError("protocol_version must be 1") from exc
    if version != 1 or payload.get("command") != expected_command:
        raise events.EventValidationError("unsupported protocol version or command")
    body = payload.get("payload")
    if not isinstance(body, dict):
        raise events.EventValidationError("command payload must be an object")
    return body, str(payload.get("idempotency_key") or request.headers.get("Idempotency-Key") or "") or None


@api_v1.post("/auth/login")
def auth_login() -> Response:
    """Verify credentials and issue an Android-compatible token pair."""

    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    username = str(payload.get("username") or "").strip()
    password = payload.get("password")
    identity = f"{request.remote_addr or 'unknown'}:{username[:32]}"
    if not rate_limit.allow("v1-login", identity, 8, 900):
        return error_response("RATE_LIMITED", "登录尝试过于频繁，请稍后再试", 429)
    user = auth_service.authenticate_user(username, password, _database())
    if user is None:
        return error_response("INVALID_CREDENTIALS", "用户名或密码不正确", 401)
    return jsonify(mobile_auth.issue_token_pair(user, _database()))


@api_v1.post("/auth/refresh")
def auth_refresh() -> Response:
    """Rotate a refresh token; the old refresh secret becomes unusable."""

    payload = _payload()
    if payload is None or not isinstance(payload.get("refresh_token"), str):
        return error_response("BAD_REQUEST", "refresh_token is required", 400)
    if not rate_limit.allow("v1-refresh", request.remote_addr or "unknown", 20, 900):
        return error_response("RATE_LIMITED", "刷新请求过于频繁，请稍后再试", 429)
    pair = mobile_auth.rotate_refresh_token(payload["refresh_token"], _database())
    if pair is None:
        return error_response("INVALID_REFRESH_TOKEN", "刷新令牌无效或已过期", 401)
    return jsonify(pair)


@api_v1.post("/auth/logout")
@_required_user
def auth_logout() -> Response:
    """Revoke only the presented bearer access token."""

    mobile_auth.revoke_access_token(request.headers.get("Authorization"), _database())
    return jsonify({"revoked": True})


@api_v1.get("/me")
@_required_user
def me() -> Response:
    return jsonify({"user": {"id": g.user["id"], "username": g.user["username"], "role": g.user.get("role", "user")}})


@api_v1.get("/me/summary")
@_required_user
def me_summary() -> Response:
    try:
        payload = usage_summary(
            user_id=g.user["id"],
            period=request.args.get("period", "day"),
            date_from=request.args.get("from"),
            date_to=request.args.get("to"),
            path=_database(),
        )
    except UsageValidationError as exc:
        return error_response("INVALID_RANGE", str(exc), 400)
    return jsonify(payload)


@api_v1.get("/events/work")
@_required_user
def get_work_events() -> Response:
    try:
        limit = int(request.args.get("limit", "50"))
    except ValueError:
        limit = 50
    try:
        offset = max(0, int(request.args.get("offset", "0")))
    except ValueError:
        offset = 0
    page = events.recent_work_events(g.user["id"], limit, offset, _database())
    return jsonify({"events": page["items"], "pagination": page["pagination"]})


@api_v1.post("/records")
@_required_user
def create_usage_record() -> Response:
    """Persist one client-entered usage record under the bearer account."""

    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        record, replayed = add_usage_result(
            user_id=g.user["id"],
            model=payload.get("model"),
            input_tokens=payload.get("input_tokens"),
            output_tokens=payload.get("output_tokens"),
            timestamp=payload.get("timestamp"),
            note=payload.get("note", ""),
            source="android",
            idempotency_key=request.headers.get("Idempotency-Key"),
            path=_database(),
        )
    except UsageValidationError as exc:
        return error_response("VALIDATION_ERROR", str(exc), 400)
    return jsonify({"record": record, "request_id": g.request_id, "replayed": replayed}), 200 if replayed else 201


@api_v1.get("/records")
@_required_user
def get_usage_records() -> Response:
    """Return a paged personal record projection for mobile and future clients."""

    try:
        payload = usage_records_page(
            user_id=g.user["id"],
            period=request.args.get("period", "day"),
            date_from=request.args.get("from"),
            date_to=request.args.get("to"),
            limit=request.args.get("limit", "50"),
            offset=request.args.get("offset", "0"),
            path=_database(),
        )
    except UsageValidationError as exc:
        return error_response("INVALID_RANGE", str(exc), 400)
    return jsonify(payload)


@api_v1.post("/provider/models")
@_required_user
def provider_models() -> Response:
    """Discover allowlisted provider models for an authenticated client."""

    if not rate_limit.allow("v1-provider-models", str(g.user["id"]), 12, 60):
        return error_response("RATE_LIMITED", "模型检测过于频繁，请稍后再试", 429)
    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        result = provider_service.discover_models(payload, _provider_config())
    except provider_service.ProviderInputError as exc:
        return error_response("PROVIDER_INPUT_INVALID", str(exc), 400)
    except ProviderNetworkError:
        return error_response("PROVIDER_UNAVAILABLE", "检测上游模型失败，请检查 URL、Key 和网络连接", 502)
    except ProviderResponseTooLarge:
        return error_response("PROVIDER_RESPONSE_TOO_LARGE", "上游响应超过 2 MB 限制", 502)
    except ValueError:
        return error_response("PROVIDER_INVALID_RESPONSE", "上游服务返回的不是 JSON", 502)
    except provider_service.ProviderUpstreamError as exc:
        return error_response("PROVIDER_ERROR", str(exc), 502, {"upstream_status": exc.status_code})
    return jsonify(result)


@api_v1.post("/proxy/chat/completions")
@_required_user
def proxy_chat_completions() -> Response:
    """Call an allowlisted provider and automatically archive usage."""

    if not rate_limit.allow("v1-provider-proxy", str(g.user["id"]), 30, 60):
        return error_response("RATE_LIMITED", "代理请求过于频繁，请稍后再试", 429)
    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        result = provider_service.proxy_chat(
            payload=payload,
            user_id=g.user["id"],
            database=_database(),
            request_id=g.request_id,
            idempotency_key=request.headers.get("Idempotency-Key"),
            config=_provider_config(),
        )
    except provider_service.ProviderInputError as exc:
        return error_response("PROVIDER_INPUT_INVALID", str(exc), 400)
    except UsageValidationError as exc:
        return error_response("USAGE_INVALID", str(exc), 502)
    except ProviderNetworkError:
        return error_response("PROVIDER_UNAVAILABLE", "调用上游服务失败，请检查 URL、Key 和网络连接", 502)
    except ProviderResponseTooLarge:
        return error_response("PROVIDER_RESPONSE_TOO_LARGE", "上游响应超过 2 MB 限制", 502)
    except ValueError:
        return error_response("PROVIDER_INVALID_RESPONSE", "上游服务返回的不是 JSON", 502)
    except provider_service.ProviderUpstreamError as exc:
        return error_response("PROVIDER_ERROR", str(exc), 502, {"upstream_status": exc.status_code})
    return jsonify(result)


@api_v1.post("/events/work")
@_required_user
def create_work_event() -> Response:
    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        event_payload, idempotency_key = _unpack_command(payload, "work_event.create")
        record, replayed = events.insert_work_event(
            g.user["id"], event_payload, getattr(g, "request_id", "unknown"), idempotency_key, _database()
        )
    except events.EventValidationError as exc:
        return error_response("EVENT_VALIDATION_ERROR", str(exc), 400)
    return jsonify({"event": record, "replayed": replayed, "request_id": g.request_id}), 200 if replayed else 201


@api_v1.get("/logs")
@_required_user
def get_logs() -> Response:
    try:
        limit = max(1, min(int(request.args.get("limit", "50")), 200))
    except ValueError:
        limit = 50
    try:
        offset = max(0, int(request.args.get("offset", "0")))
    except ValueError:
        offset = 0
    page = events.recent_app_logs(g.user["id"], limit, offset, _database())
    return jsonify({
        "logs": page["items"],
        "pagination": page["pagination"],
    })


@api_v1.post("/logs")
@_required_user
def create_log() -> Response:
    payload = _payload()
    if payload is None:
        return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        log_payload, _ = _unpack_command(payload, "app_log.create")
        record = events.insert_app_log(g.user["id"], log_payload, getattr(g, "request_id", "unknown"), _database())
    except events.EventValidationError as exc:
        return error_response("LOG_VALIDATION_ERROR", str(exc), 400)
    return jsonify({"log": record}), 201


@api_v1.get("/admin/overview")
@_required_admin
def admin_overview() -> Response:
    return jsonify(admin_service.get_overview(g.user["id"], g.request_id, _database()))


@api_v1.get("/admin/users")
@_required_admin
def admin_users() -> Response:
    return jsonify(admin_service.list_users(
        g.user["id"], g.request_id, _database(), request.args.get("limit"), request.args.get("offset")
    ))


@api_v1.get("/admin/users/<int:user_id>/records")
@_required_admin
def admin_user_activity(user_id: int) -> Response:
    activity = admin_service.get_user_activity(
        g.user["id"], user_id, g.request_id, _database(), request.args.get("limit")
    )
    if activity is None:
        return error_response("USER_NOT_FOUND", "用户不存在", 404)
    return jsonify(activity)


@api_v1.get("/admin/export")
@_required_admin
def admin_export() -> Response:
    kind = str(request.args.get("kind", "usage") or "usage").strip().lower()
    try:
        csv_bytes = admin_service.export_csv(g.user["id"], kind, g.request_id, _database())
    except admin_service.AdminExportTooLargeError:
        return error_response(
            "EXPORT_TOO_LARGE",
            "导出结果超过安全边界，请缩小范围或分批导出",
            413,
            {"max_rows": admin_service.EXPORT_MAX_ROWS, "max_bytes": admin_service.EXPORT_MAX_BYTES},
        )
    except admin_service.AdminApplicationError as exc:
        return error_response("INVALID_EXPORT_KIND", str(exc), 400)
    response = Response(csv_bytes, mimetype="text/csv; charset=utf-8")
    response.headers["Content-Disposition"] = f"attachment; filename=admin_{kind}.csv"
    return response


@api_v1.get("/health")
def health() -> Response:
    return jsonify({"status": "ok", "protocol_version": 1})

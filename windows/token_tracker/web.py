"""Flask Web application, JSON API, and OpenAI-compatible usage proxy.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Compose HTTP routes, sessions, security policy, and service calls.
"""

from __future__ import annotations

import hmac
import os
import secrets
from functools import wraps
from typing import Any, Callable

from flask import (
    Flask,
    Response,
    abort,
    flash,
    g,
    jsonify,
    redirect,
    render_template,
    request,
    session,
    url_for,
)

from . import access_logging, admin_service, auth_service, db, provider_service, rate_limit, request_ids
from .api_contract import error_response
from .api_v1 import api_v1
from .mobile_auth import resolve_access_user_id
from .services import (
    USAGE_EXPORT_MAX_BYTES,
    USAGE_EXPORT_MAX_ROWS,
    UsageExportTooLargeError,
    UsageValidationError,
    add_usage,
    export_usage_csv,
    usage_records_page,
    usage_summary,
)
from .settings import build_settings, resolve_database


def create_app(db_path: str | os.PathLike[str] | None = None) -> Flask:
    """Compose the modular Flask application around one configured database."""

    app = Flask(__name__, template_folder="templates", static_folder="static")
    access_logging.install_server_log_filters()
    configured_db = resolve_database(db_path)
    app.config.from_mapping(build_settings(configured_db))
    db.init_db(app.config["DATABASE"])
    rate_limit.configure(
        app.config["DATABASE"],
        persistent=app.config.get("RUNTIME_MODE") in {"shared", "production", "lan"},
    )
    app.register_blueprint(api_v1)

    @app.before_request
    def load_current_user() -> None:
        """Resolve session or bearer identity and enforce browser CSRF policy."""

        # A short correlation id lets a user report one failing request without
        # putting provider credentials or traceback details into the response.
        g.request_id = request_ids.resolve(request.headers.get(request_ids.HEADER_NAME))
        g.request_started_at = access_logging.start_timer()
        g.user = None
        authorization = request.headers.get("Authorization")
        user_id = resolve_access_user_id(authorization, app.config["DATABASE"]) if authorization else session.get("user_id")
        if user_id is not None:
            g.user = db.find_user_by_id(int(user_id), app.config["DATABASE"])
            if g.user is None:
                session.clear()
        g.auth_method = "bearer" if authorization else "session"

        if request.method == "POST":
            # CSRF is checked centrally so every browser-side write route gets
            # the same protection. Bearer-authenticated v1 calls and the public
            # token exchange endpoints are intentionally outside cookie CSRF.
            token_auth = request.path.startswith("/api/v1/") and authorization
            public_token_exchange = request.path in {"/api/v1/auth/login", "/api/v1/auth/refresh"}
            external_ingest = request.path == "/api/v1/ingest/usage"
            if token_auth or public_token_exchange or external_ingest:
                return
            expected = session.get("csrf_token")
            provided = request.headers.get("X-CSRF-Token") or request.form.get("csrf_token")
            if not expected or not provided or not hmac.compare_digest(expected, provided):
                abort(400, description="CSRF token missing or invalid")

    @app.after_request
    def add_security_headers(response: Response) -> Response:
        """Apply the shared browser security, cache, and request-id headers."""

        # Keep this policy close to response creation: a new page or JSON route
        # should inherit the same browser boundary without remembering headers.
        response.headers.setdefault("X-Content-Type-Options", "nosniff")
        response.headers.setdefault("X-Frame-Options", "DENY")
        response.headers.setdefault("Referrer-Policy", "strict-origin-when-cross-origin")
        response.headers.setdefault(
            "Content-Security-Policy",
            "default-src 'self'; "
            "script-src 'self'; "
            "style-src 'self' https://fonts.googleapis.com; "
            "font-src 'self' https://fonts.gstatic.com; "
            "img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'",
        )
        if request.path.startswith(("/api/", "/api/v1/")) or request.path in {
            "/login",
            "/register",
            "/dashboard",
            "/admin",
        }:
            # Authenticated projections, token exchange responses, and provider
            # payloads must not be retained by a browser or shared proxy.
            response.headers.setdefault("Cache-Control", "no-store")
            response.headers.setdefault("Pragma", "no-cache")
            response.headers.setdefault("Expires", "0")
        if request.is_secure:
            response.headers.setdefault("Strict-Transport-Security", "max-age=31536000; includeSubDomains")
        response.headers.setdefault(request_ids.HEADER_NAME, getattr(g, "request_id", "unknown"))
        access_logging.record_request(
            app.logger,
            request,
            response,
            getattr(g, "request_id", "unknown"),
            getattr(g, "request_started_at", None),
        )
        return response

    @app.context_processor
    def inject_template_values() -> dict[str, Any]:
        """Expose request-scoped template values without leaking credentials."""

        return {"csrf_token": get_csrf_token()}

    @app.get("/")
    def index() -> Response:
        """Route visitors to the appropriate authenticated application surface."""

        if not g.user:
            return redirect(url_for("login"))
        return redirect(url_for("admin" if g.user.get("role") == "admin" else "dashboard"))

    @app.get("/favicon.ico")
    def favicon() -> Response:
        """Serve the existing vector mark for browsers that probe `.ico`."""

        return app.send_static_file("favicon.svg")

    @app.get("/privacy")
    def privacy() -> str:
        """Explain the local data boundary before classmates create accounts."""

        return render_template("privacy.html")

    @app.route("/register", methods=["GET", "POST"])
    def register() -> str | Response:
        """Render registration and create a local account with rate limiting."""

        if g.user:
            return redirect(url_for("admin" if g.user.get("role") == "admin" else "dashboard"))
        if request.method == "POST":
            if not rate_limit.allow("register", request.remote_addr or "unknown", 8, 900):
                flash("注册请求过于频繁，请稍后再试。", "error")
                return render_template("register.html"), 429
            username = request.form.get("username", "").strip()
            password = request.form.get("password", "")
            try:
                user = auth_service.register_user(username, password, app.config["DATABASE"])
            except auth_service.AuthValidationError as exc:
                flash(str(exc), "error")
            except auth_service.AuthConflictError as exc:
                flash(str(exc), "error")
            except Exception:
                flash("创建账户失败，请稍后重试。", "error")
                return render_template("register.html"), 500
            else:
                session.clear()
                session["user_id"] = user["id"]
                session["csrf_token"] = secrets.token_urlsafe(32)
                return redirect(url_for("dashboard"))
        return render_template("register.html")

    @app.route("/login", methods=["GET", "POST"])
    def login() -> str | Response:
        """Authenticate a local account and establish a browser session."""

        if g.user:
            return redirect(url_for("admin" if g.user.get("role") == "admin" else "dashboard"))
        if request.method == "POST":
            if not rate_limit.allow("login", request.remote_addr or "unknown", 10, 900):
                flash("登录尝试过于频繁，请 15 分钟后再试。", "error")
                return render_template("login.html"), 429
            username = request.form.get("username", "").strip()
            password = request.form.get("password", "")
            user = auth_service.authenticate_user(username, password, app.config["DATABASE"])
            if user is None:
                flash("用户名或密码不正确。", "error")
            else:
                session.clear()
                session["user_id"] = user["id"]
                session["csrf_token"] = secrets.token_urlsafe(32)
                return redirect(url_for("admin" if user.get("role") == "admin" else "dashboard"))
        return render_template("login.html")

    @app.post("/logout")
    @login_required
    def logout() -> Response:
        """Clear the current browser session and return to login."""

        session.clear()
        return redirect(url_for("login"))

    @app.get("/dashboard")
    @login_required
    def dashboard() -> str:
        """Render the personal usage dashboard shell."""

        return render_template("dashboard.html", user=g.user)

    @app.get("/admin")
    @admin_required
    def admin() -> str:
        """Render the administrator overview shell and record the page view."""

        admin_service.record_page_view(g.user["id"], g.request_id, app.config["DATABASE"])
        return render_template("admin.html", user=g.user)

    @app.get("/api/summary")
    @login_required
    def api_summary() -> Response:
        """Return the current user's aggregated dashboard summary as JSON."""

        try:
            payload = usage_summary(
                user_id=g.user["id"],
                period=request.args.get("period", "day"),
                date_from=request.args.get("from"),
                date_to=request.args.get("to"),
                path=app.config["DATABASE"],
            )
        except UsageValidationError as exc:
            return error_response("INVALID_RANGE", str(exc), 400)
        return jsonify(payload)

    @app.route("/api/records", methods=["GET", "POST"])
    @login_required
    def api_records() -> Response:
        """List or append the current user's usage records."""

        if request.method == "GET":
            try:
                payload = usage_records_page(
                    user_id=g.user["id"],
                    period=request.args.get("period", "day"),
                    date_from=request.args.get("from"),
                    date_to=request.args.get("to"),
                    limit=100,
                    offset=0,
                    path=app.config["DATABASE"],
                )
            except UsageValidationError as exc:
                return error_response("INVALID_RANGE", str(exc), 400)
            return jsonify({"records": payload["records"]})

        payload = request.get_json(silent=True) or {}
        if not isinstance(payload, dict):
            return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
        try:
            record = add_usage(
                user_id=g.user["id"],
                model=payload.get("model"),
                input_tokens=payload.get("input_tokens"),
                output_tokens=payload.get("output_tokens"),
                timestamp=payload.get("timestamp"),
                note=payload.get("note", ""),
                path=app.config["DATABASE"],
            )
        except UsageValidationError as exc:
            return error_response("VALIDATION_ERROR", str(exc), 400)
        return jsonify({"record": record}), 201

    @app.get("/api/export")
    @login_required
    def api_export() -> Response:
        """Download the current user's bounded usage range as CSV."""

        try:
            csv_bytes = export_usage_csv(
                user_id=g.user["id"],
                period=request.args.get("period", "day"),
                date_from=request.args.get("from"),
                date_to=request.args.get("to"),
                path=app.config["DATABASE"],
            )
        except UsageExportTooLargeError:
            return error_response(
                "EXPORT_TOO_LARGE",
                "导出结果超过安全边界，请缩小范围或分批导出",
                413,
                {"max_rows": USAGE_EXPORT_MAX_ROWS, "max_bytes": USAGE_EXPORT_MAX_BYTES},
            )
        except UsageValidationError as exc:
            return error_response("INVALID_RANGE", str(exc), 400)
        response = Response(csv_bytes, mimetype="text/csv; charset=utf-8")
        response.headers["Content-Disposition"] = "attachment; filename=token_usage.csv"
        return response

    @app.post("/api/provider/models")
    @login_required
    def provider_models() -> Response:
        """Discover selectable models before the first tracked call."""

        if not rate_limit.allow("provider-models", str(g.user["id"]), 12, 60):
            return error_response("RATE_LIMITED", "模型检测过于频繁，请稍后再试", 429)
        payload = request.get_json(silent=True)
        if not isinstance(payload, dict):
            return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
        try:
            result = provider_service.discover_models(payload, provider_service.runtime_config(app.config))
        except provider_service.ProviderInputError as exc:
            return error_response("PROVIDER_INPUT_INVALID", str(exc), 400)
        except provider_service.ProviderNetworkError:
            return error_response("PROVIDER_UNAVAILABLE", "检测上游模型失败，请检查 URL、Key 和网络连接", 502)
        except provider_service.ProviderResponseTooLarge:
            return error_response("PROVIDER_RESPONSE_TOO_LARGE", "上游响应超过 2 MB 限制", 502)
        except ValueError:
            return error_response("PROVIDER_INVALID_RESPONSE", "上游服务返回的不是 JSON", 502)
        except provider_service.ProviderUpstreamError as exc:
            return error_response("PROVIDER_ERROR", str(exc), 502, {"upstream_status": exc.status_code})
        return jsonify(result)

    @app.post("/api/proxy/chat/completions")
    @login_required
    def proxy_chat_completions() -> Response:
        """Proxy one allowlisted provider call and persist returned usage."""

        if not rate_limit.allow("proxy", str(g.user["id"]), 30, 60):
            return error_response("RATE_LIMITED", "代理请求过于频繁，请稍后再试", 429)
        payload = request.get_json(silent=True)
        if not isinstance(payload, dict):
            return error_response("BAD_REQUEST", "请求 JSON 必须是对象", 400)
        try:
            result = provider_service.proxy_chat(
                payload=payload,
                user_id=g.user["id"],
                database=app.config["DATABASE"],
                request_id=g.request_id,
                idempotency_key=request.headers.get("Idempotency-Key"),
                config=provider_service.runtime_config(app.config),
            )
        except provider_service.ProviderInputError as exc:
            return error_response("PROVIDER_INPUT_INVALID", str(exc), 400)
        except UsageValidationError as exc:
            return error_response("USAGE_INVALID", str(exc), 502)
        except provider_service.ProviderNetworkError:
            return error_response("PROVIDER_UNAVAILABLE", "调用上游服务失败，请检查 URL、Key 和网络连接", 502)
        except provider_service.ProviderResponseTooLarge:
            return error_response("PROVIDER_RESPONSE_TOO_LARGE", "上游响应超过 2 MB 限制", 502)
        except ValueError:
            return error_response("PROVIDER_INVALID_RESPONSE", "上游服务返回的不是 JSON", 502)
        except provider_service.ProviderUpstreamError as exc:
            return error_response("PROVIDER_ERROR", str(exc), 502, {"upstream_status": exc.status_code})
        return jsonify(result)

    @app.get("/api/health")
    def health() -> Response:
        """Expose a minimal liveness response without authentication data."""

        return jsonify(status="ok")

    @app.errorhandler(400)
    def bad_request(error: Any) -> tuple[Response, int]:
        """Render validation and CSRF failures in page or JSON form."""

        description = getattr(error, "description", "请求无效")
        if request.path.startswith("/api/"):
            code = "CSRF_INVALID" if str(description).startswith("CSRF") else "BAD_REQUEST"
            return error_response(code, description, 400)
        return render_template("error.html", title="请求无效", message=description), 400

    @app.errorhandler(404)
    def not_found(error: Any) -> tuple[Response, int]:
        """Render a consistent not-found response for pages and APIs."""

        if request.path.startswith("/api/"):
            return error_response("NOT_FOUND", "请求的资源不存在", 404)
        return render_template("error.html", title="页面不存在", message="请求的页面不存在"), 404

    @app.errorhandler(405)
    def method_not_allowed(error: Any) -> tuple[Response, int]:
        """Render a consistent method-not-allowed response for pages and APIs."""

        if request.path.startswith("/api/"):
            return error_response("METHOD_NOT_ALLOWED", "当前请求方法不受支持", 405)
        return render_template("error.html", title="操作不支持", message="当前请求方法不受支持"), 405

    @app.errorhandler(413)
    def request_too_large(error: Any) -> tuple[Response, int]:
        """Render a consistent payload-size error for pages and APIs."""

        if request.path.startswith("/api/"):
            return error_response("PAYLOAD_TOO_LARGE", "请求内容过大，最大支持 256 KB", 413)
        return render_template("error.html", title="请求过大", message="请求内容过大，最大支持 256 KB"), 413

    @app.errorhandler(500)
    def internal_error(error: Any) -> tuple[Response, int]:
        """Render a safe generic server error without exposing traceback data."""

        if request.path.startswith("/api/"):
            return error_response("INTERNAL_ERROR", "服务器内部错误", 500)
        return render_template("error.html", title="服务器错误", message="服务器暂时无法完成请求"), 500

    return app


def login_required(view: Callable[..., Any]) -> Callable[..., Any]:
    """Require an authenticated session or bearer token for a route."""

    @wraps(view)
    def wrapped(*args: Any, **kwargs: Any) -> Any:
        if g.user is None:
            if request.path.startswith("/api/"):
                return error_response("AUTH_REQUIRED", "请先登录", 401)
            return redirect(url_for("login"))
        return view(*args, **kwargs)

    return wrapped


def admin_required(view: Callable[..., Any]) -> Callable[..., Any]:
    """Enforce the server-side admin role for pages and JSON controllers."""

    @wraps(view)
    def wrapped(*args: Any, **kwargs: Any) -> Any:
        if g.user is None:
            if request.path.startswith("/api/"):
                return error_response("AUTH_REQUIRED", "请先登录", 401)
            return redirect(url_for("login"))
        if g.user.get("role") != "admin":
            if request.path.startswith("/api/"):
                return error_response("ADMIN_REQUIRED", "需要管理员权限", 403)
            flash("当前账户没有管理员权限。", "error")
            return redirect(url_for("dashboard"))
        return view(*args, **kwargs)

    return wrapped


def get_csrf_token() -> str:
    """Return the current browser CSRF token, creating it when necessary."""

    token = session.get("csrf_token")
    if not token:
        token = secrets.token_urlsafe(32)
        session["csrf_token"] = token
    return token

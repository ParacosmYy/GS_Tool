"""Local OpenAI-compatible gateway foundation.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep local provider forwarding and Usage Ingest reporting outside the
         central Web session process.
Module: Local integration / provider gateway
"""

from __future__ import annotations

from dataclasses import dataclass
import hmac
import ipaddress
from typing import Any
from urllib.parse import urlparse, urlunparse

from flask import Flask, Response, g, jsonify, request

from . import ingest_auth, providers, request_ids


GATEWAY_ACCESS_HEADER = "Authorization"
USAGE_STATUS_HEADER = "X-AI-Tracker-Usage"
MAX_INGEST_URL_LENGTH = 2048
MAX_GATEWAY_TOKEN_LENGTH = 256
LOOPBACK_HOSTS = frozenset({"127.0.0.1", "::1", "localhost"})


class GatewayConfigError(ValueError):
    """Raised when the local gateway cannot start safely."""


@dataclass(frozen=True)
class GatewayConfig:
    """Immutable runtime configuration; secrets remain process-memory only."""

    upstream_url: str
    provider_key: str
    ingest_url: str
    ingest_token: str
    host: str
    gateway_token: str | None
    allow_http: bool
    timeout: int = 120
    report_timeout: int = 10
    maximum_request_bytes: int = 256 * 1024
    maximum_response_bytes: int = 2 * 1024 * 1024


def build_config(
    *,
    upstream_url: Any,
    provider_key: Any,
    ingest_url: Any,
    ingest_token: Any,
    host: Any = "127.0.0.1",
    gateway_token: Any = None,
    allow_http: bool = False,
    timeout: Any = 120,
    report_timeout: Any = 10,
    maximum_request_bytes: Any = 256 * 1024,
    maximum_response_bytes: Any = 2 * 1024 * 1024,
) -> GatewayConfig:
    """Validate startup inputs before a socket can be opened."""

    host_value = str(host or "127.0.0.1").strip()
    if not host_value:
        raise GatewayConfigError("gateway host is required")
    try:
        canonical_upstream = providers.normalize_base_url(
            upstream_url,
            [str(upstream_url or "")],
            allow_http=allow_http,
        )
    except providers.UsageValidationError as exc:
        raise GatewayConfigError(str(exc)) from exc
    key_value = _bounded_secret(provider_key, providers.MAX_API_KEY_LENGTH, "provider_key")
    ingest_value = _bounded_secret(ingest_token, ingest_auth.MAX_TOKEN_LENGTH, "ingest_token")
    gateway_value = _optional_secret(gateway_token, MAX_GATEWAY_TOKEN_LENGTH, "gateway_token")
    if gateway_value and hmac.compare_digest(gateway_value, ingest_value):
        raise GatewayConfigError("gateway_token and ingest_token must be different")
    if not _is_loopback(host_value) and not gateway_value:
        raise GatewayConfigError("non-loopback gateway requires a separate gateway_token")
    try:
        ingest_endpoint = _normalize_ingest_url(ingest_url, allow_http)
        timeout_value = _positive_int(timeout, "timeout")
        report_timeout_value = _positive_int(report_timeout, "report_timeout")
        request_bytes = _positive_int(maximum_request_bytes, "maximum_request_bytes")
        response_bytes = _positive_int(maximum_response_bytes, "maximum_response_bytes")
    except ValueError as exc:
        raise GatewayConfigError(str(exc)) from exc
    return GatewayConfig(
        upstream_url=canonical_upstream,
        provider_key=key_value,
        ingest_url=ingest_endpoint,
        ingest_token=ingest_value,
        host=host_value,
        gateway_token=gateway_value,
        allow_http=bool(allow_http),
        timeout=timeout_value,
        report_timeout=report_timeout_value,
        maximum_request_bytes=request_bytes,
        maximum_response_bytes=response_bytes,
    )


def create_gateway_app(config: GatewayConfig) -> Flask:
    """Compose the local gateway without creating a central database/session."""

    app = Flask("token_tracker.gateway")
    app.config["MAX_CONTENT_LENGTH"] = config.maximum_request_bytes

    @app.before_request
    def guard_request() -> None:
        g.request_id = request_ids.resolve(request.headers.get(request_ids.HEADER_NAME))
        if request.path == "/health" or _authorized(config):
            return
        response, status = _error("GATEWAY_AUTH_REQUIRED", "gateway access token 无效", 401)
        return response, status

    @app.after_request
    def add_gateway_headers(response: Response) -> Response:
        response.headers.setdefault("Cache-Control", "no-store")
        response.headers.setdefault(request_ids.HEADER_NAME, getattr(g, "request_id", "unknown"))
        return response

    @app.errorhandler(413)
    def request_too_large(_exception: Any) -> tuple[Response, int]:
        return _error("REQUEST_TOO_LARGE", "gateway 请求超过大小限制", 413)

    @app.get("/health")
    def health() -> Response:
        return jsonify({"status": "ok", "protocol_version": 1, "mode": "local-gateway"})

    return app


def _authorized(config: GatewayConfig) -> bool:
    if not config.gateway_token:
        return True
    authorization = request.headers.get(GATEWAY_ACCESS_HEADER, "")
    if not authorization.startswith("Bearer "):
        return False
    presented = authorization[7:].strip()
    return bool(presented) and hmac.compare_digest(presented, config.gateway_token)


def _error(code: str, message: str, status: int) -> tuple[Response, int]:
    return jsonify({"error": {"code": code, "message": message}, "request_id": getattr(g, "request_id", "unknown")}), status


def _bounded_secret(value: Any, maximum: int, field_name: str) -> str:
    text = str(value or "").strip()
    if not text:
        raise GatewayConfigError(f"{field_name} is required")
    if len(text) > maximum:
        raise GatewayConfigError(f"{field_name} must be {maximum} characters or fewer")
    return text


def _optional_secret(value: Any, maximum: int, field_name: str) -> str | None:
    text = str(value or "").strip()
    if not text:
        return None
    if len(text) > maximum:
        raise GatewayConfigError(f"{field_name} must be {maximum} characters or fewer")
    return text


def _normalize_ingest_url(value: Any, allow_http: bool) -> str:
    text = str(value or "").strip().rstrip("/")
    if not text or len(text) > MAX_INGEST_URL_LENGTH:
        raise ValueError(f"ingest_url must be 1-{MAX_INGEST_URL_LENGTH} characters")
    parsed = urlparse(text)
    schemes = {"https", "http"} if allow_http else {"https"}
    if parsed.scheme not in schemes or not parsed.netloc:
        raise ValueError("ingest_url must be a complete HTTPS URL")
    if parsed.username or parsed.password or parsed.query or parsed.fragment:
        raise ValueError("ingest_url must not contain credentials, query, or fragment")
    if not parsed.path.endswith("/api/v1/ingest/usage"):
        raise ValueError("ingest_url must end with /api/v1/ingest/usage")
    return urlunparse((parsed.scheme, parsed.netloc, parsed.path, "", "", ""))


def _positive_int(value: Any, field_name: str) -> int:
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{field_name} must be a positive integer") from exc
    if parsed <= 0:
        raise ValueError(f"{field_name} must be a positive integer")
    return parsed


def _is_loopback(host: str) -> bool:
    if host.casefold() in LOOPBACK_HOSTS:
        return True
    try:
        return ipaddress.ip_address(host).is_loopback
    except ValueError:
        return False

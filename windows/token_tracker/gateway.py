"""Local OpenAI-compatible gateway foundation.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep local provider forwarding and Usage Ingest reporting outside the
         central Web session process.
Module: Local integration / provider gateway
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
import hmac
import ipaddress
import json
import os
import re
import uuid
from typing import Any
from urllib.parse import urlparse, urlunparse

from flask import Flask, Response, g, jsonify, request, stream_with_context
import requests

from . import ingest_auth, providers, request_ids
from .gateway_reporting import UsageReporter


GATEWAY_ACCESS_HEADER = "Authorization"
USAGE_STATUS_HEADER = "X-AI-Tracker-Usage"
MAX_INGEST_URL_LENGTH = 2048
MAX_GATEWAY_TOKEN_LENGTH = 256
MAX_IDEMPOTENCY_KEY_LENGTH = 160
LOOPBACK_HOSTS = frozenset({"127.0.0.1", "::1", "localhost"})
_ENVIRONMENT_NAME = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")


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


def read_environment_secret(
    variable_name: Any,
    field_name: str,
    maximum: int,
    *,
    required: bool = True,
) -> str | None:
    """Read one secret by validated variable name without logging its value."""

    name = str(variable_name or "").strip()
    if not _ENVIRONMENT_NAME.fullmatch(name):
        raise GatewayConfigError(f"{field_name} environment variable name is invalid")
    value = os.getenv(name, "").strip()
    if not value and not required:
        return None
    return _bounded_secret(value, maximum, field_name)


def create_gateway_app(config: GatewayConfig) -> Flask:
    """Compose the local gateway without creating a central database/session."""

    app = Flask("token_tracker.gateway")
    app.config["MAX_CONTENT_LENGTH"] = config.maximum_request_bytes
    app.extensions["usage_reporter"] = UsageReporter(
        config.ingest_url,
        config.ingest_token,
        config.report_timeout,
    )

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

    @app.get("/v1/models")
    @app.get("/models")
    def models() -> Any:
        return _list_models(config)

    @app.post("/v1/chat/completions")
    @app.post("/chat/completions")
    def chat_completions() -> Any:
        return _chat_completions(config, app.extensions["usage_reporter"])

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
    return (
        jsonify({"error": {"code": code, "message": message}, "request_id": getattr(g, "request_id", "unknown")}),
        status,
    )


def _list_models(config: GatewayConfig) -> Any:
    """Forward model discovery with the configured upstream identity only."""

    try:
        response = providers.list_models(
            config.upstream_url,
            config.provider_key,
            [config.upstream_url],
            config.allow_http,
            config.timeout,
        )
        body = providers.decode_json_response(response, config.maximum_response_bytes)
    except providers.ProviderNetworkError:
        return _error("UPSTREAM_UNAVAILABLE", "gateway 无法连接上游 provider", 502)
    except providers.ProviderResponseTooLarge:
        return _error("UPSTREAM_RESPONSE_TOO_LARGE", "上游模型响应超过大小限制", 502)
    except ValueError:
        return _error("UPSTREAM_INVALID_RESPONSE", "上游模型响应不是有效 JSON", 502)
    if 300 <= response.status_code <= 399:
        return _error("UPSTREAM_REDIRECT", "上游 provider 返回了被禁止的重定向", 502)
    return _json_body(body, _upstream_status(response.status_code))


def _chat_completions(config: GatewayConfig, reporter: UsageReporter) -> Any:
    """Proxy one JSON chat call and report authoritative usage afterward."""

    payload = request.get_json(silent=True)
    if not isinstance(payload, dict):
        return _error("BAD_REQUEST", "请求 JSON 必须是对象", 400)
    try:
        chat_request = _prepare_gateway_request(payload, config)
    except providers.UsageValidationError as exc:
        return _error("PROVIDER_INPUT_INVALID", str(exc), 400)
    try:
        provider_response = providers.call_chat(chat_request, config.timeout)
    except providers.ProviderNetworkError:
        return _error("UPSTREAM_UNAVAILABLE", "gateway 调用上游 provider 失败", 502)
    if not 200 <= provider_response.status_code <= 299:
        if 300 <= provider_response.status_code <= 399:
            provider_response.close()
            return _error("UPSTREAM_REDIRECT", "上游 provider 返回了被禁止的重定向", 502)
        return _decode_error_response(provider_response, config)
    if chat_request.payload.get("stream") is True:
        return _stream_response(provider_response, chat_request.model, config, reporter)
    try:
        body = providers.decode_json_response(provider_response, config.maximum_response_bytes)
    except providers.ProviderResponseTooLarge:
        return _error("UPSTREAM_RESPONSE_TOO_LARGE", "上游响应超过大小限制", 502)
    except ValueError:
        return _error("UPSTREAM_INVALID_RESPONSE", "上游响应不是有效 JSON", 502)
    usage, input_tokens, output_tokens = providers.extract_usage(body)
    status = _usage_status(
        config,
        reporter,
        usage,
        _response_model(body, chat_request.model),
        input_tokens,
        output_tokens,
        _request_idempotency_key(),
    )
    return _json_body(body, 200, {USAGE_STATUS_HEADER: status})


def _prepare_gateway_request(payload: dict[str, Any], config: GatewayConfig) -> providers.ChatRequest:
    """Replace client routing/credential fields before entering the adapter."""

    internal_payload = dict(payload)
    internal_payload["base_url"] = config.upstream_url
    internal_payload["api_key"] = config.provider_key
    if internal_payload.get("stream") is True:
        stream_options = internal_payload.get("stream_options")
        if stream_options is None:
            internal_payload["stream_options"] = {"include_usage": True}
        elif isinstance(stream_options, dict):
            internal_payload["stream_options"] = {**stream_options, "include_usage": True}
    return providers.prepare_chat_request(
        internal_payload,
        [config.upstream_url],
        config.allow_http,
        allow_stream=True,
        user_agent=request.headers.get("User-Agent"),
    )


def _decode_error_response(response: requests.Response, config: GatewayConfig) -> Any:
    try:
        body = providers.decode_json_response(response, config.maximum_response_bytes)
    except providers.ProviderResponseTooLarge:
        return _error("UPSTREAM_RESPONSE_TOO_LARGE", "上游错误响应超过大小限制", 502)
    except ValueError:
        return _error("UPSTREAM_INVALID_RESPONSE", "上游错误响应不是有效 JSON", 502)
    return _json_body(body, _upstream_status(response.status_code))


def _json_body(body: Any, status: int, headers: Mapping[str, str] | None = None) -> Response:
    response = jsonify(body)
    response.status_code = status
    for name, value in (headers or {}).items():
        response.headers[name] = value
    return response


def _upstream_status(status_code: int) -> int:
    if 200 <= status_code <= 299 or 400 <= status_code <= 599:
        return status_code
    return 502


def _response_model(body: Any, requested_model: str) -> str:
    if isinstance(body, dict):
        response_model = str(body.get("model") or "").strip()
        if response_model:
            return response_model
    return requested_model


def _request_idempotency_key() -> str:
    candidate = request.headers.get("Idempotency-Key", "").strip()
    if candidate and len(candidate) <= MAX_IDEMPOTENCY_KEY_LENGTH:
        return candidate
    return f"gateway-{uuid.uuid4().hex}"


def _usage_status(
    config: GatewayConfig,
    reporter: UsageReporter,
    usage: dict[str, Any] | None,
    model: str,
    input_tokens: Any,
    output_tokens: Any,
    idempotency_key: str,
) -> str:
    if usage is None:
        return "missing"
    return reporter.report(model, input_tokens, output_tokens, idempotency_key)


def _stream_response(
    provider_response: requests.Response,
    requested_model: str,
    config: GatewayConfig,
    reporter: UsageReporter,
) -> Response | tuple[Response, int]:
    """Pass through SSE while retaining only bounded usage metadata."""

    if _advertised_size_exceeds(provider_response, config.maximum_response_bytes):
        provider_response.close()
        return _error("UPSTREAM_RESPONSE_TOO_LARGE", "上游流式响应超过大小限制", 502)
    accumulator = _SSEUsageAccumulator(config.maximum_response_bytes)
    idempotency_key = _request_idempotency_key()

    @stream_with_context
    def generate() -> Any:
        final_status = "missing"
        total_bytes = 0
        try:
            for chunk in provider_response.iter_content(chunk_size=16 * 1024):
                if not chunk:
                    continue
                chunk_bytes = chunk.encode("utf-8") if isinstance(chunk, str) else bytes(chunk)
                total_bytes += len(chunk_bytes)
                if total_bytes > config.maximum_response_bytes:
                    final_status = "stream-failed"
                    break
                try:
                    accumulator.feed(chunk_bytes)
                except ValueError:
                    final_status = "stream-failed"
                    break
                yield chunk_bytes
            else:
                accumulator.finish()
                usage, input_tokens, output_tokens = accumulator.usage_values()
                final_status = _usage_status(
                    config,
                    reporter,
                    usage,
                    accumulator.model or requested_model,
                    input_tokens,
                    output_tokens,
                    idempotency_key,
                )
        except requests.RequestException:
            final_status = "stream-failed"
        finally:
            provider_response.close()
        yield f": ai-token-tracker-usage={final_status}\n\n".encode("utf-8")

    return Response(
        generate(),
        content_type="text/event-stream; charset=utf-8",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
            USAGE_STATUS_HEADER: "pending",
        },
    )


class _SSEUsageAccumulator:
    """Parse SSE data events without retaining the generated assistant text."""

    def __init__(self, maximum_buffer_bytes: int) -> None:
        self._buffer = ""
        self._maximum_buffer_bytes = maximum_buffer_bytes
        self.model: str | None = None
        self.usage: dict[str, Any] | None = None

    def feed(self, chunk: bytes) -> None:
        self._buffer += chunk.decode("utf-8", errors="replace")
        self._buffer = self._buffer.replace("\r\n", "\n").replace("\r", "\n")
        if len(self._buffer.encode("utf-8")) > self._maximum_buffer_bytes:
            raise ValueError("SSE event exceeds the response safety boundary")
        while "\n\n" in self._buffer:
            event, self._buffer = self._buffer.split("\n\n", 1)
            self._consume_event(event)

    def finish(self) -> None:
        if self._buffer.strip():
            self._consume_event(self._buffer)
        self._buffer = ""

    def usage_values(self) -> tuple[dict[str, Any] | None, Any, Any]:
        if self.usage is None:
            return None, None, None
        input_tokens = self.usage.get("prompt_tokens", self.usage.get("input_tokens", self.usage.get("promptTokens")))
        output_tokens = self.usage.get("completion_tokens", self.usage.get("output_tokens", self.usage.get("completionTokens")))
        return self.usage, input_tokens, output_tokens

    def _consume_event(self, event: str) -> None:
        data_lines = [line[5:].lstrip() for line in event.split("\n") if line.startswith("data:")]
        if not data_lines:
            return
        data = "\n".join(data_lines).strip()
        if not data or data == "[DONE]":
            return
        try:
            payload = json.loads(data)
        except (TypeError, ValueError):
            return
        if not isinstance(payload, dict):
            return
        model = str(payload.get("model") or "").strip()
        if model:
            self.model = model
        usage = _chunk_usage(payload)
        if usage is not None:
            self.usage = usage


def _chunk_usage(payload: dict[str, Any]) -> dict[str, Any] | None:
    usage = payload.get("usage")
    if isinstance(usage, dict):
        return usage
    choices = payload.get("choices")
    if isinstance(choices, list):
        for choice in choices:
            if isinstance(choice, dict) and isinstance(choice.get("usage"), dict):
                return choice["usage"]
    return None


def _advertised_size_exceeds(response: requests.Response, maximum_bytes: int) -> bool:
    try:
        advertised = response.headers.get("Content-Length")
        return bool(advertised and int(advertised) > maximum_bytes)
    except (TypeError, ValueError):
        return False


def _bounded_secret(value: Any, maximum: int, field_name: str) -> str:
    text = str(value or "").strip()
    if not text:
        raise GatewayConfigError(f"{field_name} is required")
    if len(text) > maximum:
        raise GatewayConfigError(f"{field_name} must be {maximum} characters or fewer")
    if "\r" in text or "\n" in text:
        raise GatewayConfigError(f"{field_name} must not contain line breaks")
    return text


def _optional_secret(value: Any, maximum: int, field_name: str) -> str | None:
    text = str(value or "").strip()
    if not text:
        return None
    if len(text) > maximum:
        raise GatewayConfigError(f"{field_name} must be {maximum} characters or fewer")
    if "\r" in text or "\n" in text:
        raise GatewayConfigError(f"{field_name} must not contain line breaks")
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

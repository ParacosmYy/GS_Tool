"""OpenAI-compatible provider boundary.

The Web layer owns HTTP routes and user sessions; this module owns the small,
security-sensitive translation between our UI contract and an upstream
provider. Keeping that boundary here makes a future Kimi/Anthropic/Gemini
adapter an additive change instead of another branch inside ``web.py``.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Validate provider boundaries and parse untrusted upstream responses.
"""

from __future__ import annotations

from dataclasses import dataclass
import json
from typing import Any, Protocol
from urllib.parse import urlparse, urlunparse

import requests

from .services import UsageValidationError


FORWARDED_CHAT_FIELDS = (
    "temperature",
    "top_p",
    "max_tokens",
    "presence_penalty",
    "frequency_penalty",
    "response_format",
    "tools",
    "tool_choice",
)


class ProviderNetworkError(RuntimeError):
    """A provider connection failed without exposing request details to users."""


class ProviderResponseTooLarge(RuntimeError):
    """The upstream JSON response exceeded the configured memory boundary."""


@dataclass(frozen=True)
class ChatRequest:
    """Validated data needed for one non-streaming usage-aware call."""

    endpoint: str
    api_key: str
    model: str
    payload: dict[str, Any]


class ProviderAdapter(Protocol):
    """Small capability boundary for one upstream provider family.

    The protocol keeps ``web.py`` independent from vendor-specific request and
    response details. A future Anthropic, Gemini, or Kimi Code adapter can be
    added behind this shape without creating another route-level branch.
    """

    name: str

    def prepare_chat_request(
        self,
        payload: dict[str, Any],
        allowed_base_urls: list[str],
        allow_http: bool,
    ) -> ChatRequest: ...

    def list_models(
        self,
        base_url: Any,
        api_key: Any,
        allowed_base_urls: list[str],
        allow_http: bool,
        timeout: int,
    ) -> requests.Response: ...

    def call_chat(self, request_data: ChatRequest, timeout: int) -> requests.Response: ...

    def extract_model_ids(self, response_json: Any) -> list[str]: ...

    def extract_usage(self, response_json: Any) -> tuple[dict[str, Any] | None, Any, Any]: ...

    def decode_response(self, response: requests.Response, maximum_bytes: int) -> Any: ...


class OpenAICompatibleAdapter:
    """Default adapter for OpenAI-compatible ``/models`` and chat APIs."""

    name = "openai-compatible"

    def prepare_chat_request(
        self,
        payload: dict[str, Any],
        allowed_base_urls: list[str],
        allow_http: bool,
    ) -> ChatRequest:
        return prepare_chat_request(payload, allowed_base_urls, allow_http)

    def list_models(
        self,
        base_url: Any,
        api_key: Any,
        allowed_base_urls: list[str],
        allow_http: bool,
        timeout: int,
    ) -> requests.Response:
        return list_models(base_url, api_key, allowed_base_urls, allow_http, timeout)

    def call_chat(self, request_data: ChatRequest, timeout: int) -> requests.Response:
        return call_chat(request_data, timeout)

    def extract_usage(self, response_json: Any) -> tuple[dict[str, Any] | None, Any, Any]:
        return extract_usage(response_json)

    def extract_model_ids(self, response_json: Any) -> list[str]:
        return extract_model_ids(response_json)

    def decode_response(self, response: requests.Response, maximum_bytes: int) -> Any:
        return decode_json_response(response, maximum_bytes)


OPENAI_COMPATIBLE_ADAPTER = OpenAICompatibleAdapter()
MAX_API_KEY_LENGTH = 4096
MAX_BASE_URL_LENGTH = 2048
MAX_MODEL_LENGTH = 200
MAX_MESSAGES = 100


def resolve_provider_adapter(provider: Any = "auto") -> ProviderAdapter:
    """Resolve a public provider alias without exposing implementation details."""

    alias = str(provider or "auto").strip().casefold()
    if alias in {"auto", "openai", "openai-compatible"}:
        return OPENAI_COMPATIBLE_ADAPTER
    raise UsageValidationError(f"暂不支持 provider: {alias}")


def required_text(value: Any, field_name: str, maximum: int = 4096) -> str:
    text = str(value or "").strip()
    if not text:
        raise UsageValidationError(f"{field_name} is required")
    if len(text) > maximum:
        raise UsageValidationError(f"{field_name} must be {maximum} characters or fewer")
    return text


def normalize_base_url(
    base_url: Any,
    allowed_base_urls: list[str],
    allow_http: bool = False,
) -> str:
    """Validate and return a canonical provider base URL.

    The allowlist is intentionally checked against the base URL before a
    resource path is appended. This prevents an otherwise valid host from
    becoming an open proxy to an arbitrary path or redirect target.
    """

    base = required_text(base_url, "base_url", MAX_BASE_URL_LENGTH).rstrip("/")
    parsed = urlparse(base)
    schemes = {"http", "https"} if allow_http else {"https"}
    if parsed.scheme not in schemes or not parsed.netloc:
        protocol_hint = "http(s)" if allow_http else "HTTPS"
        raise UsageValidationError(f"base_url 必须是完整的 {protocol_hint} URL")
    if parsed.username or parsed.password or parsed.query or parsed.fragment:
        raise UsageValidationError("base_url 不应包含账号、密码、查询参数或片段")

    path = parsed.path.rstrip("/")
    if path.endswith("/chat/completions"):
        path = path[: -len("/chat/completions")].rstrip("/")
    canonical = urlunparse((parsed.scheme, parsed.netloc, path, "", "", ""))
    allowed = {item.strip().rstrip("/") for item in allowed_base_urls if item.strip()}
    if not allowed:
        raise UsageValidationError("服务端尚未配置 TOKEN_TRACKER_ALLOWED_BASE_URLS 白名单")
    if canonical not in allowed:
        raise UsageValidationError("该 base_url 不在 TOKEN_TRACKER_ALLOWED_BASE_URLS 白名单中")
    return canonical


def resource_endpoint(
    base_url: Any,
    resource: str,
    allowed_base_urls: list[str],
    allow_http: bool = False,
) -> str:
    """Build one allowlisted resource URL such as ``models`` or chat endpoint."""

    base = normalize_base_url(base_url, allowed_base_urls, allow_http)
    parsed = urlparse(base)
    path = parsed.path.rstrip("/")
    resource_path = f"{path}/{resource.lstrip('/')}" if path else f"/{resource.lstrip('/')}"
    return urlunparse((parsed.scheme, parsed.netloc, resource_path, "", "", ""))


def prepare_chat_request(
    payload: dict[str, Any],
    allowed_base_urls: list[str],
    allow_http: bool = False,
) -> ChatRequest:
    """Validate the browser contract and create a safe upstream payload."""

    endpoint = resource_endpoint(
        payload.get("base_url"),
        "chat/completions",
        allowed_base_urls,
        allow_http,
    )
    api_key = required_text(payload.get("api_key"), "api_key", MAX_API_KEY_LENGTH)
    model = required_text(payload.get("model"), "model", MAX_MODEL_LENGTH)
    messages = payload.get("messages")
    if not isinstance(messages, list) or not messages:
        raise UsageValidationError("messages must be a non-empty list")
    if len(messages) > MAX_MESSAGES:
        raise UsageValidationError(f"messages must contain {MAX_MESSAGES} items or fewer")
    if payload.get("stream"):
        raise UsageValidationError("当前代理先支持非流式请求，以便从响应 usage 自动记录")

    request_payload: dict[str, Any] = {"model": model, "messages": messages, "stream": False}
    for field in FORWARDED_CHAT_FIELDS:
        if field in payload:
            request_payload[field] = payload[field]
    return ChatRequest(endpoint=endpoint, api_key=api_key, model=model, payload=request_payload)


def call_chat(request_data: ChatRequest, timeout: int) -> requests.Response:
    """Call once with redirects disabled so the allowlist cannot be bypassed."""

    try:
        return requests.post(
            request_data.endpoint,
            headers={
                "Authorization": f"Bearer {request_data.api_key}",
                "Content-Type": "application/json",
            },
            json=request_data.payload,
            timeout=timeout,
            allow_redirects=False,
            stream=True,
        )
    except requests.RequestException as exc:
        raise ProviderNetworkError from exc


def list_models(base_url: Any, api_key: Any, allowed_base_urls: list[str], allow_http: bool, timeout: int) -> requests.Response:
    """Fetch model metadata without persisting the supplied API key."""

    endpoint = resource_endpoint(base_url, "models", allowed_base_urls, allow_http)
    key = required_text(api_key, "api_key", MAX_API_KEY_LENGTH)
    try:
        return requests.get(
            endpoint,
            headers={
                "Authorization": f"Bearer {key}",
                "Accept": "application/json",
            },
            timeout=timeout,
            allow_redirects=False,
            stream=True,
        )
    except requests.RequestException as exc:
        raise ProviderNetworkError from exc


def decode_json_response(response: requests.Response, maximum_bytes: int) -> Any:
    """Decode an upstream JSON body while enforcing a bounded read.

    ``MAX_CONTENT_LENGTH`` protects browser requests, not remote responses.
    Reading chunks here prevents a provider or compromised endpoint from
    allocating unbounded memory before we can return a useful error.
    """

    try:
        advertised = response.headers.get("Content-Length")
        if advertised and int(advertised) > maximum_bytes:
            raise ProviderResponseTooLarge
    except (TypeError, ValueError):
        # A malformed length header is not trusted; the chunk limit below is
        # still authoritative.
        pass

    body = bytearray()
    try:
        try:
            for chunk in response.iter_content(chunk_size=16 * 1024):
                if not chunk:
                    continue
                body.extend(chunk)
                if len(body) > maximum_bytes:
                    raise ProviderResponseTooLarge
            return json.loads(bytes(body).decode("utf-8"))
        except requests.RequestException as exc:
            raise ProviderNetworkError from exc
    finally:
        response.close()


def extract_model_ids(response_json: Any) -> list[str]:
    """Extract sorted model IDs from the common OpenAI ``data`` shape."""

    if not isinstance(response_json, dict) or not isinstance(response_json.get("data"), list):
        return []
    models: set[str] = set()
    for item in response_json["data"]:
        if isinstance(item, dict):
            model_id = str(item.get("id") or "").strip()
            if model_id:
                models.add(model_id)
    return sorted(models, key=str.casefold)


def extract_usage(response_json: Any) -> tuple[dict[str, Any] | None, Any, Any]:
    """Read usage aliases without inventing counts when a provider omits them."""

    usage = response_json.get("usage") if isinstance(response_json, dict) else None
    if not isinstance(usage, dict):
        return None, None, None
    input_tokens = usage.get("prompt_tokens", usage.get("input_tokens", usage.get("promptTokens")))
    output_tokens = usage.get("completion_tokens", usage.get("output_tokens", usage.get("completionTokens")))
    return usage, input_tokens, output_tokens

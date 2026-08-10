"""Provider use cases shared by browser and bearer API routes.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep provider validation, bounded upstream calls, usage extraction,
         and automatic token recording in one application service.
Module: Application / provider collection service

API keys are accepted as ephemeral inputs only. This module never returns a
key, writes one to SQLite, or places one in an application log.
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
from typing import Any

from .providers import (
    ProviderNetworkError,
    ProviderResponseTooLarge,
    ProviderAdapter,
    resolve_provider_adapter,
)
from .provider_projection import project_response
from .services import UsageValidationError, add_usage_result


class ProviderInputError(UsageValidationError):
    """Raised when a provider request cannot pass the configured boundary."""


class ProviderUpstreamError(RuntimeError):
    """Raised when an allowlisted provider returns an unsuccessful status."""

    def __init__(self, status_code: int, message: str) -> None:
        super().__init__(message)
        self.status_code = status_code


@dataclass(frozen=True)
class ProviderRuntimeConfig:
    """Security and resource limits copied from the current application config."""

    allowed_base_urls: list[str]
    allow_http: bool
    timeout: int
    maximum_response_bytes: int


def runtime_config(settings: Mapping[str, Any]) -> ProviderRuntimeConfig:
    """Project Flask settings into the provider service boundary."""

    return ProviderRuntimeConfig(
        allowed_base_urls=list(settings["ALLOWED_BASE_URLS"]),
        allow_http=bool(settings["ALLOW_HTTP_PROXY"]),
        timeout=int(settings["PROXY_TIMEOUT"]),
        maximum_response_bytes=int(settings["PROXY_MAX_RESPONSE_BYTES"]),
    )


def discover_models(payload: dict[str, Any], config: ProviderRuntimeConfig) -> dict[str, Any]:
    """Discover model IDs without persisting the submitted provider key."""

    adapter = _resolve_adapter(payload)
    try:
        response = adapter.list_models(
            payload.get("base_url"),
            payload.get("api_key"),
            config.allowed_base_urls,
            config.allow_http,
            config.timeout,
        )
    except ProviderNetworkError:
        raise
    response_json = _decode(adapter, response, config.maximum_response_bytes)
    if response.status_code >= 400:
        raise ProviderUpstreamError(response.status_code, "上游模型列表请求失败")
    models = adapter.extract_model_ids(response_json)
    if not models:
        raise ProviderInputError("上游没有返回可识别的模型列表")
    return {"provider": adapter.name, "models": models}


def proxy_chat(
    payload: dict[str, Any],
    user_id: int,
    database: str,
    request_id: str,
    idempotency_key: str | None,
    config: ProviderRuntimeConfig,
) -> dict[str, Any]:
    """Call an allowlisted provider and record authoritative usage once."""

    adapter = _resolve_adapter(payload)
    try:
        chat_request = adapter.prepare_chat_request(
            payload,
            config.allowed_base_urls,
            config.allow_http,
        )
    except UsageValidationError as exc:
        raise ProviderInputError(str(exc)) from exc
    try:
        provider_response = adapter.call_chat(chat_request, config.timeout)
    except ProviderNetworkError:
        raise
    response_json = _decode(adapter, provider_response, config.maximum_response_bytes)
    if provider_response.status_code >= 400:
        raise ProviderUpstreamError(provider_response.status_code, "上游服务返回错误")
    response_projection = project_response(response_json)

    usage, input_tokens, output_tokens = adapter.extract_usage(response_json)
    if usage is None:
        return {
            "provider": adapter.name,
            "response": response_projection,
            "usage": None,
            "recorded": False,
            "replayed": False,
            "request_id": request_id,
            "record": None,
            "warning": "响应没有 usage 字段，因此未自动记录 token。可手动补录。",
        }
    if input_tokens is None or output_tokens is None:
        return {
            "provider": adapter.name,
            "response": response_projection,
            "usage": usage,
            "recorded": False,
            "replayed": False,
            "request_id": request_id,
            "record": None,
            "warning": "usage 中缺少输入或输出 token 字段，因此未自动记录。",
        }

    served_model = response_json.get("model") if isinstance(response_json, dict) else None
    record, replayed = add_usage_result(
        user_id=user_id,
        model=served_model or chat_request.model,
        input_tokens=input_tokens,
        output_tokens=output_tokens,
        note=payload.get("note", "通过代理调用"),
        source="proxy",
        path=database,
        idempotency_key=idempotency_key,
    )
    return {
        "provider": adapter.name,
        "response": response_projection,
        "usage": usage,
        "recorded": True,
        "replayed": replayed,
        "request_id": request_id,
        "record": record,
        "warning": None,
    }


def _resolve_adapter(payload: dict[str, Any]) -> ProviderAdapter:
    try:
        return resolve_provider_adapter(payload.get("provider"))
    except UsageValidationError as exc:
        raise ProviderInputError(str(exc)) from exc


def _decode(adapter: ProviderAdapter, response: Any, maximum_response_bytes: int) -> Any:
    """Decode through the adapter so response-size and close rules stay shared."""

    try:
        return adapter.decode_response(response, maximum_response_bytes)
    except (ProviderResponseTooLarge, ProviderNetworkError, ValueError):
        raise

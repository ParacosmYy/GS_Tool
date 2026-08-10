"""Project untrusted provider responses into a bounded client contract.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep raw upstream payloads out of browser, Android, and log surfaces.
Module: Application / provider response projection
"""

from __future__ import annotations

from collections.abc import Mapping, Sequence
from typing import Any


MAX_PROVIDER_TEXT_CHARS = 20_000
MAX_PROVIDER_ID_CHARS = 200


def project_response(response: Any) -> dict[str, Any]:
    """Return only the stable, bounded response fields clients can render.

    Provider payloads may contain hidden reasoning, tool arguments, vendor
    metadata, echoed request content, or other fields that are not part of our
    public API. Only the first choice's user-visible text is projected. The
    usage object is returned separately by ``provider_service`` after its
    numeric validation, so it is deliberately not copied here.
    """

    if not isinstance(response, Mapping):
        return {"choices": []}

    projected: dict[str, Any] = {}
    for field in ("id", "model"):
        value = _bounded_text(response.get(field), MAX_PROVIDER_ID_CHARS)
        if value:
            projected[field] = value

    choices = response.get("choices")
    is_choice_sequence = isinstance(choices, Sequence) and not isinstance(choices, (str, bytes))
    first_choice = choices[0] if is_choice_sequence and choices else None
    choice_projection = _project_choice(first_choice)
    projected["choices"] = [choice_projection] if choice_projection else []
    return projected


def _project_choice(choice: Any) -> dict[str, Any] | None:
    if not isinstance(choice, Mapping):
        return None

    message = choice.get("message")
    content = _content_text(message.get("content") if isinstance(message, Mapping) else None)
    if not content:
        content = _bounded_text(choice.get("text"), MAX_PROVIDER_TEXT_CHARS)
    if not content:
        return None

    message_projection = {"role": "assistant", "content": content}
    result: dict[str, Any] = {"message": message_projection}
    finish_reason = _bounded_text(choice.get("finish_reason"), 40)
    if finish_reason:
        result["finish_reason"] = finish_reason
    return result


def _content_text(content: Any) -> str:
    if isinstance(content, str):
        return content[:MAX_PROVIDER_TEXT_CHARS]
    if not isinstance(content, Sequence) or isinstance(content, (str, bytes)):
        return ""

    parts: list[str] = []
    remaining = MAX_PROVIDER_TEXT_CHARS
    for item in content:
        if not isinstance(item, Mapping):
            continue
        text = _bounded_text(item.get("text"), remaining)
        if not text:
            continue
        parts.append(text)
        remaining -= len(text)
        if remaining <= 0:
            break
    return "\n".join(parts)[:MAX_PROVIDER_TEXT_CHARS]


def _bounded_text(value: Any, maximum: int) -> str:
    if not isinstance(value, str):
        return ""
    return value.strip()[:maximum]

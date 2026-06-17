"""Text decoding helpers for controller-facing serial input."""

from __future__ import annotations


def decode_injected_text(text: str) -> str:
    return text.replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t")

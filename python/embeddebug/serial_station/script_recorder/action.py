"""脚本动作。"""
from __future__ import annotations
from dataclasses import dataclass
from typing import Any

@dataclass(frozen=True)
class ScriptAction:
    """单个脚本动作。"""
    type: str
    payload: str
    timestamp_ms: int
    label: str = ""
    SEND = "SEND"; RECEIVE = "RECEIVE"; CONNECT = "CONNECT"; DISCONNECT = "DISCONNECT"; DELAY = "DELAY"; WAIT = "WAIT"

    def to_dict(self) -> dict[str, Any]:
        return {"type": self.type, "payload": self.payload, "timestamp_ms": self.timestamp_ms, "label": self.label}

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> ScriptAction:
        return cls(type=str(data["type"]), payload=str(data["payload"]), timestamp_ms=int(data["timestamp_ms"]), label=str(data.get("label", "")))

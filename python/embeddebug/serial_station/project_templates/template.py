"""工程模板数据模型。"""
from __future__ import annotations
from dataclasses import dataclass, field
from typing import Any

@dataclass
class ProjectTemplate:
    """单个工程模板。"""
    name: str = ""
    description: str = ""
    transport_config: dict[str, Any] = field(default_factory=dict)
    protocol: str = "raw_data"
    commands: list[str] = field(default_factory=list)
    settings: dict[str, Any] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        return {"name": self.name, "description": self.description, "transport_config": dict(self.transport_config), "protocol": self.protocol, "commands": list(self.commands), "settings": dict(self.settings)}

    @classmethod
    def from_dict(cls, data: dict[str, Any] | None) -> ProjectTemplate:
        if not isinstance(data, dict):
            return cls()
        return cls(name=str(data.get("name", "")), description=str(data.get("description", "")), transport_config=dict(data.get("transport_config", {})), protocol=str(data.get("protocol", "raw_data")), commands=list(data.get("commands", [])), settings=dict(data.get("settings", {})))

    def validate(self) -> None:
        if not self.name.strip():
            raise ValueError("name 不能为空")
        if not self.protocol.strip():
            raise ValueError("protocol 不能为空")

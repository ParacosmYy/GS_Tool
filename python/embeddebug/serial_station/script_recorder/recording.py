"""脚本录制集合。"""
from __future__ import annotations
import json
from dataclasses import dataclass, field
from pathlib import Path
from embeddebug.serial_station.script_recorder.action import ScriptAction

@dataclass
class ScriptRecording:
    """一组有序动作及其元数据。"""
    actions: list[ScriptAction] = field(default_factory=list)
    name: str = ""
    created_at: str = ""
    description: str = ""

    @property
    def action_count(self) -> int:
        return len(self.actions)

    @property
    def duration_ms(self) -> int:
        return max((a.timestamp_ms for a in self.actions), default=0)

    def add(self, action: ScriptAction) -> None:
        self.actions.append(action)

    def to_json(self, path: str | Path | None = None) -> str | None:
        text = json.dumps({"name": self.name, "created_at": self.created_at, "description": self.description, "actions": [a.to_dict() for a in self.actions]}, ensure_ascii=False, indent=2)
        if path is None:
            return text
        Path(path).write_text(text, encoding="utf-8")
        return None

    @classmethod
    def from_json(cls, source: str | Path) -> ScriptRecording:
        text = str(source)
        try:
            p = Path(text)
            if p.exists():
                text = p.read_text(encoding="utf-8")
        except (OSError, ValueError):
            pass
        data = json.loads(text)
        return cls(actions=[ScriptAction.from_dict(d) for d in data.get("actions", [])], name=str(data.get("name", "")), created_at=str(data.get("created_at", "")), description=str(data.get("description", "")))

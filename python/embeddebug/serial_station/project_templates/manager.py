"""工程模板管理器。"""
from __future__ import annotations
import json
from pathlib import Path
from embeddebug.serial_station.project_templates.builtins import BuiltInTemplates
from embeddebug.serial_station.project_templates.template import ProjectTemplate

class TemplateManager:
    """基于 JSON 文件的模板管理器。"""

    def __init__(self, storage_path: str | Path | None = None) -> None:
        self._path = Path(storage_path) if storage_path else None
        self._templates: dict[str, ProjectTemplate] = {}

    def save(self, path: str | Path | None = None) -> None:
        target = Path(path) if path else self._path
        if target is None:
            raise ValueError("未指定存储路径")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps({n: t.to_dict() for n, t in self._templates.items()}, ensure_ascii=False, indent=2), encoding="utf-8")

    def load(self, path: str | Path | None = None) -> None:
        target = Path(path) if path else self._path
        if target is None or not target.exists():
            self._templates = {}
            return
        data = json.loads(target.read_text(encoding="utf-8"))
        self._templates = {n: ProjectTemplate.from_dict(d) for n, d in data.items()}

    def list_templates(self) -> list[ProjectTemplate]:
        return [self._templates[n] for n in sorted(self._templates)]

    def get(self, name: str) -> ProjectTemplate | None:
        return self._templates.get((name or "").strip())

    def add(self, template: ProjectTemplate) -> None:
        template.validate()
        self._templates[template.name] = template

    def delete(self, name: str) -> bool:
        if name in self._templates:
            del self._templates[name]
            return True
        return False

    def seed_builtins(self) -> int:
        added = 0
        for t in BuiltInTemplates.all():
            if t.name not in self._templates:
                self._templates[t.name] = t
                added += 1
        return added

"""Profile persistence service for Python Serial Station."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any


class SerialProfileService:
    """Save and load JSON serial station profiles."""

    def save(self, path: str | Path, profile: dict[str, Any]) -> None:
        output_path = Path(path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(
            json.dumps(profile, ensure_ascii=False, indent=2, sort_keys=True),
            encoding="utf-8",
        )

    def load(self, path: str | Path) -> dict[str, Any]:
        return dict(json.loads(Path(path).read_text(encoding="utf-8")))

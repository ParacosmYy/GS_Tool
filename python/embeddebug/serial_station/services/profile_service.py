"""Profile persistence service for Python Serial Station."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from embeddebug.shared import OperationResult


class SerialProfileService:
    """Save and load JSON serial station profiles."""

    def save_result(self, path: str | Path, profile: dict[str, Any]) -> OperationResult[Path]:
        try:
            self.save(path, profile)
        except OSError as exc:
            return OperationResult.failure("profile_save_failed", str(exc))
        return OperationResult.success(Path(path))

    def save(self, path: str | Path, profile: dict[str, Any]) -> None:
        output_path = Path(path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(
            json.dumps(profile, ensure_ascii=False, indent=2, sort_keys=True),
            encoding="utf-8",
        )

    def load_result(self, path: str | Path) -> OperationResult[dict[str, Any]]:
        try:
            return OperationResult.success(self.load(path))
        except (OSError, json.JSONDecodeError, TypeError) as exc:
            return OperationResult.failure("profile_load_failed", str(exc))

    def load(self, path: str | Path) -> dict[str, Any]:
        return dict(json.loads(Path(path).read_text(encoding="utf-8")))

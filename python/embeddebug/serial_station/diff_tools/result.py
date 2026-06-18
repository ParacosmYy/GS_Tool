"""对比结果。"""
from __future__ import annotations
from dataclasses import dataclass, field
from typing import Any

@dataclass
class DiffResult:
    """数据对比结果。"""
    summary: dict[str, Any] = field(default_factory=dict)
    row_diffs: list[dict[str, Any]] = field(default_factory=list)
    is_identical: bool = False

    def format_text(self) -> str:
        s = self.summary
        return "\n".join([f"[DiffResult] identical={self.is_identical}", f"  matching={s.get('matching_cells',0)} differing={s.get('differing_cells',0)} max_diff={s.get('max_diff',0.0)}"])

    def to_dict(self) -> dict[str, Any]:
        return {"summary": dict(self.summary), "row_diffs": [dict(d) for d in self.row_diffs], "is_identical": self.is_identical}

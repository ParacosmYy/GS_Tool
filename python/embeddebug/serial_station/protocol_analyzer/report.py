"""协议分析报告数据结构。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class AnalysisReport:
    """一次协议分析的只读汇总。"""
    stats: dict[str, Any] = field(default_factory=dict)
    errors: list[str] = field(default_factory=list)
    duration_s: float = 0.0

    @property
    def has_errors(self) -> bool:
        return bool(self.errors)

    def format_text(self) -> str:
        lines = ["[AnalysisReport]", f"  duration: {self.duration_s:.3f}s"]
        for key in ("frame_count", "frame_count_tx", "frame_count_rx", "total_bytes", "avg_frame_size", "frequency_hz", "protocol_errors"):
            val = self.stats.get(key, 0)
            lines.append(f"  {key}: {val:.3f}" if isinstance(val, float) else f"  {key}: {val}")
        lines.append(f"  errors: {len(self.errors)}")
        for e in self.errors:
            lines.append(f"    - {e}")
        return "\n".join(lines)

    def to_dict(self) -> dict[str, Any]:
        return {"stats": dict(self.stats), "errors": list(self.errors), "duration_s": self.duration_s, "has_errors": self.has_errors}

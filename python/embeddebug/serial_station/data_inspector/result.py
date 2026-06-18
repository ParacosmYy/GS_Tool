"""检查结果数据结构。"""
from __future__ import annotations
import time
from dataclasses import dataclass, field
from typing import Any

@dataclass
class InspectionResult:
    """一次数据检查的完整结果。"""
    channel_stats: dict[str, dict[str, Any]] = field(default_factory=dict)
    correlations: dict[tuple[str, str], float] = field(default_factory=dict)
    outliers: list[dict[str, Any]] = field(default_factory=list)
    threshold: float = 0.0
    timestamp_ns: int = field(default_factory=time.time_ns)

    def format_text(self) -> str:
        lines = ["[InspectionResult]", f"  channels: {len(self.channel_stats)}", f"  outliers: {len(self.outliers)}"]
        for name in sorted(self.channel_stats):
            s = self.channel_stats[name]
            lines.append(f"  - {name}: min={s['min']:.3f} max={s['max']:.3f} mean={s['mean']:.3f} std={s['std']:.3f}")
        return "\n".join(lines)

    def to_dict(self) -> dict[str, Any]:
        return {"channel_stats": dict(self.channel_stats), "correlations": {f"{a}|{b}": v for (a, b), v in self.correlations.items()}, "outliers": list(self.outliers), "threshold": self.threshold, "timestamp_ns": self.timestamp_ns}

"""对比配置。"""
from __future__ import annotations
from dataclasses import dataclass, field

@dataclass
class DiffConfig:
    """数据对比配置。"""
    tolerance: float = 0.0
    ignore_columns: list[str] = field(default_factory=list)
    align_by_timestamp: bool = False
    max_display_rows: int = 100

    def validate(self) -> None:
        if self.tolerance < 0.0:
            raise ValueError("tolerance 不能为负")
        if self.max_display_rows < 1:
            raise ValueError("max_display_rows 必须 >= 1")

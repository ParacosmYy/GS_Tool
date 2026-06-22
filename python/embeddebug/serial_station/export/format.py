"""导出格式枚举与导出配置数据结构。"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from pathlib import Path


class ExportFormat(Enum):
    """支持的导出格式枚举。"""

    CSV = "csv"
    TSV = "tsv"
    JSON = "json"
    NUMPY = "numpy"

    @classmethod
    def from_extension(cls, path: str | Path) -> ExportFormat:
        suffix = Path(path).suffix.lower().lstrip(".")
        for member in cls:
            if member.value == suffix:
                return member
        raise ValueError(f"无法识别的导出扩展名: {suffix!r}")

    @property
    def delimiter(self) -> str:
        return "," if self is ExportFormat.CSV else "\t"


@dataclass(frozen=True)
class ExportConfig:
    """导出选项的不可变配置。"""

    format: ExportFormat
    time_range: tuple[float, float] | None = None
    channels: tuple[str, ...] | None = None
    include_header: bool = True
    decimal_places: int = 6

    def __post_init__(self) -> None:
        if not isinstance(self.format, ExportFormat):
            raise ValueError("format 必须是 ExportFormat 枚举")
        if self.time_range is not None:
            start, end = self.time_range
            if start > end:
                raise ValueError(f"time_range 起始 {start} 不能大于结束 {end}")
        if self.channels is not None and len(self.channels) == 0:
            raise ValueError("channels 为空时请传 None 表示导出全部")
        if self.decimal_places < 0:
            raise ValueError(f"decimal_places 不能为负: {self.decimal_places}")
        if self.channels is not None:
            object.__setattr__(self, "channels", tuple(self.channels))

    @classmethod
    def for_format(cls, fmt: ExportFormat) -> ExportConfig:
        return cls(format=fmt)

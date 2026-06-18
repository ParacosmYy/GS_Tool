"""搜索结果数据模型。"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Any


class SearchCategory(Enum):
    """可索引的搜索类别。"""
    LOG = auto()
    COMMAND = auto()
    SETTING = auto()
    HELP = auto()


@dataclass(frozen=True)
class SearchResult:
    """单条搜索命中结果。"""
    category: SearchCategory
    title: str
    subtitle: str = ""
    match_snippet: str = ""
    score: int = 0
    data: dict[str, Any] = field(default_factory=dict)

    def relevance_sort_key(self) -> tuple[int, str, int]:
        return (-self.score, self.title, self.category.value)

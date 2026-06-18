"""SearchService：在 SearchIndex 之上提供面向业务场景的便捷封装。"""

from __future__ import annotations

from typing import Any

from embeddebug.serial_station.search.index import SearchIndex
from embeddebug.serial_station.search.result import SearchCategory, SearchResult


class SearchService:
    """全局搜索服务。"""

    def __init__(self, index: SearchIndex | None = None) -> None:
        self._index = index if index is not None else SearchIndex()

    @property
    def index(self) -> SearchIndex:
        return self._index

    def index_log_entries(self, entries: list[dict[str, Any]]) -> None:
        self._index.add(SearchCategory.LOG, entries)

    def index_commands(self, commands: list[dict[str, Any]]) -> None:
        self._index.add(SearchCategory.COMMAND, commands)

    def index_settings(self, settings: list[dict[str, Any]]) -> None:
        self._index.add(SearchCategory.SETTING, settings)

    def index_help(self, helps: list[dict[str, Any]]) -> None:
        self._index.add(SearchCategory.HELP, helps)

    def clear(self, category: SearchCategory) -> None:
        self._index.clear(category)

    def clear_all(self) -> None:
        self._index.clear_all()

    def search_all(self, query: str, max_results: int = 20) -> list[SearchResult]:
        return self._index.search(query=query, max_results=max_results)

    def search_category(self, query: str, category: SearchCategory, max_results: int = 20) -> list[SearchResult]:
        return self._index.search(query=query, max_results=max_results, category=category)

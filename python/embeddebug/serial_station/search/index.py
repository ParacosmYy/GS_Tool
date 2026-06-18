"""按类别维护条目的搜索索引。"""

from __future__ import annotations

from typing import Any

from embeddebug.serial_station.search.result import SearchCategory, SearchResult

_MAX_ITEMS = 5000


def fuzzy_score(query: str, target: str) -> int:
    """子序列模糊匹配评分。"""
    if not query:
        return 0
    ql = query.lower()
    tl = target.lower()
    if ql in tl:
        return 100 - tl.index(ql) + (10 if tl.startswith(ql) else 0)
    score = 0
    consecutive = 0
    ti = 0
    for qc in ql:
        found = False
        while ti < len(tl):
            if tl[ti] == qc:
                consecutive += 1
                score += consecutive + (5 if ti == 0 else 0)
                ti += 1
                found = True
                break
            ti += 1
            consecutive = 0
        if not found:
            return -1
    return score


class SearchIndex:
    """按类别维护的条目索引。"""

    def __init__(self) -> None:
        self._items: dict[SearchCategory, list[dict[str, Any]]] = {cat: [] for cat in SearchCategory}

    def add(self, category: SearchCategory, items: list[dict[str, Any]]) -> None:
        bucket = self._items[category]
        for item in items:
            entry = dict(item)
            entry.setdefault("title", "")
            entry.setdefault("subtitle", "")
            bucket.append(entry)
        overflow = len(bucket) - _MAX_ITEMS
        if overflow > 0:
            del bucket[:overflow]

    def clear(self, category: SearchCategory) -> None:
        self._items[category] = []

    def clear_all(self) -> None:
        for cat in self._items:
            self._items[cat] = []

    def category_count(self, category: SearchCategory) -> int:
        return len(self._items[category])

    def search(self, query: str, max_results: int = 20, category: SearchCategory | None = None) -> list[SearchResult]:
        if max_results <= 0:
            return []
        results: list[SearchResult] = []
        cats = [category] if category is not None else list(SearchCategory)
        for cat in cats:
            for entry in self._items[cat]:
                title = str(entry.get("title", ""))
                subtitle = str(entry.get("subtitle", ""))
                haystack = f"{title} {subtitle}".strip()
                score = fuzzy_score(query, haystack)
                if score < 0:
                    continue
                results.append(SearchResult(category=cat, title=title, subtitle=subtitle, match_snippet=haystack[:48], score=score, data=dict(entry)))
        results.sort(key=lambda r: r.relevance_sort_key())
        return results[:max_results]

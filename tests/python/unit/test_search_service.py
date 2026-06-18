"""全局搜索服务单测。"""

from __future__ import annotations

from embeddebug.serial_station.search import SearchCategory, SearchIndex, SearchResult, SearchService
from embeddebug.serial_station.search.index import fuzzy_score


def test_category_has_four_kinds():
    assert {c.name for c in SearchCategory} == {"LOG", "COMMAND", "SETTING", "HELP"}


def test_relevance_sort_key():
    high = SearchResult(SearchCategory.LOG, "abc", score=50)
    low = SearchResult(SearchCategory.LOG, "abc", score=10)
    items = sorted([low, high], key=lambda r: r.relevance_sort_key())
    assert items[0].score == 50


def test_fuzzy_exact_ranks_higher():
    assert fuzzy_score("conn", "Connect") > fuzzy_score("cnn", "Connect")


def test_fuzzy_no_match():
    assert fuzzy_score("xyz", "Connect") == -1


def test_index_add_and_search():
    idx = SearchIndex()
    idx.add(SearchCategory.COMMAND, [{"title": "Connect Device"}, {"title": "Clear Log"}])
    results = idx.search("connect")
    assert len(results) == 1
    assert results[0].title == "Connect Device"


def test_search_empty_returns_all():
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "alpha"}, {"title": "beta"}])
    results = idx.search("")
    assert len(results) == 2


def test_max_results():
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": f"line {i} connect"} for i in range(10)])
    assert len(idx.search("connect", max_results=3)) == 3


def test_clear_category():
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "a"}])
    idx.clear(SearchCategory.LOG)
    assert idx.search("a") == []


def test_service_search_all():
    svc = SearchService()
    svc.index_commands([{"title": "Reboot"}])
    svc.index_log_entries([{"title": "reboot complete"}])
    results = svc.search_all("reboot")
    titles = [r.title for r in results]
    assert "Reboot" in titles
    assert "reboot complete" in titles


def test_service_scoped_search():
    svc = SearchService()
    svc.index_commands([{"title": "cmd"}])
    svc.index_settings([{"title": "set"}])
    assert len(svc.search_category("cmd", SearchCategory.COMMAND)) == 1
    assert len(svc.search_category("cmd", SearchCategory.SETTING)) == 0

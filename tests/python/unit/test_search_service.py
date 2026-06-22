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


# ---- Batch 128: fuzzy_score 边界扩展（子序列 / 连续 / startswith / 大小写） ----


def test_fuzzy_empty_query_returns_zero():
    """空 query 不参与匹配，返回 0（让 search 把所有条目都纳入）。"""
    assert fuzzy_score("", "anything") == 0


def test_fuzzy_exact_substring_returns_high_score():
    """完整子串命中：100 - index。"""
    # "conn" 在 "Connect" 是子串（lower 比较），index=0
    assert fuzzy_score("conn", "Connect") == 100 - 0 + 10  # startswith bonus


def test_fuzzy_substring_offset_reduces_score():
    """子串越靠后分越低：100 - index。"""
    # "device" 在 "Connect Device" 中从 index=8 开始（含空格）
    early = fuzzy_score("device", "Connect Device")
    assert early == 100 - 8  # 无 startswith bonus


def test_fuzzy_startswith_bonus_added_to_substring_score():
    """target 以 query 开头时 +10 bonus。"""
    assert fuzzy_score("conn", "Connect") == 100 + 10
    # "Reconnect" 中 conn 从 index=2 开始，无 startswith
    assert fuzzy_score("conn", "Reconnect") == 100 - 2


def test_fuzzy_case_insensitive_match():
    """query 与 target 大小写不同时仍匹配，且用 lower() 比较。"""
    assert fuzzy_score("CONN", "connect device") > 0
    assert fuzzy_score("conn", "CONNECT DEVICE") > 0
    assert fuzzy_score("CoNn", "cOnNeCt") == 100 + 10  # startswith 仍生效


def test_fuzzy_subsequence_non_contiguous_match():
    """非连续子序列 'cnn' 匹配 'Connect'（走子序列路径，实测=9）。"""
    assert fuzzy_score("cnn", "Connect") == 9


def test_fuzzy_consecutive_characters_score_higher():
    """连续命中累积 consecutive，分数高于散落匹配。"""
    consecutive_score = fuzzy_score("abc", "xxabcxx")
    scattered_score = fuzzy_score("abc", "aXbXcX")
    assert consecutive_score > scattered_score > 0


def test_fuzzy_startswith_position_bonus_only_for_first_char():
    """单字符走子串分支（含 startswith +10）；多字符走子序列路径时首字符 +5。"""
    assert fuzzy_score("a", "abc") == 110  # 子串 + startswith
    assert fuzzy_score("a", "xya") == 98   # 子串 index=2
    # 'bc' 子序列在 'xxbXc'：b at 2 (+1) + c at 4 (+1) = 2（无 startswith bonus）
    assert fuzzy_score("bc", "xxbXc") == 2


def test_fuzzy_query_longer_than_target_returns_negative_one():
    """query 字符比 target 多时无法匹配，返回 -1。"""
    assert fuzzy_score("abcdefgh", "abc") == -1


def test_fuzzy_missing_character_in_target_returns_negative_one():
    """target 中缺少 query 的某个字符时返回 -1。"""
    assert fuzzy_score("xyz", "Connect") == -1
    assert fuzzy_score("abz", "abc") == -1


def test_fuzzy_single_character_match():
    """单字符 query 走子串路径：100 - index。"""
    # 'z' 在 "pizza" index=2 → 100-2 = 98
    assert fuzzy_score("z", "pizza") == 98


def test_fuzzy_single_character_at_start():
    """单字符 query 在 target[0] → 子串 + startswith bonus = 110。"""
    assert fuzzy_score("p", "pizza") == 110


def test_fuzzy_repeated_query_characters():
    """query 含重复字符：子串优先于子序列。"""
    # 'nn' 在 "Connect" 不是子串 → 走子序列路径
    score = fuzzy_score("nn", "Connect")
    assert score > 0  # 子序列匹配
    # 'nn' 在 "connecting" 是子串 (index=2) → 100-2 = 98
    assert fuzzy_score("nn", "connecting") == 98


def test_fuzzy_substring_takes_precedence_over_subsequence():
    """当 query 是 target 的子串时，走子串分支（更高分），不走子序列分支。"""
    # 'on' in "Connect" at index 1 → 100 - 1 = 99（子串）
    # 若走子序列路径会更低
    assert fuzzy_score("on", "Connect") == 99


# ---- SearchIndex 容量与截断 ----


def test_search_index_caps_at_max_items_per_category():
    """单个 category 超过 _MAX_ITEMS=5000 时截断旧条目。"""
    idx = SearchIndex()
    # 添加 5001 个条目
    items = [{"title": f"item{i}"} for i in range(5001)]
    idx.add(SearchCategory.LOG, items)
    # 截断后只剩 5000
    assert idx.category_count(SearchCategory.LOG) == 5000


def test_search_index_truncation_keeps_latest_entries():
    """截断保留最新条目，丢弃最早的；用零填充标题避免子串干扰。"""
    idx = SearchIndex()
    items = [{"title": f"item{i:04d}"} for i in range(5001)]  # item0000..item5000
    idx.add(SearchCategory.LOG, items)
    # item0000 应已被截断
    titles = {r.title for r in idx.search("item0000", max_results=10)}
    assert "item0000" not in titles
    # item5000 应保留
    assert any(r.title == "item5000" for r in idx.search("item5000", max_results=5))


def test_search_index_max_results_zero_returns_empty():
    """max_results=0 直接返回空列表。"""
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "alpha"}])
    assert idx.search("alpha", max_results=0) == []


def test_search_index_negative_max_results_returns_empty():
    """max_results<0 也返回空（防御 max_results <= 0 分支）。"""
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "alpha"}])
    assert idx.search("alpha", max_results=-5) == []


def test_search_index_clear_all_resets_every_category():
    """clear_all 清空所有类别。"""
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "log1"}])
    idx.add(SearchCategory.COMMAND, [{"title": "cmd1"}])
    idx.clear_all()
    assert idx.category_count(SearchCategory.LOG) == 0
    assert idx.category_count(SearchCategory.COMMAND) == 0


def test_search_index_results_sorted_by_relevance_desc():
    """search 结果按 relevance_sort_key 降序（score 高在前）。"""
    idx = SearchIndex()
    idx.add(
        SearchCategory.LOG,
        [
            {"title": "Connect"},          # 'conn' startswith → 110
            {"title": "Reconnect Device"},  # 'conn' substring at idx=2 → 98
            {"title": "Disconnect"},        # 'conn' substring at idx=4 → 96
        ],
    )
    results = idx.search("conn")
    assert [r.title for r in results] == ["Connect", "Reconnect Device", "Disconnect"]


def test_search_index_match_snippet_truncated_to_48_chars():
    """match_snippet 是 title+subtitle 拼接后前 48 字符。"""
    idx = SearchIndex()
    long_title = "x" * 60
    idx.add(SearchCategory.LOG, [{"title": long_title}])
    results = idx.search("x")
    assert len(results) == 1
    assert len(results[0].match_snippet) == 48


def test_search_index_add_preserves_extra_data_fields():
    """add 后的 entry 保留原始 data 字段，复制到 SearchResult.data。"""
    idx = SearchIndex()
    idx.add(SearchCategory.LOG, [{"title": "evt", "id": 42, "meta": "info"}])
    results = idx.search("evt")
    assert len(results) == 1
    assert results[0].data["id"] == 42
    assert results[0].data["meta"] == "info"

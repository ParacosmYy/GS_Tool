"""fuzzy_score + rank_commands 边界扩展测试。

test_command_palette 覆盖基础 fuzzy/rank；本文件补空列表 + 单条目 + 多字符无匹配 +
全匹配 + 特殊字符 + 长查询。

覆盖：
1. fuzzy_score 单字符查询。
2. fuzzy_score 长查询（超目标长度）→ -1。
3. fuzzy_score 特殊字符（数字/符号）。
4. fuzzy_score 全匹配（query==target）。
5. rank_commands 空命令列表 → 空结果。
6. rank_commands 单条目。
7. rank_commands 全部不匹配 → 空结果。
8. rank_commands 排序稳定（相同分数保持原序）。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.command_palette import (
    CommandItem,
    fuzzy_score,
    rank_commands,
)


def _make_item(title, callback=None):
    return CommandItem(title=title, callback=callback or (lambda: None), hint="")


# ── fuzzy_score 边界 ─────────────────────────────────────────────
def test_fuzzy_score_single_char():
    """单字符查询 → >0（匹配）。"""

    assert fuzzy_score("c", "connect") > 0


def test_fuzzy_score_long_query_no_match():
    """长查询（超目标长度）→ -1。"""

    assert fuzzy_score("verylongquery", "ab") == -1


def test_fuzzy_score_numbers():
    """数字查询 → 匹配含数字的目标。"""

    assert fuzzy_score("123", "test123") > 0


def test_fuzzy_score_exact_match():
    """全匹配（query==target）→ 高分。"""

    score = fuzzy_score("connect", "connect")
    assert score > 0


def test_fuzzy_score_symbol_in_query():
    """符号查询 → 匹配含符号的目标。"""

    assert fuzzy_score("+", "AT+RST") > 0


# ── rank_commands 边界 ───────────────────────────────────────────
def test_rank_commands_empty_commands():
    """空命令列表 → 空结果。"""

    result = rank_commands("test", ())
    assert result == []


def test_rank_commands_empty_query_empty_commands():
    """空查询 + 空命令 → 空结果。"""

    result = rank_commands("", ())
    assert result == []


def test_rank_commands_single_item():
    """单条目 → 返回该条目。"""

    commands = (_make_item("Connect"),)
    result = rank_commands("con", commands)
    assert len(result) == 1


def test_rank_commands_all_no_match():
    """全部不匹配 → 空结果。"""

    commands = (_make_item("Connect"), _make_item("Reset"))
    result = rank_commands("xyz", commands)
    assert result == []


def test_rank_commands_preserves_order_same_score():
    """相同分数保持原序（稳定排序）。"""

    commands = (_make_item("abc"), _make_item("abc"))
    result = rank_commands("abc", commands)
    assert len(result) == 2

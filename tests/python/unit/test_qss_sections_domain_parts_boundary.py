"""qss_sections_domain_parts 私有 helper 边界测试。

私有 QSS helper 此前无直接测试（grep 0 命中）。
本文件覆盖 9 个 helper 返回字符串契约 + serialStation objectName 覆盖。

覆盖：
1. _panel_roots 返回非空 str + 含 serialStation panel objectName。
2. _field_labels 返回非空 str + 含 serialStation。
3. _status_labels 返回非空 str + 含 serialStation。
4. _text_views 返回非空 str + 含 serialStation。
5. _main_buttons 返回非空 str + 含 serialStation。
6. _secondary_buttons 返回非空 str + 含 serialStation。
7. _inputs 返回非空 str + 含 serialStation。
8. _checkboxes 返回非空 str + 含 serialStation。
9. _tables_and_trees 返回非空 str + 含 serialStation。
10. 所有 helper 返回 str 类型。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_domain_parts import (
    _checkboxes,
    _field_labels,
    _inputs,
    _main_buttons,
    _panel_roots,
    _secondary_buttons,
    _status_labels,
    _tables_and_trees,
    _text_views,
)


def _assert_nonempty_str_with_serialstation(result, helper_name):
    assert isinstance(result, str), f"{helper_name} 应返回 str"
    assert len(result) > 0, f"{helper_name} 应返回非空 str"
    assert "serialStation" in result, f"{helper_name} 应含 serialStation objectName"


# ── 各 helper 契约 ───────────────────────────────────────────────
def test_panel_roots_contract():
    _assert_nonempty_str_with_serialstation(_panel_roots(), "_panel_roots")


def test_field_labels_contract():
    _assert_nonempty_str_with_serialstation(_field_labels(), "_field_labels")


def test_status_labels_contract():
    _assert_nonempty_str_with_serialstation(_status_labels(), "_status_labels")


def test_text_views_contract():
    _assert_nonempty_str_with_serialstation(_text_views(), "_text_views")


def test_main_buttons_contract():
    _assert_nonempty_str_with_serialstation(_main_buttons(), "_main_buttons")


def test_secondary_buttons_contract():
    _assert_nonempty_str_with_serialstation(_secondary_buttons(), "_secondary_buttons")


def test_inputs_contract():
    _assert_nonempty_str_with_serialstation(_inputs(), "_inputs")


def test_checkboxes_contract():
    _assert_nonempty_str_with_serialstation(_checkboxes(), "_checkboxes")


def test_tables_and_trees_contract():
    _assert_nonempty_str_with_serialstation(_tables_and_trees(), "_tables_and_trees")


# ── _main_buttons 含状态 ─────────────────────────────────────────
def test_main_buttons_has_hover_or_pressed():
    result = _main_buttons()
    lowered = result.lower()
    assert "hover" in lowered or "pressed" in lowered or ":checked" in lowered


def test_secondary_buttons_has_hover():
    result = _secondary_buttons()
    assert "hover" in result.lower() or ":disabled" in result.lower()

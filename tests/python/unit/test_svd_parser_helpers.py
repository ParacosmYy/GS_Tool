"""SVD parser 纯 helper 单元测试（_text / _int / _access）。

补强 test_svd_parser.py 未直接断言的 svd/parser.py 私有 helper：
- _text：取子元素文本，缺失/None/空文本 → default；strip 空白。
- _int：0x 十六进制 + 十进制自动识别；非法/空 → default；None 元素 → default。
- _access：read-only/write-only/read-write 三值；未识别/缺失 → READ_WRITE 兜底。
- _ACCESS_BY_TEXT：3 键映射 Access 全集。
"""

from __future__ import annotations

import xml.etree.ElementTree as ET

from embeddebug.serial_station.svd.model import Access
from embeddebug.serial_station.svd.parser import (
    _ACCESS_BY_TEXT,
    _access,
    _int,
    _text,
)


def _make_element(xml: str) -> ET.Element:
    """从 XML 片段构造 Element（含子元素）。"""

    return ET.fromstring(xml)


# ── _text ────────────────────────────────────────────────────────────────


def test_text_returns_child_text():
    """存在子元素 → 返回其文本（strip 后）。"""

    el = _make_element("<root><name>GPIOA</name></root>")
    assert _text(el, "name") == "GPIOA"


def test_text_strips_whitespace():
    """子元素文本前后空白被 strip。"""

    el = _make_element("<root><name>  GPIOA  </name></root>")
    assert _text(el, "name") == "GPIOA"


def test_text_missing_child_returns_default():
    """子元素不存在 → default（默认空串）。"""

    el = _make_element("<root><other>x</other></root>")
    assert _text(el, "name") == ""
    assert _text(el, "name", "fallback") == "fallback"


def test_text_none_element_returns_default():
    """el 为 None → default。"""

    assert _text(None, "name") == ""
    assert _text(None, "name", "fb") == "fb"


def test_text_empty_child_text_returns_default():
    """子元素存在但文本为 None（<name/>）→ default。"""

    el = _make_element("<root><name/></root>")
    assert _text(el, "name") == ""
    assert _text(el, "name", "fb") == "fb"


def test_text_preserves_internal_spaces():
    """文本内部空格保留（仅 strip 首尾）。"""

    el = _make_element("<root><desc>hello world foo</desc></root>")
    assert _text(el, "desc") == "hello world foo"


# ── _int ─────────────────────────────────────────────────────────────────


def test_int_decimal():
    """十进制文本 → int。"""

    el = _make_element("<root><offset>42</offset></root>")
    assert _int(el, "offset") == 42


def test_int_hex_with_0x_prefix():
    """0x 前缀十六进制 → int（base=0 自动识别）。"""

    el = _make_element("<root><offset>0xFF</offset></root>")
    assert _int(el, "offset") == 255


def test_int_negative_decimal():
    """负数十进制 → int。"""

    el = _make_element("<root><offset>-5</offset></root>")
    assert _int(el, "offset") == -5


def test_int_missing_returns_default():
    """子元素不存在 → default（默认 0）。"""

    el = _make_element("<root><other>x</other></root>")
    assert _int(el, "offset") == 0
    assert _int(el, "offset", 99) == 99


def test_int_none_element_returns_default():
    """el 为 None → default。"""

    assert _int(None, "offset") == 0
    assert _int(None, "offset", 7) == 7


def test_int_invalid_value_returns_default():
    """非法数字文本 → default（不抛）。"""

    el = _make_element("<root><offset>not_a_number</offset></root>")
    assert _int(el, "offset") == 0
    assert _int(el, "offset", 42) == 42


def test_int_empty_text_returns_default():
    """空文本 → default。"""

    el = _make_element("<root><offset></offset></root>")
    assert _int(el, "offset") == 0


# ── _access ──────────────────────────────────────────────────────────────


def test_access_read_only():
    el = _make_element("<field><access>read-only</access></field>")
    assert _access(el) == Access.READ_ONLY


def test_access_write_only():
    el = _make_element("<field><access>write-only</access></field>")
    assert _access(el) == Access.WRITE_ONLY


def test_access_read_write():
    el = _make_element("<field><access>read-write</access></field>")
    assert _access(el) == Access.READ_WRITE


def test_access_missing_defaults_to_read_write():
    """缺 <access> → READ_WRITE 兜底。"""

    el = _make_element("<field><name>x</name></field>")
    assert _access(el) == Access.READ_WRITE


def test_access_none_element_defaults_to_read_write():
    """el 为 None → READ_WRITE。"""

    assert _access(None) == Access.READ_WRITE


def test_access_unknown_text_defaults_to_read_write():
    """未识别 access 文本 → READ_WRITE（SVD 规范未列出的值兜底）。"""

    el = _make_element("<field><access>read-writeOnce</access></field>")
    assert _access(el) == Access.READ_WRITE


# ── _ACCESS_BY_TEXT 常量映射完备性 ──────────────────────────────────────


def test_access_by_text_covers_three_values():
    """_ACCESS_BY_TEXT 3 键映射到 Access 全集（read-only/write-only/read-write）。"""

    assert set(_ACCESS_BY_TEXT.keys()) == {"read-only", "write-only", "read-write"}
    assert set(_ACCESS_BY_TEXT.values()) == {
        Access.READ_ONLY,
        Access.WRITE_ONLY,
        Access.READ_WRITE,
    }


def test_access_by_text_values_distinct():
    """3 个 Access 枚举值互不相同。"""

    assert len(set(_ACCESS_BY_TEXT.values())) == 3

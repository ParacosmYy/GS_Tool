"""IconManager _read_svg + reset 边界测试。

_read_svg 此前无直接测试。icon() 在 offscreen 下渲染慢，
本文件只覆盖 _read_svg 纯文本 + reset 清空（不调 icon 渲染）。

覆盖：
1. _read_svg 已知图标返回非空 str。
2. _read_svg 未知图标返回 None。
3. reset 清空缓存（_cache == {}）。
4. _read_svg 返回内容含 svg 标签。
5. _read_svg 多个图标均可读。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.icons import IconManager


def test_read_svg_known_icon():
    manager = IconManager()
    svg = manager._read_svg("send")
    assert svg is not None
    assert "<svg" in svg.lower() or "<?xml" in svg.lower()


def test_read_svg_unknown_icon():
    manager = IconManager()
    svg = manager._read_svg("nonexistent_icon_xyz")
    assert svg is None


def test_reset_clears_cache():
    manager = IconManager()
    manager.reset()
    assert manager._cache == {}


def test_read_svg_contains_svg_tag():
    manager = IconManager()
    svg = manager._read_svg("cable")
    assert svg is not None
    assert "svg" in svg.lower()


def test_read_svg_multiple_icons():
    """多个图标均可读取。"""

    manager = IconManager()
    for name in ("send", "cable", "settings"):
        svg = manager._read_svg(name)
        assert svg is not None, f"_read_svg('{name}') 应返回非空"


def test_cache_is_dict():
    """_cache 是 dict 类型。"""

    manager = IconManager()
    manager.reset()
    assert isinstance(manager._cache, dict)


def test_reset_multiple_no_crash():
    """reset 多次不崩。"""

    manager = IconManager()
    manager.reset()
    manager.reset()
    manager.reset()

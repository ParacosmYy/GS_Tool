"""Batch 24 测试：panel_animations.stagger 死代码清理守护。

Batch 21 用 stagger_fade（纯淡入，不 move 控件）接入工作台卡片后，旧 stagger
（card_enter 含 slide_in 变体，会 move 控件与布局冲突）成为零生产消费者的死代码。
Batch 24 删除 stagger，本测试固化「模块只保留 stagger_fade」事实，防止回归。

覆盖：
1. stagger（card_enter 变体）已从 panel_animations 移除。
2. stagger_fade（纯淡入变体，在用）保留。
3. 其他在用 API（card_enter/fade_in/fade_out/slide_in）保留。
4. 模块无 card_enter 变体 stagger 的 import 残留。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def test_stagger_removed():
    """stagger（card_enter 含 slide_in 变体）应已从 panel_animations 移除。"""

    from embeddebug.serial_station.ui import panel_animations

    assert not hasattr(panel_animations, "stagger"), (
        "stagger (card_enter variant) is legacy dead code replaced by stagger_fade; "
        "remove it"
    )


def test_stagger_fade_present():
    """stagger_fade（纯淡入变体，在用）应保留。"""

    from embeddebug.serial_station.ui import panel_animations

    assert hasattr(panel_animations, "stagger_fade")
    assert callable(panel_animations.stagger_fade)


def test_in_use_apis_preserved():
    """其他在用 API（card_enter/fade_in/fade_out/slide_in）应保留。"""

    from embeddebug.serial_station.ui import panel_animations

    for name in ("card_enter", "fade_in", "fade_out", "slide_in"):
        assert hasattr(panel_animations, name), f"{name} should be preserved"


def test_module_docstring_documents_stagger_fade_not_stagger():
    """模块 docstring 应文档化 stagger_fade（非已删除的 stagger）。"""

    from embeddebug.serial_station.ui import panel_animations

    doc = inspect.getdoc(panel_animations)
    assert doc is not None
    assert "stagger_fade" in doc
    # docstring 不应再列 stagger 作为 API（历史说明里提及 OK，但 API 列表不应有）。
    # 检查「- ``stagger``：」形式的 API 条目不存在。
    assert "- ``stagger``：" not in doc
    assert "- ``stagger``:" not in doc


def test_main_window_uses_stagger_fade_not_stagger():
    """main_window 应使用 stagger_fade（非已删除的 stagger）。"""

    from embeddebug.serial_station.ui import main_window

    src = inspect.getsource(main_window)
    assert "stagger_fade" in src
    # 不应有 panel_animations.stagger( 调用。
    import re

    calls = re.findall(r"\bstagger\s*\(", src)
    assert calls == [], "main_window should not call stagger (removed)"


def test_stagger_fade_still_works(qtbot):
    """删除 stagger 后 stagger_fade 仍正常工作（回归守护）。"""

    from PyQt6.QtWidgets import QFrame
    from embeddebug.serial_station.ui.panel_animations import stagger_fade

    cards = [QFrame() for _ in range(3)]
    for c in cards:
        qtbot.addWidget(c)
    anims = stagger_fade(cards, delay_ms=50)
    assert len(anims) == 3

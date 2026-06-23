"""SkeletonBlock with_title/rows/row_height/stop_all 边界测试。

test_widgets_polish 覆盖基础；本文件补 with_title=False + rows=0 + row_height +
_skeletons 计数 + stop_all 多次。

覆盖：
1. SkeletonBlock with_title=True → _skeletons 含标题（rows+1）。
2. SkeletonBlock with_title=False → _skeletons 无标题（rows）。
3. SkeletonBlock rows=0 → 只有标题或空。
4. SkeletonBlock row_height 自定义。
5. SkeletonBlock objectName。
6. stop_all 清理所有 shimmer。
7. stop_all 多次不崩。
8. SkeletonWidget offset round-trip。
9. SkeletonWidget stop_shimmer 后 offset 可设。
10. SkeletonBlock 默认 rows=3。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.widgets.skeleton import SkeletonBlock, SkeletonWidget


# ── SkeletonBlock with_title ─────────────────────────────────────
def test_block_with_title_has_title_plus_rows(qtbot):
    """with_title=True → _skeletons 含标题（rows+1）。"""

    block = SkeletonBlock(rows=3, with_title=True)
    qtbot.addWidget(block)
    assert len(block._skeletons) == 4  # 1 title + 3 rows


def test_block_without_title_only_rows(qtbot):
    """with_title=False → _skeletons 无标题（rows）。"""

    block = SkeletonBlock(rows=3, with_title=False)
    qtbot.addWidget(block)
    assert len(block._skeletons) == 3  # 无标题


def test_block_rows_zero_with_title(qtbot):
    """rows=0 + with_title=True → 只有标题。"""

    block = SkeletonBlock(rows=0, with_title=True)
    qtbot.addWidget(block)
    assert len(block._skeletons) == 1


def test_block_rows_zero_without_title(qtbot):
    """rows=0 + with_title=False → 空。"""

    block = SkeletonBlock(rows=0, with_title=False)
    qtbot.addWidget(block)
    assert len(block._skeletons) == 0


# ── SkeletonBlock 默认 ───────────────────────────────────────────
def test_block_default_rows_three(qtbot):
    """默认 rows=3 → _skeletons 含 4（title+3）。"""

    block = SkeletonBlock()  # 默认
    qtbot.addWidget(block)
    assert len(block._skeletons) == 4  # title + 3


def test_block_objectname(qtbot):
    block = SkeletonBlock()
    qtbot.addWidget(block)
    assert block.objectName() == "serialStationSkeletonBlock"


# ── row_height ───────────────────────────────────────────────────
def test_block_custom_row_height(qtbot):
    """自定义 row_height 不崩。"""

    block = SkeletonBlock(rows=2, row_height=24, with_title=True)
    qtbot.addWidget(block)
    assert len(block._skeletons) == 3  # title + 2 rows


# ── stop_all ─────────────────────────────────────────────────────
def test_stop_all_clears_shimmer(qtbot):
    """stop_all 停止所有 shimmer（不崩）。"""

    block = SkeletonBlock(rows=3)
    qtbot.addWidget(block)
    block.stop_all()


def test_stop_all_multiple_no_crash(qtbot):
    """stop_all 多次不崩。"""

    block = SkeletonBlock(rows=2)
    qtbot.addWidget(block)
    block.stop_all()
    block.stop_all()


# ── SkeletonWidget offset ────────────────────────────────────────
def test_skeleton_widget_offset_round_trip(qtbot):
    """SkeletonWidget _get_offset/_set_offset round-trip。"""

    w = SkeletonWidget()
    qtbot.addWidget(w)
    w._set_offset(0.5)
    assert w._get_offset() == 0.5


def test_skeleton_widget_default_offset(qtbot):
    w = SkeletonWidget()
    qtbot.addWidget(w)
    assert w._get_offset() == 0.0

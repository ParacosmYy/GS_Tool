"""ThemeTransition _track/_discard/is_running + 常量边界测试。

test_theme_switching 覆盖 run/transition_theme；本文件补 _track/_discard/is_running
内部 GC 跟踪 + 常量契约 + _discard 安全（移除不存在元素不抛）。

覆盖：
1. OPACITY_DIP 常量（0.6）。
2. DIP_MS 常量（120）。
3. RISE_MS 常量（180）。
4. is_running 初始 False（无活跃过渡）。
5. _track 添加到 _active。
6. _discard 从 _active 移除。
7. _discard 移除不存在元素不抛（ValueError 吞）。
8. _track + _discard 往返后 _active 空。
9. is_running 反映 _active 非空。
10. 多次 _track 累积。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from PyQt6.QtCore import QSequentialAnimationGroup

from embeddebug.serial_station.ui.theme.theme_transition import (
    DIP_MS,
    OPACITY_DIP,
    RISE_MS,
    ThemeTransition,
)


# ── 常量 ──────────────────────────────────────────────────────────
def test_opacity_dip_constant():
    assert OPACITY_DIP == 0.6


def test_dip_ms_constant():
    assert DIP_MS == 120


def test_rise_ms_constant():
    assert RISE_MS == 180


def test_rise_ms_greater_than_dip_ms():
    """回亮比暗下去慢（视觉收束感）。"""

    assert RISE_MS > DIP_MS


# ── is_running 初始 ───────────────────────────────────────────────
def test_is_running_initial_false():
    """_active 清空后 is_running() False。"""

    ThemeTransition._active.clear()
    assert ThemeTransition.is_running() is False


# ── _track + _discard ─────────────────────────────────────────────
def test_track_adds_to_active():
    ThemeTransition._active.clear()
    group = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(group)
    assert group in ThemeTransition._active


def test_discard_removes_from_active():
    ThemeTransition._active.clear()
    group = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(group)
    ThemeTransition._discard(group)
    assert group not in ThemeTransition._active


def test_discard_nonexistent_no_crash():
    """_discard 移除不存在元素不抛（ValueError 吞）。"""

    ThemeTransition._active.clear()
    foreign = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._discard(foreign)  # 不在 _active，不抛


def test_track_discard_round_trip_empty():
    """_track + _discard 往返后 _active 空。"""

    ThemeTransition._active.clear()
    group = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(group)
    ThemeTransition._discard(group)
    assert len(ThemeTransition._active) == 0


# ── is_running 反映 _active ───────────────────────────────────────
def test_is_running_true_when_active_nonempty():
    ThemeTransition._active.clear()
    group = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(group)
    assert ThemeTransition.is_running() is True


def test_is_running_false_after_discard():
    ThemeTransition._active.clear()
    group = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(group)
    ThemeTransition._discard(group)
    assert ThemeTransition.is_running() is False


# ── 多次 _track 累积 ──────────────────────────────────────────────
def test_track_multiple_accumulates():
    ThemeTransition._active.clear()
    g1 = MagicMock(spec=QSequentialAnimationGroup)
    g2 = MagicMock(spec=QSequentialAnimationGroup)
    g3 = MagicMock(spec=QSequentialAnimationGroup)
    ThemeTransition._track(g1)
    ThemeTransition._track(g2)
    ThemeTransition._track(g3)
    assert len(ThemeTransition._active) == 3
    # 清理
    ThemeTransition._active.clear()

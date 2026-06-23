"""AnimationController add/play/stop_all/active_count 边界测试。

test_animations_effects 覆盖基础 lifecycle；本文件补 add + 空列表 play +
stop_all 清理 + active_count 状态。

覆盖：
1. add 注册动画（_animations 增长）。
2. active_count 初始 0。
3. play_sequential 空列表不崩。
4. play_parallel 空列表不崩。
5. play_sequential 返回 QSequentialAnimationGroup。
6. play_parallel 返回 QParallelAnimationGroup。
7. stop_all 清空 _animations + _groups。
8. stop_all 后 active_count==0。
9. 多次 add 累积。
10. stop_all 幂等（多次不崩）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from PyQt6.QtCore import QParallelAnimationGroup, QPropertyAnimation, QSequentialAnimationGroup
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.controller import AnimationController


def _mock_anim(running=False):
    """构造 mock 动画（state() 返回 Running/Stopped）。"""

    anim = MagicMock()
    from PyQt6.QtCore import QAbstractAnimation

    anim.State = QAbstractAnimation.State
    anim.state.return_value = (
        QAbstractAnimation.State.Running if running else QAbstractAnimation.State.Stopped
    )
    return anim


def _real_anim(parent=None):
    """构造真实 QPropertyAnimation（play_sequential/parallel 需要 QAbstractAnimation）。"""

    w = QWidget()
    target = QPropertyAnimation(w, b"pos")
    target.setDuration(10)
    return target


# ── add + active_count ───────────────────────────────────────────
def test_active_count_initial_zero():
    ctrl = AnimationController()
    assert ctrl.active_count == 0


def test_add_grows_animations_list():
    ctrl = AnimationController()
    anim = _mock_anim()
    ctrl.add(anim)
    assert len(ctrl._animations) == 1


def test_add_multiple_accumulates():
    ctrl = AnimationController()
    for _ in range(5):
        ctrl.add(_mock_anim())
    assert len(ctrl._animations) == 5


def test_active_count_counts_running():
    """active_count 统计 state()==Running 的动画。"""

    ctrl = AnimationController()
    ctrl.add(_mock_anim(running=True))
    ctrl.add(_mock_anim(running=False))
    ctrl.add(_mock_anim(running=True))
    assert ctrl.active_count == 2


# ── play_sequential / play_parallel ──────────────────────────────
def test_play_sequential_empty_list_no_crash():
    ctrl = AnimationController()
    group = ctrl.play_sequential([])
    assert isinstance(group, QSequentialAnimationGroup)


def test_play_parallel_empty_list_no_crash():
    ctrl = AnimationController()
    group = ctrl.play_parallel([])
    assert isinstance(group, QParallelAnimationGroup)


def test_play_sequential_returns_group(qtbot):
    ctrl = AnimationController()
    group = ctrl.play_sequential([_real_anim(ctrl), _real_anim(ctrl)])
    assert isinstance(group, QSequentialAnimationGroup)


def test_play_parallel_returns_group(qtbot):
    ctrl = AnimationController()
    group = ctrl.play_parallel([_real_anim(ctrl), _real_anim(ctrl)])
    assert isinstance(group, QParallelAnimationGroup)


def test_play_adds_to_groups_list(qtbot):
    ctrl = AnimationController()
    ctrl.play_sequential([_real_anim(ctrl)])
    ctrl.play_parallel([_real_anim(ctrl)])
    assert len(ctrl._groups) == 2


# ── stop_all ─────────────────────────────────────────────────────
def test_stop_all_clears_animations():
    ctrl = AnimationController()
    ctrl.add(_mock_anim())
    ctrl.add(_mock_anim())
    ctrl.stop_all()
    assert ctrl._animations == []


def test_stop_all_clears_groups(qtbot):
    ctrl = AnimationController()
    ctrl.play_sequential([_real_anim(ctrl)])
    ctrl.play_parallel([_real_anim(ctrl)])
    ctrl.stop_all()
    assert ctrl._groups == []


def test_stop_all_idempotent():
    """stop_all 多次不崩。"""

    ctrl = AnimationController()
    ctrl.add(_mock_anim())
    ctrl.stop_all()
    ctrl.stop_all()  # 再次
    ctrl.stop_all()


def test_stop_all_resets_active_count():
    ctrl = AnimationController()
    ctrl.add(_mock_anim(running=True))
    ctrl.stop_all()
    assert ctrl.active_count == 0

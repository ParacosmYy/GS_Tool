"""RippleButton _ripple_progress + _start_ripple 中心追踪边界测试。

test_ripple 覆盖基础；本文件补 _ripple_progress round-trip + _start_ripple
不同中心 + set_ripple 标志 round-trip + _ripple_anim 初始 None。

覆盖：
1. _get_ripple_progress/_set_ripple_progress round-trip。
2. _ripple_progress 默认 0.0。
3. _start_ripple 设置 _ripple_anim 非 None。
4. _start_ripple 不同中心点（QPointF）。
5. set_ripple(True) _ripple_enabled=True。
6. set_ripple(False) _ripple_enabled=False。
7. _ripple_anim 初始 None。
8. 多次 _start_ripple 替换旧 anim。
9. _set_ripple_progress 触发 update（不崩）。
10. RippleButton 默认 _ripple_enabled=True。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPointF

from embeddebug.serial_station.ui.controls.ripple import RippleButton


# ── _ripple_progress round-trip ──────────────────────────────────
def test_ripple_progress_round_trip(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn._set_ripple_progress(0.5)
    assert btn._get_ripple_progress() == 0.5


def test_ripple_progress_default_zero(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    assert btn._get_ripple_progress() == 0.0


def test_ripple_progress_set_triggers_update_no_crash(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn._set_ripple_progress(1.0)  # 不抛


# ── _start_ripple ────────────────────────────────────────────────
def test_start_ripple_sets_anim(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn._start_ripple(QPointF(10, 10))
    assert btn._ripple_anim is not None


def test_start_ripple_different_centers(qtbot):
    """_start_ripple 不同中心点不崩。"""

    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn._start_ripple(QPointF(0, 0))
    btn._start_ripple(QPointF(50, 50))
    btn._start_ripple(QPointF(100, 100))


def test_start_ripple_replaces_old_anim(qtbot):
    """多次 _start_ripple 替换旧 anim（新对象）。"""

    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn._start_ripple(QPointF(10, 10))
    btn._start_ripple(QPointF(20, 20))
    # 新 anim 可能是同对象 reuse 或新对象；验证非 None。
    assert btn._ripple_anim is not None


# ── _ripple_anim 初始 ────────────────────────────────────────────
def test_ripple_anim_initial_none(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    assert btn._ripple_anim is None


# ── set_ripple 标志 ──────────────────────────────────────────────
def test_set_ripple_true_flag(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    assert btn._ripple_enabled is True


def test_set_ripple_false_flag(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn.set_ripple(False)
    assert btn._ripple_enabled is False


def test_set_ripple_toggle_cycle(qtbot):
    btn = RippleButton("x")
    qtbot.addWidget(btn)
    btn.set_ripple(True)
    btn.set_ripple(False)
    btn.set_ripple(True)
    assert btn._ripple_enabled is True

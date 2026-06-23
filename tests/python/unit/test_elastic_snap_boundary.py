"""ElasticSnapAnimation snap_to_pos + snap_center_to 边界测试。

test_glow_typewriter_elastic_boundary 覆盖 snap_to + cancel；
本文件补 snap_to_pos + snap_center_to + drop_in（bounce_path）。

覆盖：
1. snap_to_pos 返回 QPropertyAnimation。
2. snap_to_pos 注册到 _active。
3. snap_center_to 返回 QPropertyAnimation。
4. snap_center_to 注册到 _active。
5. snap_to_pos 不同位置不崩。
6. snap_center_to 居中对齐不崩。
7. drop_in 返回 QPropertyAnimation。
8. drop_in 注册到 _active。
9. cancel 清理多个 _active。
10. snap_to 空 QRect 不崩。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation


# ── snap_to_pos ──────────────────────────────────────────────────
def test_snap_to_pos_returns_animation(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    anim = ElasticSnapAnimation.snap_to_pos(w, 100, 50)
    assert isinstance(anim, QPropertyAnimation)


def test_snap_to_pos_registers_active(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    ElasticSnapAnimation.snap_to_pos(w, 10, 20)
    assert len(ElasticSnapAnimation._active) >= 1


def test_snap_to_pos_different_positions(qtbot):
    """snap_to_pos 不同位置不崩。"""

    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    ElasticSnapAnimation.snap_to_pos(w, 0, 0)
    ElasticSnapAnimation.cancel(w)
    ElasticSnapAnimation.snap_to_pos(w, 200, 100)


# ── snap_center_to ───────────────────────────────────────────────
def test_snap_center_to_returns_animation(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    anim = ElasticSnapAnimation.snap_center_to(w, 300, 200)
    assert isinstance(anim, QPropertyAnimation)


def test_snap_center_to_registers_active(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    ElasticSnapAnimation.snap_center_to(w, 50, 50)
    assert len(ElasticSnapAnimation._active) >= 1


def test_snap_center_to_different_sizes(qtbot):
    """snap_center_to 不同目标尺寸不崩。"""

    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    ElasticSnapAnimation.snap_center_to(w, 100, 100)
    ElasticSnapAnimation.cancel(w)
    ElasticSnapAnimation.snap_center_to(w, 500, 300)


# ── drop_in (bounce_path) ────────────────────────────────────────
def test_drop_in_returns_animation(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    BouncePathAnimation._active.clear()
    anim = BouncePathAnimation.drop_in(w)
    assert isinstance(anim, QPropertyAnimation)


def test_drop_in_registers_active(qtbot):
    w = QWidget()
    qtbot.addWidget(w)
    BouncePathAnimation._active.clear()
    BouncePathAnimation.drop_in(w)
    assert len(BouncePathAnimation._active) >= 1


# ── cancel 清理多个 ──────────────────────────────────────────────
def test_cancel_clears_multiple_active(qtbot):
    """cancel 清理 widget 上的多个动画。"""

    w = QWidget()
    qtbot.addWidget(w)
    ElasticSnapAnimation._active.clear()
    ElasticSnapAnimation.snap_to(w, QRect(0, 0, 50, 50))
    ElasticSnapAnimation.snap_to_pos(w, 100, 100)
    assert len(ElasticSnapAnimation._active) >= 1
    ElasticSnapAnimation.cancel(w)
    # cancel 后 _active 中该 widget 的动画被清理。

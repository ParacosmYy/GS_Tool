"""动画工厂类测试：BouncePath / ElasticSnap / Typewriter。

合并自 test_bounce_path.py / test_elastic_snap.py / test_typewriter.py。
覆盖构造、关键帧、缓动、时长、GC 防护（_track 范式）。
"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPoint, QPropertyAnimation, QRect, QVariantAnimation
from PyQt6.QtWidgets import QLabel, QWidget

from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
from embeddebug.serial_station.ui.animations.elastic_snap import ElasticSnapAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.animations.typewriter import TypewriterAnimation


@pytest.fixture(autouse=True)
def _clean_active():
    for c in (BouncePathAnimation, ElasticSnapAnimation, TypewriterAnimation):
        c._active.clear()


def _widget(qtbot, x=0, y=0, w=100, h=40):
    wid = QWidget(); wid.setGeometry(x, y, w, h); qtbot.addWidget(wid)
    return wid


def test_drop_in_smoke(qtbot):
    anim = BouncePathAnimation.drop_in(_widget(qtbot))
    assert isinstance(anim, QPropertyAnimation)


def test_drop_in_duration_easing(qtbot):
    anim = BouncePathAnimation.drop_in(_widget(qtbot))
    assert anim.duration() == AnimationTokens.DURATION_SLOW
    assert anim.easingCurve().type() == QEasingCurve.Type.OutBounce


def test_drop_in_registered_for_gc(qtbot):
    anim = BouncePathAnimation.drop_in(_widget(qtbot))
    assert anim in BouncePathAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in BouncePathAnimation._active


def test_drop_in_end_geometry_restored(qtbot):
    wid = _widget(qtbot)
    orig = QRect(wid.geometry())
    anim = BouncePathAnimation.drop_in(wid)
    anim.setCurrentTime(anim.duration())
    assert wid.geometry() == orig


def test_drop_in_end_value_is_original(qtbot):
    wid = _widget(qtbot)
    orig = QRect(wid.geometry())
    assert QRect(BouncePathAnimation.drop_in(wid).endValue()) == orig


def test_drop_in_has_squash_keyframe(qtbot):
    wid = _widget(qtbot); orig = QRect(wid.geometry())
    kvs = BouncePathAnimation.drop_in(wid).keyValues()
    sq = QRect(next(v for p, v in kvs if abs(p - 0.75) < 0.01))
    assert sq.height() < orig.height() and sq.width() >= orig.width()


def test_slide_bounce_smoke(qtbot):
    anim = BouncePathAnimation.slide_bounce(_widget(qtbot), from_x=-120)
    assert isinstance(anim, QPropertyAnimation)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL


def test_slide_bounce_easing_out_back(qtbot):
    anim = BouncePathAnimation.slide_bounce(_widget(qtbot), from_x=-120)
    assert anim.easingCurve().type() == QEasingCurve.Type.OutBack


def test_slide_bounce_start_from_x(qtbot):
    anim = BouncePathAnimation.slide_bounce(_widget(qtbot), from_x=-120)
    assert QRect(anim.startValue()).x() == -120


def test_slide_bounce_end_is_original(qtbot):
    wid = _widget(qtbot)
    orig = QRect(wid.geometry())
    assert QRect(BouncePathAnimation.slide_bounce(wid, from_x=-120).endValue()) == orig


def test_slide_bounce_registered_for_gc(qtbot):
    anim = BouncePathAnimation.slide_bounce(_widget(qtbot), from_x=-120)
    assert anim in BouncePathAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in BouncePathAnimation._active


def test_snap_to_smoke(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(200, 300, 100, 80))
    assert isinstance(anim, QPropertyAnimation)


def test_snap_to_start_end_values(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    target = QRect(200, 300, 100, 80)
    anim = ElasticSnapAnimation.snap_to(widget, target)
    assert anim.startValue() == QRect(widget.geometry())
    assert anim.endValue() == target


def test_snap_to_duration_default(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(200, 300, 100, 80))
    assert anim.duration() == AnimationTokens.DURATION_SLOWER


def test_snap_to_duration_custom(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(200, 300, 100, 80), duration_ms=500)
    assert anim.duration() == 500


def test_snap_to_easing_elastic(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(200, 300, 100, 80))
    assert anim.easingCurve().type() == QEasingCurve.Type.OutElastic


def test_snap_to_noop_when_already_at_target(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(widget.geometry()))
    assert isinstance(anim, QPropertyAnimation)
    assert anim in ElasticSnapAnimation._active
    assert anim.duration() == 1


def test_snap_to_registered_for_gc(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(200, 300, 100, 80))
    assert anim in ElasticSnapAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in ElasticSnapAnimation._active


def test_snap_to_pos_convenience(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=120, h=60)
    end = ElasticSnapAnimation.snap_to_pos(widget, 250, 350).endValue()
    assert isinstance(end, QRect)
    assert end.x() == 250 and end.y() == 350
    assert end.width() == 120 and end.height() == 60


def test_snap_center_to_convenience(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=120, h=60)
    end = ElasticSnapAnimation.snap_center_to(widget, 400, 500).endValue()
    assert isinstance(end, QRect)
    assert end.center() == QPoint(400, 500)
    assert end.width() == 120 and end.height() == 60


def test_cancel_stops_animations_for_widget(qtbot):
    widget = _widget(qtbot, x=10, y=20, w=100, h=80)
    anim = ElasticSnapAnimation.snap_to(widget, QRect(500, 600, 100, 80))
    anim.start()
    assert anim.state() == QAbstractAnimation.State.Running or anim in ElasticSnapAnimation._active
    ElasticSnapAnimation.cancel(widget)
    assert anim.state() != QAbstractAnimation.State.Running and anim not in ElasticSnapAnimation._active


def test_tw_run_smoke(qtbot):
    qtbot.addWidget(QLabel())
    assert isinstance(TypewriterAnimation.run("hello"), QVariantAnimation)


def test_tw_run_end_value_is_text_length():
    assert TypewriterAnimation.run("hello").endValue() == 5


def test_tw_run_empty_text():
    anim = TypewriterAnimation.run("")
    assert anim.endValue() == 0
    assert anim.duration() >= AnimationTokens.DURATION_INSTANT


def test_tw_run_unicode_text():
    assert TypewriterAnimation.run("你好世界").endValue() == 4


def test_tw_run_uses_linear_easing():
    anim = TypewriterAnimation.run("hello")
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear


def test_tw_run_cps_scales_duration():
    dur_slow = TypewriterAnimation.run("hello world", cps=30).duration()
    dur_fast = TypewriterAnimation.run("hello world", cps=60).duration()
    assert abs(dur_fast - dur_slow / 2) <= 5


def test_tw_run_cps_zero_does_not_crash():
    assert TypewriterAnimation.run("hello", cps=0).duration() > 0


def test_tw_run_duration_lower_bounded():
    assert TypewriterAnimation.run("a", cps=1000).duration() >= AnimationTokens.DURATION_INSTANT


def test_tw_run_registered_for_gc():
    anim = TypewriterAnimation.run("hello")
    assert anim in TypewriterAnimation._active
    anim.stop()


def test_tw_run_discarded_after_finished(qtbot):
    anim = TypewriterAnimation.run("hi", cps=1000)
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    assert anim not in TypewriterAnimation._active


def test_tw_run_with_label_updates_progressively(qtbot):
    label = QLabel(); qtbot.addWidget(label)
    anim = TypewriterAnimation.run_with_label(label, "hello", cps=1000)
    anim.setCurrentTime(anim.duration() // 2)
    assert label.text() == "hello"[: len(label.text())] and 0 <= len(label.text()) <= 5
    anim.stop()


def test_tw_run_with_label_final_full_text(qtbot):
    label = QLabel(); qtbot.addWidget(label)
    anim = TypewriterAnimation.run_with_label(label, "hello", cps=1000)
    anim.setCurrentTime(anim.duration() + 10)
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000)
    assert label.text() == "hello"


def test_tw_run_with_label_returns_anim(qtbot):
    label = QLabel(); qtbot.addWidget(label)
    anim = TypewriterAnimation.run_with_label(label, "hello", cps=100)
    assert isinstance(anim, QVariantAnimation)
    assert anim.state() == QAbstractAnimation.State.Running
    anim.stop()

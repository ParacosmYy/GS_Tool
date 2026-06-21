"""打字机文字动画测试。

覆盖 ``TypewriterAnimation``：
- ``run`` 基本 smoke / endValue / 空文本 / Unicode / cps 时长缩放。
- GC 防护（``_track`` 范式）：注册到 ``_active``，``finished`` 后移除。
- ``run_with_label`` 自动接线：进度推进时 label 显示前缀，完成时显示完整文本。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation, QVariantAnimation
from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.animations.typewriter import TypewriterAnimation


# ── run：基本属性 ──────────────────────────────────────────────────
def test_run_smoke(qtbot):
    """run 应返回非 None 的 QVariantAnimation。"""

    anim = TypewriterAnimation.run("hello")
    qtbot.addWidget(_dummy_parent_for(anim))
    assert anim is not None
    assert isinstance(anim, QVariantAnimation)


def test_run_end_value_is_text_length(qtbot):
    """endValue 应等于 len(text)（按字符而非字节计数）。"""

    anim = TypewriterAnimation.run("hello")
    assert anim.endValue() == 5


def test_run_empty_text(qtbot):
    """空文本不应崩溃，endValue == 0。"""

    anim = TypewriterAnimation.run("")
    assert anim.endValue() == 0
    # 空文本时长应被下限钳到 DURATION_INSTANT（避免 0ms 动画不可见）。
    assert anim.duration() >= AnimationTokens.DURATION_INSTANT


def test_run_unicode_text(qtbot):
    """中文文本：len == 字符数（4），不是字节数（12）。

    Python 3 str 的 len 是 Unicode code point 数，本测试锁定这一契约。
    """

    anim = TypewriterAnimation.run("你好世界")
    assert anim.endValue() == 4  # 字符数，不是 UTF-8 字节数


def test_run_uses_linear_easing(qtbot):
    """打字机应匀速（LINEAR），缓入缓出会破坏逐字节奏。"""

    from PyQt6.QtCore import QEasingCurve

    anim = TypewriterAnimation.run("hello")
    assert anim.easingCurve().type() == QEasingCurve.Type.Linear


# ── cps：时长缩放 ─────────────────────────────────────────────────
def test_run_cps_scales_duration(qtbot):
    """cps 翻倍 → 时长减半（线性反比）。

    允许 ±5ms 取整误差（int 截断）。
    """

    anim_slow = TypewriterAnimation.run("hello world", cps=30)
    anim_fast = TypewriterAnimation.run("hello world", cps=60)
    dur_slow = anim_slow.duration()
    dur_fast = anim_fast.duration()
    # cps=60 时长应约为 cps=30 的一半。
    assert abs(dur_fast - dur_slow / 2) <= 5, (
        f"cps=60 duration {dur_fast}ms should be ~half of cps=30 {dur_slow}ms"
    )


def test_run_cps_zero_does_not_crash(qtbot):
    """cps=0 不应除零崩溃（内部按 1 处理）。"""

    anim = TypewriterAnimation.run("hello", cps=0)
    assert anim.duration() > 0


def test_run_duration_lower_bounded(qtbot):
    """极短文本时长应被钳到 DURATION_INSTANT 下限。"""

    anim = TypewriterAnimation.run("a", cps=1000)
    assert anim.duration() >= AnimationTokens.DURATION_INSTANT


# ── GC 防护（_track 范式） ────────────────────────────────────────
def test_run_registered_for_gc(qtbot):
    """run 返回的动画应注册到 _active 防 GC。"""

    TypewriterAnimation._active.clear()
    anim = TypewriterAnimation.run("hello")
    assert anim in TypewriterAnimation._active
    anim.stop()
    TypewriterAnimation._active.clear()


def test_run_discarded_after_finished(qtbot):
    """动画 finished 后应自动从 _active 移除。"""

    TypewriterAnimation._active.clear()
    anim = TypewriterAnimation.run("hi", cps=1000)
    assert anim in TypewriterAnimation._active
    anim.start()
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000
    )
    assert anim not in TypewriterAnimation._active


# ── run_with_label：自动接线 ──────────────────────────────────────
def test_run_with_label_updates_progressively(qtbot):
    """动画进行到一半时，label 文本应是原文本的某个前缀。"""

    text = "hello"
    label = QLabel()
    qtbot.addWidget(label)
    TypewriterAnimation._active.clear()

    anim = TypewriterAnimation.run_with_label(label, text, cps=1000)
    duration = anim.duration()
    # 推进到中点（QVariantAnimation.setCurrentTime 会 emit valueChanged）。
    anim.setCurrentTime(duration // 2)
    # 文本应是 text 的前缀（长度 <= len(text)）。
    assert label.text() == text[: len(label.text())], (
        f"label '{label.text()}' must be a prefix of '{text}'"
    )
    assert 0 <= len(label.text()) <= len(text)
    anim.stop()
    TypewriterAnimation._active.clear()


def test_run_with_label_final_full_text(qtbot):
    """动画完成后，label 应显示完整文本（finished 兜底）。"""

    text = "hello"
    label = QLabel()
    qtbot.addWidget(label)
    TypewriterAnimation._active.clear()

    anim = TypewriterAnimation.run_with_label(label, text, cps=1000)
    duration = anim.duration()
    # 推进到 >= duration，触发完成。
    anim.setCurrentTime(duration + 10)
    qtbot.waitUntil(
        lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=2000
    )
    assert label.text() == text, f"finished label should be full text, got '{label.text()}'"
    TypewriterAnimation._active.clear()


def test_run_with_label_returns_anim(qtbot):
    """run_with_label 应返回已启动的动画，便于调用方中途 stop。"""

    label = QLabel()
    qtbot.addWidget(label)
    TypewriterAnimation._active.clear()

    anim = TypewriterAnimation.run_with_label(label, "hello", cps=100)
    assert isinstance(anim, QVariantAnimation)
    # 已启动（run_with_label 内部调用了 start）。
    assert anim.state() == QAbstractAnimation.State.Running
    anim.stop()
    TypewriterAnimation._active.clear()


# ── 辅助 ──────────────────────────────────────────────────────────
def _dummy_parent_for(_anim: QVariantAnimation):
    """QVariantAnimation 无 parent 参数，返回一个 dummy widget 仅为 qtbot.addWidget 兼容。

    实际上 QVariantAnimation 不需要 parent（GC 由 _active 列表托管），
    这里只是为了测试函数有 qtbot.addWidget 可调用形态的占位。"""

    return QLabel()

"""淡入淡出过渡：页面/面板切换。

提供单次淡入/淡出与交叉过渡。全部基于 ``QGraphicsOpacityEffect`` + ``QPropertyAnimation``。

设计要点：
- 每次调用都会重新挂载一个新的 ``QGraphicsOpacityEffect`` 到目标 widget，避免与
  hover_lift 等其他 graphics effect 冲突（QWidget 同一时刻只能有一个 graphicsEffect）。
- ``cross_fade`` 修正了旧实现的竞态（旧版用 ``QTimer.singleShot`` 并行调度，新面板
  show 不在动画组内，连续触发会错位）：改为 ``QSequentialAnimationGroup`` 串行，
  新面板在淡出完成的 ``finished`` 回调里 show，时序确定。
- 所有动画时长默认引用 ``AnimationTokens``，调用方可覆盖。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, QParallelAnimationGroup, QSequentialAnimationGroup
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class FadeTransition:
    """淡入淡出动画工厂。"""

    @staticmethod
    def fade_in(widget: QWidget, duration: int = AnimationTokens.DURATION_NORMAL) -> QPropertyAnimation:
        """淡入：透明度 0 → 1。

        会重新挂载 opacity effect 并 ``show`` widget。
        """

        effect = QGraphicsOpacityEffect(widget)
        effect.setOpacity(0.0)
        widget.setGraphicsEffect(effect)
        widget.show()
        anim = QPropertyAnimation(effect, b"opacity", widget)
        anim.setDuration(duration)
        anim.setStartValue(0.0)
        anim.setEndValue(1.0)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return anim

    @staticmethod
    def fade_out(widget: QWidget, duration: int = AnimationTokens.DURATION_FAST) -> QPropertyAnimation:
        """淡出：透明度 1 → 0，完成后隐藏 widget。"""

        effect = QGraphicsOpacityEffect(widget)
        effect.setOpacity(1.0)
        widget.setGraphicsEffect(effect)
        anim = QPropertyAnimation(effect, b"opacity", widget)
        anim.setDuration(duration)
        anim.setStartValue(1.0)
        anim.setEndValue(0.0)
        anim.setEasingCurve(AnimationTokens.EASE_IN)
        anim.finished.connect(widget.hide)
        return anim

    @staticmethod
    def cross_fade(out_widget: QWidget, in_widget: QWidget,
                   duration: int = AnimationTokens.DURATION_SLOW) -> QParallelAnimationGroup:
        """并行交叉淡出淡入：旧面板淡出 + 新面板淡入同时进行（无中间空白间隙）。

        Batch 48 修复：从 QSequentialAnimationGroup（先淡出再淡入，中间有
        完全透明间隙）迁移到 QParallelAnimationGroup（同时淡出淡入，视觉平滑）。
        对标 Material Design container transform 的 cross-fade 模式。

        返回的 group 已绑定到 ``out_widget`` 防止 GC，调用方需 ``.start()``。
        """

        group = QParallelAnimationGroup(out_widget)
        fade_out = FadeTransition.fade_out(out_widget, duration)
        fade_in = FadeTransition.fade_in(in_widget, duration)
        group.addAnimation(fade_out)
        group.addAnimation(fade_in)
        return group

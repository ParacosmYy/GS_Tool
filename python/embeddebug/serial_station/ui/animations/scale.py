"""缩放动画：按钮按压回弹、面板弹入、hover 放大。

采用中心对齐的 ``geometry`` 缩放（保持中心点不变，仅改变 size）作为视觉缩放手段。

为什么不用 ``QGraphicsScale``：``QGraphicsScale`` / ``QGraphicsTransform`` 只能
作用于 ``QGraphicsScene`` 中的 ``QGraphicsItem``，``QWidget`` 没有 ``setTransformations``
接口，``QGraphicsEffect`` 体系也不支持缩放变换。对普通 ``QWidget``（按钮/卡片/标签），
中心对齐的 geometry 缩放在视觉上等效于 transform scale，且实现稳定可预测。

注意：geometry 缩放会短暂改变 widget 尺寸；调用方应确保 widget 不在会被父布局
立即挤压重排的位置（绝大多数按钮/图标按钮在 ``QHBoxLayout``/工具栏中表现良好）。

用法示例（按钮按压）::

    anim = ScaleAnimation.press(button)
    anim.start()  # 自动防 GC，无需手动持有
"""

from __future__ import annotations


from PyQt6.QtCore import QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ScaleAnimation:
    """控件缩放动画工厂。

    所有方法返回的 ``QPropertyAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: list[QPropertyAnimation] = []

    @classmethod
    def _track(cls, anim: QPropertyAnimation) -> QPropertyAnimation:
        """注册动画到活跃列表，完成时自动移除。"""

        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QPropertyAnimation) -> None:
        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def _scaled_rect(widget: QWidget, factor: float):
        """返回以原 rect 中心为锚、按 factor 缩放后的 QRect。"""

        orig = widget.geometry()
        new_w = max(1, int(orig.width() * factor))
        new_h = max(1, int(orig.height() * factor))
        # 必须 copy orig：QRect 是可变对象，直接 alias 会让下面的 setWidth/
        # setHeight 同时改写 orig，导致 moveCenter(orig.center()) 取到已缩放
        # 后的中心而非原中心（缩放会偏移）。copy 后 orig.center() 保持原值。
        scaled = QRect(orig)
        scaled.setWidth(new_w)
        scaled.setHeight(new_h)
        scaled.moveCenter(orig.center())
        return scaled

    @staticmethod
    def press(widget: QWidget) -> QPropertyAnimation:
        """按压回弹：缩小到 SCALE_PRESSED 再弹回 1.0。

        用 OutBack 缓动产生轻微过冲回弹，时长 FAST（160ms），符合
        Material/iOS 按压反馈节奏。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        orig = widget.geometry()
        pressed = ScaleAnimation._scaled_rect(widget, AnimationTokens.SCALE_PRESSED)
        anim.setStartValue(orig)
        anim.setKeyValueAt(0.5, pressed)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return ScaleAnimation._track(anim)

    @staticmethod
    def pop_in(widget: QWidget) -> QPropertyAnimation:
        """弹入：从 SCALE_POP_IN 缩放到 1.0（面板/弹窗出现效果）。

        调用前 widget 应已 show 且处于最终几何位置；动画从缩小态弹到原态。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = widget.geometry()
        small = ScaleAnimation._scaled_rect(widget, AnimationTokens.SCALE_POP_IN)
        anim.setStartValue(small)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return ScaleAnimation._track(anim)

    @staticmethod
    def bounce(widget: QWidget) -> QPropertyAnimation:
        """弹跳：短暂放大到 SCALE_BOUNCE 再回 1.0（通知出现）。"""

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = widget.geometry()
        big = ScaleAnimation._scaled_rect(widget, AnimationTokens.SCALE_BOUNCE)
        anim.setStartValue(orig)
        anim.setKeyValueAt(0.4, big)
        anim.setEndValue(orig)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BOUNCE)
        return ScaleAnimation._track(anim)

    # ── 分离式 press_down / press_up（Batch 14） ────────────────────
    # 与 ``press()``（单一合并动画，绑 clicked）的区别：这对方法分别绑
    # ``pressed``/``released`` 信号——press 时**立即**缩小（即时响应），release 时
    # OutBack 回弹（弹性释放）。观感更「物理」：手指按下即陷，松手弹回。
    # 接入任意 QPushButton 见 ``micro_interactions.install_scale_press``。

    @staticmethod
    def press_down(widget: QWidget) -> QPropertyAnimation:
        """按压陷下：1.0 → SCALE_PRESSED（0.96），EASE_OUT 即时陷下。

        绑 ``pressed`` 信号：手指按下瞬间触发，快速（INSTANT=100ms）陷下，
        给「按下即响应」的物理感。陷下后保持，直到 ``press_up`` 在 release 时弹回。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_INSTANT)
        orig = widget.geometry()
        anim.setStartValue(orig)
        anim.setEndValue(ScaleAnimation._scaled_rect(widget, AnimationTokens.SCALE_PRESSED))
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return ScaleAnimation._track(anim)

    @staticmethod
    def press_up(widget: QWidget, orig_rect: QRect | None = None) -> QPropertyAnimation:
        """释放回弹：当前 → 原态，EASE_OUT_BACK 轻微过冲回弹。

        绑 ``released`` 信号：手指松开时从陷下态弹回原态，OutBack 产生轻微过冲，
        时长 FAST（160ms）。与 ``press_down`` 配对使用（``install_scale_press`` 自动接线）。

        Args:
            widget: 目标控件。
            orig_rect: press 前的原始几何（由 ``install_scale_press`` 在 pressed 时
                捕获并传入）。``None`` 时用 ``widget.geometry()``（仅当 widget 未陷下
                时正确；陷下态下应显式传 orig_rect）。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        cur = widget.geometry()
        target = orig_rect if orig_rect is not None else QRect(cur)
        # 以陷下态中心为锚归位（press_down 居中陷下，回弹也居中回）。
        restored = QRect(target)
        restored.moveCenter(cur.center())
        anim.setStartValue(cur)
        anim.setEndValue(restored)
        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return ScaleAnimation._track(anim)

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

from typing import List

from PyQt6.QtCore import QPropertyAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ScaleAnimation:
    """控件缩放动画工厂。

    所有方法返回的 ``QPropertyAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QPropertyAnimation] = []

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
        scaled = orig
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

    @staticmethod
    def hover_in(widget: QWidget) -> QPropertyAnimation:
        """hover 放大：1.0 → 1.03 极轻微放大（与阴影抬升配合，时长 FAST）。"""

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        orig = widget.geometry()
        anim.setStartValue(orig)
        anim.setEndValue(ScaleAnimation._scaled_rect(widget, 1.03))
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return ScaleAnimation._track(anim)

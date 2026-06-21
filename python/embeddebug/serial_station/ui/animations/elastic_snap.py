"""弹性吸附动画。从当前 geometry 弹性吸附到目标 geometry，OutElastic 缓动产生
弹簧过冲效果。用于拖拽释放后磁吸到目标位置、面板自动归位、命令面板目标定位。

为什么用 OutElastic：``OutElastic`` 在终点附近产生若干次衰减振荡（先过冲再回弹），
比 ``OutBack`` 更具「弹簧感」，适合需要明显磁吸/归位反馈的场景。但 OutElastic 在
``duration`` 偏短时振荡不可见，因此默认时长采用 ``DURATION_SLOWER``（600ms）以
留出振荡空间。

与 ``scale.py`` 同样采用 ``QPropertyAnimation`` + ``b"geometry"`` + 类级 ``_active``
防 GC 范式：调用方拿到动画对象后可直接 ``.start()``，动画完成自动从 ``_active``
移除，无需手动持有引用。

用法示例（拖拽释放后磁吸到屏幕边缘）::

    target = QRect(0, 0, panel.width(), panel.height())
    anim = ElasticSnapAnimation.snap_to(panel, target)
    anim.start()

注意：``snap_to`` 在 ``target_rect == 当前 geometry`` 时仍返回一个被追踪的
「无操作」动画（duration=1ms），以保证调用方拿到的永远是非 ``None`` 对象，
逻辑分支无需特判。
"""

from __future__ import annotations

from typing import List

from PyQt6.QtCore import QPoint, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ElasticSnapAnimation:
    """弹性吸附动画工厂。

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
        """安全移除动画引用（不在列表时静默忽略）。"""

        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def snap_to(
        widget: QWidget,
        target_rect: QRect,
        duration_ms: int | None = None,
    ) -> QPropertyAnimation:
        """从当前 geometry 弹性吸附到 ``target_rect``。

        Args:
            widget: 目标控件。
            target_rect: 吸附终点几何。
            duration_ms: 自定义时长（毫秒）。``None`` 时使用
                ``AnimationTokens.DURATION_SLOWER``（600ms），以保证 OutElastic
                有足够振荡周期。

        Returns:
            已注册防 GC 的 ``QPropertyAnimation``。当 ``target_rect`` 与当前
            geometry 完全一致时，返回一个 duration=1ms 的无操作动画，调用方
            仍可拿到非 ``None`` 对象，无需特判。
        """

        current = QRect(widget.geometry())
        anim = QPropertyAnimation(widget, b"geometry", widget)

        # 防御：目标 == 当前 → 返回 tracked 无操作动画，保证调用方拿到的永远
        # 非 None，避免外层 if-None 分支。duration=1 让其瞬时结束但仍合法。
        if target_rect == current:
            anim.setDuration(1)
            anim.setStartValue(current)
            anim.setEndValue(target_rect)
            anim.setEasingCurve(AnimationTokens.EASE_OUT_ELASTIC)
            return ElasticSnapAnimation._track(anim)

        anim.setStartValue(current)
        anim.setEndValue(target_rect)
        anim.setDuration(
            duration_ms if duration_ms is not None
            else AnimationTokens.DURATION_SLOWER
        )
        anim.setEasingCurve(AnimationTokens.EASE_OUT_ELASTIC)
        return ElasticSnapAnimation._track(anim)

    @staticmethod
    def snap_to_pos(
        widget: QWidget,
        x: int,
        y: int,
        duration_ms: int | None = None,
    ) -> QPropertyAnimation:
        """便捷重载：吸附到 (x, y)，保持控件当前尺寸。"""

        target_rect = QRect(x, y, widget.width(), widget.height())
        return ElasticSnapAnimation.snap_to(widget, target_rect, duration_ms)

    @staticmethod
    def snap_center_to(
        widget: QWidget,
        center_x: int,
        center_y: int,
        duration_ms: int | None = None,
    ) -> QPropertyAnimation:
        """便捷重载：以 (center_x, center_y) 为中心吸附，保持控件当前尺寸。"""

        # copy 当前 geometry 作为目标底版：仅移动中心点，尺寸保持当前值。
        target = QRect(widget.geometry())
        target.moveCenter(QPoint(center_x, center_y))
        return ElasticSnapAnimation.snap_to(widget, target, duration_ms)

    @staticmethod
    def cancel(widget: QWidget) -> None:
        """停止所有作用于 ``widget`` 的活跃吸附动画。

        用于拖拽中途再次按下/被抢占时中断进行中的吸附。已停止的动画不会
        自动从 ``_active`` 移除（``finished`` 不会 emit），这里手动移除防泄漏。
        """

        for anim in list(ElasticSnapAnimation._active):
            if anim.targetObject() is widget:
                anim.stop()
                ElasticSnapAnimation._discard(anim)

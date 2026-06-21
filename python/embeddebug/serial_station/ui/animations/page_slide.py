"""页面切换滑入动画：控件从屏幕外某一方向滑入到最终位置。

与 ``slide.py``（``SlideAnimation``，基于 ``pos`` 的小幅滑入，distance=60px）
互补：本模块聚焦**整页/整面板入场过渡**——控件完全从屏幕外飞入归位，营造
「页面切换」的层次感，适合多页导航、面板入场、抽屉展开等场景。

为什么基于 ``geometry`` 属性而非 ``pos``：
- ``pos`` 仅改 ``QPoint``，``geometry`` 同时管 ``x, y, w, h``，与
  ``BouncePathAnimation`` 同源，便于后续叠加尺寸关键帧。
- 页面切换通常需要控件「整体平移」，``geometry`` 语义更贴合。

GC 安全范式（与 ``BouncePathAnimation`` / ``ScaleAnimation`` 一致）：
- 类级 ``_active`` 列表持有进行中的动画引用，防止 Python 侧 GC 导致动画被
  提前回收（Qt C++ 对象在 Python 无引用时会断开）。
- ``_track`` 注册动画并连接 ``finished`` → ``_discard``，动画完成自动从列表移除。
- 调用方无需手动持有引用，但调用方应在合适时机 ``anim.start()`` 启动。

缓动曲线说明：
- 使用 ``QEasingCurve.Type.OutQuart``（减速至停止，比 OutCubic 更柔和）。
- ``AnimationTokens`` 当前未导出 ``EASE_OUT_QUART`` token，且本任务不得修改
  ``tokens.py``，故此处直接引用底层枚举值。待 ``tokens.py`` 扩充后应改为
  ``AnimationTokens.EASE_OUT_QUART`` 以保持单一真相源。

方向语义（``SlideDirection`` 复用 ``slide.py`` 中的枚举）：
- LEFT：从左侧屏幕外滑入，起点 x = 原位 x − 控件自身宽度。
- RIGHT：从右侧屏幕外滑入，起点 x = 原位 x + 父控件宽度（无父则 +300）。
- UP：从上方屏幕外滑入，起点 y = 原位 y − 控件自身高度。
- DOWN：从下方屏幕外滑入，起点 y = 原位 y + 父控件高度（无父则 +300）。

用法示例::

    anim = PageSlideAnimation.slide_in(panel, SlideDirection.LEFT)
    anim.start()  # 自动防 GC

注意：动画期间会修改 ``widget.geometry()``；调用方应确保 widget 调用前已处于
最终几何位置（``setGeometry`` / 布局完成后再调用）。
"""

from __future__ import annotations

from typing import List

from PyQt6.QtCore import QEasingCurve, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.slide import SlideDirection
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

# 无父控件时使用的默认屏幕外偏移（像素）。
_DEFAULT_NO_PARENT_OFFSET = 300


class PageSlideAnimation:
    """页面切换滑入动画工厂。

    所有方法返回的 ``QPropertyAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。

    与 ``SlideAnimation`` 的区别：本类做**整页/整面板**从屏幕外飞入的过渡
    （偏移量 = 控件或父控件尺寸），后者做小幅 ``pos`` 滑入（默认 60px）。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QPropertyAnimation] = []

    @classmethod
    def _track(cls, anim: QPropertyAnimation) -> QPropertyAnimation:
        """注册动画到活跃列表，完成时自动移除。

        Args:
            anim: 待防护的 QPropertyAnimation。

        Returns:
            传入的 anim（链式调用）。
        """

        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QPropertyAnimation) -> None:
        """动画完成后从活跃列表移除（防内存泄漏）。

        Args:
            anim: 已完成的 QPropertyAnimation。
        """

        try:
            cls._active.remove(anim)
        except ValueError:
            # 动画可能已被显式清理，幂等忽略。
            pass

    @staticmethod
    def _offscreen_start(widget: QWidget, direction: SlideDirection) -> QRect:
        """返回控件原 geometry 平移到屏幕外（指定方向）后的 QRect。

        - LEFT/UP：使用控件自身宽/高作为偏移（总能保证完全移出原位）。
        - RIGHT/DOWN：使用父控件宽/高作为偏移（保证移出父可视区）；
          无父控件时退化为 ``_DEFAULT_NO_PARENT_OFFSET``。

        Args:
            widget: 目标控件。
            direction: 滑入方向。

        Returns:
            屏幕外起点 QRect（尺寸与原 geometry 一致）。
        """

        orig = QRect(widget.geometry())
        if direction == SlideDirection.LEFT:
            return orig.translated(-orig.width(), 0)
        if direction == SlideDirection.RIGHT:
            parent = widget.parentWidget()
            dx = parent.width() if parent is not None else _DEFAULT_NO_PARENT_OFFSET
            return orig.translated(dx, 0)
        if direction == SlideDirection.UP:
            return orig.translated(0, -orig.height())
        # SlideDirection.DOWN
        parent = widget.parentWidget()
        dy = parent.height() if parent is not None else _DEFAULT_NO_PARENT_OFFSET
        return orig.translated(0, dy)

    @staticmethod
    def slide_in(
        widget: QWidget, direction: SlideDirection = SlideDirection.LEFT
    ) -> QPropertyAnimation:
        """从屏幕外指定方向滑入到控件当前 geometry。

        视觉：控件起点在屏幕外（按方向偏移），终点回到原 geometry，期间以
        ``OutQuart`` 缓动减速归位，时长 ``DURATION_NORMAL=240ms``。

        Args:
            widget: 目标控件（调用前应已 ``show`` 且处于最终几何位置）。
            direction: 滑入方向，默认 LEFT。

        Returns:
            已注册到 ``_active`` 的 QPropertyAnimation（调用方负责 ``start()``）。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = QRect(widget.geometry())

        start = PageSlideAnimation._offscreen_start(widget, direction)
        anim.setStartValue(start)
        anim.setEndValue(orig)

        # OutQuart：减速至停止，比 OutCubic 更柔和，适合整页入场。
        anim.setEasingCurve(QEasingCurve.Type.OutQuart)
        return PageSlideAnimation._track(anim)

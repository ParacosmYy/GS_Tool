"""路径弹跳动画：drop_in 从上方落下并轻微挤压，slide_bounce 从侧面滑入回弹。

与 ``scale.py``（纯几何缩放）互补：本模块聚焦**位移动画**——控件沿某条路径
从一处移动到另一处，期间叠加缩放/挤压等微动作，营造「物理落地」的视觉。

为什么基于 ``geometry`` 属性而非 ``pos``：
- ``pos`` 仅改 ``QPoint``（左上角），尺寸固定。无法叠加挤压（height × 0.92）
  和缩放入场（SCALE_POP_IN=0.6）效果。
- ``geometry`` 同时管 ``x, y, w, h``，可以一次性表达「位移 + 尺寸」组合关键帧，
  对应物理直觉：物体下落时受惯性拉伸，撞地时被压扁（width × 1.04, height × 0.92），
  回弹后恢复原尺寸。

GC 安全范式（与 ``ScaleAnimation`` 一致）：
- 类级 ``_active`` 列表持有进行中的动画引用，防止 Python 侧 GC 导致动画被
  提前回收（Qt C++ 对象在 Python 无引用时会断开）。
- ``_track`` 注册动画并连接 ``finished`` → ``_discard``，动画完成自动从列表移除。
- 调用方无需手动持有引用，但调用方应在合适时机 ``anim.start()`` 启动。

用法示例::

    anim = BouncePathAnimation.drop_in(card)
    anim.start()  # 自动防 GC

注意：动画期间会短暂修改 ``widget.geometry()``；调用方应确保 widget 不在会被
父布局立即挤压重排的位置（顶级面板/弹窗/独立卡片均安全）。
"""

from __future__ import annotations

from typing import List

from PyQt6.QtCore import QPropertyAnimation, QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class BouncePathAnimation:
    """控件路径弹跳动画工厂。

    所有方法返回的 ``QPropertyAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。

    与 ``ScaleAnimation`` 的区别：本类聚焦**带位移的路径动画**（下落、滑入），
    关键帧包含位置变化；``ScaleAnimation`` 仅做中心锚定的纯缩放。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QPropertyAnimation] = []

    # drop_in 视觉参数（与 tokens 中的 SCALE_POP_IN 配合）。
    DROP_INITIAL_SCALE = AnimationTokens.SCALE_POP_IN  # 0.6
    DROP_VERTICAL_OFFSET = AnimationTokens.DROP_VERTICAL_OFFSET  # -50
    SQUASH_HEIGHT_RATIO = AnimationTokens.SQUASH_HEIGHT_RATIO  # 0.92
    SQUASH_WIDTH_RATIO = AnimationTokens.SQUASH_WIDTH_RATIO  # 1.04

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
    def _offset_rect(widget: QWidget, dx: int, dy: int) -> QRect:
        """返回 ``widget.geometry()`` 平移 (dx, dy) 后的 QRect 副本。

        ``QRect.translated`` 返回新对象，不修改原 geometry，安全。

        Args:
            widget: 目标控件。
            dx: 水平平移（像素，正值右移、负值左移）。
            dy: 垂直平移（像素，正值下移、负值上移）。

        Returns:
            平移后的 QRect（尺寸不变）。
        """

        return QRect(widget.geometry()).translated(dx, dy)

    @staticmethod
    def _scaled_rect_centered(widget: QWidget, factor: float) -> QRect:
        """返回以原 rect 中心为锚、按 factor 缩放后的 QRect（复用 ScaleAnimation 范式）。

        Args:
            widget: 目标控件。
            factor: 缩放比例（< 1.0 缩小，> 1.0 放大）。

        Returns:
            中心锚定的缩放 QRect。
        """

        # 复制 scale.py 中 _scaled_rect 的实现逻辑，避免跨模块私有依赖。
        orig = widget.geometry()
        new_w = max(1, int(orig.width() * factor))
        new_h = max(1, int(orig.height() * factor))
        # 必须 copy orig：QRect 可变，直接 alias 会让 setWidth/setHeight 改写原值。
        scaled = QRect(orig)
        scaled.setWidth(new_w)
        scaled.setHeight(new_h)
        scaled.moveCenter(orig.center())
        return scaled

    @staticmethod
    def drop_in(widget: QWidget) -> QPropertyAnimation:
        """从上方落下并轻微挤压：缩放入场 + 下落 + 落地挤压 + 回弹归位。

        视觉分四段（关键帧）：
        1. 起点（0.0）：缩放到 SCALE_POP_IN=0.6 且上移 50px（从上方小尺寸下落）。
        2. 0.55：到达原位（缩放回 1.0，位置归位，模拟「落地」瞬间）。
        3. 0.75：挤压（高度 × 0.92、宽度 × 1.04，模拟冲击面积变形）。
        4. 终点（1.0）：恢复原 geometry（弹回）。

        缓动 EASE_OUT_BOUNCE 让前半段下落带弹跳节奏，时长 DURATION_SLOW=360ms。

        Args:
            widget: 目标控件（调用前应已 show 且处于最终几何位置）。

        Returns:
            已注册到 ``_active`` 的 QPropertyAnimation（调用方负责 ``start()``）。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_SLOW)
        orig = QRect(widget.geometry())

        # 关键帧 1：缩小 + 上移（从上方下落，尺寸亦小）。
        small = BouncePathAnimation._scaled_rect_centered(
            widget, BouncePathAnimation.DROP_INITIAL_SCALE
        )
        start = QRect(small)
        start.translate(0, BouncePathAnimation.DROP_VERTICAL_OFFSET)
        anim.setStartValue(start)

        # 关键帧 2：落地——回到原位（缩放与位置均归位）。
        anim.setKeyValueAt(0.55, orig)

        # 关键帧 3：挤压——高度收缩、宽度膨胀（面积守恒直觉），中心保持。
        squash_w = max(1, int(orig.width() * BouncePathAnimation.SQUASH_WIDTH_RATIO))
        squash_h = max(1, int(orig.height() * BouncePathAnimation.SQUASH_HEIGHT_RATIO))
        squash = QRect(orig)
        squash.setWidth(squash_w)
        squash.setHeight(squash_h)
        squash.moveCenter(orig.center())
        anim.setKeyValueAt(0.75, squash)

        # 关键帧 4：恢复原 geometry。
        anim.setEndValue(orig)

        anim.setEasingCurve(AnimationTokens.EASE_OUT_BOUNCE)
        return BouncePathAnimation._track(anim)

    @staticmethod
    def slide_bounce(widget: QWidget, from_x: int) -> QPropertyAnimation:
        """从侧面滑入并轻微回弹：起点横向偏移到 ``from_x``，终点回到原 geometry。

        适用于面板/通知从左侧滑入主视图的场景。与 ``drop_in`` 的差异：
        - 不做缩放，仅水平位移。
        - 缓动 EASE_OUT_BACK 让终点带轻微过冲（横向弹一下）。
        - 时长 DURATION_NORMAL=240ms（比 drop_in 快，因无下落节奏）。

        Args:
            widget: 目标控件（调用前应已处于最终几何位置）。
            from_x: 起点左上角 x 坐标。``< 0`` 表示从屏幕左侧外滑入。

        Returns:
            已注册到 ``_active`` 的 QPropertyAnimation（调用方负责 ``start()``）。
        """

        anim = QPropertyAnimation(widget, b"geometry", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        orig = QRect(widget.geometry())

        # 起点：横向平移到 from_x，尺寸不变。
        start = QRect(orig)
        start.moveLeft(from_x)
        anim.setStartValue(start)
        anim.setEndValue(orig)

        anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
        return BouncePathAnimation._track(anim)

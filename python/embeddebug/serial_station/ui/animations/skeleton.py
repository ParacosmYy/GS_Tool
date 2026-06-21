"""骨架屏闪烁动画（SkeletonAnimation）：加载占位符的呼吸脉冲效果。

与 ``pulse.py``（状态指示呼吸）互补：本模块聚焦**加载占位**——控件以
``windowOpacity`` 在 0.3 → 1.0 → 0.3 之间循环脉冲，营造「内容正在加载」的
视觉。适用于异步数据未到达前的占位卡片、骨架行视图、占位图表等场景。

为什么基于 ``QVariantAnimation + windowOpacity`` 而非 ``QPropertyAnimation``：
- ``windowOpacity`` 是 QWidget 的 Q_PROPERTY，但其 setter 在非顶层窗口上为
  no-op；用 ``QVariantAnimation`` + ``valueChanged`` 信号驱动，使我们能完全
  控制数值流，避开 ``QPropertyAnimation`` 对目标属性元类型/绑定的检查复杂度，
  便于跨平台一致与未来切换（如改用 ``QGraphicsOpacityEffect`` 仅需改一行）。
- 关键帧 ``(0.0, 0.3), (0.5, 1.0), (1.0, 0.3)`` 形成对称脉冲（首暗-中亮-
  尾暗），配合 EASE_IN_OUT（InOutCubic）让中段过渡更平滑。

GC 安全范式（与 ``BouncePathAnimation`` / ``ScaleAnimation`` 一致）：
- 类级 ``_active`` 列表持有进行中的动画引用，防止 Python 侧 GC 导致动画被
  提前回收（Qt C++ 对象在 Python 无引用时会断开）。
- ``_track`` 注册动画并连接 ``finished`` → ``_discard``，动画完成自动从列表移除。
- 调用方无需手动持有引用，但调用方应在合适时机 ``anim.start()`` 启动。

用法示例::

    anim = SkeletonAnimation.shimmer(placeholder_card, loops=3)
    anim.start()  # 自动防 GC；3 次脉冲后自动停止

注意：``QWidget.setWindowOpacity`` 仅对顶层窗口（独立窗口/对话框）实际生效；
对内嵌 widget 仅作为数值动画演示（不抛异常但不改变视觉），如需对内嵌控件做
透明度脉冲，调用方可基于本工厂返回的动画自行改写 valueChanged 槽以驱动
``QGraphicsOpacityEffect``。
"""

from __future__ import annotations

from typing import List

from PyQt6.QtCore import QVariantAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class SkeletonAnimation:
    """加载占位骨架闪烁动画工厂。

    所有方法返回的 ``QVariantAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。

    与 ``PulseAnimation`` 的区别：本类聚焦**加载占位场景**——固定 0.3 ↔ 1.0
    不透明度脉冲，关键帧对称；``PulseAnimation`` 用于状态指示（在线/离线/数据流），
    数值范围与节奏由其自有 token 决定。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QVariantAnimation] = []

    # shimmer 视觉参数（与 tokens 中 DURATION_SLOWER 配合）。
    SHIMMER_MIN_OPACITY = 0.3   # 闪烁最低不透明度（半隐，骨架未填充态）
    SHIMMER_MAX_OPACITY = 1.0   # 闪烁最高不透明度（完全可见，就绪态峰值）
    SHIMMER_MID_POSITION = 0.5  # 峰值关键帧位置（对称脉冲中点）
    SHIMMER_DEFAULT_LOOPS = 3   # 默认脉冲循环次数（呼吸节奏）

    @classmethod
    def _track(cls, anim: QVariantAnimation) -> QVariantAnimation:
        """注册动画到活跃列表，完成时自动移除。

        Args:
            anim: 待防护的 QVariantAnimation。

        Returns:
            传入的 anim（链式调用）。
        """

        cls._active.append(anim)
        # 注意：此处使用 lambda 闭包捕获 anim，符合 MUST NOT 规则——闭包仅引用
        # anim 局部变量、调用 cls._discard 静态逻辑，不捕获 self（cls 也不会
        # 触发 QPropertyAnimation.finished + deleteLater 的悬挂访问）。
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QVariantAnimation) -> None:
        """动画完成后从活跃列表移除（防内存泄漏）。

        Args:
            anim: 已完成的 QVariantAnimation。
        """

        try:
            cls._active.remove(anim)
        except ValueError:
            # 动画可能已被显式清理，幂等忽略。
            pass

    @staticmethod
    def shimmer(widget: QWidget, loops: int = 3) -> QVariantAnimation:
        """加载占位骨架闪烁：windowOpacity 在 0.3 → 1.0 → 0.3 之间脉冲循环。

        视觉为对称脉冲：
        1. 起点（0.0）：opacity 0.3（半隐，模拟「未填充」骨架态）。
        2. 中点（0.5）：opacity 1.0（完全可见，模拟「就绪」瞬间的峰值亮度）。
        3. 终点（1.0）：opacity 0.3（回落到半隐，等待下一次脉冲）。

        缓动 ``EASE_IN_OUT``（InOutCubic）让首尾过渡平滑对称，单次时长
        ``DURATION_SLOWER=600ms``（呼吸灯节奏），默认循环 3 次后自动停止。

        Args:
            widget: 目标控件（顶层窗口效果最佳；内嵌 widget 仅作数值演示，
                ``setWindowOpacity`` 为 no-op 但不抛异常）。
            loops: 脉冲循环次数（默认 ``SHIMMER_DEFAULT_LOOPS=3``）。
                ``-1`` 表示无限循环，调用方需在合适时机 ``stop()`` 并显式释放。

        Returns:
            已注册到 ``_active`` 的 QVariantAnimation（调用方负责 ``start()``）。
        """

        anim = QVariantAnimation()
        anim.setDuration(AnimationTokens.DURATION_SLOWER)
        anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        anim.setLoopCount(loops)

        # 关键帧：对称脉冲（0.3 → 1.0 → 0.3）。
        anim.setKeyValues([
            (0.0, SkeletonAnimation.SHIMMER_MIN_OPACITY),
            (SkeletonAnimation.SHIMMER_MID_POSITION,
             SkeletonAnimation.SHIMMER_MAX_OPACITY),
            (1.0, SkeletonAnimation.SHIMMER_MIN_OPACITY),
        ])

        # 数值变化驱动 widget.setWindowOpacity。闭包仅捕获 widget 并调用其 Qt
        # 方法；QVariantAnimation 的 valueChanged 信号生命周期与 anim 一致，
        # 不存在 finished + deleteLater 的悬挂访问风险。
        anim.valueChanged.connect(lambda v: widget.setWindowOpacity(v))

        return SkeletonAnimation._track(anim)

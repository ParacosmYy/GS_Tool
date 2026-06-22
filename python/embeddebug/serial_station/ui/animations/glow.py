"""发光动画：accent 色脉冲发光效果。

提供 accent 色脉冲发光效果，基于 ``QGraphicsDropShadowEffect`` 的
``blurRadius`` 动画。用于连接成功提示、错误高亮、新通知出现。

与 ``pulse.py`` 的区别：``pulse`` 基于 ``QGraphicsOpacityEffect`` 整体透明度
呼吸（状态指示灯），``glow`` 基于 ``QGraphicsDropShadowEffect`` 的 blurRadius
在控件外侧产生 accent 色辉光（强调性提示，连接成功 / 错误 / 通知）。两者目标
效果不同，互不替代。

用法示例::

    # 连接成功：3 次脉冲发光
    GlowAnimation.pulse(connect_btn, loops=3).start()

    # 错误高亮：持续发光（调用方在错误清除后调 clear）
    GlowAnimation.steady(error_label, intensity=1.0)

    # 清除发光
    GlowAnimation.clear(widget)

所有 ``pulse`` 返回的动画都注册到类级活跃列表自动防 GC，动画完成自动移除。
"""

from __future__ import annotations


from PyQt6.QtCore import QPropertyAnimation
from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P

# 脉冲发光总时长上限（避免大 loops 时动画过长导致交互卡顿）。
_MAX_PULSE_DURATION_MS = 3000

# 基础 accent 色 alpha（脉冲起点/终点的辉光强度）。
_BASE_ALPHA = 180

# steady 默认 alpha（持续发光略强于脉冲基准）。
_STEADY_BASE_ALPHA = 200


class GlowAnimation:
    """accent 色发光动画工厂。

    所有 ``pulse`` 返回的 ``QPropertyAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。``steady`` 与 ``clear`` 直接修改 effect，无动画引用，
    调用方需自行持有 effect 引用（effect 已绑定到 widget，生命周期由 widget 管理）。
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
        """安全移除动画（已不在列表时忽略）。"""

        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def _attach_effect(widget: QWidget) -> QGraphicsDropShadowEffect:
        """为控件附加 accent 色 drop shadow effect。

        - 若控件已挂 ``QGraphicsDropShadowEffect``，复用之（避免覆盖调用方配置）。
        - 若控件挂了其他类型 effect（如 opacity），**不覆盖**——返回新创建但未挂
          载的 effect（避免吞掉调用方的 opacity effect）；此时调用方应意识到发光
          不可见，但不会破坏既有 effect 体系。
        - 若无 effect，创建新的并绑定。

        Args:
            widget: 目标控件。

        Returns:
            将用于动画的 ``QGraphicsDropShadowEffect``。
        """

        existing = widget.graphicsEffect()
        if isinstance(existing, QGraphicsDropShadowEffect):
            return existing

        effect = QGraphicsDropShadowEffect(widget)
        accent = QColor(P.ACCENT)
        accent.setAlpha(_BASE_ALPHA)
        effect.setColor(accent)
        effect.setBlurRadius(AnimationTokens.SHADOW_BLUR_NORMAL)
        effect.setOffset(0, 0)

        # 仅当控件当前没有任何 effect 时才挂载（保护其他类型 effect）。
        if existing is None:
            widget.setGraphicsEffect(effect)
        return effect

    @staticmethod
    def pulse(widget: QWidget, loops: int = 3) -> QPropertyAnimation:
        """脉冲发光：blurRadius 在 NORMAL 与 HOVER 之间往返 ``loops`` 次。

        用于连接成功提示、错误高亮、新通知出现等需要短暂吸引注意力的场景。
        动画以 accent 色辉光环绕控件，从 ``SHADOW_BLUR_NORMAL`` 升到
        ``SHADOW_BLUR_HOVER`` 再回落，循环 ``loops`` 次后停在 NORMAL。

        Args:
            widget: 目标控件。
            loops: 脉冲往返次数（默认 3）。总时长 = ``DURATION_SLOWER * loops``，
                上限 3000ms（避免长动画阻塞交互）。

        Returns:
            已注册防 GC 的 ``QPropertyAnimation``（property = ``b"blurRadius"``）。
        """

        effect = GlowAnimation._attach_effect(widget)
        anim = QPropertyAnimation(effect, b"blurRadius", widget)

        # 总时长：DURATION_SLOWER(600ms) * loops，上限 3000ms。
        duration = min(
            AnimationTokens.DURATION_SLOWER * max(1, loops),
            _MAX_PULSE_DURATION_MS,
        )
        anim.setDuration(duration)

        # 构造 loops 次振荡：每个完整循环 NORMAL→HOVER→NORMAL 占两个半周期。
        # 用 setKeyValueAt 在 [0, 1] 上均匀打点，确保首尾都在 NORMAL（不留余晖）。
        normal = AnimationTokens.SHADOW_BLUR_NORMAL
        hover = AnimationTokens.SHADOW_BLUR_HOVER
        # 总段数 = loops * 2（NORMAL→HOVER、HOVER→NORMAL 各算一段）。
        segments = max(1, loops) * 2
        for i in range(segments + 1):
            t = i / segments
            # 偶数索引（含 0 与 segments）→ NORMAL，奇数 → HOVER。
            anim.setKeyValueAt(t, normal if i % 2 == 0 else hover)

        anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        return GlowAnimation._track(anim)

    @staticmethod
    def steady(
        widget: QWidget, intensity: float = 1.0
    ) -> QGraphicsDropShadowEffect:
        """持续发光：设置固定 blurRadius 与 alpha（不动画）。

        用于需要长期保持强调态的场景（错误未消除、在线状态、活跃选中）。
        调用方在不再需要时调 :meth:`clear` 移除发光。

        Args:
            widget: 目标控件。
            intensity: 强度因子 (0.0-1.0+)。
                - blurRadius = ``SHADOW_BLUR_HOVER * intensity``
                - alpha = ``200 * intensity``

        Returns:
            已配置的 ``QGraphicsDropShadowEffect``（effect 已绑定到 widget，
            无需调用方持有；返回以便进一步自定义）。
        """

        effect = GlowAnimation._attach_effect(widget)
        # intensity 截断到 [0, 1.5] 防止极端值产生异常模糊。
        clamped = max(0.0, min(1.5, float(intensity)))
        effect.setBlurRadius(int(AnimationTokens.SHADOW_BLUR_HOVER * clamped))
        color = QColor(P.ACCENT)
        color.setAlpha(int(_STEADY_BASE_ALPHA * clamped))
        effect.setColor(color)
        return effect

    @staticmethod
    def clear(widget: QWidget) -> None:
        """清除发光：将 effect 的 blurRadius 与 alpha 归零。

        仅当控件已挂 ``QGraphicsDropShadowEffect`` 时生效；其他类型 effect
        或无 effect 时静默返回（不破坏既有 effect 体系）。

        Args:
            widget: 目标控件。
        """

        effect = widget.graphicsEffect()
        if isinstance(effect, QGraphicsDropShadowEffect):
            effect.setBlurRadius(0)
            color = QColor(effect.color())
            color.setAlpha(0)
            effect.setColor(color)

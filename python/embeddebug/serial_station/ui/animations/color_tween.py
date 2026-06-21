"""颜色属性补间动画（ColorTweenAnimation）：两个颜色之间的平滑过渡工厂。

参考 VOFA+ 串口调试工具的「连接按钮」交互范式：未连接时按钮为深蓝色
（如 ``#1e3a8a``），点击后串口握手成功，按钮颜色平滑过渡为浅蓝色
（如 ``#3b82f6``），用颜色变化暗示「连接状态切换」。同样适用于：

- 在线/离线状态指示灯的颜色过渡（红 → 绿）。
- 输入框聚焦/失焦边框色过渡（灰 → 蓝）。
- 数据流方向指示（接收 → 发送）的色调过渡。

为什么基于 ``QVariantAnimation + valueChanged`` 而非 ``QPropertyAnimation``：

- 颜色属性（``color`` / ``background-color`` / ``border-color``）在 Qt 中由
  QSS 样式系统管理，并非控件的 Q_PROPERTY；``QPropertyAnimation`` 无法直接驱动。
- ``QVariantAnimation`` 提供纯数值动画能力，由 ``valueChanged`` 信号驱动调用方
  自定义的 setter（``setProperty`` 或 ``setStyleSheet`` 替换），灵活性最高。
- 颜色以 ``(r, g, b, a)`` 元组作为关键帧值；``QVariantAnimation`` 内建对
  ``QVariantList`` 的线性插值能力使颜色在两关键帧之间平滑过渡，无需继承
  自定义可插值类型。

GC 安全范式（与 ``BouncePathAnimation`` / ``SkeletonAnimation`` 一致）：

- 类级 ``_active`` 列表持有进行中的动画引用，防止 Python 侧 GC 导致动画被
  提前回收（Qt C++ 对象在 Python 无引用时会断开）。
- ``_track`` 注册动画并连接 ``finished`` → ``_discard``，动画完成自动从列表移除。
- 调用方无需手动持有引用，但调用方应在合适时机 ``anim.start()`` 启动。

用法示例::

    anim = ColorTweenAnimation.tween(button, "color", "#1e3a8a", "#3b82f6")
    anim.start()  # 自动防 GC；按钮颜色在 240ms 内从深蓝过渡到浅蓝

注意：``tween`` 默认通过 ``target.setProperty`` 设置动态属性。若希望驱动
实际 QSS 视觉变化，请使用 ``tween_stylesheet``（基于 QSS 字符串替换），
或调用方自行连接 ``anim.valueChanged`` 槽以驱动自定义 setter。
"""

from __future__ import annotations

from typing import List, Tuple, Union

from PyQt6.QtCore import QEasingCurve, QObject, QVariantAnimation
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class ColorTweenAnimation:
    """颜色属性补间动画工厂。

    所有方法返回的 ``QVariantAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成自动从列表移除。调用方无需手动持有引用。

    与 ``SkeletonAnimation`` 的区别：本类聚焦**颜色过渡**——关键帧值为
    ``(r, g, b, a)`` 元组，由调用方决定如何应用到目标（动态属性 / QSS /
    自定义 setter）；``SkeletonAnimation`` 仅驱动 ``windowOpacity`` 单浮点。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QVariantAnimation] = []

    @classmethod
    def _track(cls, anim: QVariantAnimation) -> QVariantAnimation:
        """注册动画到活跃列表，完成时自动移除。

        Args:
            anim: 待防护的 QVariantAnimation。

        Returns:
            传入的 anim（链式调用）。
        """

        cls._active.append(anim)
        # 闭包仅引用 anim 与 cls._discard 静态逻辑，符合 MUST NOT 规则
        # （不捕获会触发悬挂访问的 self 或 deleteLater 链）。
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
    def _parse_color(s: str) -> Tuple[int, int, int, int]:
        """解析颜色字符串为 ``(r, g, b, a)`` 元组（a 为 0-255 整数）。

        支持格式：

        - ``#RRGGBB``（6 位十六进制，alpha 默认 255）。
        - ``#RRGGBBAA``（8 位十六进制，alpha 在末尾两位）。
        - ``rgba(r, g, b, a)``（a 为 0-1 浮点或 0-255 整数；启发式：<= 1.0
          视为 0-1 分数，> 1.0 视为 0-255 整数）。

        Args:
            s: 颜色字符串。

        Returns:
            ``(r, g, b, a)`` 四元组，每个分量为 0-255 整数。

        Raises:
            ValueError: 颜色格式不支持或字段数不正确。
        """

        text = s.strip()
        if text.startswith("#"):
            hex_body = text[1:]
            if len(hex_body) == 6:
                r = int(hex_body[0:2], 16)
                g = int(hex_body[2:4], 16)
                b = int(hex_body[4:6], 16)
                return (r, g, b, 255)
            if len(hex_body) == 8:
                r = int(hex_body[0:2], 16)
                g = int(hex_body[2:4], 16)
                b = int(hex_body[4:6], 16)
                a = int(hex_body[6:8], 16)
                return (r, g, b, a)
            raise ValueError(f"Unsupported hex color length: {s!r}")
        if text.lower().startswith("rgba"):
            inner = text[text.index("(") + 1:text.rindex(")")]
            parts = [p.strip() for p in inner.split(",")]
            if len(parts) != 4:
                raise ValueError(f"Expected 4 components in rgba(): {s!r}")
            r = int(parts[0])
            g = int(parts[1])
            b = int(parts[2])
            a_raw = float(parts[3])
            # 启发式：<= 1.0 视为 0-1 浮点 alpha，否则视为 0-255 整数。
            a = round(a_raw * 255) if a_raw <= 1.0 else int(a_raw)
            return (r, g, b, max(0, min(255, a)))
        raise ValueError(f"Unsupported color format: {s!r}")

    @staticmethod
    def _format_color(t: Tuple[int, int, int, int]) -> str:
        """将 ``(r, g, b, a)`` 元组格式化为 QSS 兼容的 ``rgba(...)`` 字符串。

        Args:
            t: ``(r, g, b, a)`` 四元组，每个分量为 0-255 整数。

        Returns:
            形如 ``"rgba(34, 211, 238, 255)"`` 的字符串，可直接嵌入 QSS。
        """

        r, g, b, a = t
        return f"rgba({r}, {g}, {b}, {a})"

    @staticmethod
    def tween(
        target: QObject,
        property_name: Union[str, bytes],
        start: str,
        end: str,
        duration: int = AnimationTokens.DURATION_NORMAL,
        easing: QEasingCurve.Type = AnimationTokens.EASE_IN_OUT,
    ) -> QVariantAnimation:
        """对 ``target`` 的 ``property_name`` 属性做颜色补间动画。

        颜色以 ``(r, g, b, a)`` 元组作为关键帧值，``QVariantAnimation`` 在两关键帧
        之间线性插值（叠加 ``easing`` 曲线）。``valueChanged`` 信号驱动槽函数将
        当前元组格式化为 ``rgba(...)`` 字符串后，先尝试调用
        ``target.set<Property>`` setter，若不存在则回退到 ``target.setProperty``。

        Args:
            target: 目标 QObject（通常为 QWidget）。
            property_name: 属性名（str 或 bytes）。用作 setter 名拼接
                （``"color"`` → ``"setColor"``）与 setProperty 键。
            start: 起点颜色字符串（``#RRGGBB`` / ``#RRGGBBAA`` / ``rgba(...)``）。
            end: 终点颜色字符串。
            duration: 时长（毫秒），默认 ``DURATION_NORMAL=240``。
            easing: 缓动曲线类型，默认 ``EASE_IN_OUT``（InOutCubic）。

        Returns:
            已注册到 ``_active`` 的 QVariantAnimation（调用方负责 ``start()``）。
        """

        start_tuple = ColorTweenAnimation._parse_color(start)
        end_tuple = ColorTweenAnimation._parse_color(end)

        anim = QVariantAnimation()
        anim.setDuration(duration)
        anim.setEasingCurve(easing)
        anim.setKeyValues([(0.0, start_tuple), (1.0, end_tuple)])

        # property_name 可能是 bytes；预计算字符串形式供 setter 名拼接。
        if isinstance(property_name, bytes):
            prop_str = property_name.decode("utf-8")
        else:
            prop_str = property_name
        setter_name = "set" + prop_str.capitalize()

        def apply(value) -> None:
            # QVariant 可能将 tuple 序列化为 QVariantList（list 形式取回）。
            v = tuple(value) if not isinstance(value, tuple) else value
            color_str = ColorTweenAnimation._format_color(v)
            setter = getattr(target, setter_name, None)
            if callable(setter):
                setter(color_str)
            else:
                target.setProperty(property_name, color_str)

        anim.valueChanged.connect(apply)
        return ColorTweenAnimation._track(anim)

    @staticmethod
    def tween_stylesheet(
        target: QWidget,
        start: str,
        end: str,
        duration: int = AnimationTokens.DURATION_NORMAL,
    ) -> QVariantAnimation:
        """对 ``target.styleSheet()`` 中的 ``start`` 颜色字面量做平滑替换动画。

        实现策略：调用时快照原始 QSS 字符串，每次 ``valueChanged`` 触发时，
        将当前插值颜色格式化为 ``rgba(...)`` 字符串，再对**原始 QSS** 中所有
        匹配 ``start`` 字面量的子串做 ``str.replace``，结果通过
        ``setStyleSheet`` 应用。这样：

        - 仅 ``start`` 颜色被替换，其他 QSS 规则保持不变。
        - 多次插值不会累积漂移（始终从原 QSS 替换，而非上一次结果）。
        - 若原 QSS 不含 ``start`` 子串，行为为 no-op（不抛异常）。

        若需要更精细的控制（如多处颜色同步过渡、或 QSS 中颜色格式不统一），
        推荐使用 ``tween`` 自行连接 ``anim.valueChanged`` 槽以驱动自定义逻辑。

        Args:
            target: 目标 QWidget（应有非空 styleSheet，否则动画无视觉变化）。
            start: 起点颜色字符串（须与 QSS 中的字面量完全匹配，包括空格）。
            end: 终点颜色字符串。
            duration: 时长（毫秒），默认 ``DURATION_NORMAL=240``。

        Returns:
            已注册到 ``_active`` 的 QVariantAnimation（调用方负责 ``start()``）。
        """

        original_qss = target.styleSheet() or ""
        start_tuple = ColorTweenAnimation._parse_color(start)
        end_tuple = ColorTweenAnimation._parse_color(end)

        anim = QVariantAnimation()
        anim.setDuration(duration)
        anim.setEasingCurve(AnimationTokens.EASE_IN_OUT)
        anim.setKeyValues([(0.0, start_tuple), (1.0, end_tuple)])

        def apply(value) -> None:
            v = tuple(value) if not isinstance(value, tuple) else value
            color_str = ColorTweenAnimation._format_color(v)
            target.setStyleSheet(original_qss.replace(start, color_str))

        anim.valueChanged.connect(apply)
        return ColorTweenAnimation._track(anim)

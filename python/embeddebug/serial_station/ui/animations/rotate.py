"""提供 widget 旋转动画。通过 QVariantAnimation 驱动 angle 0→360°，调用方接 valueChanged 在 paintEvent 中应用旋转。用于刷新按钮、加载 spinner、连接中指示。

为什么不用 ``QPropertyAnimation``：``QWidget`` 没有原生 ``rotation`` 属性，
``QGraphicsRotation`` 只能作用于 ``QGraphicsItem``。对普通 ``QWidget``，调用方需
自行 override ``paintEvent``，在 ``QPainter`` 中调用 ``painter.rotate(angle)``，
本模块只负责产生连续角度值并通过 ``valueChanged`` 抛出。

典型用法::

    anim = RotateAnimation.spin(duration_ms=800, loops=1)
    anim.valueChanged.connect(self._on_rotate)  # self 重写 paintEvent
    anim.start()  # 自动防 GC，无需手动持有

连续旋转（加载 spinner / 连接中指示）::

    self._spin = RotateAnimation.spin_continuous()
    self._spin.valueChanged.connect(self.update)
    self._spin.start()
    # 不再需要时：
    RotateAnimation.release(self._spin)
"""

from __future__ import annotations

from typing import List

from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QVariantAnimation, pyqtSignal

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class RotateAnimation:
    """控件旋转动画工厂。

    所有方法返回的 ``QVariantAnimation`` 都已注册到类级活跃列表自动防 GC。
    对有限次旋转（``spin``），动画 ``finished`` 自动从列表移除；
    对连续旋转（``spin_continuous``，``loopCount=-1`` 不会 emit finished），
    调用方必须显式调用 :meth:`release` 停止并移除，避免泄漏。

    注意：``QVariantAnimation`` 的 ``valueChanged`` 信号类型为 ``pyqtSignal(object)``
    （实际值为 float），调用方槽签名应接收一个 float 参数。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（有限动画完成自动从列表移除）。
    _active: List[QVariantAnimation] = []

    @classmethod
    def _track(cls, anim: QVariantAnimation) -> QVariantAnimation:
        """注册动画到活跃列表，完成时自动移除（仅对有限动画生效）。"""

        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QVariantAnimation) -> None:
        """安全从活跃列表移除（不存在时静默忽略）。"""

        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def spin(
        duration_ms: int = 800,
        loops: int = 1,
        clockwise: bool = True,
    ) -> QVariantAnimation:
        """有限次旋转：``angle`` 从 0 平滑到 ``360 * loops``（或负向），LINEAR 匀速。

        将 ``loops`` 烘焙进 ``endValue`` 和 ``duration``，保持单一动画对象
        （避免 ``setLoopCount>1`` 引起的插值重置跳变）。``loopCount`` 固定为 1。

        Args:
            duration_ms: 单圈时长（毫秒）。总时长 = ``duration_ms * loops``。
            loops: 旋转圈数。默认 1。
            clockwise: ``True`` 顺时针（angle 递增到 +360*loops）；
                ``False`` 逆时针（angle 递减到 -360*loops）。

        Returns:
            已注册防 GC 的 ``QVariantAnimation``，调用方接 ``valueChanged`` 应用旋转。
        """

        anim = QVariantAnimation()
        sign = 1.0 if clockwise else -1.0
        anim.setStartValue(0.0)
        anim.setEndValue(sign * 360.0 * loops)
        anim.setDuration(duration_ms * loops)
        anim.setEasingCurve(AnimationTokens.LINEAR)
        anim.setLoopCount(1)
        return RotateAnimation._track(anim)

    @staticmethod
    def spin_continuous(
        clockwise: bool = True,
        interval_ms: int = 16,
        degrees_per_sec: float = 360.0,
    ) -> QVariantAnimation:
        """连续无限旋转：``loopCount=-1``，用于加载 spinner / 连接中指示等长期动画。

        实现：以 60 秒为一个循环周期（``duration=60000ms``），``endValue`` 设为
        ``degrees_per_sec * 60``（默认 60 秒 × 360°/s = 21600°），LINEAR 匀速，
        ``loopCount=-1`` 无限循环。``interval_ms`` 参数保留以备将来按帧率调优，
        当前实现未直接依赖（Qt 动画由事件循环驱动，刷新率跟随系统）。

        重要：无限动画不会 emit ``finished``，因此 ``_track`` 注册的自动清理钩子
        不会触发。调用方必须在控件销毁 / 不再需要时显式调用 :meth:`release`，
        否则动画对象会一直留在 :attr:`_active` 列表中造成泄漏。

        Args:
            clockwise: ``True`` 顺时针，``False`` 逆时针。
            interval_ms: 预期帧间隔（毫秒），保留供调优（默认 16 ≈ 60fps）。
            degrees_per_sec: 角速度（度/秒），默认 360°/s（每秒一圈）。

        Returns:
            已注册防 GC 的 ``QVariantAnimation``（``loopCount=-1``）。
        """

        anim = QVariantAnimation()
        sign = 1.0 if clockwise else -1.0
        # 60 秒一个循环周期；endValue 对应周期内累积角度。
        cycle_ms = 60000
        anim.setStartValue(0.0)
        anim.setEndValue(sign * degrees_per_sec * 60.0)
        anim.setDuration(cycle_ms)
        anim.setEasingCurve(AnimationTokens.LINEAR)
        anim.setLoopCount(-1)
        # interval_ms 当前未直接影响 Qt 动画驱动，预留接口。
        _ = interval_ms
        return RotateAnimation._track(anim)

    @classmethod
    def release(cls, anim: QVariantAnimation) -> None:
        """停止并移除一个动画（主要用于 ``spin_continuous`` 的无限动画清理）。

        对有限动画也可调用：若仍在运行则先 ``stop``，再从 :attr:`_active` 移除。
        幂等：重复调用安全（``stop`` 对已停止动画无副作用，``_discard`` 容忍缺失）。

        Args:
            anim: 待释放的动画（通常由 :meth:`spin` / :meth:`spin_continuous` 返回）。
        """

        if anim.state() != QAbstractAnimation.State.Stopped:
            anim.stop()
        cls._discard(anim)

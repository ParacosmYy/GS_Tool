"""打字机文字动画。逐字符 reveal 文本，用于状态消息出现、命令行输出模拟、欢迎语。

通过 QVariantAnimation 驱动 progress（int 0 → len(text)），调用方在
``valueChanged`` 回调中对原文本切片显示，实现稳定的字符级 reveal。

设计要点：
- 使用 ``QVariantAnimation``（int 值）而非 ``QPropertyAnimation``，因为目标
  ``QLabel`` 没有 ``text`` 这种数字属性可绑；让调用方在 ``valueChanged`` 中
  自行 ``setText(text[:value])`` 更灵活，也便于多个 label 共用同一进度。
- 时长按 ``cps``（characters per second）计算，保证不同长度文本节奏一致。
- 缓动使用 ``LINEAR``：打字机应匀速，不应有缓入缓出（那会让人觉得卡顿）。
- 动画注册到类级 ``_active`` 列表防 GC（对齐 ``ScaleAnimation`` 范式），
  ``finished`` 自动移除。

用法示例::

    # 方式 A：手动接 valueChanged（自定义渲染）
    anim = TypewriterAnimation.run("hello world", cps=40)
    anim.valueChanged.connect(lambda v: label.setText(text[:v]))
    anim.start()

    # 方式 B：便利方法（自动接线 + 完成兜底）
    TypewriterAnimation.run_with_label(label, "hello world", cps=40)
"""

from __future__ import annotations

from typing import TYPE_CHECKING, List

from PyQt6.QtCore import QVariantAnimation, pyqtSignal

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

if TYPE_CHECKING:
    from PyQt6.QtWidgets import QLabel


class TypewriterAnimation:
    """打字机文字动画工厂。

    所有方法返回的 ``QVariantAnimation`` 都已注册到类级活跃列表自动防 GC，
    动画完成（或被显式 stop 后 emit ``finished``）自动从列表移除。
    调用方无需手动持有引用，但若需要中途停止，应保留返回值以便 ``stop()``。
    """

    # 持有进行中的动画引用，防止 Python 侧 GC（动画完成自动从列表移除）。
    _active: List[QVariantAnimation] = []

    @classmethod
    def _track(cls, anim: QVariantAnimation) -> QVariantAnimation:
        """注册动画到活跃列表，完成时自动移除。"""

        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QVariantAnimation) -> None:
        """从活跃列表安全移除（不存在时静默，避免重复 finished 触发报错）。"""

        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def run(text: str, cps: int = 30, start_delay_ms: int = 0) -> QVariantAnimation:
        """创建一个逐字符 reveal 的 ``QVariantAnimation``（未启动）。

        Args:
            text: 目标文本。``len(text)`` 作为 ``endValue``（按字符计数，
                Python 3 ``str`` 的 ``len`` 是字符数而非字节数，中英文/emoji 通用）。
            cps: characters per second，打字速度。默认 30 cps 接近人类快速阅读节奏。
            start_delay_ms: 起始延迟（毫秒）。**当前为 advisory only**：本工厂不
                实际应用延迟（QVariantAnimation 的 ``setStartValue`` 是值而非时间），
                需要延迟时调用方应自行 ``QTimer.singleShot(delay, anim.start)``。
                传入负值会被归零。保留参数位以稳定接口契约。

        Returns:
            未启动的 ``QVariantAnimation``。调用方负责 ``start()`` 或用
            ``run_with_label`` 便利方法自动接线。
        """

        anim = QVariantAnimation()
        anim.setStartValue(0)
        # Python str len 即字符数（Unicode code point 数），中英文/emoji 一致。
        anim.setEndValue(len(text))
        # 时长按 cps 反推：duration_ms = chars / cps * 1000。下限 INSTANT（100ms）
        # 防止空/极短文本时动画时长为 0 导致不可见或 valueChanged 不触发。
        # cps 至少为 1，避免除零（cps=0 在语义上不合理，按 1 处理）。
        effective_cps = max(1, cps)
        duration_ms = max(
            AnimationTokens.DURATION_INSTANT,
            int(len(text) / effective_cps * 1000),
        )
        anim.setDuration(duration_ms)
        # 打字机匀速：缓入缓出会让首尾字符停留过久，破坏"逐字打"的节奏感。
        anim.setEasingCurve(AnimationTokens.LINEAR)
        # start_delay_ms 当前 advisory only（见 docstring），不实际应用，
        # 显式归零以避免调用方误以为已生效。负值也归零。
        _ = max(0, start_delay_ms)
        return TypewriterAnimation._track(anim)

    @staticmethod
    def run_with_label(
        label: "QLabel", text: str, cps: int = 30
    ) -> QVariantAnimation:
        """便利方法：创建打字机动画并自动接线到 ``QLabel.setText``。

        连接 ``valueChanged`` 到 ``label.setText(text[:value])``，动画完成时
        兜底设置完整文本（防止动画被提前 stop 时 label 卡在中间片段）。

        Args:
            label: 目标 ``QLabel``。
            text: 要逐字显示的完整文本。
            cps: characters per second，打字速度。

        Returns:
            已启动的 ``QVariantAnimation``（调用方可保留引用以便中途 ``stop()``）。
        """

        anim = TypewriterAnimation.run(text, cps=cps)

        def _on_value(value: int) -> None:
            # value 是 QVariantAnimation 当前 int 进度（0 → len(text)）。
            label.setText(text[:value])

        def _on_finished() -> None:
            # 兜底：动画完成（含提前 stop 触发的 finished）时确保完整文本。
            label.setText(text)

        anim.valueChanged.connect(_on_value)
        anim.finished.connect(_on_finished)
        anim.start()
        return anim

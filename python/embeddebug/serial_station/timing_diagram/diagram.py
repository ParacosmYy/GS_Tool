"""时序图聚合与导出。

``TimingDiagram`` 维护一组命名信号，提供添加、查询、批量测量以及文本
表格导出能力。该类是 timing_diagram 模块的对外入口。
"""

from __future__ import annotations

from typing import Any, Sequence

from embeddebug.serial_station.timing_diagram.measurement import TimingMeasurement
from embeddebug.serial_station.timing_diagram.signal import TimingSignal

__all__ = ["TimingDiagram"]

_NAME_WIDTH = 16
_TIME_WIDTH = 12


class TimingDiagram:
    """多信号时序图。

    维护 ``名称 -> TimingSignal`` 的有序映射，并提供组装、查询与测量
    导出能力。

    Attributes:
        default_setup_window_ns: 建立时间测量的默认窗口 (ns)。
    """

    def __init__(self, *, default_setup_window_ns: float = 10.0) -> None:
        self._signals: dict[str, TimingSignal] = {}
        self.default_setup_window_ns: float = default_setup_window_ns

    # ------------------------------------------------------------------
    # 信号管理
    # ------------------------------------------------------------------
    def add_signal(
        self,
        name: str,
        transitions: Sequence[tuple[float, bool]],
        *,
        initial_level: bool = False,
    ) -> TimingSignal:
        """添加一个信号到时序图。

        Args:
            name: 信号名称；重名将覆盖既有信号。
            transitions: ``(time_ns, new_level)`` 列表。
            initial_level: 初始电平。

        Returns:
            新建的 ``TimingSignal``。
        """

        signal = TimingSignal(
            name=name,
            transitions=list(transitions),
            initial_level=initial_level,
        )
        self._signals[name] = signal
        return signal

    def add_signal_obj(self, signal: TimingSignal) -> TimingSignal:
        """直接添加已构造的 ``TimingSignal``。"""

        self._signals[signal.name] = signal
        return signal

    def get_signal(self, name: str) -> TimingSignal | None:
        """按名称获取信号；不存在返回 ``None``。"""

        return self._signals.get(name)

    def list_signals(self) -> list[TimingSignal]:
        """按添加顺序返回所有信号。"""

        return list(self._signals.values())

    def signal_names(self) -> list[str]:
        """按添加顺序返回所有信号名称。"""

        return list(self._signals.keys())

    def __len__(self) -> int:
        return len(self._signals)

    def __contains__(self, name: object) -> bool:
        return name in self._signals

    # ------------------------------------------------------------------
    # 测量
    # ------------------------------------------------------------------
    def compute_all_measurements(self) -> dict[str, Any]:
        """对所有信号计算周期与占空比等单信号测量。

        Returns:
            嵌套字典，结构为
            ``{signal_name: {"period": float, "duty_cycle": float, "edges": int}}``。
        """

        result: dict[str, Any] = {}
        for name, signal in self._signals.items():
            result[name] = {
                "period": TimingMeasurement.period(signal),
                "duty_cycle": TimingMeasurement.duty_cycle(signal),
                "edges": len(signal.edges()),
            }
        return result

    # ------------------------------------------------------------------
    # 导出
    # ------------------------------------------------------------------
    def to_text_table(self) -> str:
        """将时序图导出为文本表格。

        表格包含每个信号的名称、跳变数量、周期、占空比以及边沿时刻摘要。
        """

        header = (
            f"{'Signal':<{_NAME_WIDTH}} "
            f"{'Edges':>5} "
            f"{'Period(ns)':>{_TIME_WIDTH}} "
            f"{'Duty':>6}"
        )
        lines = ["[TimingDiagram]", header, "-" * len(header)]
        if not self._signals:
            lines.append("(no signals)")
            return "\n".join(lines)

        for name, signal in self._signals.items():
            period = TimingMeasurement.period(signal)
            duty = TimingMeasurement.duty_cycle(signal)
            edge_count = len(signal.edges())
            lines.append(
                f"{name:<{_NAME_WIDTH}} "
                f"{edge_count:>5} "
                f"{period:>{_TIME_WIDTH}.3f} "
                f"{duty:>6.2%}"
            )

        lines.append("")
        lines.append("Edge timeline:")
        for name, signal in self._signals.items():
            times = ", ".join(f"{e.time_ns:g}" for e in signal.edges()) or "-"
            lines.append(f"  {name}: {times}")
        return "\n".join(lines)

    def __repr__(self) -> str:
        return f"TimingDiagram(signals={list(self._signals)})"

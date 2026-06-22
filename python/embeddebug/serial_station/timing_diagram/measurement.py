"""时序图测量工具。

提供建立时间、保持时间、传播延迟、周期与占空比等常见数字时序参数的
计算方法。所有方法均为纯函数形式，直接作用于 ``TimingSignal``。
"""

from __future__ import annotations

from collections.abc import Sequence

from embeddebug.serial_station.timing_diagram.signal import TimingEdge, TimingSignal

__all__ = ["TimingMeasurement"]


class TimingMeasurement:
    """对 ``TimingSignal`` 进行时序参数计算的静态工具集。

    该类不维护任何状态，所有方法均可作为静态方法直接调用。
    """

    # ------------------------------------------------------------------
    # 建立时间 / 保持时间
    # ------------------------------------------------------------------
    @staticmethod
    def setup_time(
        data_signal: TimingSignal,
        clock_signal: TimingSignal,
        setup_window_ns: float,
    ) -> float:
        """计算数据相对于时钟的建立时间。

        定义为：在时钟有效边沿 (默认上升沿) 前 ``setup_window_ns`` 窗口
        内，数据最后一次发生跳变的时刻到 ``clock_edge - setup_window_ns``
        的差值。若窗口内数据未发生变化，则认为建立时间充足，返回
        ``setup_window_ns``。

        Args:
            data_signal: 数据信号。
            clock_signal: 时钟信号。
            setup_window_ns: 建立窗口长度 (ns)。

        Returns:
            建立时间 (ns)，不足时为负数；无有效时钟边沿返回 0。
        """

        clock_edges = TimingMeasurement._clock_edges(clock_signal)
        if not clock_edges:
            return 0.0

        data_changes = sorted(t for t, _ in data_signal.transitions)
        min_setup = setup_window_ns
        for edge_ns in clock_edges:
            window_start = edge_ns - setup_window_ns
            # 落在 (window_start, edge_ns] 内的数据跳变
            within = [t for t in data_changes if window_start < t <= edge_ns]
            # 数据在该窗口内最后一次变化距离窗口起点的余量
            setup = setup_window_ns - (within[-1] - window_start) if within else setup_window_ns
            min_setup = min(min_setup, setup)
        return min_setup

    @staticmethod
    def hold_time(
        data_signal: TimingSignal,
        clock_signal: TimingSignal,
    ) -> float:
        """计算数据相对于时钟的保持时间。

        定义为：时钟有效边沿 (默认上升沿) 之后，数据首次发生跳变的时刻
        与该时钟边沿之间的时间差。若边沿后数据不再变化，返回 ``0.0``。

        Args:
            data_signal: 数据信号。
            clock_signal: 时钟信号。

        Returns:
            保持时间 (ns)；无有效时钟边沿返回 0。
        """

        clock_edges = TimingMeasurement._clock_edges(clock_signal)
        if not clock_edges:
            return 0.0

        data_changes = sorted(t for t, _ in data_signal.transitions)
        min_hold = float("inf")
        for edge_ns in clock_edges:
            after = [t for t in data_changes if t > edge_ns]
            if after:
                min_hold = min(min_hold, after[0] - edge_ns)
        return 0.0 if min_hold == float("inf") else min_hold

    # ------------------------------------------------------------------
    # 传播延迟 / 周期 / 占空比
    # ------------------------------------------------------------------
    @staticmethod
    def propagation_delay(
        in_signal: TimingSignal,
        out_signal: TimingSignal,
    ) -> float:
        """计算输入到输出的传播延迟。

        取输入首个边沿与输出首个边沿之间的时间差 (out - in)。若任一信号
        缺少边沿，返回 ``0.0``。

        Args:
            in_signal: 输入信号。
            out_signal: 输出信号。

        Returns:
            传播延迟 (ns)。
        """

        in_edges = TimingMeasurement._edge_times(in_signal)
        out_edges = TimingMeasurement._edge_times(out_signal)
        if not in_edges or not out_edges:
            return 0.0
        return out_edges[0] - in_edges[0]

    @staticmethod
    def period(signal: TimingSignal) -> float:
        """计算信号的周期。

        取相邻同类边沿 (默认上升沿) 间隔的平均值。若上升沿少于 2 个，则
        回退到所有边沿的相邻平均间隔；边沿不足时返回 ``0.0``。

        Args:
            signal: 周期性信号。

        Returns:
            平均周期 (ns)。
        """

        rising = signal.transition_times(target=True)
        if len(rising) >= 2:
            gaps = [b - a for a, b in zip(rising, rising[1:], strict=False)]
            return sum(gaps) / len(gaps)
        all_edges = signal.transition_times()
        if len(all_edges) >= 2:
            gaps = [b - a for a, b in zip(all_edges, all_edges[1:], strict=False)]
            return sum(gaps) / len(gaps)
        return 0.0

    @staticmethod
    def duty_cycle(signal: TimingSignal) -> float:
        """计算信号占空比。

        定义为高电平持续时间占 ``[0, end_time]`` 区间总长度的比例，取值
        区间 ``[0.0, 1.0]``。信号无跳变或总时长为 0 时返回 ``0.0``。

        Args:
            signal: 周期性信号。

        Returns:
            占空比 (0~1)。
        """

        high_duration = signal.duration_of_state(True)
        total = signal.end_time
        if total <= 0:
            return 0.0
        return high_duration / total

    # ------------------------------------------------------------------
    # 内部辅助
    # ------------------------------------------------------------------
    @staticmethod
    def _edge_times(signal: TimingSignal) -> list[float]:
        """返回信号所有边沿 (实际发生电平变化的) 时刻。"""

        return [e.time_ns for e in signal.edges()]

    @staticmethod
    def _clock_edges(
        signal: TimingSignal,
        *,
        rising: bool = True,
    ) -> list[float]:
        """返回时钟信号的有效边沿时刻。"""

        edges: Sequence[TimingEdge] = signal.edges()
        if rising:
            return [e.time_ns for e in edges if e.is_rising]
        return [e.time_ns for e in edges if e.is_falling]

"""时序图信号数据结构。

本模块定义逻辑信号的基本载体 ``TimingSignal``，以及用于描述单次边沿
的 ``TimingEdge`` 和从采样链路采集得到的原始样本 ``LogicSample``。

时间单位统一为纳秒 (ns)，电平用布尔值表示 (True = 高电平)。
"""

from __future__ import annotations

from dataclasses import dataclass, field
from collections.abc import Iterable, Sequence

__all__ = ["LogicSample", "TimingEdge", "TimingSignal"]


@dataclass(frozen=True)
class LogicSample:
    """逻辑分析仪采样点。

    不可变记录，用于从硬件采样链路构造信号。
    """

    time_ns: float
    level: bool


@dataclass(frozen=True)
class TimingEdge:
    """信号上的一次电平跳变。

    Attributes:
        time_ns: 跳变发生时刻 (ns)。
        from_level: 跳变前的电平。
        to_level: 跳变后的电平。
    """

    time_ns: float
    from_level: bool
    to_level: bool

    @property
    def is_rising(self) -> bool:
        """是否为上升沿。"""

        return (not self.from_level) and self.to_level

    @property
    def is_falling(self) -> bool:
        """是否为下降沿。"""

        return self.from_level and (not self.to_level)


@dataclass
class TimingSignal:
    """单个逻辑信号。

    通过一系列 (time_ns, new_level) 跳变定义电平随时间的演化。信号在
    第一个跳变之前保持 ``initial_level``，最后一个跳变之后保持其末态。

    Attributes:
        name: 信号名称。
        transitions: 跳变列表，元素为 ``(time_ns, new_level)``。
        initial_level: 初始电平，默认低电平。
    """

    name: str
    transitions: list[tuple[float, bool]] = field(default_factory=list)
    initial_level: bool = False

    def __post_init__(self) -> None:
        self.transitions = sorted(self.transitions, key=lambda item: item[0])

    # ------------------------------------------------------------------
    # 构造工具
    # ------------------------------------------------------------------
    @classmethod
    def from_samples(
        cls,
        name: str,
        samples: Iterable[LogicSample],
        *,
        initial_level: bool = False,
    ) -> TimingSignal:
        """从 ``LogicSample`` 序列构造信号。

        仅在电平发生变化时记录跳变，连续相同样本会被压缩。
        """

        transitions: list[tuple[float, bool]] = []
        last_level: bool | None = None
        for sample in sorted(samples, key=lambda s: s.time_ns):
            if last_level is None:
                if sample.level != initial_level:
                    transitions.append((sample.time_ns, sample.level))
                last_level = sample.level
                continue
            if sample.level != last_level:
                transitions.append((sample.time_ns, sample.level))
                last_level = sample.level
        return cls(name=name, transitions=transitions, initial_level=initial_level)

    # ------------------------------------------------------------------
    # 查询接口
    # ------------------------------------------------------------------
    def level_at(self, time_ns: float) -> bool:
        """返回指定时刻的电平。"""

        current = self.initial_level
        for t, level in self.transitions:
            if t <= time_ns:
                current = level
            else:
                break
        return current

    def to_levels(self, time_points: Sequence[float]) -> list[bool]:
        """返回多个时间点上的电平列表。"""

        return [self.level_at(t) for t in time_points]

    def edges(self) -> list[TimingEdge]:
        """返回信号中所有的边沿。"""

        result: list[TimingEdge] = []
        prev = self.initial_level
        for t, level in self.transitions:
            if level != prev:
                result.append(TimingEdge(time_ns=t, from_level=prev, to_level=level))
                prev = level
        return result

    def transition_times(self, *, target: bool | None = None) -> list[float]:
        """返回跳变时刻。

        Args:
            target: 若给定，仅返回跳向该电平的时刻。
        """

        if target is None:
            return [t for t, _ in self.transitions]
        return [t for t, lvl in self.transitions if lvl == target]

    @property
    def start_time(self) -> float:
        """信号首次跳变的时刻；无跳变时返回 0。"""

        return self.transitions[0][0] if self.transitions else 0.0

    @property
    def end_time(self) -> float:
        """信号最后一次跳变的时刻；无跳变时返回 0。"""

        return self.transitions[-1][0] if self.transitions else 0.0

    def duration_of_state(self, state: bool) -> float:
        """返回信号处于指定电平的总时长 (ns)。

        计算区间为 ``[0, end_time]``。若信号无跳变，则在该区间内整体
        保持 ``initial_level``。
        """

        if not self.transitions:
            return self.end_time if self.initial_level == state else 0.0

        total = 0.0
        # 第一段：[0, first_transition)
        total += self._segment(self.start_time, state)
        # 相邻跳变之间的区间
        for prev, nxt in zip(self.transitions, self.transitions[1:], strict=False):
            total += self._segment_between(prev[0], nxt[0], prev[1], state)
        # 末段：最后一个跳变之后没有持续时间 (单点)
        return total

    # ------------------------------------------------------------------
    # 内部辅助
    # ------------------------------------------------------------------
    def _segment(self, duration: float, state: bool) -> float:
        """计算 ``[0, duration)`` 区间内处于 ``initial_level`` 的时长。"""

        return duration if self.initial_level == state else 0.0

    def _segment_between(
        self,
        start: float,
        stop: float,
        level: bool,
        state: bool,
    ) -> float:
        """计算 ``[start, stop)`` 区间内处于 ``level`` 的时长。"""

        return (stop - start) if level == state else 0.0

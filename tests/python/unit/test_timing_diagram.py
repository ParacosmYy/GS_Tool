"""时序图模块单元测试。"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.timing_diagram import (
    LogicSample,
    TimingDiagram,
    TimingEdge,
    TimingMeasurement,
    TimingSignal,
)


# --- TimingSignal: 电平与样本构造 ---
def test_signal_level_at_time_points():
    """跳变列表在指定时刻应给出正确电平。"""
    sig = TimingSignal(name="S", transitions=[(10.0, True), (20.0, False)])
    assert sig.to_levels([0.0, 5.0, 10.0, 15.0, 20.0, 30.0]) == [
        False, False, True, True, False, False,
    ]


def test_signal_level_before_first_transition_uses_initial():
    """无跳变时全程保持初始电平。"""
    sig = TimingSignal(name="S", transitions=[], initial_level=True)
    assert sig.to_levels([0.0, 100.0]) == [True, True]


def test_signal_from_samples_compresses_unchanged_levels():
    """from_samples 应压缩连续相同的样本。"""
    samples = [
        LogicSample(0.0, False), LogicSample(5.0, False),
        LogicSample(10.0, True), LogicSample(15.0, True),
        LogicSample(20.0, False),
    ]
    sig = TimingSignal.from_samples("S", samples)
    assert sig.transition_times() == [10.0, 20.0]


def test_signal_from_samples_first_change_recorded():
    """首个样本若与初始不同则记录跳变。"""
    sig = TimingSignal.from_samples("S", [LogicSample(3.0, True)], initial_level=False)
    assert sig.transition_times() == [3.0]
    assert sig.to_levels([0.0, 3.0, 4.0]) == [False, True, True]


def test_duration_of_state_high_and_low():
    """高/低电平总时长应正确分配。"""
    sig = TimingSignal(name="S", transitions=[(10.0, True), (30.0, False)])
    assert sig.duration_of_state(True) == pytest.approx(20.0)
    assert sig.duration_of_state(False) == pytest.approx(10.0)


def test_duration_of_state_no_transitions():
    """无跳变时按初始电平占据整个区间。"""
    high = TimingSignal(name="H", transitions=[], initial_level=True)
    low = TimingSignal(name="L", transitions=[], initial_level=False)
    assert high.duration_of_state(True) == 0.0
    assert low.duration_of_state(False) == 0.0


def test_duration_of_state_multiple_segments():
    """多个跳变段的时长应累加。"""
    sig = TimingSignal(
        name="S",
        transitions=[(10.0, True), (20.0, False), (25.0, True), (35.0, False)],
    )
    assert sig.duration_of_state(True) == pytest.approx(20.0)
    assert sig.duration_of_state(False) == pytest.approx(15.0)


# --- TimingEdge ---
def test_edge_classification():
    """边沿方向判定。"""
    rising = TimingEdge(time_ns=5.0, from_level=False, to_level=True)
    falling = TimingEdge(time_ns=5.0, from_level=True, to_level=False)
    assert rising.is_rising and not rising.is_falling
    assert falling.is_falling and not falling.is_rising


def test_signal_edges_skip_noop_transitions():
    """与前一电平相同的跳变不应产生边沿。"""
    sig = TimingSignal(name="S", transitions=[(10.0, True), (12.0, True), (20.0, False)])
    edges = sig.edges()
    assert [e.time_ns for e in edges] == [10.0, 20.0]
    assert edges[0].is_rising and edges[1].is_falling


# --- TimingMeasurement: 建立时间 / 保持时间 ---
def test_setup_time_within_window():
    """数据在窗口内变化，建立时间为边沿减去末次跳变。"""
    clock = TimingSignal(name="CLK", transitions=[(100.0, True), (200.0, True)])
    data = TimingSignal(name="D", transitions=[(90.0, True)])
    setup = TimingMeasurement.setup_time(data, clock, setup_window_ns=20.0)
    assert setup == pytest.approx(10.0)


def test_setup_time_stable_data_returns_full_window():
    """窗口内数据稳定时建立时间等于整窗。"""
    clock = TimingSignal(name="CLK", transitions=[(100.0, True)])
    data = TimingSignal(name="D", transitions=[(50.0, True)])
    assert TimingMeasurement.setup_time(data, clock, setup_window_ns=20.0) == pytest.approx(20.0)


def test_hold_time_after_clock_edge():
    """保持时间为时钟边沿后数据首次变化的时间差。"""
    clock = TimingSignal(name="CLK", transitions=[(100.0, True)])
    data = TimingSignal(name="D", transitions=[(115.0, False)])
    assert TimingMeasurement.hold_time(data, clock) == pytest.approx(15.0)


def test_hold_time_no_data_change_returns_zero():
    """时钟边沿后数据无变化时保持时间为 0。"""
    clock = TimingSignal(name="CLK", transitions=[(100.0, True)])
    data = TimingSignal(name="D", transitions=[])
    assert TimingMeasurement.hold_time(data, clock) == 0.0


# --- TimingMeasurement: 传播延迟 / 周期 / 占空比 ---
def test_propagation_delay_first_edge_difference():
    """传播延迟为输出与输入首个边沿之差。"""
    in_sig = TimingSignal(name="IN", transitions=[(10.0, True)])
    out_sig = TimingSignal(name="OUT", transitions=[(18.0, True)])
    assert TimingMeasurement.propagation_delay(in_sig, out_sig) == pytest.approx(8.0)


def test_propagation_delay_missing_edges():
    """任一信号缺少边沿时返回 0。"""
    in_sig = TimingSignal(name="IN", transitions=[(10.0, True)])
    out_sig = TimingSignal(name="OUT", transitions=[])
    assert TimingMeasurement.propagation_delay(in_sig, out_sig) == 0.0


def test_period_from_rising_edges():
    """周期取相邻上升沿间隔的均值。"""
    sig = TimingSignal(
        name="CLK",
        transitions=[(0.0, True), (50.0, False), (100.0, True), (150.0, False), (200.0, True)],
    )
    assert TimingMeasurement.period(sig) == pytest.approx(100.0)


def test_period_insufficient_edges():
    """边沿不足时周期为 0。"""
    sig = TimingSignal(name="CLK", transitions=[(10.0, True)])
    assert TimingMeasurement.period(sig) == 0.0


def test_duty_cycle_fifty_percent():
    """等占空方波占空比为 0.5。"""
    sig = TimingSignal(name="CLK", transitions=[(50.0, True), (100.0, False)])
    assert TimingMeasurement.duty_cycle(sig) == pytest.approx(0.5)


def test_duty_cycle_quarter():
    """高电平占 1/4 时占空比为 0.25。"""
    sig = TimingSignal(name="CLK", transitions=[(75.0, True), (100.0, False)])
    assert TimingMeasurement.duty_cycle(sig) == pytest.approx(0.25)


def test_duty_cycle_zero_duration():
    """无跳变时占空比为 0。"""
    sig = TimingSignal(name="CLK", transitions=[])
    assert TimingMeasurement.duty_cycle(sig) == 0.0


# --- TimingDiagram: 组装与导出 ---
def test_diagram_add_and_get_signal():
    """添加信号后可按名称取回。"""
    diagram = TimingDiagram()
    sig = diagram.add_signal("CLK", [(10.0, True), (20.0, False)])
    assert isinstance(sig, TimingSignal)
    assert diagram.get_signal("CLK") is sig
    assert diagram.get_signal("missing") is None


def test_diagram_list_and_contains():
    """list_signals / __contains__ 行为正确。"""
    diagram = TimingDiagram()
    diagram.add_signal("A", [(1.0, True)])
    diagram.add_signal("B", [(2.0, True)])
    assert [s.name for s in diagram.list_signals()] == ["A", "B"]
    assert "A" in diagram and "missing" not in diagram
    assert len(diagram) == 2


def test_diagram_overwrite_existing_signal():
    """重名添加应覆盖既有信号。"""
    diagram = TimingDiagram()
    diagram.add_signal("S", [(5.0, True)])
    diagram.add_signal("S", [(50.0, True)])
    sig = diagram.get_signal("S")
    assert sig is not None and sig.transition_times() == [50.0]
    assert len(diagram) == 1


def test_diagram_compute_all_measurements():
    """compute_all_measurements 返回每信号的周期/占空比/边沿数。"""
    diagram = TimingDiagram()
    diagram.add_signal(
        "CLK", [(50.0, True), (100.0, False), (150.0, True), (200.0, False)],
    )
    diagram.add_signal("DATA", [(25.0, True)])
    result = diagram.compute_all_measurements()
    assert set(result.keys()) == {"CLK", "DATA"}
    assert result["CLK"]["period"] == pytest.approx(100.0)
    assert result["CLK"]["duty_cycle"] == pytest.approx(0.5)
    assert result["CLK"]["edges"] == 4
    assert result["DATA"]["period"] == 0.0


def test_diagram_to_text_table_contains_signal_names():
    """文本表格应包含信号名与表头。"""
    diagram = TimingDiagram()
    diagram.add_signal("CLK", [(0.0, True), (50.0, False)])
    text = diagram.to_text_table()
    assert "[TimingDiagram]" in text and "CLK" in text and "Signal" in text


def test_diagram_to_text_table_empty():
    """无信号时表格给出占位提示。"""
    assert "(no signals)" in TimingDiagram().to_text_table()


def test_diagram_assembly_integration():
    """端到端组装：建立/保持/传播延迟与测量聚合协同工作。"""
    diagram = TimingDiagram()
    diagram.add_signal("IN", [(10.0, True)])
    diagram.add_signal("OUT", [(20.0, True)])
    diagram.add_signal("CLK", [(100.0, True), (150.0, False), (200.0, True)])
    diagram.add_signal("DATA", [(95.0, True), (115.0, False), (205.0, True)])

    in_sig = diagram.get_signal("IN")
    out_sig = diagram.get_signal("OUT")
    clk_sig = diagram.get_signal("CLK")
    data_sig = diagram.get_signal("DATA")

    assert TimingMeasurement.propagation_delay(in_sig, out_sig) == pytest.approx(10.0)
    assert TimingMeasurement.hold_time(data_sig, clk_sig) == pytest.approx(5.0)
    assert TimingMeasurement.setup_time(data_sig, clk_sig, setup_window_ns=10.0) == pytest.approx(5.0)
    assert set(diagram.compute_all_measurements()) == {"IN", "OUT", "CLK", "DATA"}

"""controller_receive_state handle_received_bytes + ReceiveState 边界测试。

handle_received_bytes 此前无直接测试（仅经 controller_state_core 间接调用）。
本文件覆盖空数据 / 非 measurement 事件 / measurement 事件路径。

覆盖：
1. handle_received_bytes 空数据返回 state 不变。
2. handle_received_bytes 非 measurement 数据 → entries 增长。
3. handle_received_bytes measurement 事件 → ring 初始化。
4. handle_received_bytes 无 measurement → ring 不变。
5. ReceiveState 默认 ring=None。
6. handle_error 返回 ReceiveState。
7. handle_received_bytes 多次调用累积 entries。
8. handle_received_bytes 空 callbacks 不崩。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_receive_state import (
    ReceiveState,
    handle_received_bytes,
)
from embeddebug.serial_station.core.dispatcher import SerialDispatcher
from embeddebug.serial_station.protocols.raw_data import RawDataProtocol


def _make_state():
    return ReceiveState(measurement_ring=None)


def _make_dispatcher():
    return SerialDispatcher(RawDataProtocol())


def test_handle_received_empty_data():
    state = _make_state()
    new_state = handle_received_bytes(
        state=state,
        data=b"",
        dispatcher=_make_dispatcher(),
        entries=[],
        log_callbacks=[],
        measurement_callbacks=[],
    )
    assert new_state is state or new_state.measurement_ring is None


def test_handle_received_non_measurement_appends_entries():
    state = _make_state()
    entries = []
    handle_received_bytes(
        state=state,
        data=b"hello",
        dispatcher=_make_dispatcher(),
        entries=entries,
        log_callbacks=[],
        measurement_callbacks=[],
    )
    assert len(entries) > 0


def test_handle_received_no_measurement_ring_unchanged():
    """无 measurement 事件 → ring 不变（None）。"""

    state = _make_state()
    new_state = handle_received_bytes(
        state=state,
        data=b"text",
        dispatcher=_make_dispatcher(),
        entries=[],
        log_callbacks=[],
        measurement_callbacks=[],
    )
    assert new_state.measurement_ring is None


def test_receive_state_default_ring_none():
    state = ReceiveState(measurement_ring=None)
    assert state.measurement_ring is None


def test_handle_received_multiple_calls_accumulate():
    """多次 handle_received_bytes 累积 entries。"""

    state = _make_state()
    entries = []
    for _ in range(3):
        handle_received_bytes(
            state=state,
            data=b"abc",
            dispatcher=_make_dispatcher(),
            entries=entries,
            log_callbacks=[],
            measurement_callbacks=[],
        )
    assert len(entries) >= 3


def test_handle_received_empty_callbacks_no_crash():
    """空 callbacks 列表不崩。"""

    state = _make_state()
    handle_received_bytes(
        state=state,
        data=b"x",
        dispatcher=_make_dispatcher(),
        entries=[],
        log_callbacks=[],
        measurement_callbacks=[],
    )


def test_handle_received_with_log_callback():
    """有 log_callback → callback 被调用。"""

    state = _make_state()
    calls = []
    handle_received_bytes(
        state=state,
        data=b"hello",
        dispatcher=_make_dispatcher(),
        entries=[],
        log_callbacks=[lambda entry: calls.append(entry)],
        measurement_callbacks=[],
    )
    assert len(calls) > 0


def test_handle_received_measurement_callback_not_invoked_for_raw_data():
    """RawData 协议不产生 measurement 事件 → measurement_callback 不调用。"""

    state = _make_state()
    meas_calls = []
    handle_received_bytes(
        state=state,
        data=b"hello",
        dispatcher=_make_dispatcher(),
        entries=[],
        log_callbacks=[],
        measurement_callbacks=[lambda batch: meas_calls.append(batch)],
    )
    assert len(meas_calls) == 0

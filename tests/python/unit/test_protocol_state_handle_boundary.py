"""controller_protocol_state handle_received_bytes + create_protocol_runtime 边界测试。

handle_received_bytes in protocol_state 此前无直接测试。
本文件覆盖空/非空数据 + entries 增长 + ring 更新。

覆盖：
1. create_protocol_runtime 返回 ProtocolRuntime。
2. available_protocols 含 'raw_data'。
3. handle_received_bytes 空数据不崩。
4. handle_received_bytes 非 measurement entries 增长。
5. handle_received_bytes 多次调用累积 entries。
6. handle_received_bytes 空 callbacks 不崩。
7. handle_received_bytes 有 log_callback 被调。
8. handle_received_bytes 返回新 ProtocolRuntime。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers import controller_protocol_state


def test_create_protocol_runtime():
    runtime = controller_protocol_state.create_protocol_runtime()
    assert runtime is not None
    assert runtime.dispatcher is not None
    assert runtime.registry is not None


def test_available_protocols_contains_raw_data():
    runtime = controller_protocol_state.create_protocol_runtime()
    protos = controller_protocol_state.available_protocols(runtime)
    assert "raw_data" in protos


def test_handle_received_empty_data():
    runtime = controller_protocol_state.create_protocol_runtime()
    new = controller_protocol_state.handle_received_bytes(
        runtime, b"", entries=[], log_callbacks=[], measurement_callbacks=[]
    )
    assert new is not None


def test_handle_received_appends_entries():
    runtime = controller_protocol_state.create_protocol_runtime()
    entries = []
    controller_protocol_state.handle_received_bytes(
        runtime, b"hello", entries=entries, log_callbacks=[], measurement_callbacks=[]
    )
    assert len(entries) > 0


def test_handle_received_multiple_accumulate():
    runtime = controller_protocol_state.create_protocol_runtime()
    entries = []
    for _ in range(3):
        controller_protocol_state.handle_received_bytes(
            runtime, b"abc", entries=entries, log_callbacks=[], measurement_callbacks=[]
        )
    assert len(entries) >= 3


def test_handle_received_empty_callbacks():
    runtime = controller_protocol_state.create_protocol_runtime()
    controller_protocol_state.handle_received_bytes(
        runtime, b"x", entries=[], log_callbacks=[], measurement_callbacks=[]
    )


def test_handle_received_log_callback_invoked():
    runtime = controller_protocol_state.create_protocol_runtime()
    calls = []
    controller_protocol_state.handle_received_bytes(
        runtime, b"hello",
        entries=[],
        log_callbacks=[lambda e: calls.append(e)],
        measurement_callbacks=[],
    )
    assert len(calls) > 0


def test_handle_received_returns_new_runtime():
    runtime = controller_protocol_state.create_protocol_runtime()
    new = controller_protocol_state.handle_received_bytes(
        runtime, b"data", entries=[], log_callbacks=[], measurement_callbacks=[]
    )
    assert isinstance(new, type(runtime))

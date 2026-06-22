"""controller_callback_state 单元测试 — 回调注册 + 错误处理。"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_callback_state import (
    add_error_callback,
    add_log_callback,
    add_measurement_callback,
    create_callback_state,
    handle_error,
)


def test_create_callback_state_defaults():
    state = create_callback_state()
    assert state.log == []
    assert state.error == []
    assert state.measurement == []


def test_add_log_callback():
    state = create_callback_state()
    cb = lambda entry: None
    add_log_callback(state, cb)
    assert len(state.log) == 1


def test_add_error_callback():
    state = create_callback_state()
    cb = lambda msg: None
    add_error_callback(state, cb)
    assert len(state.error) == 1


def test_add_measurement_callback():
    state = create_callback_state()
    cb = lambda batch: None
    add_measurement_callback(state, cb)
    assert len(state.measurement) == 1


def test_handle_error_invokes_callbacks():
    """handle_error 触发 error 回调 + 添加 error 日志条目。"""
    state = create_callback_state()
    errors = []
    add_error_callback(state, lambda msg: errors.append(msg))
    entries = []
    handle_error("test error", state, entries)
    assert errors == ["test error"]
    assert len(entries) == 1
    assert entries[0].direction == "error"


def test_handle_error_invokes_log_callback():
    """handle_error 也触发 log 回调（error 条目加入 entries 后通知）。"""
    state = create_callback_state()
    log_entries = []
    add_log_callback(state, lambda entry: log_entries.append(entry))
    entries = []
    handle_error("err", state, entries)
    assert len(log_entries) == 1

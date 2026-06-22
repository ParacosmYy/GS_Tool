"""会话保存/恢复模块单元测试。

覆盖序列化往返、落盘读写、自动节流、崩溃恢复检测与缺失字段兼容等关键
路径，正向与异常路径成对存在（约束 6.1）。
"""

from __future__ import annotations

import json
import os
import time

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")



from embeddebug.serial_station.session import (
    AUTOSAVE_INTERVAL_S,
    SESSION_EXTENSION,
    SessionManager,
    SessionSerializer,
    SessionState,
)

def _sample_state(timestamp_ns: int = 1_700_000_000_000_000_000) -> SessionState:
    """构造覆盖所有字段的状态样本。"""

    return SessionState(
        window_geometry={"x": 100, "y": 50, "width": 1600, "height": 900},
        transport_mode="tcp_client",
        protocol="modbus_rtu",
        connection_config={"port": "COM7", "baudrate": 9600, "host": "127.0.0.1"},
        command_history=["ping", "status?", "reset"],
        log_filter="tx",
        active_tab="waveform",
        timestamp_ns=timestamp_ns,
    )

# ---- 序列化往返 ----

def test_serialize_deserialize_round_trip_preserves_all_fields():
    state = _sample_state()

    restored = SessionSerializer.deserialize(SessionSerializer.serialize(state))

    assert restored.to_dict() == state.to_dict()

def test_serialize_emits_version_and_payload_envelope():
    document = json.loads(SessionSerializer.serialize(_sample_state()))

    assert document["version"] == 1
    assert document["payload"]["transport_mode"] == "tcp_client"
    assert document["payload"]["protocol"] == "modbus_rtu"

def test_deserialize_empty_string_returns_defaults():
    state = SessionSerializer.deserialize("")

    assert isinstance(state, SessionState)
    assert state.transport_mode == "uart"
    assert state.command_history == []

def test_deserialize_malformed_json_returns_defaults():
    state = SessionSerializer.deserialize("{not json")

    assert isinstance(state, SessionState)
    assert state.protocol == "raw_data"

def test_deserialize_missing_fields_get_defaults():
    payload = json.dumps({"version": 1, "payload": {"transport_mode": "rtt"}})

    state = SessionSerializer.deserialize(payload)

    assert state.transport_mode == "rtt"
    assert state.protocol == "raw_data"
    assert state.window_geometry["width"] == 1280
    assert state.command_history == []
    assert state.timestamp_ns == 0

def test_deserialize_tolerates_wrong_field_types():
    payload = json.dumps(
        {
            "version": 1,
            "payload": {
                "window_geometry": "oops",
                "connection_config": None,
                "command_history": "not-a-list",
                "timestamp_ns": "12",
            },
        }
    )

    state = SessionSerializer.deserialize(payload)

    assert state.window_geometry["width"] == 1280
    assert state.connection_config["baudrate"] == 115200
    assert state.command_history == []
    assert state.timestamp_ns == 12

def test_deserialize_accepts_legacy_bare_dict_document():
    legacy = json.dumps({"transport_mode": "udp", "protocol": "raw_data"})

    state = SessionSerializer.deserialize(legacy)

    assert state.transport_mode == "udp"

def test_state_from_dict_none_returns_defaults():
    assert SessionState.from_dict(None) == SessionState()

# ---- SessionManager: save / load / clear ----

def test_save_and_load_round_trip_with_tmp_path(tmp_path):
    manager = SessionManager()
    path = tmp_path / f"workspace{SESSION_EXTENSION}"
    state = _sample_state()

    assert manager.save(state, path) is True
    assert path.is_file()
    # 原子写入后不应遗留临时文件。
    assert [p for p in tmp_path.iterdir() if p.suffix == ".tmp"] == []

    restored = manager.load(path)

    assert restored is not None
    assert restored.to_dict() == state.to_dict()

def test_load_missing_file_returns_none(tmp_path):
    assert SessionManager().load(tmp_path / "missing.edsession") is None

def test_load_corrupt_file_returns_default_state(tmp_path):
    path = tmp_path / "broken.edsession"
    path.write_text("garbage", encoding="utf-8")

    state = SessionManager().load(path)

    assert isinstance(state, SessionState)
    assert state.transport_mode == "uart"

def test_clear_removes_session_file(tmp_path):
    manager = SessionManager()
    path = tmp_path / "workspace.edsession"
    manager.save(_sample_state(), path)
    assert path.exists()

    assert manager.clear(path) is True
    assert not path.exists()

def test_clear_when_missing_returns_true(tmp_path):
    assert SessionManager().clear(tmp_path / "ghost.edsession") is True

def test_save_creates_missing_parent_directory(tmp_path):
    manager = SessionManager()
    path = tmp_path / "deep" / "nested" / "session.edsession"

    assert manager.save(_sample_state(), path) is True
    assert path.is_file()

# ---- 崩溃恢复检测 ----

def test_has_crash_recovery_false_for_missing_file(tmp_path):
    assert SessionManager().has_crash_recovery(tmp_path / "none.edsession") is False

def test_has_crash_recovery_true_after_save(tmp_path):
    manager = SessionManager()
    path = tmp_path / "auto.edsession"
    manager.save(_sample_state(), path)

    assert manager.has_crash_recovery(path) is True

def test_has_crash_recovery_false_for_empty_file(tmp_path):
    path = tmp_path / "empty.edsession"
    path.write_text("", encoding="utf-8")

    assert SessionManager().has_crash_recovery(path) is False

# ---- autosave 节流 ----

def test_autosave_first_call_writes_immediately(tmp_path):
    manager = SessionManager(now_ns=time.time_ns())
    path = tmp_path / "auto.edsession"

    assert manager.autosave(_sample_state(), path) is True
    assert path.is_file()

def test_autosave_subsequent_call_within_interval_is_throttled(tmp_path):
    manager = SessionManager(now_ns=time.time_ns())
    path = tmp_path / "auto.edsession"

    assert manager.autosave(_sample_state(), path) is True
    first_mtime = path.stat().st_mtime_ns

    assert manager.autosave(_sample_state(), path) is False
    assert path.stat().st_mtime_ns == first_mtime

def test_autosave_fires_again_after_interval(tmp_path):
    manager = SessionManager(now_ns=time.time_ns())
    path = tmp_path / "auto.edsession"

    manager.autosave(_sample_state(), path)
    # 回拨最近保存时间戳，模拟已超过节流窗口。
    manager._last_autosave_ns -= int(AUTOSAVE_INTERVAL_S * 1_000_000_000) + 1

    assert manager.autosave(_sample_state(), path) is True

def test_autosave_returns_false_when_save_fails(tmp_path, monkeypatch):
    manager = SessionManager(now_ns=time.time_ns())
    monkeypatch.setattr(SessionManager, "save", lambda self, state, p: False)

    assert manager.autosave(_sample_state(), tmp_path / "auto.edsession") is False

# ---- 版本前向兼容 ----

def test_unknown_version_still_parses_payload(tmp_path):
    document = {"version": 999, "payload": {"transport_mode": "rtt", "active_tab": "console"}}
    path = tmp_path / "future.edsession"
    path.write_text(json.dumps(document), encoding="utf-8")

    state = SessionManager().load(path)

    assert state is not None
    assert state.transport_mode == "rtt"
    assert state.active_tab == "console"

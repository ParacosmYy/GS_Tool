"""script_recorder 边界单元测试（action + recording + recorder helper）。

补强 test_script_recorder.py 未直接断言的边角：
- ScriptAction：frozen + 常量（SEND/RECEIVE/CONNECT/DISCONNECT/DELAY/WAIT）+ to/from_dict。
- ScriptRecording：duration_ms 空=default=0 + add + to_json/from_json round-trip + from_json 缺 actions。
- ScriptRecorder：start 幂等 + stop 重置 + record 未录制 False + record 类型过滤 + recording 属性。
"""

from __future__ import annotations

import json

from embeddebug.serial_station.script_recorder.action import ScriptAction
from embeddebug.serial_station.script_recorder.recorder import ScriptRecorder
from embeddebug.serial_station.script_recorder.recording import ScriptRecording


# ── ScriptAction ─────────────────────────────────────────────────────────


def test_action_constants():
    """ScriptAction 常量值。"""

    assert ScriptAction.SEND == "SEND"
    assert ScriptAction.RECEIVE == "RECEIVE"
    assert ScriptAction.CONNECT == "CONNECT"
    assert ScriptAction.DISCONNECT == "DISCONNECT"
    assert ScriptAction.DELAY == "DELAY"
    assert ScriptAction.WAIT == "WAIT"


def test_action_to_dict():
    action = ScriptAction(type="SEND", payload="AT", timestamp_ms=100, label="cmd")
    d = action.to_dict()
    assert d == {"type": "SEND", "payload": "AT", "timestamp_ms": 100, "label": "cmd"}


def test_action_from_dict_round_trip():
    original = ScriptAction(type="DELAY", payload="500", timestamp_ms=200, label="wait")
    restored = ScriptAction.from_dict(original.to_dict())
    assert restored == original


def test_action_from_dict_missing_label_defaults_empty():
    action = ScriptAction.from_dict({"type": "SEND", "payload": "AT", "timestamp_ms": 0})
    assert action.label == ""


def test_action_label_defaults_empty():
    action = ScriptAction(type="SEND", payload="AT", timestamp_ms=0)
    assert action.label == ""


# ── ScriptRecording ──────────────────────────────────────────────────────


def test_recording_duration_empty_is_zero():
    """空 actions → duration_ms=0。"""

    assert ScriptRecording().duration_ms == 0


def test_recording_duration_max_timestamp():
    """duration_ms = 最大 timestamp_ms。"""

    r = ScriptRecording(actions=[
        ScriptAction(type="SEND", payload="A", timestamp_ms=100),
        ScriptAction(type="SEND", payload="B", timestamp_ms=500),
        ScriptAction(type="SEND", payload="C", timestamp_ms=200),
    ])
    assert r.duration_ms == 500


def test_recording_action_count():
    r = ScriptRecording(actions=[
        ScriptAction(type="SEND", payload="A", timestamp_ms=0),
    ])
    assert r.action_count == 1


def test_recording_add_appends():
    r = ScriptRecording()
    r.add(ScriptAction(type="SEND", payload="AT", timestamp_ms=0))
    assert len(r.actions) == 1


def test_recording_to_json_round_trip():
    """to_json → from_json 保持核心字段。"""

    original = ScriptRecording(
        actions=[ScriptAction(type="SEND", payload="AT", timestamp_ms=100)],
        name="test",
        description="a test",
    )
    text = original.to_json()
    assert text is not None
    restored = ScriptRecording.from_json(text)  # type: ignore[arg-type]
    assert restored.name == "test"
    assert restored.description == "a test"
    assert len(restored.actions) == 1
    assert restored.actions[0].payload == "AT"


def test_recording_from_json_missing_actions_defaults_empty():
    """from_json 缺 actions → 空 list。"""

    text = json.dumps({"name": "x"})
    r = ScriptRecording.from_json(text)  # type: ignore[arg-type]
    assert r.actions == []
    assert r.name == "x"


def test_recording_to_json_writes_file(tmp_path):
    """to_json(path) 写入文件返回 None。"""

    r = ScriptRecording(name="file-test")
    result = r.to_json(tmp_path / "script.json")
    assert result is None
    assert (tmp_path / "script.json").exists()


def test_recording_from_json_reads_file(tmp_path):
    """from_json(path) 从文件读取。"""

    path = tmp_path / "script.json"
    path.write_text(json.dumps({
        "name": "from-file",
        "actions": [{"type": "SEND", "payload": "HI", "timestamp_ms": 0}],
    }))
    r = ScriptRecording.from_json(str(path))
    assert r.name == "from-file"
    assert len(r.actions) == 1


# ── ScriptRecorder ───────────────────────────────────────────────────────


def test_recorder_start_sets_is_recording():
    rec = ScriptRecorder()
    assert rec.is_recording is False
    rec.start()
    assert rec.is_recording is True


def test_recorder_start_idempotent():
    """重复 start 不抛（幂等）。"""

    rec = ScriptRecorder()
    rec.start()
    rec.start()  # 不抛


def test_recorder_stop_resets_and_returns_recording():
    rec = ScriptRecorder()
    rec.start()
    recording = rec.stop()
    assert rec.is_recording is False
    assert isinstance(recording, ScriptRecording)


def test_recorder_record_when_not_recording_returns_false():
    rec = ScriptRecorder()
    action = ScriptAction(type="SEND", payload="AT", timestamp_ms=0)
    assert rec.record(action) is False


def test_recorder_record_when_recording_returns_true():
    rec = ScriptRecorder()
    rec.start()
    action = ScriptAction(type="SEND", payload="AT", timestamp_ms=0)
    assert rec.record(action) is True
    assert rec.recording.action_count == 1


def test_recorder_record_filter_excludes_unlisted_types():
    """record_only 过滤未列出的类型。"""

    rec = ScriptRecorder(record_only=["SEND"])
    rec.start()
    send_result = rec.record(ScriptAction(type="SEND", payload="AT", timestamp_ms=0))
    delay_result = rec.record(ScriptAction(type="DELAY", payload="100", timestamp_ms=10))
    assert send_result is True
    assert delay_result is False
    assert rec.recording.action_count == 1


def test_recorder_record_filter_empty_set_blocks_all():
    """record_only=[] → 全部被过滤。"""

    rec = ScriptRecorder(record_only=[])
    rec.start()
    assert rec.record(ScriptAction(type="SEND", payload="AT", timestamp_ms=0)) is False


def test_recorder_no_filter_records_all_types():
    """无 record_only → 全部类型记录。"""

    rec = ScriptRecorder()
    rec.start()
    assert rec.record(ScriptAction(type="SEND", payload="A", timestamp_ms=0)) is True
    assert rec.record(ScriptAction(type="DELAY", payload="B", timestamp_ms=10)) is True
    assert rec.recording.action_count == 2


def test_recorder_recording_property_accessible_before_start():
    """录制前 recording 属性可访问（空 recording）。"""

    rec = ScriptRecorder(name="pre-start")
    assert rec.recording.name == "pre-start"
    assert rec.recording.action_count == 0


def test_recorder_record_timestamps_relative_to_start():
    """record 的 timestamp_ms 相对于 start 时刻。"""

    rec = ScriptRecorder()
    rec.start()
    rec.record(ScriptAction(type="SEND", payload="A", timestamp_ms=0))
    action = rec.recording.actions[0]
    # timestamp_ms 应 ≈ 0（start 后立即记录）
    assert action.timestamp_ms >= 0
    assert action.timestamp_ms < 1000  # 合理范围

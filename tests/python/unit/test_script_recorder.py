"""脚本录制器单元测试。"""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import time
from embeddebug.serial_station.script_recorder import ScriptAction, ScriptRecorder, ScriptRecording, ScriptPlayer

def test_action_roundtrip():
    a = ScriptAction(ScriptAction.SEND, "AA", 100, "lbl")
    assert ScriptAction.from_dict(a.to_dict()) == a

def test_recorder_sequence():
    rec = ScriptRecorder(name="t")
    rec.start()
    assert rec.record(ScriptAction(ScriptAction.SEND, "01", 0)) is True
    time.sleep(0.005)
    assert rec.record(ScriptAction(ScriptAction.RECEIVE, "OK", 0)) is True
    result = rec.stop()
    assert result.action_count == 2
    stamps = [a.timestamp_ms for a in result.actions]
    assert stamps == sorted(stamps)

def test_recorder_not_recording():
    rec = ScriptRecorder()
    assert rec.record(ScriptAction(ScriptAction.SEND, "x", 0)) is False

def test_recorder_filter():
    rec = ScriptRecorder(record_only={ScriptAction.SEND})
    rec.start()
    assert rec.record(ScriptAction(ScriptAction.SEND, "x", 0)) is True
    assert rec.record(ScriptAction(ScriptAction.RECEIVE, "y", 0)) is False
    assert rec.stop().action_count == 1

def test_recording_json_roundtrip():
    r = ScriptRecording(name="rt", actions=[ScriptAction(ScriptAction.SEND, "AA", 0)])
    text = r.to_json()
    restored = ScriptRecording.from_json(text)
    assert restored.name == "rt" and restored.action_count == 1

def test_recording_duration():
    r = ScriptRecording(actions=[ScriptAction("S", "a", 0), ScriptAction("S", "b", 500)])
    assert r.duration_ms == 500

def test_player_load_and_step(qtbot):
    rec = ScriptRecording(name="p", actions=[ScriptAction(ScriptAction.SEND, "01", 0), ScriptAction(ScriptAction.SEND, "02", 100)])
    player = ScriptPlayer()
    player.load(rec)
    assert player.is_loaded and player.action_count == 2
    received = []
    player.action_ready.connect(received.append)
    assert player.step_next().payload == "01"
    assert player.step_next().payload == "02"
    assert player.step_next() is None
    assert len(received) == 2

def test_player_speed_clamp():
    p = ScriptPlayer()
    p.set_speed(100)
    assert p.speed == ScriptPlayer.MAX_SPEED
    p.set_speed(0.01)
    assert p.speed == ScriptPlayer.MIN_SPEED

def test_player_stop_resets():
    p = ScriptPlayer()
    p.load(ScriptRecording(actions=[ScriptAction("S", "x", 0)]))
    p.step_next()
    p.stop()
    assert p.current_index == 0


# ---- Batch 143: ScriptRecorder/Player/Recording/Action 边界扩展 ----


def test_script_action_constants():
    """ScriptAction 类常量定义 6 种动作类型。"""
    for name in ("SEND", "RECEIVE", "CONNECT", "DISCONNECT", "DELAY", "WAIT"):
        assert getattr(ScriptAction, name) == name


def test_script_action_is_frozen():
    """ScriptAction 是 frozen dataclass。"""
    import pytest
    a = ScriptAction("S", "x", 0)
    with pytest.raises((AttributeError, TypeError)):
        a.payload = "y"


def test_script_action_from_dict_defaults_and_to_dict_fields():
    """from_dict 缺 label 兜底空串；to_dict 输出 4 字段。"""
    a = ScriptAction.from_dict({"type": "S", "payload": "x", "timestamp_ms": 10})
    assert a.label == ""
    full = ScriptAction("S", "data", 100, "lbl")
    d = full.to_dict()
    assert set(d.keys()) == {"type", "payload", "timestamp_ms", "label"}


def test_recording_empty_state():
    """空录制 duration_ms=0, action_count=0。"""
    r = ScriptRecording()
    assert r.duration_ms == 0
    assert r.action_count == 0


def test_recording_to_json_round_trip_preserves_metadata():
    """to_json/from_json round-trip 保留 name/created_at/description。"""
    r = ScriptRecording(
        name="test-session",
        created_at="2026-06-22 10:00:00",
        description="测试描述",
        actions=[ScriptAction("S", "AA", 0, "cmd1")],
    )
    text = r.to_json()
    restored = ScriptRecording.from_json(text)
    assert restored.name == "test-session"
    assert restored.created_at == "2026-06-22 10:00:00"
    assert restored.description == "测试描述"
    assert restored.action_count == 1
    assert restored.actions[0].label == "cmd1"


def test_recording_to_json_writes_to_file(tmp_path):
    """to_json(path) 写文件并返回 None。"""
    r = ScriptRecording(name="f", actions=[ScriptAction("S", "x", 0)])
    path = tmp_path / "rec.json"
    result = r.to_json(path)
    assert result is None
    assert path.exists()
    restored = ScriptRecording.from_json(path)
    assert restored.name == "f"


def test_recording_from_json_missing_actions_defaults_empty():
    """from_json 对缺失 actions 字段用空列表兜底。"""
    import json
    text = json.dumps({"name": "x"})  # 无 actions
    r = ScriptRecording.from_json(text)
    assert r.name == "x"
    assert r.action_count == 0


def test_recorder_start_idempotent_and_stop_resets():
    """start 幂等（不重置 _start）；stop 后 is_recording=False, _start=None。"""
    rec = ScriptRecorder(name="t")
    rec.start()
    first_start = rec._start
    rec.start()  # 二次调用应提前 return
    assert rec._start is first_start
    rec.stop()
    assert rec.is_recording is False
    assert rec._start is None


def test_recorder_record_assigns_relative_timestamp():
    """record 后 action 的 timestamp_ms 是相对 start 的毫秒数（覆盖原值）。"""
    rec = ScriptRecorder(name="t")
    rec.start()
    result = rec.record(ScriptAction(ScriptAction.SEND, "x", 999))
    assert result is True
    stopped = rec.stop()
    assert 0 <= stopped.actions[0].timestamp_ms < 1000


def test_recorder_record_only_with_multiple_types():
    """record_only 多类型过滤器允许所有列出的类型。"""
    rec = ScriptRecorder(record_only={ScriptAction.SEND, ScriptAction.CONNECT})
    rec.start()
    assert rec.record(ScriptAction(ScriptAction.SEND, "x", 0)) is True
    assert rec.record(ScriptAction(ScriptAction.CONNECT, "COM1", 0)) is True
    assert rec.record(ScriptAction(ScriptAction.RECEIVE, "y", 0)) is False
    assert rec.stop().action_count == 2


def test_player_initial_state_and_step_without_load(qtbot):
    """新建 player 未加载；未 load 时 step_next 返回 None + emit finished。"""
    p = ScriptPlayer()
    assert p.is_loaded is False
    assert p.action_count == 0
    assert p.speed == 1.0
    finished = []
    p.finished.connect(lambda: finished.append(True))
    assert p.step_next() is None
    assert len(finished) == 1


def test_player_step_at_end_emits_finished_and_load_resets(qtbot):
    """step_next 越界 emit finished + 停止；load 内部 stop 重置 index。"""
    rec = ScriptRecording(actions=[ScriptAction("S", "x", 0)])
    p = ScriptPlayer()
    p.load(rec)
    p.play()
    finished = []
    p.finished.connect(lambda: finished.append(True))
    p.step_next()
    assert p.step_next() is None  # 越界
    assert len(finished) == 1
    # load 另一个 recording 重置 index
    p.load(ScriptRecording(actions=[ScriptAction("S", "y", 0)]))
    assert p.current_index == 0


def test_player_play_requires_load_and_pause_allows_step(qtbot):
    """play 未 load 不进入播放；pause 后 step_next 仍可推进。"""
    p = ScriptPlayer()
    p.play()  # 无 recording
    assert p.step_next() is None

    rec = ScriptRecording(actions=[ScriptAction("S", "a", 0), ScriptAction("S", "b", 10)])
    p2 = ScriptPlayer()
    p2.load(rec)
    p2.play()
    p2.pause()
    assert p2.step_next().payload == "a"  # pause 不阻断手动 step


def test_player_position_changed_signal(qtbot):
    """step_next 触发 position_changed 信号。"""
    rec = ScriptRecording(actions=[ScriptAction("S", "a", 0), ScriptAction("S", "b", 10)])
    p = ScriptPlayer()
    p.load(rec)
    positions = []
    p.position_changed.connect(lambda i: positions.append(i))
    p.step_next()
    p.step_next()
    assert positions == [1, 2]


def test_player_speed_boundary_values():
    """set_speed 边界值（0.25/1.0/4.0）原值保留。"""
    p = ScriptPlayer()
    for v in (0.25, 1.0, 4.0):
        p.set_speed(v)
        assert p.speed == v

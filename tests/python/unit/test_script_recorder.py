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

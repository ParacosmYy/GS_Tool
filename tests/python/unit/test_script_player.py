"""ScriptPlayer 脚本回放器边界测试。

覆盖（无 transport/IO，纯信号驱动）：
1. MIN_SPEED/MAX_SPEED 常量契约。
2. __init__ 默认状态（未加载 / index 0 / speed 1.0 / 未播放）。
3. is_loaded / action_count / current_index / speed 属性。
4. set_speed clamp（低于 MIN / 高于 MAX / 正常 / 边界值）。
5. load 重置 index + stop 既有播放；action_count 跟随 recording。
6. play / pause / stop 生命周期（play 在未加载无效 / stop 重置 index）。
7. step_next 遍历全部 action + position_changed 递增 + 越界 finished emit + 返回 None。
8. action_ready 信号携带 ScriptAction。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.script_recorder.action import ScriptAction
from embeddebug.serial_station.script_recorder.player import ScriptPlayer
from embeddebug.serial_station.script_recorder.recording import ScriptRecording


def _make_recording(n: int = 3) -> ScriptRecording:
    actions = [
        ScriptAction(type="send", payload=f"cmd{i}", timestamp_ms=i * 100)
        for i in range(n)
    ]
    return ScriptRecording(actions=actions, name="test")


# ── 常量契约 ──────────────────────────────────────────────────────
def test_speed_constants_contract():
    assert ScriptPlayer.MIN_SPEED == 0.25
    assert ScriptPlayer.MAX_SPEED == 4.0
    assert ScriptPlayer.MIN_SPEED < ScriptPlayer.MAX_SPEED


# ── __init__ 默认状态 ────────────────────────────────────────────
def test_init_defaults(qtbot):
    from PyQt6.QtCore import QObject

    parent = QObject()
    player = ScriptPlayer(parent)
    assert player.is_loaded is False
    assert player.action_count == 0
    assert player.current_index == 0
    assert player.speed == 1.0


# ── set_speed clamp ───────────────────────────────────────────────
def test_set_speed_normal(qtbot):
    player = ScriptPlayer()
    player.set_speed(2.0)
    assert player.speed == 2.0


def test_set_speed_below_min_clamped(qtbot):
    player = ScriptPlayer()
    player.set_speed(0.1)
    assert player.speed == ScriptPlayer.MIN_SPEED


def test_set_speed_above_max_clamped(qtbot):
    player = ScriptPlayer()
    player.set_speed(10.0)
    assert player.speed == ScriptPlayer.MAX_SPEED


def test_set_speed_boundary_values(qtbot):
    """边界值 MIN/MAX 应原样接受。"""

    player = ScriptPlayer()
    player.set_speed(ScriptPlayer.MIN_SPEED)
    assert player.speed == ScriptPlayer.MIN_SPEED
    player.set_speed(ScriptPlayer.MAX_SPEED)
    assert player.speed == ScriptPlayer.MAX_SPEED


# ── load ──────────────────────────────────────────────────────────
def test_load_sets_recording_and_resets_index(qtbot):
    player = ScriptPlayer()
    rec = _make_recording(3)
    player.load(rec)
    assert player.is_loaded is True
    assert player.action_count == 3
    assert player.current_index == 0


def test_load_stops_existing_playback(qtbot):
    """load 应先 stop（重置 index + 停止播放）。"""

    player = ScriptPlayer()
    player.load(_make_recording(3))
    player.step_next()
    assert player.current_index == 1
    player.load(_make_recording(2))  # 重新 load
    assert player.current_index == 0
    assert player.action_count == 2


# ── play / pause / stop 生命周期 ─────────────────────────────────
def test_play_without_load_no_effect(qtbot):
    """未加载 play 无效（不抛异常）。"""

    player = ScriptPlayer()
    player.play()  # 不崩溃


def test_play_at_end_no_effect(qtbot):
    """index 已到末尾 play 无效。"""

    player = ScriptPlayer()
    player.load(_make_recording(2))
    player.step_next()
    player.step_next()
    # index == action_count，play 不应推进。
    player.play()


def test_stop_resets_index(qtbot):
    player = ScriptPlayer()
    player.load(_make_recording(3))
    player.step_next()
    assert player.current_index == 1
    player.stop()
    assert player.current_index == 0


# ── step_next 遍历 + 信号 ────────────────────────────────────────
def test_step_next_returns_actions_in_order(qtbot):
    player = ScriptPlayer()
    player.load(_make_recording(3))
    actions = []
    for _ in range(3):
        a = player.step_next()
        assert a is not None
        actions.append(a.payload)
    assert actions == ["cmd0", "cmd1", "cmd2"]


def test_step_next_emits_position_changed(qtbot):
    player = ScriptPlayer()
    player.load(_make_recording(3))
    positions = []
    player.position_changed.connect(lambda i: positions.append(i))
    player.step_next()
    player.step_next()
    assert positions == [1, 2]


def test_step_next_emits_action_ready(qtbot):
    player = ScriptPlayer()
    player.load(_make_recording(1))
    received = []
    player.action_ready.connect(lambda a: received.append(a))
    player.step_next()
    assert len(received) == 1
    assert isinstance(received[0], ScriptAction)


def test_step_next_at_end_emits_finished_and_returns_none(qtbot):
    player = ScriptPlayer()
    player.load(_make_recording(1))
    player.step_next()  # 消耗唯一 action
    finished_emitted = []
    player.finished.connect(lambda: finished_emitted.append(True))
    result = player.step_next()  # 越界
    assert result is None
    assert finished_emitted == [True]


def test_step_next_without_load_returns_none(qtbot):
    """未加载 step_next 返回 None（不抛异常）。"""

    player = ScriptPlayer()
    assert player.step_next() is None

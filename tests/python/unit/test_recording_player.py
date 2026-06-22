"""录制回放器 RecordingPlayer 单元测试 — 属性 + 控制。

覆盖：初始状态、play/pause/stop、set_speed 范围限制、
progress 计算、total_batches/current_index。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def test_player_initial_state(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    assert p.is_playing is False
    assert p.total_batches == 0
    assert p.current_index == 0
    assert p.progress == 0.0
    assert p.speed == 1.0


def test_player_play_pause(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    p.play()
    assert p.is_playing is True
    p.pause()
    assert p.is_playing is False


def test_player_stop_resets(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    p.play()
    p.stop()
    assert p.is_playing is False
    assert p.current_index == 0


def test_player_set_speed_range(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    p.set_speed(2.0)
    assert p.speed == 2.0
    p.set_speed(0.5)
    assert p.speed == 0.5


def test_player_set_speed_clamped_high(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    p.set_speed(10.0)
    assert p.speed == 4.0  # max 4.0


def test_player_set_speed_clamped_low(qapp):
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    p.set_speed(0.01)
    assert p.speed == 0.25  # min 0.25


def test_player_load_invalid_path_returns_false(qapp, tmp_path):
    from embeddebug.serial_station.recording.format import RecordingFormat
    from embeddebug.serial_station.recording.player import RecordingPlayer
    p = RecordingPlayer()
    assert p.load(tmp_path / "nonexistent.csv", RecordingFormat.CSV) is False


def test_player_load_valid_csv(qapp, tmp_path):
    """加载有效 CSV 文件。"""
    from embeddebug.serial_station.recording.format import RecordingFormat, RecordingWriter, RecordingHeader
    from embeddebug.serial_station.recording.player import RecordingPlayer
    import numpy as np
    from embeddebug.serial_station.core.measurements import ChannelBatch

    header = RecordingHeader(start_time_ns=0, dt_ns=10000, channel_names=("ch0",))
    writer = RecordingWriter(RecordingFormat.CSV, header)
    path = tmp_path / "test.csv"
    writer.open(path)
    writer.write_batch(ChannelBatch(channel_names=("ch0",), values=np.array([[1.0]], dtype=np.float32)))
    writer.close()

    p = RecordingPlayer()
    assert p.load(path, RecordingFormat.CSV) is True
    assert p.total_batches >= 1


# ---- Batch 127 边界扩展：step_next / seek / progress / load stop 重置 ----


def _player_with_n_batches(qapp, tmp_path, n: int):
    """构造一个加载了 n 个 batch 的 player（用于 step_next/seek 等测试）。

    JSONL 格式每行一个 batch，iter_batches 一一对应；CSV 会合并成 1 个 batch。
    """
    from embeddebug.serial_station.recording.format import (
        RecordingFormat,
        RecordingHeader,
        RecordingWriter,
    )
    from embeddebug.serial_station.recording.player import RecordingPlayer
    import numpy as np
    from embeddebug.serial_station.core.measurements import ChannelBatch

    header = RecordingHeader(start_time_ns=0, dt_ns=10000, channel_names=("ch0",))
    writer = RecordingWriter(RecordingFormat.JSONL, header)
    path = tmp_path / f"test_{n}.jsonl"
    writer.open(path)
    for i in range(n):
        writer.write_batch(
            ChannelBatch(channel_names=("ch0",), values=np.array([[float(i)]], dtype=np.float32))
        )
    writer.close()

    p = RecordingPlayer()
    assert p.load(path, RecordingFormat.JSONL) is True
    assert p.total_batches == n
    return p


def test_player_step_next_returns_batch_and_advances_index(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 3)
    assert p.current_index == 0
    batch = p.step_next()
    assert batch is not None
    assert p.current_index == 1


def test_player_step_next_emits_batch_available_signal(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 2)
    received = []
    p.batch_available.connect(lambda b: received.append(b))
    p.step_next()
    assert len(received) == 1


def test_player_step_next_emits_position_changed_signal(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 2)
    positions = []
    p.position_changed.connect(lambda i: positions.append(i))
    p.step_next()
    assert positions == [1]


def test_player_step_next_at_end_emits_finished_and_stops(qapp, tmp_path):
    """所有 batch 用尽后 step_next 返回 None，emit finished，并停止播放。"""
    p = _player_with_n_batches(qapp, tmp_path, 1)
    p.play()
    p.step_next()  # 消耗唯一一个 batch
    finished_emitted = []
    p.finished.connect(lambda: finished_emitted.append(True))
    result = p.step_next()  # 越界
    assert result is None
    assert p.is_playing is False
    assert len(finished_emitted) == 1


def test_player_progress_reflects_step_advancement(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 4)
    assert p.progress == 0.0
    p.step_next()
    assert p.progress == 0.25
    p.step_next()
    p.step_next()
    assert p.progress == 0.75


def test_player_seek_clamps_to_valid_range(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 3)
    p.seek(99)  # 超出上界
    assert p.current_index == 3
    p.seek(-5)  # 低于下界
    assert p.current_index == 0
    p.seek(2)  # 合法
    assert p.current_index == 2


def test_player_seek_emits_position_changed(qapp, tmp_path):
    p = _player_with_n_batches(qapp, tmp_path, 5)
    positions = []
    p.position_changed.connect(lambda i: positions.append(i))
    p.seek(3)
    assert positions == [3]


def test_player_load_stops_playback_and_resets_index(qapp, tmp_path):
    """load() 内部调用 stop()，应暂停播放并清零 index。"""
    from embeddebug.serial_station.recording.format import (
        RecordingFormat,
        RecordingHeader,
        RecordingWriter,
    )
    import numpy as np
    from embeddebug.serial_station.core.measurements import ChannelBatch

    p = _player_with_n_batches(qapp, tmp_path, 3)
    p.play()
    p.step_next()
    p.step_next()
    assert p.current_index == 2
    assert p.is_playing is True
    # 准备另一个 JSONL 文件（2 batches）并加载
    header = RecordingHeader(start_time_ns=0, dt_ns=10000, channel_names=("ch0",))
    writer = RecordingWriter(RecordingFormat.JSONL, header)
    path = tmp_path / "test_other.jsonl"
    writer.open(path)
    writer.write_batch(
        ChannelBatch(channel_names=("ch0",), values=np.array([[1.0]], dtype=np.float32))
    )
    writer.close()

    assert p.load(path, RecordingFormat.JSONL) is True
    assert p.is_playing is False
    assert p.current_index == 0


def test_player_step_next_on_empty_returns_none(qapp):
    """未加载任何 batch 时 step_next 返回 None。"""
    from embeddebug.serial_station.recording.player import RecordingPlayer

    p = RecordingPlayer()
    assert p.step_next() is None

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

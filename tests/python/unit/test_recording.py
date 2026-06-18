"""录制模块单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.recording import (
    RecordingExporter,
    RecordingFormat,
    RecordingHeader,
    RecordingPlayer,
    RecordingReader,
    RecordingSegment,
    RecordingTimeline,
    RecordingWriter,
)


def _header() -> RecordingHeader:
    return RecordingHeader(start_time_ns=1_000_000_000, dt_ns=10_000_000, channel_names=("temp", "volt"))


def _batches() -> list[ChannelBatch]:
    names = ("temp", "volt")
    return [
        ChannelBatch(names, np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32), 1_000_000_000, 10_000_000),
        ChannelBatch(names, np.array([[5.0, 6.0], [7.0, 8.0]], dtype=np.float32), 1_020_000_000, 10_000_000),
    ]


@pytest.mark.parametrize("fmt", [RecordingFormat.CSV, RecordingFormat.JSONL])
def test_recording_format_round_trip(fmt, tmp_path):
    ext = "csv" if fmt == RecordingFormat.CSV else "jsonl"
    path = tmp_path / f"rec.{ext}"
    writer = RecordingWriter(fmt, _header())
    writer.open(path)
    for batch in _batches():
        writer.write_batch(batch)
    writer.close()

    reader = RecordingReader(fmt)
    header = reader.open(path)
    assert header.channel_names == ("temp", "volt")
    batches = list(reader.iter_batches())
    reader.close()
    expected = np.vstack([b.values for b in _batches()])
    actual = np.vstack([b.values for b in batches])
    assert np.allclose(actual, expected)


def test_timeline_segments_gaps_seek():
    tl = RecordingTimeline(gap_threshold_ns=5)
    tl.add_segment(RecordingSegment(0, 100, 10))
    tl.add_segment(RecordingSegment(200, 300, 10))
    assert tl.total_duration_ns == 300
    assert len(tl.gaps()) == 1
    seg, offset = tl.seek(250)
    assert seg.start_ns == 200 and offset == 50
    assert tl.seek(150) is None


def test_player_load_step_seek(qtbot, tmp_path):
    path = tmp_path / "rec.jsonl"
    writer = RecordingWriter(RecordingFormat.JSONL, _header())
    writer.open(path)
    for batch in _batches():
        writer.write_batch(batch)
    writer.close()

    player = RecordingPlayer()
    assert player.load(path, RecordingFormat.JSONL) is True
    assert player.total_batches == 2

    received: list[ChannelBatch] = []
    player.batch_available.connect(received.append)
    player.play()
    player.step_next()
    player.step_next()
    assert len(received) == 2
    assert player.progress == 1.0


def test_player_speed_clamps():
    player = RecordingPlayer()
    player.set_speed(10.0)
    assert player.speed == 4.0
    player.set_speed(0.01)
    assert player.speed == 0.25


def test_exporter_csv_to_jsonl(tmp_path):
    source = tmp_path / "src.csv"
    writer = RecordingWriter(RecordingFormat.CSV, _header())
    writer.open(source)
    for batch in _batches():
        writer.write_batch(batch)
    writer.close()

    target = tmp_path / "out.jsonl"
    assert RecordingExporter().export(source, target, RecordingFormat.JSONL) is True
    reader = RecordingReader(RecordingFormat.JSONL)
    reader.open(target)
    batches = list(reader.iter_batches())
    reader.close()
    assert np.allclose(np.vstack([b.values for b in batches]), np.vstack([b.values for b in _batches()]))


def test_exporter_channel_subset(tmp_path):
    source = tmp_path / "src.jsonl"
    writer = RecordingWriter(RecordingFormat.JSONL, _header())
    writer.open(source)
    for batch in _batches():
        writer.write_batch(batch)
    writer.close()

    target = tmp_path / "sub.csv"
    assert RecordingExporter().export(source, target, RecordingFormat.CSV, channels=["volt"]) is True
    reader = RecordingReader(RecordingFormat.CSV)
    header = reader.open(target)
    assert header.channel_names == ("volt",)

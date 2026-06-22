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


# ---- Batch 131: RecordingExporter 失败路径 + _channel_indices 边界 ----


def _write_source(tmp_path, name="src.jsonl", fmt=RecordingFormat.JSONL):
    """写入 _batches() 到 tmp_path/name 并返回路径。"""
    source = tmp_path / name
    writer = RecordingWriter(fmt, _header())
    writer.open(source)
    for batch in _batches():
        writer.write_batch(batch)
    writer.close()
    return source


def test_exporter_returns_false_on_missing_source(tmp_path):
    """源文件不存在 → OSError 被 catch → 返回 False。"""
    target = tmp_path / "out.jsonl"
    result = RecordingExporter().export(
        tmp_path / "nonexistent.jsonl", target, RecordingFormat.JSONL
    )
    assert result is False
    assert not target.exists()  # 失败时不写出


def test_exporter_returns_false_on_unknown_channel(tmp_path):
    """channels 含未知通道名 → KeyError 被 catch → 返回 False。"""
    source = _write_source(tmp_path)
    assert RecordingExporter().export(
        source, tmp_path / "out.csv", RecordingFormat.CSV, channels=["ghost"]
    ) is False


def test_exporter_none_channels_selects_all(tmp_path):
    """channels=None 选择全部通道。"""
    source = _write_source(tmp_path)
    target = tmp_path / "all.csv"
    assert RecordingExporter().export(source, target, RecordingFormat.CSV, channels=None) is True
    header = RecordingReader(RecordingFormat.CSV).open(target)
    assert header.channel_names == ("temp", "volt")


def test_exporter_empty_channels_selects_all(tmp_path):
    """channels=[] 空列表等价于 None（选全部通道）。"""
    source = _write_source(tmp_path)
    target = tmp_path / "empty.csv"
    assert RecordingExporter().export(source, target, RecordingFormat.CSV, channels=[]) is True
    header = RecordingReader(RecordingFormat.CSV).open(target)
    assert header.channel_names == ("temp", "volt")


def test_exporter_multiple_channels_preserves_order(tmp_path):
    """channels 显式指定顺序时按指定顺序输出（不按原始顺序）。"""
    source = _write_source(tmp_path)
    target = tmp_path / "reordered.csv"
    assert RecordingExporter().export(
        source, target, RecordingFormat.CSV, channels=["volt", "temp"]
    ) is True
    header = RecordingReader(RecordingFormat.CSV).open(target)
    assert header.channel_names == ("volt", "temp")


def test_exporter_channel_subset_only_writes_selected_values(tmp_path):
    """通道子集导出时，values 矩阵只包含选定列。"""
    source = _write_source(tmp_path)
    target = tmp_path / "only_volt.jsonl"
    assert RecordingExporter().export(
        source, target, RecordingFormat.JSONL, channels=["volt"]
    ) is True
    reader = RecordingReader(RecordingFormat.JSONL)
    reader.open(target)
    batches = list(reader.iter_batches())
    reader.close()
    values = np.vstack([b.values for b in batches])
    assert values.shape[1] == 1
    assert np.allclose(values.flatten(), [2.0, 4.0, 6.0, 8.0])


def test_exporter_same_format_converts(tmp_path):
    """同格式导出（jsonl→jsonl）也能工作（复制语义）。"""
    source = _write_source(tmp_path)
    target = tmp_path / "copy.jsonl"
    assert RecordingExporter().export(source, target, RecordingFormat.JSONL) is True
    assert target.exists()


def test_exporter_channel_indices_helper_none_and_empty():
    """_channel_indices(header, None/[]) 返回全部索引 [0, 1]。"""
    header = _header()
    assert RecordingExporter._channel_indices(header, None) == [0, 1]
    assert RecordingExporter._channel_indices(header, []) == [0, 1]


def test_exporter_channel_indices_helper_known_names_preserves_order():
    """_channel_indices 按名称查找索引，保留指定顺序。"""
    header = _header()
    assert RecordingExporter._channel_indices(header, ["temp"]) == [0]
    assert RecordingExporter._channel_indices(header, ["volt"]) == [1]
    assert RecordingExporter._channel_indices(header, ["volt", "temp"]) == [1, 0]


def test_exporter_channel_indices_helper_unknown_name_raises_keyerror():
    """未知名称抛 KeyError（在 export 外调用时直接抛，不被 catch）。"""
    with pytest.raises(KeyError):
        RecordingExporter._channel_indices(_header(), ["ghost"])

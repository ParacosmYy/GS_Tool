"""RecordingWriter/RecordingReader context manager + 流式 IO 边界测试。

test_recording_format.py 覆盖 Header + from_extension；test_recording.py 覆盖
round-trip 集成。本文件聚焦 Writer/Reader 的 context manager + open/close +
write_batch 多批 + iter_batches 流式 + close 幂等 + __enter__/__exit__ 契约。

注意：__enter__ 返回 self 但不 open 文件（需显式 open）。
"""

from __future__ import annotations

import json

import numpy as np

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.recording.format import (
    RecordingFormat,
    RecordingHeader,
    RecordingReader,
    RecordingWriter,
)


def _header(names=("temp", "volt")) -> RecordingHeader:
    return RecordingHeader(start_time_ns=1_000_000, dt_ns=10_000, channel_names=names)


def _batch(t0=1_000_000, dt=10_000, values=None):
    if values is None:
        values = np.array([[24.5, 3.3], [25.0, 3.4]], dtype=np.float32)
    return ChannelBatch(channel_names=("temp", "volt"), values=values, t0_ns=t0, dt_ns=dt)


def _write_file(path, fmt, batches):
    w = RecordingWriter(fmt, _header())
    w.open(path)
    for b in batches:
        w.write_batch(b)
    w.close()


# ── RecordingWriter __enter__/__exit__ ───────────────────────────
def test_writer_enter_returns_self():
    """__enter__ 返回 self（不自动 open 文件）。"""

    w = RecordingWriter(RecordingFormat.CSV, _header())
    assert w.__enter__() is w


def test_writer_exit_calls_close(tmp_path):
    """__exit__ 应调用 close（即使 _handle 为 None 也不抛）。"""

    w = RecordingWriter(RecordingFormat.CSV, _header())
    w.__exit__(None, None, None)  # _handle None → 安全


def test_writer_exit_closes_open_handle(tmp_path):
    """open 后 __exit__ 应关闭 handle。"""

    path = tmp_path / "out.csv"
    w = RecordingWriter(RecordingFormat.CSV, _header())
    w.open(path)
    w.write_batch(_batch())
    w.__exit__(None, None, None)
    assert path.read_text(encoding="utf-8").startswith("# EDREC")


# ── RecordingWriter open 写 header ───────────────────────────────
def test_writer_open_writes_csv_header(tmp_path):
    path = tmp_path / "out.csv"
    w = RecordingWriter(RecordingFormat.CSV, _header(("a", "b")))
    w.open(path)
    w.close()
    lines = path.read_text(encoding="utf-8").splitlines()
    assert lines[0].startswith("# EDREC")
    assert json.loads(lines[0][len("# EDREC "):])["channel_names"] == ["a", "b"]
    assert lines[1] == "t_ns,a,b"


def test_writer_open_writes_jsonl_header(tmp_path):
    path = tmp_path / "out.jsonl"
    w = RecordingWriter(RecordingFormat.JSONL, _header(("x",)))
    w.open(path)
    w.close()
    first = json.loads(path.read_text(encoding="utf-8").splitlines()[0])
    assert "__header__" in first


# ── write_batch 多批 ─────────────────────────────────────────────
def test_writer_write_batch_multiple_csv(tmp_path):
    path = tmp_path / "out.csv"
    _write_file(path, RecordingFormat.CSV, [_batch(), _batch(t0=2_000_000)])
    lines = path.read_text(encoding="utf-8").splitlines()
    # header(2) + 2 batches × 2 rows = 6 lines。
    assert len(lines) == 6


def test_writer_write_batch_multiple_jsonl(tmp_path):
    path = tmp_path / "out.jsonl"
    _write_file(path, RecordingFormat.JSONL, [_batch(), _batch(t0=2_000_000)])
    lines = path.read_text(encoding="utf-8").splitlines()
    # header(1) + 2 batches = 3 lines。
    assert len(lines) == 3


# ── close 幂等 ────────────────────────────────────────────────────
def test_writer_close_idempotent(tmp_path):
    path = tmp_path / "out.csv"
    w = RecordingWriter(RecordingFormat.CSV, _header())
    w.open(path)
    w.close()
    w.close()  # 再次不抛
    w.close()


# ── RecordingReader ───────────────────────────────────────────────
def test_reader_open_returns_header(tmp_path):
    path = tmp_path / "out.csv"
    _write_file(path, RecordingFormat.CSV, [_batch()])
    reader = RecordingReader(RecordingFormat.CSV)
    header = reader.open(path)
    assert header.channel_names == ("temp", "volt")
    reader.close()


def test_reader_iter_batches_streaming(tmp_path):
    """两批写入 → 读取合并为连续 batch（reader 按 dt 累积连续行）。"""

    path = tmp_path / "out.csv"
    _write_file(path, RecordingFormat.CSV, [_batch(), _batch(t0=2_000_000)])
    reader = RecordingReader(RecordingFormat.CSV)
    reader.open(path)
    batches = list(reader.iter_batches())
    reader.close()
    # 两批各 2 行 = 4 行总数据（reader 可能合并为 1 个 batch）。
    total_rows = sum(b.values.shape[0] for b in batches)
    assert total_rows == 4


def test_reader_close_idempotent(tmp_path):
    path = tmp_path / "out.csv"
    _write_file(path, RecordingFormat.CSV, [_batch()])
    reader = RecordingReader(RecordingFormat.CSV)
    reader.open(path)
    reader.close()
    reader.close()


# ── 完整往返 CSV ──────────────────────────────────────────────────
def test_csv_roundtrip_preserves_values(tmp_path):
    path = tmp_path / "rt.csv"
    original = np.array([[1.5, 2.5], [3.5, 4.5]], dtype=np.float32)
    _write_file(path, RecordingFormat.CSV, [_batch(values=original)])
    reader = RecordingReader(RecordingFormat.CSV)
    reader.open(path)
    batches = list(reader.iter_batches())
    reader.close()
    assert len(batches) == 1
    np.testing.assert_allclose(batches[0].values, original, rtol=1e-6)


# ── 完整往返 JSONL ────────────────────────────────────────────────
def test_jsonl_roundtrip_preserves_values(tmp_path):
    path = tmp_path / "rt.jsonl"
    original = np.array([[10.0, 20.0], [30.0, 40.0]], dtype=np.float32)
    _write_file(path, RecordingFormat.JSONL, [_batch(values=original)])
    reader = RecordingReader(RecordingFormat.JSONL)
    reader.open(path)
    batches = list(reader.iter_batches())
    reader.close()
    assert len(batches) == 1
    np.testing.assert_allclose(batches[0].values, original, rtol=1e-6)


def test_reader_iter_batches_empty_file(tmp_path):
    """只有 header 无 batch → iter_batches 返回空。"""

    path = tmp_path / "empty.csv"
    w = RecordingWriter(RecordingFormat.CSV, _header())
    w.open(path)
    w.close()
    reader = RecordingReader(RecordingFormat.CSV)
    reader.open(path)
    batches = list(reader.iter_batches())
    reader.close()
    assert batches == []

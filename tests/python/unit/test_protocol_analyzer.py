"""协议分析模块单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.protocol_analyzer import DIRECTION_RX, DIRECTION_TX, AnalysisReport, FrameAnalyzer, ProtocolFrame


def _frame(direction, payload, ts_ns, decoded=None):
    return ProtocolFrame(raw_bytes=payload, timestamp_ns=ts_ns, direction=direction, decoded=decoded or {})


def test_frame_hex_dump():
    f = ProtocolFrame(raw_bytes=b"AB\xff")
    dump = f.hex_dump()
    assert "41" in dump and "42" in dump and "AB." in dump


def test_frame_empty_hex_dump():
    assert "<empty>" in ProtocolFrame(raw_bytes=b"").hex_dump()


def test_frame_to_dict():
    f = ProtocolFrame(raw_bytes=b"\x10", direction=DIRECTION_TX, decoded={"cmd": "READ"})
    d = f.to_dict()
    assert d["raw_hex"] == "10" and d["direction"] == DIRECTION_TX and d["decoded"] == {"cmd": "READ"}


def test_analyzer_feed_and_count():
    a = FrameAnalyzer()
    a.feed(_frame(DIRECTION_TX, b"\x01", 0))
    a.feed(_frame(DIRECTION_RX, b"\x02\x03", 1_000_000))
    assert a.frame_count == 2


def test_analyzer_stats():
    a = FrameAnalyzer()
    a.feed(_frame(DIRECTION_TX, b"\x01\x02", 0))
    a.feed(_frame(DIRECTION_RX, b"\x03\x04\x05", 1_000_000))
    r = a.analyze()
    assert r.stats["frame_count_tx"] == 1 and r.stats["frame_count_rx"] == 1
    assert r.stats["total_bytes"] == 5


def test_analyzer_frequency():
    a = FrameAnalyzer()
    for i in range(5):
        a.feed(_frame(DIRECTION_RX, b"\x00", i * 200_000_000))
    r = a.analyze()
    # 5 frames over 0.8s window → ~6.25 Hz (frequency depends on actual span)
    assert r.stats["frequency_hz"] > 0


def test_analyzer_empty():
    r = FrameAnalyzer().analyze()
    assert r.stats["frame_count"] == 0 and not r.has_errors


def test_error_detection():
    a = FrameAnalyzer(error_patterns={"status": r"ERR"})
    a.feed(_frame(DIRECTION_RX, b"\x01", 0, {"status": "OK"}))
    a.feed(_frame(DIRECTION_RX, b"\x02", 1_000_000, {"status": "ERR_FAIL"}))
    r = a.analyze()
    assert r.has_errors and len(r.errors) == 1 and "ERR" in r.errors[0]


def test_report_format_text():
    r = AnalysisReport(stats={"frame_count": 3}, errors=["e1"], duration_s=0.5)
    text = r.format_text()
    assert "frame_count: 3" in text and "errors: 1" in text


def test_report_to_dict():
    r = AnalysisReport(stats={"x": 1}, errors=["e"])
    d = r.to_dict()
    assert d["has_errors"] is True and d["errors"] == ["e"]

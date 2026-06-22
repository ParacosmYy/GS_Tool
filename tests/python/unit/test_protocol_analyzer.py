"""协议分析模块单元测试。"""

from __future__ import annotations

import os
import time

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


# ---- 边界扩展（Batch 124 补强）----


def test_frame_bytearray_normalized_to_bytes():
    """__post_init__ 把 bytearray 转成 bytes（frozen dataclass 用 object.__setattr__）。"""
    frame = ProtocolFrame(raw_bytes=bytearray(b"data"))
    assert isinstance(frame.raw_bytes, bytes)
    assert frame.raw_bytes == b"data"


def test_frame_frozen_dataclass_rejects_mutation():
    frame = ProtocolFrame(raw_bytes=b"x")
    with pytest.raises((AttributeError, TypeError)):
        frame.direction = DIRECTION_TX


def test_frame_hex_dump_multi_line_offset_increments():
    """>= 17 字节时 offset 按 0x10 递增。"""
    data = bytes(range(20))
    lines = ProtocolFrame(raw_bytes=data).hex_dump().split("\n")
    assert len(lines) == 2
    assert lines[0].startswith("0000:")
    assert lines[1].startswith("0010:")


def test_frame_hex_dump_non_printable_renders_dot():
    """0x00/0x01/0x7F/0xFF 在 ascii 部分渲染为 '.'。"""
    dump = ProtocolFrame(raw_bytes=b"\x00\x01\x7F\xFF").hex_dump()
    # ascii 列是最后一部分
    assert dump.rstrip().endswith("....")


def test_frame_timestamp_defaults_to_recent_now():
    before = time.time_ns()
    frame = ProtocolFrame(raw_bytes=b"")
    after = time.time_ns()
    assert before <= frame.timestamp_ns <= after


def test_frame_size_property_matches_raw_length():
    assert ProtocolFrame(raw_bytes=b"hello").size == 5
    assert ProtocolFrame(raw_bytes=b"").size == 0


def test_analyzer_duration_floored_to_min_duration():
    """时间戳差远小于 _MIN_DURATION_S=1ms 时，duration 被回退到 1ms。"""
    a = FrameAnalyzer()
    now = time.time_ns()
    a.feed(_frame(DIRECTION_RX, b"a", now))
    a.feed(_frame(DIRECTION_RX, b"b", now + 100))  # 100ns 差
    assert a.analyze().duration_s == pytest.approx(1e-3, rel=1e-6)


def test_analyzer_frequency_real_duration():
    """有真实时间差时 frequency = frames / duration。"""
    a = FrameAnalyzer()
    base = time.time_ns()
    for i in range(10):
        a.feed(_frame(DIRECTION_RX, b"x", base + i * 10_000_000))  # 10 帧 / 0.09s
    assert a.analyze().stats["frequency_hz"] == pytest.approx(100.0, rel=0.15)


def test_analyzer_detect_errors_skips_missing_field():
    """frame.decoded 不含该 field 时跳过，不报错。"""
    a = FrameAnalyzer(error_patterns={"ghost": r"FAIL"})
    a.feed(_frame(DIRECTION_RX, b"x", 0, {"cmd": "AT"}))
    assert a.analyze().errors == []


def test_analyzer_detect_errors_handles_non_string_value():
    """decoded 字段非字符串（如 int）时，转 str 后再匹配。"""
    a = FrameAnalyzer(error_patterns={"code": r"404"})
    a.feed(_frame(DIRECTION_RX, b"x", 0, {"code": 404}))
    errors = a.analyze().errors
    assert len(errors) == 1
    assert "404" in errors[0]


def test_analyzer_no_error_patterns_returns_empty_errors():
    """构造时未传 error_patterns，ERROR 字面值也不应报错。"""
    a = FrameAnalyzer()
    a.feed(_frame(DIRECTION_RX, b"x", 0, {"cmd": "ERROR"}))
    r = a.analyze()
    assert r.errors == []
    assert r.stats["protocol_errors"] == 0


def test_analyzer_clear_resets_frame_count():
    a = FrameAnalyzer()
    a.feed(_frame(DIRECTION_TX, b"a", 0))
    a.clear()
    assert a.frame_count == 0


def test_report_format_text_renders_floats_with_three_decimals():
    """stats 中的 float 值用 .3f 渲染。"""
    r = AnalysisReport(stats={"avg_frame_size": 20.0, "frequency_hz": 50.5})
    text = r.format_text()
    assert "avg_frame_size: 20.000" in text
    assert "frequency_hz: 50.500" in text


def test_report_format_text_missing_stat_keys_render_zero():
    """缺失 stat key 时用 .get(key, 0) → 显示 0。"""
    text = AnalysisReport().format_text()
    assert "frame_count: 0" in text
    assert "errors: 0" in text


def test_report_has_errors_reflects_list_state():
    assert AnalysisReport(errors=["e"]).has_errors is True
    assert AnalysisReport(errors=[]).has_errors is False


def test_report_to_dict_default_empty():
    d = AnalysisReport().to_dict()
    assert d == {"stats": {}, "errors": [], "duration_s": 0.0, "has_errors": False}

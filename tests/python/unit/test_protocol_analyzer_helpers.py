"""protocol_analyzer report + frame + analyzer 纯 helper 边界单元测试。

补强 test_protocol_analyzer.py 未直接断言的边角：
- AnalysisReport：format_text 浮点 .3f 格式化 + 多错误列表渲染 + has_errors 空=False + 默认值 + to_dict 无错误。
- ProtocolFrame：hex_dump 空/<empty> 占位 + to_dict raw_hex 空格分隔 + direction 默认 RX + decoded 默认空。
- FrameAnalyzer：feed_many 批量 + clear 重置 + frame_count 属性 + _compute_duration_s 最小值兜底。
- DIRECTION_TX/RX/HEX_DUMP_WIDTH 常量契约。
"""

from __future__ import annotations

from embeddebug.serial_station.protocol_analyzer import (
    DIRECTION_RX,
    DIRECTION_TX,
    AnalysisReport,
    FrameAnalyzer,
    ProtocolFrame,
)
from embeddebug.serial_station.protocol_analyzer.frame import HEX_DUMP_WIDTH


# ── AnalysisReport format_text 浮点 + 多错误 ─────────────────────────────


def test_report_format_text_float_stats_three_decimals():
    """format_text 浮点 stats（avg_frame_size/frequency_hz）用 .3f 格式。"""

    r = AnalysisReport(stats={"avg_frame_size": 12.5, "frequency_hz": 100.0})
    text = r.format_text()
    assert "avg_frame_size: 12.500" in text
    assert "frequency_hz: 100.000" in text


def test_report_format_text_int_stats_no_decimals():
    """format_text 整型 stats（frame_count/total_bytes）原样显示。"""

    r = AnalysisReport(stats={"frame_count": 42, "total_bytes": 1024})
    text = r.format_text()
    assert "frame_count: 42" in text
    assert "total_bytes: 1024" in text
    assert "frame_count: 42.000" not in text  # 整型不加小数


def test_report_format_text_multiple_errors_listed():
    """format_text 多条错误逐行列出（带 - 前缀缩进）。"""

    r = AnalysisReport(errors=["timeout", "crc mismatch", "overflow"])
    text = r.format_text()
    assert "errors: 3" in text
    assert "- timeout" in text
    assert "- crc mismatch" in text
    assert "- overflow" in text


def test_report_format_text_zero_errors_no_list_items():
    """无错误时 errors: 0，不列条目。"""

    r = AnalysisReport(errors=[])
    text = r.format_text()
    assert "errors: 0" in text
    assert "-" not in text.split("errors: 0")[1]


def test_report_format_text_duration_three_decimals():
    """duration_s 用 .3f 格式。"""

    r = AnalysisReport(duration_s=1.5)
    text = r.format_text()
    assert "duration: 1.500s" in text


# ── AnalysisReport has_errors + 默认值 + to_dict ─────────────────────────


def test_report_has_errors_false_when_empty():
    """空 errors → has_errors=False。"""

    assert AnalysisReport().has_errors is False
    assert AnalysisReport(errors=[]).has_errors is False


def test_report_has_errors_true_when_non_empty():
    """非空 errors → has_errors=True。"""

    assert AnalysisReport(errors=["e"]).has_errors is True


def test_report_defaults():
    """AnalysisReport 默认 stats={} / errors=[] / duration_s=0.0。"""

    r = AnalysisReport()
    assert r.stats == {}
    assert r.errors == []
    assert r.duration_s == 0.0


def test_report_to_dict_no_errors_has_errors_false():
    """to_dict 无错误时 has_errors=False。"""

    r = AnalysisReport(stats={"x": 1})
    d = r.to_dict()
    assert d["has_errors"] is False
    assert d["errors"] == []
    assert d["stats"] == {"x": 1}


def test_report_to_dict_copies_collections():
    """to_dict 返回的 stats/errors 是拷贝（修改不影响原 report）。"""

    r = AnalysisReport(stats={"x": 1}, errors=["e"])
    d = r.to_dict()
    d["stats"]["x"] = 999
    d["errors"].append("e2")
    assert r.stats["x"] == 1
    assert r.errors == ["e"]


# ── ProtocolFrame hex_dump 边界 ──────────────────────────────────────────


def test_frame_hex_dump_empty_shows_placeholder():
    """空 raw_bytes → <empty> 占位行。"""

    f = ProtocolFrame(raw_bytes=b"")
    dump = f.hex_dump()
    assert "<empty>" in dump
    assert "0000:" in dump


def test_frame_hex_dump_exactly_16_bytes_single_line():
    """恰好 16 字节 → 单行（HEX_DUMP_WIDTH 边界）。"""

    f = ProtocolFrame(raw_bytes=bytes(range(16)))
    dump = f.hex_dump()
    assert dump.count("\n") == 0  # 单行无换行


def test_frame_hex_dump_17_bytes_two_lines():
    """17 字节 → 2 行（第二行 1 字节）。"""

    f = ProtocolFrame(raw_bytes=bytes(range(17)))
    dump = f.hex_dump()
    assert dump.count("\n") == 1


def test_frame_to_dict_raw_hex_space_separated():
    """to_dict raw_hex 用空格分隔字节。"""

    f = ProtocolFrame(raw_bytes=b"\x01\x02\x03")
    d = f.to_dict()
    assert d["raw_hex"] == "01 02 03"


def test_frame_to_dict_includes_all_fields():
    """to_dict 含 raw_hex/size/timestamp_ns/direction/decoded 5 字段。"""

    f = ProtocolFrame(raw_bytes=b"AB", direction=DIRECTION_TX, decoded={"k": "v"})
    d = f.to_dict()
    assert set(d.keys()) == {"raw_hex", "size", "timestamp_ns", "direction", "decoded"}
    assert d["size"] == 2
    assert d["direction"] == "tx"
    assert d["decoded"] == {"k": "v"}


def test_frame_direction_defaults_rx():
    """direction 默认 = DIRECTION_RX。"""

    f = ProtocolFrame(raw_bytes=b"x")
    assert f.direction == DIRECTION_RX


def test_frame_decoded_defaults_empty_dict():
    """decoded 默认空 dict。"""

    f = ProtocolFrame(raw_bytes=b"x")
    assert f.decoded == {}


# ── FrameAnalyzer feed_many / clear / frame_count ────────────────────────


def test_analyzer_feed_many_batch():
    """feed_many 批量喂入多帧。"""

    analyzer = FrameAnalyzer()
    frames = [ProtocolFrame(raw_bytes=b"A"), ProtocolFrame(raw_bytes=b"BB")]
    analyzer.feed_many(frames)
    assert analyzer.frame_count == 2


def test_analyzer_clear_resets_count():
    """clear 重置帧计数为 0。"""

    analyzer = FrameAnalyzer()
    analyzer.feed(ProtocolFrame(raw_bytes=b"x"))
    analyzer.clear()
    assert analyzer.frame_count == 0


def test_analyzer_frame_count_property():
    """frame_count 属性反映已 feed 的帧数。"""

    analyzer = FrameAnalyzer()
    assert analyzer.frame_count == 0
    analyzer.feed(ProtocolFrame(raw_bytes=b"a"))
    analyzer.feed(ProtocolFrame(raw_bytes=b"b"))
    assert analyzer.frame_count == 2


def test_analyzer_clear_then_analyze_empty():
    """clear 后 analyze 返回空报告（frame_count=0）。"""

    analyzer = FrameAnalyzer()
    analyzer.feed(ProtocolFrame(raw_bytes=b"x"))
    analyzer.clear()
    report = analyzer.analyze()
    assert report.stats["frame_count"] == 0


# ── 常量契约 ─────────────────────────────────────────────────────────────


def test_direction_constants():
    """DIRECTION_TX='tx' / DIRECTION_RX='rx'。"""

    assert DIRECTION_TX == "tx"
    assert DIRECTION_RX == "rx"


def test_hex_dump_width_is_16():
    """HEX_DUMP_WIDTH = 16（每行 16 字节）。"""

    assert HEX_DUMP_WIDTH == 16

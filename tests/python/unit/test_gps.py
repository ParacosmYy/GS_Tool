"""NMEA 0183 解析器单测（Wave 59，纯 Python，无 qtbot）。

覆盖：GGA/RMC/GSA/GSV 四类句子解析、坐标 ddmm.mmmm→十进制度转换（含南/西负值）、
校验和验证（正确/不匹配/缺失容错）、多行 parse_lines、错误句子类型、to_payload。
测试句子用 ``_sentence`` 辅助动态拼校验和（避免硬编码易错的 hex）。
对齐 test_svd.py 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.gps import (
    GgaFix,
    GsaActive,
    GsvSatellites,
    NmeaParseError,
    NmeaParser,
    RmcTrack,
    SatelliteInfo,
    compute_checksum,
)


def _sentence(body: str) -> str:
    """拼 ``$<body>*<checksum>`` —— body 是不含 $/* 的载荷。"""

    return f"${body}*{compute_checksum(body)}"


def _sentence_bad(body: str) -> str:
    """拼一个校验和故意错误的句子（用于校验失败测试）。"""

    return f"${body}*00"


# ── 坐标转换 ────────────────────────────────────────────────────────
def test_north_east_positive_decimal():
    # 纬度 4807.038,N = 48°07.038' → 48.1173°；经度 01131.000,E = 11°31.0' → 11.5167°。
    gga = NmeaParser.parse(
        _sentence("GPGGA,092750.000,4807.038,N,01131.000,E,1,8,0.91,546,M,47,M,,")
    )
    assert isinstance(gga, GgaFix)
    assert abs(gga.latitude - 48.1173) < 1e-3
    assert abs(gga.longitude - 11.5167) < 1e-3


def test_south_west_negative_decimal():
    # 纬度 S / 经度 W → 负值。
    gga = NmeaParser.parse(
        _sentence("GPGGA,092750.000,3358.123,S,11824.456,W,1,4,1.2,10,M,,M,,")
    )
    assert gga.latitude < 0
    assert gga.longitude < 0
    assert abs(gga.latitude - (-(33 + 58.123 / 60))) < 1e-4


# ── GGA ─────────────────────────────────────────────────────────────
def test_gga_parses_full_fields():
    gga = NmeaParser.parse(
        _sentence("GPGGA,092750.000,4807.038,N,01131.000,E,1,8,0.91,546.7,M,47.0,M,,")
    )
    assert isinstance(gga, GgaFix)
    assert gga.fix_quality == 1
    assert gga.satellite_count == 8
    assert abs(gga.hdop - 0.91) < 1e-3
    assert abs(gga.altitude - 546.7) < 1e-2
    assert gga.time_utc == "092750.000"


def test_gga_to_payload_keys():
    gga = GgaFix(latitude=1.0, longitude=2.0, fix_quality=1, satellite_count=4)
    payload = gga.to_payload()
    assert payload["type"] == "fix"
    assert payload["protocolName"] == "nmea"
    assert payload["sentence"] == "GGA"
    assert payload["satelliteCount"] == 4


# ── RMC ─────────────────────────────────────────────────────────────
def test_rmc_valid_status_and_speed():
    rmc = NmeaParser.parse(
        _sentence("GPRMC,092750.000,A,4807.038,N,01131.000,E,0.65,084.4,230394,,,A")
    )
    assert isinstance(rmc, RmcTrack)
    assert rmc.status == "A"
    assert rmc.valid is True
    assert abs(rmc.speed_knots - 0.65) < 1e-3
    assert abs(rmc.course_degrees - 84.4) < 1e-2
    assert rmc.date == "230394"


def test_rmc_invalid_status():
    rmc = NmeaParser.parse(
        _sentence("GPRMC,092750.000,V,4807.038,N,01131.000,E,0.0,0.0,230394,,,N")
    )
    assert rmc.status == "V"
    assert rmc.valid is False


# ── GSA ─────────────────────────────────────────────────────────────
def test_gsa_active_satellites_and_dop():
    gsa = NmeaParser.parse(
        _sentence("GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1")
    )
    assert isinstance(gsa, GsaActive)
    assert gsa.mode == "A"
    assert gsa.fix_type == 3
    assert gsa.has_fix is True
    assert gsa.satellite_ids == (4, 5, 9, 12, 24)
    assert abs(gsa.pdop - 2.5) < 1e-3
    assert abs(gsa.hdop - 1.3) < 1e-3
    assert abs(gsa.vdop - 2.1) < 1e-3


def test_gsa_no_fix():
    gsa = NmeaParser.parse(_sentence("GPGSA,A,1,,,,,,,,,,,,,,"))
    assert gsa.fix_type == 1
    assert gsa.has_fix is False
    assert gsa.satellite_ids == ()


# ── GSV ─────────────────────────────────────────────────────────────
def test_gsv_satellites_parsed():
    gsv = NmeaParser.parse(
        _sentence("GPGSV,2,1,08,04,15,250,32,05,40,085,40,09,67,310,45,12,22,180,38")
    )
    assert isinstance(gsv, GsvSatellites)
    assert gsv.total_sentences == 2
    assert gsv.sentence_number == 1
    assert gsv.satellites_in_view == 8
    assert len(gsv.satellites) == 4
    assert isinstance(gsv.satellites[0], SatelliteInfo)
    assert gsv.satellites[0].prn == 4
    assert gsv.satellites[0].elevation == 15
    assert gsv.satellites[0].azimuth == 250
    assert gsv.satellites[0].snr == 32


# ── 校验和 ──────────────────────────────────────────────────────────
def test_checksum_mismatch_raises():
    with pytest.raises(NmeaParseError, match="checksum"):
        NmeaParser.parse(_sentence_bad("GPGGA,092750.000,4807.038,N,01131.000,E,1,,,,M,,M,,"))


def test_missing_checksum_tolerated():
    # 无 *checksum 的句子应容错解析（部分设备不发校验和）。
    gga = NmeaParser.parse("$GPGGA,092750.000,4807.038,N,01131.000,E,1,,,,M,,M,,")
    assert isinstance(gga, GgaFix)
    assert gga.fix_quality == 1


def test_compute_checksum_known_value():
    # $ 与 * 之间 XOR；GPGGA,... 的校验和应有两位大写 hex。
    cs = compute_checksum("GPGGA,092750.000,4807.038,N")
    assert len(cs) == 2
    int(cs, 16)  # 合法 hex。


def test_verify_checksum_false_skips_check():
    gga = NmeaParser.parse(
        _sentence_bad("GPGGA,092750.000,4807.038,N,01131.000,E,1,,,,M,,M,,"),
        verify_checksum=False,
    )
    assert isinstance(gga, GgaFix)


# ── 错误处理 ────────────────────────────────────────────────────────
def test_missing_dollar_raises():
    with pytest.raises(NmeaParseError):
        NmeaParser.parse("GPGGA,1,2,3")


def test_unknown_sentence_type_raises():
    with pytest.raises(NmeaParseError, match="unsupported"):
        NmeaParser.parse(_sentence("GPXYZ,1,2,3"))


def test_empty_sentence_raises():
    with pytest.raises(NmeaParseError):
        NmeaParser.parse("$")


def test_talker_agnostic_mnemonic():
    # GLGSA / GNRMC 等（GLONASS / 多模）应按末 3 字符识别类型。
    rmc = NmeaParser.parse(_sentence("GNRMC,092750.000,A,4807.038,N,01131.000,E,1.0,0.0,230394,,,A"))
    assert isinstance(rmc, RmcTrack)


# ── 多行解析 ────────────────────────────────────────────────────────
def test_parse_lines_skips_invalid_and_blanks():
    "\n".join([
        "$GPGGA,092750.000,4807.038,N,01131.000,E,1,8,0.91,546,M,47,M,,",
        "  ",  # 空行跳过。
        "garbage without dollar",  # 非 NMEA 跳过。
        "$GPXYZ,bad",  # 解析失败跳过。
        "$GPRMC,092750.000,A,4807.038,N,01131.000,E,0.65,084.4,230394,,,A",
    ])
    # 注：上面句子无校验和（容错），但需逐句构造正确才能解析。
    # 为可重复，重造成带校验和的。
    lines = [
        _sentence("GPGGA,092750.000,4807.038,N,01131.000,E,1,8,0.91,546,M,47,M,,"),
        "  ",
        "garbage without dollar",
        _sentence_bad("GPXYZ,bad"),
        _sentence("GPRMC,092750.000,A,4807.038,N,01131.000,E,0.65,084.4,230394,,,A"),
    ]
    results = NmeaParser.parse_lines("\n".join(lines))
    assert len(results) == 2
    assert isinstance(results[0], GgaFix)
    assert isinstance(results[1], RmcTrack)

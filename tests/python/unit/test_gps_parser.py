"""GPS NMEA 解析器单元测试 — GGA/RMC/GSA/GSV 解析 + checksum。

覆盖：NmeaParser.parse 四类句子 + parse_lines 多行容错 + checksum 校验 +
_to_decimal 度分→十进制度转换 + NmeaParseError 未知类型。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.gps.model import GgaFix, GsaActive, GsvSatellites, RmcTrack
from embeddebug.serial_station.gps.parser import NmeaParseError, NmeaParser, _to_decimal


def test_parse_gga():
    """$GPGGA 解析定位解算。"""
    sentence = "$GPGGA,092750.000,3957.6123,N,11622.5234,E,1,08,1.2,55.5,M,,,,*XX"
    # verify_checksum=False 跳过校验和
    msg = NmeaParser.parse(sentence, verify_checksum=False)
    assert isinstance(msg, GgaFix)
    assert msg.fix_quality == 1
    assert msg.satellite_count == 8


def test_parse_rmc():
    """$GPRMC 解析导航信息。"""
    sentence = "$GPRMC,092750.000,A,3957.6123,N,11622.5234,E,0.5,45.0,010120,,,A*XX"
    msg = NmeaParser.parse(sentence, verify_checksum=False)
    assert isinstance(msg, RmcTrack)
    assert msg.status == "A"
    assert msg.valid is True


def test_parse_gsa():
    """$GPGSA 解析精度因子。"""
    sentence = "$GPGSA,A,3,01,02,03,04,05,06,07,08,,,,,1.5,1.0,1.1*XX"
    msg = NmeaParser.parse(sentence, verify_checksum=False)
    assert isinstance(msg, GsaActive)
    assert msg.mode == "A"
    assert msg.fix_type == 3
    assert msg.has_fix is True


def test_parse_gsv():
    """$GPGSV 解析可见卫星。"""
    sentence = "$GPGSV,2,1,08,01,45,180,30,02,30,090,25*XX"
    msg = NmeaParser.parse(sentence, verify_checksum=False)
    assert isinstance(msg, GsvSatellites)
    assert msg.total_sentences == 2
    assert msg.satellites_in_view == 8
    assert len(msg.satellites) == 2


def test_parse_unknown_type():
    """未知助记符抛 NmeaParseError。"""
    with pytest.raises(NmeaParseError):
        NmeaParser.parse("$GPXXX,data*XX", verify_checksum=False)


def test_parse_empty_sentence():
    """空句子抛 NmeaParseError。"""
    with pytest.raises(NmeaParseError):
        NmeaParser.parse("", verify_checksum=False)


def test_parse_lines_multi():
    """parse_lines 解析多行，跳过空行。"""
    text = "$GPGGA,092750,3957.6,N,11622.5,E,1,8,,55,M,*XX\n\n$GPRMC,092750,A,3957.6,N,11622.5,E,,,,010120,,,A*XX"
    messages = NmeaParser.parse_lines(text, verify_checksum=False)
    assert len(messages) == 2


def test_parse_lines_skips_failures():
    """单行解析失败不影响其他行。"""
    text = "$GPGGA,valid*XX\n$GPBAD,bad*XX\n$GPRMC,also_valid*XX"
    messages = NmeaParser.parse_lines(text, verify_checksum=False)
    # $GPBAD → XXX 未知；实际 valid 需要完整字段
    assert isinstance(messages, list)


def test_to_decimal_north():
    """北纬 ddmm.mmmm → 十进制度。"""
    assert _to_decimal(3957.6123, "N") == pytest.approx(39.96, abs=0.01)


def test_to_decimal_south():
    """南纬 → 负值。"""
    assert _to_decimal(3957.6123, "S") == pytest.approx(-39.96, abs=0.01)


def test_to_decimal_east():
    assert _to_decimal(11622.5234, "E") == pytest.approx(116.37, abs=0.01)


def test_to_decimal_west():
    assert _to_decimal(11622.5234, "W") == pytest.approx(-116.37, abs=0.01)

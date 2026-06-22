"""GPS NMEA 数据模型单元测试 — to_payload + 属性。

覆盖：GgaFix/RmcTrack/GsaActive/SatelliteInfo/GsvSatellites 的 to_payload
输出 camelCase 键、frozen 不可变、valid/has_fix 属性。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.gps.model import (
    GgaFix,
    GsaActive,
    GsvSatellites,
    RmcTrack,
    SatelliteInfo,
)


def test_gga_fix_to_payload():
    fix = GgaFix(latitude=39.9, longitude=116.4, fix_quality=1, satellite_count=8)
    payload = fix.to_payload()
    assert payload["type"] == "fix"
    assert payload["protocolName"] == "nmea"
    assert payload["sentence"] == "GGA"
    assert payload["latitude"] == 39.9
    assert payload["fixQuality"] == 1
    assert payload["satelliteCount"] == 8


def test_rmc_track_valid_property():
    valid = RmcTrack(latitude=0, longitude=0, status="A")
    invalid = RmcTrack(latitude=0, longitude=0, status="V")
    assert valid.valid is True
    assert invalid.valid is False


def test_rmc_track_to_payload_camel_case():
    track = RmcTrack(latitude=1.0, longitude=2.0, status="A", speed_knots=12.5, course_degrees=90.0)
    payload = track.to_payload()
    assert payload["speedKnots"] == 12.5
    assert payload["courseDegrees"] == 90.0


def test_gsa_active_has_fix():
    no_fix = GsaActive(mode="A", fix_type=1)
    fix_2d = GsaActive(mode="A", fix_type=2)
    fix_3d = GsaActive(mode="A", fix_type=3)
    assert no_fix.has_fix is False
    assert fix_2d.has_fix is True
    assert fix_3d.has_fix is True


def test_gsa_active_to_payload_satellite_ids():
    gsa = GsaActive(mode="A", fix_type=3, satellite_ids=(1, 2, 3), pdop=1.5)
    payload = gsa.to_payload()
    assert payload["satelliteIds"] == [1, 2, 3]
    assert payload["fixType"] == 3


def test_satellite_info_to_payload():
    sat = SatelliteInfo(prn=7, elevation=45, azimuth=180, snr=30)
    payload = sat.to_payload()
    assert payload == {"prn": 7, "elevation": 45, "azimuth": 180, "snr": 30}


def test_gsv_satellites_to_payload_nested():
    sats = (SatelliteInfo(prn=1, elevation=10, azimuth=20, snr=15),)
    gsv = GsvSatellites(total_sentences=2, sentence_number=1, satellites_in_view=8, satellites=sats)
    payload = gsv.to_payload()
    assert payload["satellites"] == [{"prn": 1, "elevation": 10, "azimuth": 20, "snr": 15}]
    assert payload["totalSentences"] == 2


def test_all_models_frozen():
    fix = GgaFix(latitude=0, longitude=0, fix_quality=1)
    with pytest.raises((AttributeError, TypeError)):
        fix.latitude = 1.0

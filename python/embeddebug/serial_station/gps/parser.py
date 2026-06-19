"""NMEA 0183 句子解析器（纯 Python，仅依赖标准库）。

入口 ``NmeaParser.parse(sentence)`` 把一句 NMEA（如 ``$GPGGA,...*CS``）转成对应
消息类型（``GgaFix``/``RmcTrack``/``GsaActive``/``GsvSatellites``）。覆盖 GGA/RMC/
GSA/GSV 四类常用句子。校验和（``$`` 与 ``*`` 之间字节的 XOR）默认验证，不匹配抛
``ValueError``。

约定（对齐 ``svd/parser.py`` / ``can/dbc.py``）：``@classmethod parse`` 公开入口，
``_parse_*`` 私有 helper，绝对 import。不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

from embeddebug.serial_station.gps.model import (
    GgaFix,
    GsaActive,
    GsvSatellites,
    NmeaMessage,
    RmcTrack,
    SatelliteInfo,
)


class NmeaParseError(ValueError):
    """NMEA 句子不合法（格式错误 / 校验和失败 / 未知句子类型）。"""


def _verify_checksum(sentence: str) -> str:
    """校验 ``$...*HH`` 校验和，返回去掉 ``$``/``*HH`` 的载荷部分。

    - 缺 ``$`` 前缀或 ``*`` 分隔 → 不校验直接返回（容错，部分设备不发校验和）。
    - 有校验和但 XOR 不匹配 → ``NmeaParseError``。
    """

    sentence = sentence.strip()
    if not sentence.startswith("$"):
        raise NmeaParseError("NMEA sentence must start with '$'")
    body = sentence[1:]
    if "*" not in body:
        return body  # 无校验和，容错。
    payload, _, checksum_hex = body.partition("*")
    expected = _xor_checksum(payload)
    try:
        actual = int(checksum_hex.strip()[:2], 16)
    except ValueError as exc:
        raise NmeaParseError(f"invalid checksum hex: {checksum_hex!r}") from exc
    if actual != expected:
        raise NmeaParseError(
            f"checksum mismatch: got 0x{actual:02X}, expected 0x{expected:02X}"
        )
    return payload


def _xor_checksum(payload: str) -> int:
    """``payload`` 字节逐字符 XOR → 校验和（0-255）。"""

    result = 0
    for char in payload:
        result ^= ord(char)
    return result & 0xFF


def _fields(payload: str) -> list[str]:
    """按逗号切分载荷字段。"""

    return payload.split(",")


def _int_or(field: str, default: int = 0) -> int:
    """字段转 int，空/非法 → default。"""

    field = field.strip()
    if not field:
        return default
    try:
        return int(field)
    except ValueError:
        return default


def _float_or(field: str, default: float = 0.0) -> float:
    """字段转 float，空/非法 → default。"""

    field = field.strip()
    if not field:
        return default
    try:
        return float(field)
    except ValueError:
        return default


def _to_decimal(coord: float, hemisphere: str) -> float:
    """``ddmm.mmmm`` → 十进制度；南/西取负。

    - ``coord``：原始数值（度度分分.分分分分）。
    - ``hemisphere``：N/S（纬度）或 E/W（经度）。
    """

    if coord == 0.0:
        return 0.0
    degrees = int(coord // 100)
    minutes = coord - degrees * 100
    decimal = degrees + minutes / 60.0
    if hemisphere in ("S", "W"):
        decimal = -decimal
    return decimal


def _parse_gga(fields: list[str]) -> GgaFix:
    # $GPGGA,time,lat,N,lon,E,quality,sats,hdop,alt,M,...
    time_utc = fields[1] if len(fields) > 1 else ""
    lat = _float_or(fields[2]) if len(fields) > 2 else 0.0
    ns = fields[3] if len(fields) > 3 else ""
    lon = _float_or(fields[4]) if len(fields) > 4 else 0.0
    ew = fields[5] if len(fields) > 5 else ""
    quality = _int_or(fields[6]) if len(fields) > 6 else 0
    sats = _int_or(fields[7]) if len(fields) > 7 else 0
    hdop = _float_or(fields[8]) if len(fields) > 8 else 0.0
    alt = _float_or(fields[9]) if len(fields) > 9 else 0.0
    return GgaFix(
        latitude=_to_decimal(lat, ns),
        longitude=_to_decimal(lon, ew),
        fix_quality=quality,
        satellite_count=sats,
        hdop=hdop,
        altitude=alt,
        time_utc=time_utc,
    )


def _parse_rmc(fields: list[str]) -> RmcTrack:
    # $GPRMC,time,status,lat,N,lon,E,speed,course,date,...
    time_utc = fields[1] if len(fields) > 1 else ""
    status = (fields[2] if len(fields) > 2 else "V").upper() or "V"
    lat = _float_or(fields[3]) if len(fields) > 3 else 0.0
    ns = fields[4] if len(fields) > 4 else ""
    lon = _float_or(fields[5]) if len(fields) > 5 else 0.0
    ew = fields[6] if len(fields) > 6 else ""
    speed = _float_or(fields[7]) if len(fields) > 7 else 0.0
    course = _float_or(fields[8]) if len(fields) > 8 else 0.0
    date = fields[9] if len(fields) > 9 else ""
    return RmcTrack(
        latitude=_to_decimal(lat, ns),
        longitude=_to_decimal(lon, ew),
        status=status,
        speed_knots=speed,
        course_degrees=course,
        date=date,
        time_utc=time_utc,
    )


def _parse_gsa(fields: list[str]) -> GsaActive:
    # $GPGSA,mode,fix,sv1..sv12,pdop,hdop,vdop
    mode = fields[1] if len(fields) > 1 else "A"
    fix_type = _int_or(fields[2]) if len(fields) > 2 else 1
    sat_ids: list[int] = []
    for i in range(3, 15):  # 卫星 PRN 占字段 3-14（最多 12 颗）。
        if i < len(fields):
            prn = _int_or(fields[i], -1)
            if prn > 0:
                sat_ids.append(prn)
    pdop = _float_or(fields[15]) if len(fields) > 15 else 0.0
    hdop = _float_or(fields[16]) if len(fields) > 16 else 0.0
    vdop = _float_or(fields[17]) if len(fields) > 17 else 0.0
    return GsaActive(
        mode=mode,
        fix_type=fix_type,
        satellite_ids=tuple(sat_ids),
        pdop=pdop,
        hdop=hdop,
        vdop=vdop,
    )


def _parse_gsv(fields: list[str]) -> GsvSatellites:
    # $GPGSV,total,num,inview,sv1,el1,az1,snr1,sv2,...
    total = _int_or(fields[1]) if len(fields) > 1 else 0
    num = _int_or(fields[2]) if len(fields) > 2 else 0
    inview = _int_or(fields[3]) if len(fields) > 3 else 0
    sats: list[SatelliteInfo] = []
    # 字段 4 起每 4 个字段一颗卫星（prn, elevation, azimuth, snr）。
    idx = 4
    while idx + 3 < len(fields):
        prn = _int_or(fields[idx], -1)
        if prn <= 0:
            idx += 4
            continue
        elevation = _int_or(fields[idx + 1])
        azimuth = _int_or(fields[idx + 2])
        snr = _int_or(fields[idx + 3])
        sats.append(SatelliteInfo(prn=prn, elevation=elevation, azimuth=azimuth, snr=snr))
        idx += 4
    return GsvSatellites(
        total_sentences=total,
        sentence_number=num,
        satellites_in_view=inview,
        satellites=tuple(sats),
    )


_SENTENCE_PARSERS = {
    "GGA": _parse_gga,
    "RMC": _parse_rmc,
    "GSA": _parse_gsa,
    "GSV": _parse_gsv,
}


class NmeaParser:
    """NMEA 0183 解析器。无实例状态，``parse`` 为类方法入口。"""

    @classmethod
    def parse(cls, sentence: str, verify_checksum: bool = True) -> NmeaMessage:
        """解析单句 NMEA → 对应消息类型。

        - ``verify_checksum=True``：校验和不匹配抛 ``NmeaParseError``。
        - 句子类型（GGA/RMC/GSA/GSV）从 talker 后的助记符取（如 ``$GPGGA`` → GGA）。
        - 未知助记符 → ``NmeaParseError``。
        """

        payload = _verify_checksum(sentence) if verify_checksum else sentence.strip().lstrip("$").split("*")[0]
        fields_ = _fields(payload)
        if not fields_ or not fields_[0]:
            raise NmeaParseError("empty NMEA sentence")
        # 第 0 字段形如 GPGGA/GNRMC/GLGSA（talker + 助记符），取末 3 字符为类型。
        mnemonic = fields_[0][-3:].upper()
        parser = _SENTENCE_PARSERS.get(mnemonic)
        if parser is None:
            raise NmeaParseError(f"unsupported NMEA sentence type: {fields_[0]!r}")
        return parser(fields_)

    @classmethod
    def parse_lines(cls, text: str, verify_checksum: bool = True) -> list[NmeaMessage]:
        """解析多行 NMEA 文本（每行一句），跳过空行与解析失败的行。

        用于批量解析 GPS 日志；单句失败不影响其他句（返回成功解析的列表）。
        """

        results: list[NmeaMessage] = []
        for line in text.splitlines():
            line = line.strip()
            if not line or not line.startswith("$"):
                continue
            try:
                results.append(cls.parse(line, verify_checksum=verify_checksum))
            except NmeaParseError:
                continue
        return results


def compute_checksum(payload: str) -> str:
    """计算 ``payload`` 的校验和，返回两位大写十六进制（便捷入口，UI 测试用）。"""

    return f"{_xor_checksum(payload):02X}"


__all__ = ["NmeaParseError", "NmeaParser", "compute_checksum"]

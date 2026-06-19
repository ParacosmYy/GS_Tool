"""NMEA 0183 数据模型（纯 Python 无外部依赖）。

封装从 NMEA 句子解析得到的位置/轨迹/卫星结构。所有值类型用
``@dataclass(frozen=True)``（不可变）。解析逻辑在 ``parser.py``，本模块只描述结构。

约定（对齐 ``svd/model.py`` / ``can/frame.py``）：仅依赖标准库；不 import PyQt、
不依赖 transport。每个模型提供 ``to_payload()`` 输出扁平 dict（ProtocolEvent 兼容，
JSON 友好 camelCase 键）。

NMEA 坐标编码：``ddmm.mmmm``（度度分分.分分分分）。纬度 N/S，经度 E/W。
解析器负责转成十进制度（北/东为正，南/西为负）。
"""

from __future__ import annotations

from dataclasses import dataclass, field


@dataclass(frozen=True)
class GgaFix:
    """``$GPGGA`` 定位解算结果（Global Positioning System Fix Data）。

    - ``latitude`` / ``longitude``：十进制度（北/东正，南/西负）。
    - ``fix_quality``：0=无效 / 1=GPS / 2=DGPS / ...
    - ``satellite_count``：参与解算的卫星数。
    - ``hdop``：水平精度因子。
    - ``altitude``：海拔（米，椭球面）。
    """

    latitude: float
    longitude: float
    fix_quality: int
    satellite_count: int = 0
    hdop: float = 0.0
    altitude: float = 0.0
    time_utc: str = ""

    def to_payload(self) -> dict[str, object]:
        return {
            "type": "fix",
            "protocolName": "nmea",
            "sentence": "GGA",
            "latitude": self.latitude,
            "longitude": self.longitude,
            "fixQuality": self.fix_quality,
            "satelliteCount": self.satellite_count,
            "hdop": self.hdop,
            "altitude": self.altitude,
            "timeUtc": self.time_utc,
        }


@dataclass(frozen=True)
class RmcTrack:
    """``$GPRMC`` 推荐最小导航信息（Recommended Minimum Navigation）。

    - ``latitude`` / ``longitude``：十进制度。
    - ``speed_knots``：地速（节）。
    - ``course_degrees``：航向（度，真北）。
    - ``status``：``"A"``=有效 / ``"V"``=无效。
    - ``date``：``ddmmyy``。
    """

    latitude: float
    longitude: float
    status: str
    speed_knots: float = 0.0
    course_degrees: float = 0.0
    date: str = ""
    time_utc: str = ""

    @property
    def valid(self) -> bool:
        """状态是否有效（``A``）。"""

        return self.status == "A"

    def to_payload(self) -> dict[str, object]:
        return {
            "type": "track",
            "protocolName": "nmea",
            "sentence": "RMC",
            "latitude": self.latitude,
            "longitude": self.longitude,
            "status": self.status,
            "speedKnots": self.speed_knots,
            "courseDegrees": self.course_degrees,
            "date": self.date,
            "timeUtc": self.time_utc,
        }


@dataclass(frozen=True)
class GsaActive:
    """``$GPGSA`` 卫星精度因子与活动卫星（GPS DOP and active satellites）。

    - ``mode``：``"A"``=自动 / ``"M"``=手动。
    - ``fix_type``：1=无解 / 2=2D / 3=3D。
    - ``satellite_ids``：参与解算的卫星 PRN 元组（最多 12）。
    - ``pdop`` / ``hdop`` / ``vdop``：精度因子。
    """

    mode: str
    fix_type: int
    satellite_ids: tuple[int, ...] = ()
    pdop: float = 0.0
    hdop: float = 0.0
    vdop: float = 0.0

    @property
    def has_fix(self) -> bool:
        """是否有 2D/3D 解（``fix_type >= 2``）。"""

        return self.fix_type >= 2

    def to_payload(self) -> dict[str, object]:
        return {
            "type": "active_sats",
            "protocolName": "nmea",
            "sentence": "GSA",
            "mode": self.mode,
            "fixType": self.fix_type,
            "satelliteIds": list(self.satellite_ids),
            "pdop": self.pdop,
            "hdop": self.hdop,
            "vdop": self.vdop,
        }


@dataclass(frozen=True)
class SatelliteInfo:
    """单颗卫星信息（来自 GSV）。"""

    prn: int
    elevation: int
    azimuth: int
    snr: int

    def to_payload(self) -> dict[str, object]:
        return {
            "prn": self.prn,
            "elevation": self.elevation,
            "azimuth": self.azimuth,
            "snr": self.snr,
        }


@dataclass(frozen=True)
class GsvSatellites:
    """``$GPGSV`` 可见卫星信息（GPS Satellites in view）。

    - ``total_sentences``：本批 GSV 句子总数。
    - ``sentence_number``：本句序号。
    - ``satellites_in_view``：可视卫星总数。
    - ``satellites``：本句描述的卫星元组（每句最多 4 颗）。
    """

    total_sentences: int
    sentence_number: int
    satellites_in_view: int
    satellites: tuple[SatelliteInfo, ...] = ()

    def to_payload(self) -> dict[str, object]:
        return {
            "type": "satellites",
            "protocolName": "nmea",
            "sentence": "GSV",
            "totalSentences": self.total_sentences,
            "sentenceNumber": self.sentence_number,
            "satellitesInView": self.satellites_in_view,
            "satellites": [s.to_payload() for s in self.satellites],
        }


# 所有 NMEA 消息的联合类型，供 parser 返回类型标注。
NmeaMessage = GgaFix | RmcTrack | GsaActive | GsvSatellites


__all__ = [
    "GgaFix",
    "GsaActive",
    "GsvSatellites",
    "NmeaMessage",
    "RmcTrack",
    "SatelliteInfo",
]

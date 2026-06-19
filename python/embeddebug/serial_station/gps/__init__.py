"""GPS / NMEA 0183 子模块。

提供 NMEA 句子解析与位置/轨迹/卫星数据模型。纯 Python、仅依赖标准库，
不 import PyQt、不依赖 transport。UI 层只 import 本 ``__init__`` 聚合的公共 API。

公开符号：
- ``GgaFix`` / ``RmcTrack`` / ``GsaActive`` / ``GsvSatellites`` / ``SatelliteInfo``：数据模型。
- ``NmeaMessage``：消息联合类型。
- ``NmeaParser``：解析入口（``parse`` / ``parse_lines``）。
- ``NmeaParseError``：解析异常。
- ``compute_checksum``：校验和计算便捷函数。
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
from embeddebug.serial_station.gps.parser import (
    NmeaParseError,
    NmeaParser,
    compute_checksum,
)

__all__ = [
    "GgaFix",
    "GsaActive",
    "GsvSatellites",
    "NmeaMessage",
    "NmeaParseError",
    "NmeaParser",
    "RmcTrack",
    "SatelliteInfo",
    "compute_checksum",
]

"""CAN / CAN-FD 调试子模块：帧、编解码、DBC 解析。"""

from embeddebug.serial_station.can.codec import CanFrameCodec, PROTOCOL_NAME
from embeddebug.serial_station.can.dbc import (
    DbcDatabase,
    DbcMessage,
    DbcSignal,
    decode_signal,
)
from embeddebug.serial_station.can.frame import (
    CanFilter,
    CanFrame,
    CanId,
)

__all__ = [
    "CanFilter",
    "CanFrame",
    "CanFrameCodec",
    "CanId",
    "DbcDatabase",
    "DbcMessage",
    "DbcSignal",
    "PROTOCOL_NAME",
    "decode_signal",
]

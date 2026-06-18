"""OTA 协议族入口 — 统一工厂按 OtaProtocolKind 构造状态机。"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    OtaBlock,
    OtaByteChannel,
    OtaProtocol,
    OtaProtocolKind,
    TransferResult,
)
from embeddebug.ota.protocols.xmodem import make_xmodem
from embeddebug.ota.protocols.ymodem import make_ymodem


def make_protocol(
    kind: OtaProtocolKind, firmware: bytes, *, filename: str = "firmware.bin"
) -> OtaProtocol:
    """按协议类型构造对应状态机。"""

    if kind in (OtaProtocolKind.XMODEM, OtaProtocolKind.XMODEM_CRC):
        return make_xmodem(firmware, kind)
    if kind in (OtaProtocolKind.YMODEM, OtaProtocolKind.YMODEM_G):
        return make_ymodem(firmware, filename, kind)
    raise ValueError(f"unsupported OTA protocol: {kind}")


__all__ = [
    "OtaBlock", "OtaByteChannel", "OtaProtocol", "OtaProtocolKind",
    "TransferResult", "make_protocol",
]

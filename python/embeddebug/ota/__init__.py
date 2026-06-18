"""EmbedDebug OTA 子系统 — X/YMODEM 固件升级引擎。

协议状态机（protocols/）与传输编排（transfer_engine/）解耦，不依赖 ui；
UI 面板通过 AppController 共享 transport + TransferEngine 驱动升级。
"""

from embeddebug.ota.protocols import (
    OtaBlock,
    OtaByteChannel,
    OtaProtocol,
    OtaProtocolKind,
    TransferResult,
    make_protocol,
)
from embeddebug.ota.transfer_engine import ProgressCallback, TransferEngine
from embeddebug.ota.transport_adapter import SerialTransportAdapter

__all__ = [
    "OtaBlock", "OtaByteChannel", "OtaProtocol", "OtaProtocolKind",
    "ProgressCallback", "SerialTransportAdapter", "TransferEngine",
    "TransferResult", "make_protocol",
]

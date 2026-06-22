"""SEGGER RTT 调试模块（纯 Python 桩）。

导出通道/配置/会话/传输桩四类公共对象。真实 J-Link 集成在后续迭代接入，
当前仅提供可在内存替身上验证的协议契约与收发语义。
"""

from embeddebug.serial_station.rtt.protocol import (
    SEGGER_RTT_CB_ID,
    SEGGER_RTT_MAGIC,
    SEGGER_RTT_MAGIC_BYTES,
    RttChannel,
    RttConfig,
    control_block_layout,
)
from embeddebug.serial_station.rtt.session import RttSession
from embeddebug.serial_station.rtt.transport_stub import RttTransportStub

__all__ = [
    "SEGGER_RTT_CB_ID",
    "SEGGER_RTT_MAGIC",
    "SEGGER_RTT_MAGIC_BYTES",
    "RttChannel",
    "RttConfig",
    "RttSession",
    "RttTransportStub",
    "control_block_layout",
]

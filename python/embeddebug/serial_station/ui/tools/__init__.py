"""Serial Station 工具面板组件库。

提供独立运行的工程师工具面板：CRC 计算器、Hex 查看、时间戳转换等。
所有工具面板自包含、可独立测试，不依赖 controller/transport/protocol。
"""

from embeddebug.serial_station.ui.tools.crc_calculator import (
    CRC_PRESETS,
    CrcCalculatorPanel,
    CrcPreset,
    compute_crc,
)

__all__ = [
    "CRC_PRESETS",
    "CrcCalculatorPanel",
    "CrcPreset",
    "compute_crc",
]

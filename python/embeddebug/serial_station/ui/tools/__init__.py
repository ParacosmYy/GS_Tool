"""Serial Station 工具面板组件库。

提供独立运行的工程师工具面板：CRC 计算器、时间戳转换、Hex 查看、字节频率分析等。
所有工具面板自包含、可独立测试，不依赖 controller/transport/protocol。
"""

from embeddebug.serial_station.ui.tools.crc_calculator import (
    CRC_PRESETS,
    CrcCalculatorPanel,
    CrcPreset,
    compute_crc,
)
from embeddebug.serial_station.ui.tools.timestamp_converter import (
    TZ_PRESETS,
    TimestampConverterPanel,
    epoch_to_datetime,
    epoch_to_hex,
    epoch_to_iso,
    iso_to_epoch,
)
from embeddebug.serial_station.ui.tools.hex_viewer import (
    HexViewerPanel,
    format_hex_dump,
    format_hex_line,
    parse_hex_input,
)
from embeddebug.serial_station.ui.tools.byte_frequency import (
    ByteFrequencyAnalyzer,
    compute_frequency,
    entropy_bits,
    top_n_bytes,
)

__all__ = [
    "CRC_PRESETS",
    "TZ_PRESETS",
    "ByteFrequencyAnalyzer",
    "CrcCalculatorPanel",
    "CrcPreset",
    "HexViewerPanel",
    "TimestampConverterPanel",
    "compute_crc",
    "compute_frequency",
    "entropy_bits",
    "epoch_to_datetime",
    "epoch_to_hex",
    "epoch_to_iso",
    "format_hex_dump",
    "format_hex_line",
    "iso_to_epoch",
    "parse_hex_input",
    "top_n_bytes",
]

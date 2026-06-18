"""驱动适配器模块：描述与探测 Serial Station 连接驱动可用性。"""

from embeddebug.serial_station.driver_adapters.info import (
    DriverInfo,
    DriverStatus,
    DriverType,
    check_availability,
)
from embeddebug.serial_station.driver_adapters.registry import DriverRegistry

__all__ = ["DriverInfo", "DriverRegistry", "DriverStatus", "DriverType", "check_availability"]

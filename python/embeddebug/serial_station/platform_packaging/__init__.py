"""跨平台打包配置模块。"""

from __future__ import annotations

from embeddebug.serial_station.platform_packaging.config import PlatformConfig
from embeddebug.serial_station.platform_packaging.platform import PlatformDetector
from embeddebug.serial_station.platform_packaging.spec import PackageSpec

__all__ = ["PackageSpec", "PlatformConfig", "PlatformDetector"]

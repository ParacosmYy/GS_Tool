"""平台配置数据结构。"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class PlatformConfig:
    """目标运行平台的基础配置快照。"""
    os_name: str
    arch: str
    python_version: str
    qt_version: str = ""

    def platform_suffix(self) -> str:
        """返回打包产物使用的平台后缀。"""
        return f"{self.os_name}-{self.arch}"

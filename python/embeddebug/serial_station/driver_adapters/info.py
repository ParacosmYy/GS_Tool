"""驱动适配器信息模型与可用性探测。"""

from __future__ import annotations

import ctypes
import ctypes.util
import importlib.util
from dataclasses import dataclass, field
from enum import Enum


class DriverType(Enum):
    """驱动实现类型。"""
    NATIVE = "native"
    PURE_PYTHON = "pure_python"
    STUB = "stub"


class DriverStatus(Enum):
    """驱动可用性状态。"""
    AVAILABLE = "available"
    NOT_INSTALLED = "not_installed"
    ERROR = "error"


@dataclass(frozen=True)
class DriverInfo:
    """单个驱动的不可变描述信息。"""
    name: str
    display_name: str
    version: str
    driver_type: DriverType
    capabilities: list[str] = field(default_factory=list)
    available: bool = False
    dll_path: str = ""


_PYTHON_PACKAGE_BY_DRIVER: dict[str, str] = {
    "BLEAK_BLE": "bleak",
    "PYTHON_CAN": "can",
    "PY_SERIAL": "serial",
    "TCP_SOCKET": "socket",
    "USB_PYUSB": "usb",
}

_NATIVE_LIB_BY_DRIVER: dict[str, tuple[str, ...]] = {
    "JLink_RTT": ("JLinkARM", "JLink_x64"),
}


def check_availability(info: DriverInfo) -> DriverStatus:
    """探测单个驱动在当前宿主环境的可用性。"""
    try:
        if info.driver_type is DriverType.STUB:
            return DriverStatus.AVAILABLE
        if info.driver_type is DriverType.NATIVE:
            return _check_native(info)
        return _check_python(info)
    except Exception:
        return DriverStatus.ERROR


def _check_python(info: DriverInfo) -> DriverStatus:
    module_name = _PYTHON_PACKAGE_BY_DRIVER.get(info.name)
    if not module_name:
        return DriverStatus.NOT_INSTALLED
    if importlib.util.find_spec(module_name) is None:
        return DriverStatus.NOT_INSTALLED
    return DriverStatus.AVAILABLE


def _check_native(info: DriverInfo) -> DriverStatus:
    candidates: list[str] = []
    if info.dll_path:
        candidates.append(info.dll_path)
    candidates.extend(_NATIVE_LIB_BY_DRIVER.get(info.name, ()))
    for lib in candidates:
        if lib and ctypes.util.find_library(lib):
            return DriverStatus.AVAILABLE
    return DriverStatus.NOT_INSTALLED

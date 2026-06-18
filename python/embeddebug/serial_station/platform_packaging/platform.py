"""跨平台运行环境探测。"""

from __future__ import annotations

import platform
import sys

from embeddebug.serial_station.platform_packaging.config import PlatformConfig


def _normalize_os_name(raw: str) -> str:
    lowered = (raw or "").lower()
    if lowered.startswith("win"):
        return "windows"
    if lowered.startswith("linux"):
        return "linux"
    if lowered.startswith("darwin") or lowered.startswith("mac"):
        return "macos"
    return lowered or "unknown"


def _normalize_arch(machine: str, bits: str) -> str:
    lowered = (machine or "").lower()
    if lowered in {"amd64", "x86_64", "x64"}:
        return "x64"
    if lowered in {"arm64", "aarch64"}:
        return "arm64"
    if lowered in {"i386", "i686", "x86"}:
        return "x86"
    if lowered.startswith("arm"):
        return "arm"
    if bits == "64bit":
        return "x64"
    if bits == "32bit":
        return "x86"
    return lowered or "unknown"


def _probe_qt_version() -> str:
    try:
        from PyQt6.QtCore import QT_VERSION_STR
    except Exception:
        return ""
    return str(QT_VERSION_STR)


class PlatformDetector:
    """跨平台运行环境探测器。"""

    @staticmethod
    def detect() -> PlatformConfig:
        os_name = _normalize_os_name(sys.platform or platform.system())
        arch = _normalize_arch(platform.machine(), platform.architecture()[0])
        python_version = platform.python_version() or ".".join(map(str, sys.version_info[:3]))
        return PlatformConfig(os_name=os_name, arch=arch, python_version=python_version, qt_version=_probe_qt_version())

    @staticmethod
    def is_windows() -> bool:
        return sys.platform.startswith("win")

    @staticmethod
    def is_linux() -> bool:
        return sys.platform.startswith("linux")

    @staticmethod
    def is_macos() -> bool:
        return sys.platform == "darwin"

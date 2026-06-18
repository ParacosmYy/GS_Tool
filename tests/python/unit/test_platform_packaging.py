"""跨平台打包配置模块单元测试。"""

from __future__ import annotations

import os
import platform
import sys

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.platform_packaging import PackageSpec, PlatformConfig, PlatformDetector


def test_config_immutable():
    c = PlatformConfig("linux", "arm64", "3.12")
    try:
        c.os_name = "windows"
    except Exception:
        return
    raise AssertionError("frozen")


def test_platform_suffix():
    assert PlatformConfig("windows", "x64", "3.12").platform_suffix() == "windows-x64"


def test_detect_returns_config():
    config = PlatformDetector.detect()
    assert config.os_name in {"windows", "linux", "macos"}
    assert config.python_version.startswith("3.")


def test_is_windows():
    assert PlatformDetector.is_windows() is sys.platform.startswith("win")


def test_spec_defaults():
    spec = PackageSpec(platform=PlatformConfig("windows", "x64", "3.12"), app_name="App", version="1.0", entry_point="main.py")
    assert spec.onedir is True
    assert spec.hidden_imports == []


def test_spec_to_args():
    spec = PackageSpec(platform=PlatformConfig("windows", "x64", "3.12"), app_name="App", version="1.0", entry_point="main.py", hidden_imports=["numpy"])
    args = spec.to_pyinstaller_args()
    assert args[0] == "--noconfirm"
    assert "--onedir" in args
    assert "numpy" in args
    assert "--windowed" in args
    assert args[-1] == "main.py"


def test_spec_onefile():
    spec = PackageSpec(platform=PlatformConfig("linux", "x64", "3.12"), app_name="App", version="1.0", entry_point="main.py", onedir=False)
    args = spec.to_pyinstaller_args()
    assert "--onefile" in args
    assert "--onedir" not in args
    assert "--windowed" not in args


def test_spec_validate():
    spec = PackageSpec(platform=PlatformConfig("windows", "x64", "3.12"), app_name="", version="", entry_point="")
    errors = spec.validate()
    assert len(errors) == 3


def test_spec_validate_path_separator():
    spec = PackageSpec(platform=PlatformConfig("windows", "x64", "3.12"), app_name="A", version="1/0", entry_point="m.py")
    assert any("路径分隔符" in e for e in spec.validate())


def test_spec_validate_pass():
    spec = PackageSpec(platform=PlatformConfig("windows", "x64", "3.12"), app_name="A", version="1.0", entry_point="m.py")
    assert spec.validate() == []

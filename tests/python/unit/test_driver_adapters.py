"""driver_adapters 模块单元测试。"""

from __future__ import annotations

import importlib.util

from embeddebug.serial_station.driver_adapters import (
    DriverInfo,
    DriverRegistry,
    DriverStatus,
    DriverType,
    check_availability,
)
from embeddebug.serial_station.driver_adapters.builtin_drivers import builtin_drivers


def test_driver_info_defaults():
    info = DriverInfo(name="X", display_name="X", version="1", driver_type=DriverType.STUB)
    assert info.capabilities == [] and info.available is False and info.dll_path == ""


def test_driver_info_is_frozen():
    info = DriverInfo(name="X", display_name="X", version="1", driver_type=DriverType.STUB)
    try:
        info.name = "Y"
    except AttributeError:
        return
    raise AssertionError("frozen")


def test_stub_always_available():
    info = DriverInfo(name="F", display_name="F", version="1", driver_type=DriverType.STUB)
    assert check_availability(info) is DriverStatus.AVAILABLE


def test_socket_available():
    info = DriverInfo(name="TCP_SOCKET", display_name="TCP", version="1", driver_type=DriverType.PURE_PYTHON)
    assert check_availability(info) is DriverStatus.AVAILABLE


def test_unknown_python_not_installed():
    info = DriverInfo(name="UNKNOWN", display_name="U", version="1", driver_type=DriverType.PURE_PYTHON)
    assert check_availability(info) is DriverStatus.NOT_INSTALLED


def test_native_missing_dll():
    info = DriverInfo(name="JLink_RTT", display_name="J", version="1", driver_type=DriverType.NATIVE, dll_path="MissingLib")
    assert check_availability(info) is DriverStatus.NOT_INSTALLED


def test_registry_register_and_get():
    reg = DriverRegistry()
    info = DriverInfo(name="S", display_name="S", version="1", driver_type=DriverType.STUB)
    reg.register(info)
    assert reg.get("S") is info
    assert reg.get("X") is None


def test_registry_rejects_duplicate():
    reg = DriverRegistry()
    info = DriverInfo(name="S", display_name="S", version="1", driver_type=DriverType.STUB)
    reg.register(info)
    try:
        reg.register(info)
    except ValueError:
        return
    raise AssertionError("dup")


def test_registry_list_order():
    reg = DriverRegistry()
    reg.register(DriverInfo(name="A", display_name="A", version="1", driver_type=DriverType.STUB))
    reg.register(DriverInfo(name="B", display_name="B", version="1", driver_type=DriverType.STUB))
    assert [d.name for d in reg.list_drivers()] == ["A", "B"]


def test_builtin_count():
    drivers = builtin_drivers()
    assert len(drivers) == 6
    assert len({d.name for d in drivers}) == 6


def test_builtin_one_native():
    native = [d for d in builtin_drivers() if d.driver_type is DriverType.NATIVE]
    assert len(native) == 1 and native[0].name == "JLink_RTT"


def test_check_all_returns_status():
    reg = DriverRegistry()
    reg.register_many(builtin_drivers())
    status = reg.check_all()
    assert set(status.keys()) == set(reg.names)
    assert status["TCP_SOCKET"] is DriverStatus.AVAILABLE


def test_check_all_empty():
    assert DriverRegistry().check_all() == {}


def test_bleak_matches_importlib():
    info = DriverInfo(name="BLEAK_BLE", display_name="BLE", version="1", driver_type=DriverType.PURE_PYTHON)
    expected = DriverStatus.AVAILABLE if importlib.util.find_spec("bleak") else DriverStatus.NOT_INSTALLED
    assert check_availability(info) is expected

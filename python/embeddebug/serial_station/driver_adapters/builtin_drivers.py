"""内置驱动信息清单。"""

from __future__ import annotations

from embeddebug.serial_station.driver_adapters.info import DriverInfo, DriverType


def builtin_drivers() -> list[DriverInfo]:
    """返回内置驱动信息清单。"""
    return [
        DriverInfo(name="JLink_RTT", display_name="SEGGER J-Link RTT", version="1.0", driver_type=DriverType.NATIVE, capabilities=["rtt", "swd", "jtag"], available=False, dll_path="JLinkARM"),
        DriverInfo(name="BLEAK_BLE", display_name="BLE (bleak)", version="1.0", driver_type=DriverType.PURE_PYTHON, capabilities=["ble", "gatt", "scan"], available=False),
        DriverInfo(name="PYTHON_CAN", display_name="CAN / CAN-FD (python-can)", version="1.0", driver_type=DriverType.PURE_PYTHON, capabilities=["can", "canfd", "dbc"], available=False),
        DriverInfo(name="PY_SERIAL", display_name="Serial (pyserial)", version="1.0", driver_type=DriverType.PURE_PYTHON, capabilities=["uart", "rs485"], available=False),
        DriverInfo(name="TCP_SOCKET", display_name="TCP Socket", version="1.0", driver_type=DriverType.PURE_PYTHON, capabilities=["tcp", "network"], available=False),
        DriverInfo(name="USB_PYUSB", display_name="USB (PyUSB)", version="1.0", driver_type=DriverType.PURE_PYTHON, capabilities=["usb", "bulk", "control"], available=False),
    ]

"""BLE GATT 树模型：设备 -> 服务 -> 特征。"""

from __future__ import annotations

from dataclasses import dataclass, field

BLE_BASE_UUID = "00000000-0000-1000-8000-00805f9b34fb"


def expand_uuid(short: int | str) -> str:
    """把 16 位短码展开为 128 位标准 UUID。"""
    if isinstance(short, str):
        text = short.strip().lower()
        if len(text) == 36:
            return text
        text = text.removeprefix("0x")
        value = int(text, 16)
    else:
        value = int(short)
    return BLE_BASE_UUID.replace("00000000", f"{value & 0xFFFFFFFF:08x}")


@dataclass(frozen=True)
class BleCharacteristic:
    """GATT 特征。"""

    uuid: str
    properties: frozenset[str] = frozenset()
    value: bytes = b""
    handle: int = 0

    @property
    def can_read(self) -> bool:
        return "read" in self.properties

    @property
    def can_write(self) -> bool:
        return "write" in self.properties

    @property
    def can_notify(self) -> bool:
        return "notify" in self.properties


@dataclass
class BleService:
    """GATT 服务。"""

    uuid: str
    characteristics: list[BleCharacteristic] = field(default_factory=list)


@dataclass
class BleDevice:
    """扫描到的 BLE 设备。"""

    address: str
    name: str = ""
    rssi: int = 0
    services: list[BleService] = field(default_factory=list)


class BleGattTree:
    """以设备为根的 GATT 树。"""

    def __init__(self, device: BleDevice | None = None) -> None:
        self._device = device or BleDevice(address="")

    @property
    def device(self) -> BleDevice:
        return self._device

    def add_service(self, service: BleService) -> BleService:
        self._device.services.append(service)
        return service

    def add_characteristic(self, service_uuid: str, char: BleCharacteristic) -> BleCharacteristic:
        service_uuid = expand_uuid(service_uuid)
        for service in self._device.services:
            if service.uuid == service_uuid:
                service.characteristics.append(char)
                return char
        raise KeyError(f"service_not_found:{service_uuid}")

    def find_by_uuid(self, uuid: str) -> BleCharacteristic | None:
        target = expand_uuid(uuid)
        for service in self._device.services:
            for char in service.characteristics:
                if char.uuid == target:
                    return char
        return None

    def services_count(self) -> int:
        return len(self._device.services)

    def characteristics_count(self) -> int:
        return sum(len(s.characteristics) for s in self._device.services)

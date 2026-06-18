"""BLE 内存传输桩。"""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.ble.codec import (
    FRAME_NOTIFY,
    FRAME_READ_RESPONSE,
    BleFrameCodec,
    BleFrameEvent,
)
from embeddebug.serial_station.ble.gatt import (
    BleCharacteristic,
    BleDevice,
    BleGattTree,
    BleService,
    expand_uuid,
)

BytesCallback = Callable[[bytes], None]
ErrorCallback = Callable[[str], None]


class BleTransportStub:
    """内存 BLE 传输桩。"""

    DEFAULT_DEVICE_ADDRESS = "AA:BB:CC:DD:EE:FF"

    def __init__(self, device: BleDevice | None = None) -> None:
        self._device = device or self._build_default_device()
        self._tree = BleGattTree(self._device)
        self._codec = BleFrameCodec()
        self._is_open = False
        self._connected_address: str | None = None
        self._notify_handles: set[int] = set()
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self.written: list[bytes] = []

    @staticmethod
    def _build_default_device() -> BleDevice:
        info = BleService(
            uuid=expand_uuid(0x180A),
            characteristics=[
                BleCharacteristic(uuid=expand_uuid(0x2A25), properties=frozenset({"read"}), value=b"EmbedDebug-Stub", handle=0x0003),
            ],
        )
        uart = BleService(
            uuid=expand_uuid(0xFFE0),
            characteristics=[
                BleCharacteristic(uuid=expand_uuid(0xFFE1), properties=frozenset({"read", "write", "notify"}), value=b"", handle=0x0011),
            ],
        )
        return BleDevice(address=BleTransportStub.DEFAULT_DEVICE_ADDRESS, name="EmbedDebug-BLE-Stub", rssi=-42, services=[info, uart])

    @property
    def is_open(self) -> bool:
        return self._is_open

    @property
    def connected_address(self) -> str | None:
        return self._connected_address

    @property
    def device(self) -> BleDevice:
        return self._device

    @property
    def tree(self) -> BleGattTree:
        return self._tree

    def open(self, config=None) -> bool:
        address = getattr(config, "port_name", None) or self._device.address
        return self.connect(address)

    def close(self) -> None:
        self._is_open = False
        self._connected_address = None
        self._notify_handles.clear()
        self._codec.reset()

    def write(self, data: bytes) -> int:
        if not self._is_open:
            self._emit_error("transport_not_open")
            return 0
        payload = bytes(data)
        self.written.append(payload)
        for event in self._codec.feed(payload):
            self._handle_request(event)
        return len(payload)

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def connect(self, device_address: str) -> bool:
        if device_address != self._device.address:
            self._emit_error(f"device_not_found:{device_address}")
            self._is_open = False
            return False
        self._connected_address = device_address
        self._is_open = True
        return True

    def discover_services(self) -> list[BleService]:
        if not self._is_open:
            self._emit_error("transport_not_open")
            return []
        return list(self._device.services)

    def subscribe(self, uuid: str) -> bool:
        char = self._tree.find_by_uuid(uuid)
        if char is None:
            self._emit_error(f"characteristic_not_found:{expand_uuid(uuid)}")
            return False
        if not char.can_notify:
            self._emit_error("characteristic_not_notifiable")
            return False
        self._notify_handles.add(char.handle)
        return True

    def emit_notify(self, uuid: str, value: bytes) -> bool:
        if not self._is_open:
            self._emit_error("transport_not_open")
            return False
        char = self._tree.find_by_uuid(uuid)
        if char is None or char.handle not in self._notify_handles:
            return False
        self._emit_bytes(BleFrameCodec.encode_frame(FRAME_NOTIFY, char.handle, value))
        return True

    def _handle_request(self, event: BleFrameEvent) -> None:
        char = self._find_by_handle(event.handle)
        if char is None:
            self._emit_error(f"handle_not_found:{event.handle:#06x}")
            return
        if event.is_write:
            if not char.can_write:
                self._emit_error("characteristic_not_writable")
                return
        else:
            self._emit_bytes(BleFrameCodec.encode_frame(FRAME_READ_RESPONSE, char.handle, char.value))

    def _find_by_handle(self, handle: int) -> BleCharacteristic | None:
        for service in self._device.services:
            for char in service.characteristics:
                if char.handle == handle:
                    return char
        return None

    def _emit_bytes(self, data: bytes) -> None:
        for callback in list(self._bytes_callbacks):
            callback(data)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)

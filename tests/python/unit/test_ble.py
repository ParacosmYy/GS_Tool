"""BLE 子模块单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ble import (
    BLE_BASE_UUID,
    BleCharacteristic,
    BleDevice,
    BleFrameCodec,
    BleFrameEvent,
    BleGattTree,
    BleService,
    BleTransportStub,
    expand_uuid,
)
from embeddebug.serial_station.ble.codec import FRAME_NOTIFY, FRAME_READ_RESPONSE, FRAME_WRITE


def test_gatt_tree_build_and_find_by_uuid():
    device = BleDevice(address="AA:BB:CC:DD:EE:FF", name="dev", rssi=-50)
    tree = BleGattTree(device)
    service = tree.add_service(BleService(uuid=expand_uuid(0x180A)))
    tree.add_characteristic(service.uuid, BleCharacteristic(uuid=expand_uuid(0x2A25), properties=frozenset({"read"}), value=b"SN-001", handle=0x0003))
    assert tree.services_count() == 1
    found = tree.find_by_uuid(0x2A25)
    assert found is not None and found.can_read and found.value == b"SN-001"


def test_gatt_tree_add_characteristic_unknown_service_raises():
    tree = BleGattTree()
    try:
        tree.add_characteristic(expand_uuid(0xFFFF), BleCharacteristic(uuid=expand_uuid(0x0001)))
    except KeyError as exc:
        assert "service_not_found" in str(exc)
    else:
        raise AssertionError("expected KeyError")


def test_expand_uuid_short_and_full():
    expanded = expand_uuid(0x180A)
    assert expanded == "0000180a-0000-1000-8000-00805f9b34fb"
    assert expand_uuid("0xFFE1") == "0000ffe1-0000-1000-8000-00805f9b34fb"


def test_frame_encode_decode_roundtrip():
    codec = BleFrameCodec()
    raw = codec.encode(BleFrameEvent(FRAME_NOTIFY, 0x0011, b"hello"))
    events = codec.feed(raw)
    assert len(events) == 1
    assert events[0].is_notify and events[0].handle == 0x0011 and events[0].value == b"hello"


def test_frame_feed_handles_split():
    codec = BleFrameCodec()
    raw = BleFrameCodec.encode_frame(FRAME_WRITE, 0x0011, b"AB") + BleFrameCodec.encode_frame(FRAME_READ_RESPONSE, 0x0003, b"CD")
    assert codec.feed(raw[:3]) == []
    rest = codec.feed(raw[3:])
    assert len(rest) == 2 and rest[0].is_write


def test_transport_connect_discover_and_write(qtbot):
    received: list[bytes] = []
    errors: list[str] = []
    stub = BleTransportStub()
    stub.on_bytes_received(received.append)
    stub.on_error(errors.append)
    assert stub.connect(BleTransportStub.DEFAULT_DEVICE_ADDRESS)
    assert stub.is_open
    services = stub.discover_services()
    assert len(services) == 2
    write_frame = BleFrameCodec.encode_frame(FRAME_WRITE, 0x0011, b"ping")
    assert stub.write(write_frame) == len(write_frame)
    stub.close()


def test_transport_connect_failure_reports_error():
    errors: list[str] = []
    stub = BleTransportStub()
    stub.on_error(errors.append)
    assert not stub.connect("00:00:00:00:00:00")
    assert errors and errors[0].startswith("device_not_found")


def test_transport_closed_write_reports_error():
    errors: list[str] = []
    stub = BleTransportStub()
    stub.on_error(errors.append)
    assert stub.write(b"\x00") == 0
    assert errors == ["transport_not_open"]


def test_transport_subscribe_and_notify(qtbot):
    received: list[bytes] = []
    stub = BleTransportStub()
    stub.on_bytes_received(received.append)
    stub.connect(BleTransportStub.DEFAULT_DEVICE_ADDRESS)
    assert stub.subscribe(0xFFE1)
    assert stub.emit_notify(0xFFE1, b"sensor=21")
    assert received
    decoded = BleFrameCodec.decode(received[-1])
    assert decoded[0].is_notify and decoded[0].value == b"sensor=21"


def test_transport_subscribe_unknown_char_reports_error():
    errors: list[str] = []
    stub = BleTransportStub()
    stub.on_error(errors.append)
    stub.connect(BleTransportStub.DEFAULT_DEVICE_ADDRESS)
    assert not stub.subscribe(0xDEAD)
    assert any("characteristic_not_found" in msg for msg in errors)

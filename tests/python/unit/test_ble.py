"""BLE 子模块单元测试 — 合并 codec/gatt/transport_stub。

覆盖：帧编解码 round-trip、GATT 模型、expand_uuid、TransportStub 行为。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ble import (
    BleCharacteristic,
    BleDevice,
    BleFrameCodec,
    BleFrameEvent,
    BleGattTree,
    BleService,
    BleTransportStub,
    expand_uuid,
)
from embeddebug.serial_station.ble.gatt import BLE_BASE_UUID
from embeddebug.serial_station.ble.codec import (
    FRAME_NOTIFY,
    FRAME_WRITE,
)


# ---- GATT 模型 ----


def test_expand_uuid_short_and_full():
    assert expand_uuid(0x180A) == "0000180a-0000-1000-8000-00805f9b34fb"
    assert expand_uuid("0x2A00") == "00002a00-0000-1000-8000-00805f9b34fb"
    full = "12345678-1234-1234-1234-123456789ABC"
    assert expand_uuid(full) == full.lower()
    assert BLE_BASE_UUID == "00000000-0000-1000-8000-00805f9b34fb"


def test_characteristic_properties():
    ch_read = BleCharacteristic(uuid="x", properties=frozenset({"read"}))
    assert ch_read.can_read and not ch_read.can_write
    ch_write = BleCharacteristic(uuid="x", properties=frozenset({"write", "read"}))
    assert ch_write.can_write and ch_write.can_read
    ch_notify = BleCharacteristic(uuid="x", properties=frozenset({"notify"}))
    assert ch_notify.can_notify
    ch_empty = BleCharacteristic(uuid="x")
    assert not ch_empty.can_read and not ch_empty.can_write


def test_gatt_tree_build_and_find():
    device = BleDevice(address="AA:BB:CC:DD:EE:FF", name="dev", rssi=-50)
    tree = BleGattTree(device)
    service = tree.add_service(BleService(uuid=expand_uuid(0x180A)))
    tree.add_characteristic(
        service.uuid,
        BleCharacteristic(uuid=expand_uuid(0x2A25), properties=frozenset({"read"}), value=b"SN-001", handle=0x0003),
    )
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


# ---- 帧编解码 ----


def test_encode_notify_structure():
    event = BleFrameEvent(FRAME_NOTIFY, handle=0x0010, value=b"\x01\x02")
    encoded = BleFrameCodec.encode(event)
    assert encoded[0] == FRAME_NOTIFY
    assert encoded[1] == 0x10  # handle low byte
    assert encoded[2] == 0x00  # handle high byte
    assert encoded[3] == 2  # value length
    assert encoded[4:] == b"\x01\x02"


def test_encode_frame_convenience():
    encoded = BleFrameCodec.encode_frame(FRAME_WRITE, 0x0005, b"\xAA")
    event = BleFrameEvent(FRAME_WRITE, 0x0005, b"\xAA")
    assert encoded == BleFrameCodec.encode(event)


def test_feed_complete_and_multiple_frames():
    codec = BleFrameCodec()
    # 单帧
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 0x0010, b"\x01\x02\x03")
    events = codec.feed(frame)
    assert len(events) == 1
    assert events[0].type == FRAME_NOTIFY and events[0].value == b"\x01\x02\x03"
    # 多帧
    f1 = BleFrameCodec.encode_frame(FRAME_NOTIFY, 1, b"\x01")
    f2 = BleFrameCodec.encode_frame(FRAME_WRITE, 2, b"\x02\x03")
    events2 = BleFrameCodec().feed(f1 + f2)
    assert len(events2) == 2 and events2[0].is_notify and events2[1].is_write


def test_feed_handles_split_and_partial():
    codec = BleFrameCodec()
    raw = BleFrameCodec.encode_frame(FRAME_WRITE, 0x0011, b"AB")
    assert codec.feed(raw[:3]) == []
    rest = codec.feed(raw[3:])
    assert len(rest) == 1


def test_encode_value_truncated_to_255():
    long_value = b"\x00" * 300
    encoded = BleFrameCodec.encode(BleFrameEvent(FRAME_NOTIFY, 0, long_value))
    assert encoded[3] == 255 and len(encoded) == 4 + 255


def test_encode_handle_masked_to_16_bits():
    event = BleFrameEvent(FRAME_NOTIFY, 0x12345, b"")
    encoded = BleFrameCodec.encode(event)
    assert encoded[1] == 0x45 and encoded[2] == 0x23


def test_feed_empty_returns_empty():
    assert BleFrameCodec().feed(b"") == []


def test_feed_zero_length_value():
    codec = BleFrameCodec()
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, 5, b"")
    events = codec.feed(frame)
    assert len(events) == 1 and events[0].value == b""


# ---- TransportStub ----


def test_stub_default_device_and_open_close():
    stub = BleTransportStub()
    assert stub.device.name == "EmbedDebug-BLE-Stub"
    assert not stub.is_open
    stub.open()
    assert stub.is_open
    assert stub.connected_address == stub.device.address
    stub.close()
    assert not stub.is_open


def test_stub_write_appends():
    stub = BleTransportStub()
    stub.open()
    stub.write(b"\x01\x02\x03")
    assert b"\x01\x02\x03" in stub.written


def test_stub_tree_has_services():
    stub = BleTransportStub()
    assert len(stub.device.services) >= 2


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

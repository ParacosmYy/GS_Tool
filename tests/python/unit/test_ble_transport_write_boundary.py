"""BleTransportStub write/_handle_request/_find_by_handle 边界测试。

test_ble_transport_stub.py 覆盖基础 write 追加；test_ble_transport_boundary.py
覆盖 connect/subscribe/emit_notify。本文件聚焦 write → _handle_request 分发
+ _find_by_handle 查找 + error 回调全分支。

覆盖：
1. write 关闭态返回 0 + error callback（transport_not_open）。
2. write 垃圾字节不崩溃（codec 无事件）。
3. write WRITE 帧到可写特征 → 不发 READ_RESPONSE（write 路径无回包）。
4. write WRITE 帧到不可写特征 → error callback（characteristic_not_writable）。
5. write READ 帧到已知 handle → _emit_bytes 发 READ_RESPONSE 回包。
6. write 未知 handle → error callback（handle_not_found）。
7. _find_by_handle 已知/未知返回值。
8. write 多帧批量分发。
"""

from __future__ import annotations

from embeddebug.serial_station.ble.codec import (
    BleFrameCodec,
    FRAME_NOTIFY,
    FRAME_WRITE,
)
from embeddebug.serial_station.ble.transport_stub import BleTransportStub


def _open_stub() -> BleTransportStub:
    stub = BleTransportStub()
    stub.connect("AA:BB:CC:DD:EE:FF")
    return stub


# ── write 关闭态 ──────────────────────────────────────────────────
def test_write_closed_returns_zero_and_emits_error():
    stub = BleTransportStub()
    errors = []
    stub.on_error(lambda m: errors.append(m))
    n = stub.write(b"data")
    assert n == 0
    assert errors == ["transport_not_open"]


def test_write_open_returns_length():
    stub = _open_stub()
    n = stub.write(b"\x01\x02\x03")
    assert n == 3


# ── write 垃圾字节 ────────────────────────────────────────────────
def test_write_garbage_does_not_crash():
    """垃圾字节 codec.feed 无事件 → 不调 _handle_request，不崩溃。"""

    stub = _open_stub()
    stub.write(b"\xFF\xFF\xFF garbage no frame")  # 不抛


def test_write_garbage_appends_to_written():
    """垃圾字节仍追加到 written（记录原始写入）。"""

    stub = _open_stub()
    stub.write(b"garbage")
    assert stub.written == [b"garbage"]


# ── write WRITE 帧路径 ────────────────────────────────────────────
def test_write_frame_to_known_writable_handle_no_read_response():
    """WRITE 帧到可写特征：write 路径不发 READ_RESPONSE 回包。"""

    stub = _open_stub()
    received = []
    stub.on_bytes_received(lambda b: received.append(b))
    # 找一个可写特征（UART FFE1 通常可写）。
    char = stub.tree.find_by_uuid("0000ffe1-0000-1000-8000-00805f9b34fb")
    assert char is not None and char.can_write
    frame = BleFrameCodec.encode_frame(FRAME_WRITE, char.handle, b"\x42")
    stub.write(frame)
    # write 路径不回 READ_RESPONSE。
    assert received == []


def test_write_frame_to_non_writable_emits_error():
    """WRITE 帧到不可写特征 → error callback characteristic_not_writable。"""

    stub = _open_stub()
    errors = []
    stub.on_error(lambda m: errors.append(m))
    # 找一个不可写特征（DEVICE_NAME 2A25 通常 read-only）。
    char = stub.tree.find_by_uuid("00002a25-0000-1000-8000-00805f9b34fb")
    assert char is not None and not char.can_write
    frame = BleFrameCodec.encode_frame(FRAME_WRITE, char.handle, b"\x42")
    stub.write(frame)
    assert "characteristic_not_writable" in errors


# ── write READ 路径（非 WRITE 帧触发 READ_RESPONSE）─────────────
def test_write_non_write_frame_emits_read_response():
    """非 WRITE 帧（如 NOTIFY 类型）→ _handle_request else 分支发 READ_RESPONSE。"""

    stub = _open_stub()
    received = []
    stub.on_bytes_received(lambda b: received.append(b))
    char = stub.tree.find_by_uuid("0000ffe1-0000-1000-8000-00805f9b34fb")
    assert char is not None
    # FRAME_NOTIFY 非 WRITE → 走 else 分支发 READ_RESPONSE。
    frame = BleFrameCodec.encode_frame(FRAME_NOTIFY, char.handle, b"")
    stub.write(frame)
    assert len(received) == 1
    # 回包首字节应是 FRAME_READ_RESPONSE。
    assert received[0][0] == 0x03


# ── write 未知 handle ─────────────────────────────────────────────
def test_write_frame_unknown_handle_emits_error():
    """WRITE/READ 帧到未知 handle → error callback handle_not_found。"""

    stub = _open_stub()
    errors = []
    stub.on_error(lambda m: errors.append(m))
    frame = BleFrameCodec.encode_frame(FRAME_WRITE, 0xFFFF, b"\x42")  # 不存在的 handle
    stub.write(frame)
    assert any("handle_not_found" in e for e in errors)


# ── _find_by_handle ───────────────────────────────────────────────
def test_find_by_handle_known_returns_char():
    stub = _open_stub()
    char = stub.tree.find_by_uuid("0000ffe1-0000-1000-8000-00805f9b34fb")
    assert char is not None
    found = stub._find_by_handle(char.handle)
    assert found is not None
    assert found.handle == char.handle


def test_find_by_handle_unknown_returns_none():
    stub = _open_stub()
    assert stub._find_by_handle(0xFFFF) is None


# ── write 多帧批量 ────────────────────────────────────────────────
def test_write_multiple_frames_dispatches_all():
    """两帧拼接写入 → _handle_request 对每帧各调一次。"""

    stub = _open_stub()
    received = []
    stub.on_bytes_received(lambda b: received.append(b))
    char = stub.tree.find_by_uuid("0000ffe1-0000-1000-8000-00805f9b34fb")
    assert char is not None
    # FRAME_NOTIFY 非 WRITE → 各发一个 READ_RESPONSE。
    f1 = BleFrameCodec.encode_frame(FRAME_NOTIFY, char.handle, b"")
    f2 = BleFrameCodec.encode_frame(FRAME_NOTIFY, char.handle, b"")
    stub.write(f1 + f2)
    assert len(received) == 2  # 两帧各回一个 READ_RESPONSE

"""FakeSerialTransport 内存传输边界测试。

覆盖（纯内存，无真实串口/IO）：
1. __init__ 默认状态（未打开 / config None / written 空 / 无回调）。
2. open 成功路径（is_open True / config 存储 / 返回 True）。
3. open 错误路径（open_error 触发 error callback / is_open False / 返回 False）。
4. close 置 is_open False。
5. write 关闭态返回 0 + error callback（transport_not_open）。
6. write 打开态追加 written + 返回字节数。
7. inject_rx 分发到所有 bytes callbacks。
8. on_bytes_received / on_error 注册（多回调）。
9. config 属性存储传入的 SerialPortConfig。
"""

from __future__ import annotations

from embeddebug.serial_station.drivers.base import SerialPortConfig
from embeddebug.serial_station.drivers.fake import FakeSerialTransport


def _config(port: str = "COM1") -> SerialPortConfig:
    return SerialPortConfig(port_name=port, baud_rate=9600)


# ── __init__ 默认状态 ────────────────────────────────────────────
def test_init_defaults():
    t = FakeSerialTransport()
    assert t.is_open is False
    assert t.config is None
    assert t.written == []


# ── open 成功路径 ────────────────────────────────────────────────
def test_open_success_returns_true():
    t = FakeSerialTransport()
    cfg = _config()
    assert t.open(cfg) is True
    assert t.is_open is True
    assert t.config is cfg


def test_open_stores_config():
    t = FakeSerialTransport()
    cfg = _config("COM3")
    t.open(cfg)
    assert t.config is not None
    assert t.config.port_name == "COM3"
    assert t.config.baud_rate == 9600


# ── open 错误路径 ────────────────────────────────────────────────
def test_open_error_returns_false_and_emits_error():
    """open_error 触发 error callback + is_open False + 返回 False。"""

    t = FakeSerialTransport(open_error="port_busy")
    errors = []
    t.on_error(lambda m: errors.append(m))
    assert t.open(_config()) is False
    assert t.is_open is False
    assert errors == ["port_busy"]


def test_open_error_does_not_store_is_open():
    t = FakeSerialTransport(open_error="denied")
    t.open(_config())
    assert t.is_open is False
    # config 仍被存储（记录尝试）。
    assert t.config is not None


# ── close ────────────────────────────────────────────────────────
def test_close_sets_not_open():
    t = FakeSerialTransport()
    t.open(_config())
    assert t.is_open is True
    t.close()
    assert t.is_open is False


# ── write ────────────────────────────────────────────────────────
def test_write_when_closed_returns_zero_and_emits_error():
    t = FakeSerialTransport()
    errors = []
    t.on_error(lambda m: errors.append(m))
    assert t.write(b"data") == 0
    assert errors == ["transport_not_open"]
    assert t.written == []


def test_write_when_open_appends_and_returns_length():
    t = FakeSerialTransport()
    t.open(_config())
    n = t.write(b"hello")
    assert n == 5
    assert t.written == [b"hello"]


def test_write_accumulates_multiple():
    t = FakeSerialTransport()
    t.open(_config())
    t.write(b"a")
    t.write(b"bb")
    t.write(b"ccc")
    assert t.written == [b"a", b"bb", b"ccc"]


def test_write_returns_bytes_copy():
    """write 应对传入 data 做 bytes() 拷贝（避免外部 mutate 影响 written）。"""

    t = FakeSerialTransport()
    t.open(_config())
    mutable = bytearray(b"orig")
    t.write(mutable)
    mutable[0] = ord("X")  # 外部修改
    assert t.written[0] == b"orig"  # written 不受影响


# ── inject_rx ────────────────────────────────────────────────────
def test_inject_rx_dispatches_to_callbacks():
    t = FakeSerialTransport()
    received_a = []
    received_b = []
    t.on_bytes_received(lambda b: received_a.append(b))
    t.on_bytes_received(lambda b: received_b.append(b))
    t.inject_rx(b"payload")
    assert received_a == [b"payload"]
    assert received_b == [b"payload"]


def test_inject_rx_no_callbacks_does_not_crash():
    t = FakeSerialTransport()
    t.inject_rx(b"data")  # 无回调注册，不崩溃


def test_inject_rx_makes_bytes_copy():
    """inject_rx 应对 data 做 bytes() 拷贝。"""

    t = FakeSerialTransport()
    received = []
    t.on_bytes_received(lambda b: received.append(b))
    mutable = bytearray(b"orig")
    t.inject_rx(mutable)
    mutable[0] = ord("X")
    assert received[0] == b"orig"


# ── on_error 多回调 ──────────────────────────────────────────────
def test_on_error_multiple_callbacks():
    t = FakeSerialTransport(open_error="boom")
    errs1 = []
    errs2 = []
    t.on_error(lambda m: errs1.append(m))
    t.on_error(lambda m: errs2.append(m))
    t.open(_config())
    assert errs1 == ["boom"]
    assert errs2 == ["boom"]


# ── 回放场景组合 ─────────────────────────────────────────────────
def test_open_write_inject_close_cycle():
    """完整生命周期：open → write → inject_rx → close。"""

    t = FakeSerialTransport()
    t.open(_config())
    received = []
    t.on_bytes_received(lambda b: received.append(b))
    t.write(b"out")
    t.inject_rx(b"in")
    assert t.written == [b"out"]
    assert received == [b"in"]
    t.close()
    assert t.is_open is False

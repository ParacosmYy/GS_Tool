"""OTA 模块单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ota import (
    OtaConfig,
    OtaProgress,
    OtaTransportStub,
    XModemTransfer,
    YModemTransfer,
    ZModemTransfer,
)
from embeddebug.serial_station.ota.config import OtaProtocol, OtaState
from embeddebug.serial_station.ota.xmodem import crc16_ccitt, checksum_8bit, SOH, STX, ACK, NAK, CAN


def _cfg(crc: bool = True, block_size: int = 128) -> OtaConfig:
    return OtaConfig(file_path="fw.bin", protocol=OtaProtocol.XMODEM, block_size=block_size, use_crc=crc)


def test_crc16_known_vector():
    assert crc16_ccitt(b"123456789") == 0x31C3


def test_checksum_8bit_wraps():
    assert checksum_8bit(bytes(range(256))) == (sum(range(256)) & 0xFF)


def test_build_block_crc_128():
    transfer = XModemTransfer(_cfg(crc=True))
    frame = transfer.build_block(1, bytes(range(128)))
    assert frame[0] == SOH and frame[1] == 1 and frame[2] == (~1) & 0xFF
    assert len(frame) == 133


def test_build_block_1024_uses_stx():
    transfer = XModemTransfer(_cfg(block_size=1024))
    frame = transfer.build_block(1, b"\xAB" * 1024)
    assert frame[0] == STX and len(frame) == 1029


def test_build_block_checksum_mode():
    transfer = XModemTransfer(_cfg(crc=False))
    data = b"\x00" * 128
    frame = transfer.build_block(1, data)
    assert len(frame) == 132


def test_parse_ack():
    assert XModemTransfer.parse_ack(ACK) == "ack"
    assert XModemTransfer.parse_ack(NAK) == "nak"
    assert XModemTransfer.parse_ack(CAN) == "can"


def test_xmodem_single_block_round_trip():
    transfer = XModemTransfer(_cfg())
    stub = OtaTransportStub()
    written = transfer.send_block(stub, 1, bytes(range(128)))
    assert written == 133
    assert transfer.handle_response(ACK) == "advance"


def test_handle_response_nak_overflow():
    transfer = XModemTransfer(OtaConfig(file_path="fw.bin", retry_count=2))
    assert transfer.handle_response(NAK) == "retry"
    assert transfer.handle_response(NAK) == "retry"
    assert transfer.handle_response(NAK) == "abort"
    assert transfer.progress.state is OtaState.FAILED


def test_handle_response_can_aborts():
    transfer = XModemTransfer(_cfg())
    assert transfer.handle_response(CAN) == "abort"


def test_ymodem_header_round_trip():
    transfer = YModemTransfer(_cfg())
    frame = transfer.build_header_block("app.bin", 1024)
    assert frame[:3] == bytes([SOH, 0x00, 0xFF])
    assert transfer.parse_header_block(frame) == ("app.bin", 1024)


def test_zmodem_zrinit_round_trip():
    z = ZModemTransfer()
    frame = z.build_hex_frame(0x01, bytes([0x20, 0, 0, 0]))
    parsed = z.parse_hex_frame(frame)
    assert parsed is not None
    ft, data4, crc = parsed
    assert ft == 0x01 and ZModemTransfer.is_crc_valid(ft, data4, crc)


def test_zmodem_rejects_wrong_data_length():
    z = ZModemTransfer()
    with pytest.raises(ValueError):
        z.build_hex_frame(0x01, b"\x01\x02\x03")


def test_progress_percent():
    progress = OtaProgress()
    assert progress.percent == 0.0
    progress.begin(total_bytes=200)
    progress.mark_sent(50)
    assert progress.percent == 25.0
    progress.mark_sent(200)
    assert progress.percent == 100.0


def test_config_validate():
    with pytest.raises(ValueError):
        OtaConfig(file_path="").validate()
    with pytest.raises(ValueError):
        OtaConfig(file_path="fw.bin", block_size=256).validate()


def test_transport_stub_records():
    stub = OtaTransportStub()
    stub.write(b"abc")
    stub.write(b"de")
    assert stub.written == [b"abc", b"de"]
    assert stub.total_bytes == b"abcde"
    stub.inject_ack(2)
    assert stub.read_byte() == ACK
    assert stub.read_byte() == ACK

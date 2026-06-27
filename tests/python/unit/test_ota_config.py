"""OTA 配置与进度模型单元测试。"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ota.config import (
    OtaConfig,
    OtaProgress,
    OtaProtocol,
    OtaState,
    _VALID_BLOCK_SIZES,
)


def test_ota_config_defaults():
    cfg = OtaConfig(file_path="firmware.bin")
    assert cfg.protocol == OtaProtocol.XMODEM
    assert cfg.block_size == 128
    assert cfg.retry_count == 10
    assert cfg.use_crc is True


def test_ota_config_validate_ok():
    OtaConfig(file_path="fw.bin").validate()  # 不抛异常


@pytest.mark.parametrize("file_path", ["", "   "])
def test_ota_config_validate_rejects_blank_path(file_path: str):
    with pytest.raises(ValueError, match="file_path"):
        OtaConfig(file_path=file_path).validate()


@pytest.mark.parametrize("block_size", [128, 1024])
def test_ota_config_validate_accepts_valid_block_sizes(block_size: int):
    OtaConfig(file_path="fw.bin", block_size=block_size).validate()


@pytest.mark.parametrize(
    ("kwargs", "message"),
    [
        ({"block_size": 256}, "block_size"),
        ({"retry_count": 0}, "retry_count"),
        ({"retry_count": -1}, "retry_count"),
    ],
)
def test_ota_config_validate_rejects_invalid_values(kwargs: dict[str, int], message: str):
    with pytest.raises(ValueError, match=message):
        OtaConfig(file_path="fw.bin", **kwargs).validate()


def test_ota_protocol_values():
    assert len(OtaProtocol) == 3
    assert {protocol.value for protocol in OtaProtocol} == {"xmodem", "ymodem", "zmodem"}
    assert OtaProtocol.XMODEM == "xmodem"
    assert isinstance(OtaProtocol.XMODEM, str)


def test_ota_state_values():
    assert len(OtaState) == 4
    assert {state.value for state in OtaState} == {"idle", "transferring", "complete", "failed"}


def test_valid_block_sizes_constant():
    assert _VALID_BLOCK_SIZES == (128, 1024)


@pytest.mark.parametrize(
    ("progress", "expected"),
    [
        (OtaProgress(), 0.0),
        (OtaProgress(sent_bytes=500, total_bytes=1000), 50.0),
        (OtaProgress(sent_bytes=1000, total_bytes=1000), 100.0),
        (OtaProgress(sent_bytes=1500, total_bytes=1000), 100.0),
        (OtaProgress(sent_bytes=1, total_bytes=3), 33.33),
    ],
)
def test_progress_percent_boundaries(progress: OtaProgress, expected: float):
    assert progress.percent == expected


def test_progress_mark_sent():
    p = OtaProgress()
    p.mark_sent(128)
    p.mark_sent(128)
    assert p.sent_bytes == 256
    assert p.block_index == 2


def test_progress_mark_sent_negative_ignored():
    p = OtaProgress()
    p.mark_sent(-10)
    assert p.sent_bytes == 0
    assert p.block_index == 1  # block_index 仍递增


def test_progress_begin_resets_counters_and_clamps_total():
    p = OtaProgress(sent_bytes=512, total_bytes=2048, block_index=4, errors=2)
    p.begin(-100)
    assert p.state == OtaState.TRANSFERRING
    assert p.total_bytes == 0
    assert p.sent_bytes == 0
    assert p.block_index == 0
    assert p.errors == 0
    assert p.percent == 0.0


def test_progress_begin_allows_transfer_percent():
    p = OtaProgress()
    p.begin(1000)
    p.mark_sent(500)
    assert p.total_bytes == 1000
    assert p.percent == 50.0


def test_progress_mark_error_and_terminal_states():
    p = OtaProgress()
    assert p.state == OtaState.IDLE
    p.mark_error()
    p.mark_error()
    assert p.errors == 2
    p.complete()
    assert p.state == OtaState.COMPLETE
    p.fail()
    assert p.state == OtaState.FAILED

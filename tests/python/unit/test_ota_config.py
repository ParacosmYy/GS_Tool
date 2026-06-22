"""OTA 配置与进度模型单元测试。

覆盖：OtaConfig.validate 验证、OtaProgress.percent 计算、mark_sent 累加、
OtaProtocol/OtaState 枚举。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ota.config import (
    OtaConfig,
    OtaProgress,
    OtaProtocol,
    OtaState,
)


def test_ota_config_defaults():
    cfg = OtaConfig(file_path="firmware.bin")
    assert cfg.protocol == OtaProtocol.XMODEM
    assert cfg.block_size == 128
    assert cfg.use_crc is True


def test_ota_config_validate_ok():
    OtaConfig(file_path="fw.bin").validate()  # 不抛异常


def test_ota_config_validate_empty_path():
    with pytest.raises(ValueError):
        OtaConfig(file_path="").validate()


def test_ota_config_validate_whitespace_path():
    with pytest.raises(ValueError):
        OtaConfig(file_path="   ").validate()


def test_ota_config_validate_invalid_block_size():
    with pytest.raises(ValueError):
        OtaConfig(file_path="fw.bin", block_size=256).validate()


def test_ota_config_validate_block_1024():
    OtaConfig(file_path="fw.bin", block_size=1024).validate()  # 不抛异常


def test_ota_config_validate_zero_retry():
    with pytest.raises(ValueError):
        OtaConfig(file_path="fw.bin", retry_count=0).validate()


def test_ota_protocol_values():
    assert OtaProtocol.XMODEM == "xmodem"
    assert OtaProtocol.YMODEM == "ymodem"
    assert OtaProtocol.ZMODEM == "zmodem"


def test_ota_state_values():
    assert OtaState.IDLE == "idle"
    assert OtaState.TRANSFERRING == "transferring"
    assert OtaState.COMPLETE == "complete"
    assert OtaState.FAILED == "failed"


def test_progress_percent_zero_total():
    p = OtaProgress()
    assert p.percent == 0.0


def test_progress_percent_half():
    p = OtaProgress(sent_bytes=500, total_bytes=1000)
    assert p.percent == 50.0


def test_progress_percent_complete():
    p = OtaProgress(sent_bytes=1000, total_bytes=1000)
    assert p.percent == 100.0


def test_progress_percent_over_100_capped():
    p = OtaProgress(sent_bytes=1500, total_bytes=1000)
    assert p.percent == 100.0


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

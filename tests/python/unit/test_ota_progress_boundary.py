"""OtaProgress 状态转换 + OtaConfig 常量边界单元测试。

补强 test_ota_config.py 未直接断言的边角：
- OtaProgress.begin：state→TRANSFERRING + 重置 sent/block/errors + total_bytes clamp。
- OtaProgress.complete：state→COMPLETE。
- OtaProgress.fail：state→FAILED。
- OtaProgress.mark_error：errors 递增。
- OtaProgress.mark_sent：block_index 递增 + sent_bytes 累加。
- OtaProgress.percent：begin 后精确计算。
- _VALID_BLOCK_SIZES 常量 + OtaConfig 默认 retry_count。
- OtaProtocol/OtaState 枚举完备性。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ota.config import (
    OtaConfig,
    OtaProgress,
    OtaProtocol,
    OtaState,
    _VALID_BLOCK_SIZES,
)


# ── OtaProgress.begin ───────────────────────────────────────────────────


def test_begin_sets_transferring_state():
    """begin → state=TRANSFERRING。"""

    p = OtaProgress()
    p.begin(1024)
    assert p.state == OtaState.TRANSFERRING


def test_begin_resets_counters():
    """begin 重置 sent_bytes/block_index/errors 为 0。"""

    p = OtaProgress()
    p.mark_sent(128)
    p.mark_error()
    p.begin(1024)
    assert p.sent_bytes == 0
    assert p.block_index == 0
    assert p.errors == 0


def test_begin_sets_total_bytes():
    """begin 设置 total_bytes。"""

    p = OtaProgress()
    p.begin(4096)
    assert p.total_bytes == 4096


def test_begin_negative_total_clamped():
    """begin 负值 total_bytes clamp 到 0。"""

    p = OtaProgress()
    p.begin(-100)
    assert p.total_bytes == 0


def test_begin_zero_total_ok():
    """begin total=0 合法（percent 返回 0）。"""

    p = OtaProgress()
    p.begin(0)
    assert p.percent == 0.0


# ── OtaProgress.complete / fail ─────────────────────────────────────────


def test_complete_sets_complete_state():
    """complete → state=COMPLETE。"""

    p = OtaProgress()
    p.begin(100)
    p.complete()
    assert p.state == OtaState.COMPLETE


def test_fail_sets_failed_state():
    """fail → state=FAILED。"""

    p = OtaProgress()
    p.begin(100)
    p.fail()
    assert p.state == OtaState.FAILED


def test_default_state_is_idle():
    """新建 OtaProgress 默认 state=IDLE。"""

    assert OtaProgress().state == OtaState.IDLE


# ── OtaProgress.mark_error ──────────────────────────────────────────────


def test_mark_error_increments():
    """mark_error 每次 errors+1。"""

    p = OtaProgress()
    p.mark_error()
    p.mark_error()
    p.mark_error()
    assert p.errors == 3


# ── OtaProgress.mark_sent 边界 ──────────────────────────────────────────


def test_mark_sent_increments_block_index():
    """mark_sent 每次 block_index+1。"""

    p = OtaProgress()
    p.mark_sent(128)
    p.mark_sent(128)
    assert p.block_index == 2


def test_mark_sent_zero_bytes():
    """mark_sent(0) → sent_bytes 不增，block_index 仍+1。"""

    p = OtaProgress()
    p.mark_sent(0)
    assert p.sent_bytes == 0
    assert p.block_index == 1


# ── OtaProgress.percent begin 后精确 ────────────────────────────────────


def test_percent_after_begin_half():
    """begin(1000) + mark_sent(500) → 50%。"""

    p = OtaProgress()
    p.begin(1000)
    p.mark_sent(500)
    assert p.percent == 50.0


def test_percent_rounds_to_two_decimals():
    """percent 保留 2 位小数。"""

    p = OtaProgress()
    p.begin(3)
    p.mark_sent(1)
    # 1/3 ≈ 33.33%
    assert p.percent == 33.33


# ── _VALID_BLOCK_SIZES 常量 ─────────────────────────────────────────────


def test_valid_block_sizes_contains_128_and_1024():
    """_VALID_BLOCK_SIZES = (128, 1024)。"""

    assert _VALID_BLOCK_SIZES == (128, 1024)


# ── OtaConfig 默认值 ───────────────────────────────────────────────────


def test_config_default_protocol_xmodem():
    """默认 protocol=XMODEM。"""

    cfg = OtaConfig(file_path="fw.bin")
    assert cfg.protocol == OtaProtocol.XMODEM


def test_config_default_block_size_128():
    """默认 block_size=128。"""

    cfg = OtaConfig(file_path="fw.bin")
    assert cfg.block_size == 128


def test_config_default_retry_count_10():
    """默认 retry_count=10。"""

    cfg = OtaConfig(file_path="fw.bin")
    assert cfg.retry_count == 10


def test_config_default_use_crc_true():
    """默认 use_crc=True。"""

    cfg = OtaConfig(file_path="fw.bin")
    assert cfg.use_crc is True


def test_config_negative_retry_raises():
    """retry_count < 0 → validate ValueError。"""

    cfg = OtaConfig(file_path="fw.bin", retry_count=-1)
    with pytest.raises(ValueError, match="retry_count"):
        cfg.validate()


# ── OtaProtocol / OtaState 枚举完备性 ──────────────────────────────────


def test_ota_protocol_has_three_members():
    """OtaProtocol 含 3 成员。"""

    assert len(OtaProtocol) == 3
    assert {p.value for p in OtaProtocol} == {"xmodem", "ymodem", "zmodem"}


def test_ota_state_has_four_members():
    """OtaState 含 4 成员。"""

    assert len(OtaState) == 4
    assert {s.value for s in OtaState} == {"idle", "transferring", "complete", "failed"}


def test_ota_protocol_is_str_enum():
    """OtaProtocol 是 str Enum（可直接当字符串用）。"""

    assert OtaProtocol.XMODEM == "xmodem"
    assert isinstance(OtaProtocol.XMODEM, str)

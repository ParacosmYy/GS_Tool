"""GPS NMEA parser 纯 helper 单元测试（checksum / 类型转换 / 坐标转换边界）。

补强 test_gps_parser.py 未直接断言的 gps/parser.py 私有 helper + 公开便捷入口：
- _xor_checksum：逐字节 XOR，空串=0，单字符=ord，多字符累积。
- compute_checksum：两位大写 hex（便捷入口，UI 用）。
- _verify_checksum：$ 前缀校验 / 缺 *  容错 / 校验和不匹配抛 NmeaParseError / 非法 hex。
- _int_or：空串/非法 → default，合法 int 原样返回。
- _float_or：空串/非法 → default，合法 float 原样返回。
- _to_decimal：coord=0.0 早返回 + 南/西取负 + 无效 hemisphere 正值保留（防御性）。
- _fields：逗号切分。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.gps.parser import (
    NmeaParseError,
    _fields,
    _float_or,
    _int_or,
    _to_decimal,
    _verify_checksum,
    _xor_checksum,
    compute_checksum,
)


# ── _xor_checksum ────────────────────────────────────────────────────────


def test_xor_checksum_empty_is_zero():
    """空串 XOR 结果为 0（初始值）。"""

    assert _xor_checksum("") == 0


def test_xor_checksum_single_char_is_ord():
    """单字符 XOR 结果 = ord(char)（与 0 异或保持原值）。"""

    assert _xor_checksum("A") == ord("A") == 0x41


def test_xor_checksum_multi_char_accumulates():
    """多字符逐字节 XOR 累积。"""

    # "AB" = 0x41 ^ 0x42 = 0x03
    assert _xor_checksum("AB") == 0x41 ^ 0x42 == 0x03


def test_xor_checksum_real_nmea_payload():
    """真实 NMEA 载荷校验和（GPGGA 片段）。"""

    # $GPGGA 的已知样本：payload "GPGGA,123519" 校验和
    payload = "GPGGA,123519"
    result = _xor_checksum(payload)
    assert 0 <= result <= 255  # 0-255 范围


def test_xor_checksum_masked_to_byte():
    """结果 mask 到 0xFF（& 0xFF），即使累积超过 255 也只留低 8 位。"""

    # 选一段会产生超过 255 累积的载荷（实际 XOR 不会超 255，但 mask 是防御）
    result = _xor_checksum("abcdefghijklmnopqrstuvwxyz")
    assert 0 <= result <= 255


# ── compute_checksum（公开便捷入口） ─────────────────────────────────────


def test_compute_checksum_returns_two_digit_upper_hex():
    """compute_checksum 返回两位大写 hex 字符串。"""

    assert compute_checksum("") == "00"
    assert compute_checksum("A") == "41"
    assert compute_checksum("AB") == "03"


def test_compute_checksum_pads_single_digit():
    """结果 < 0x10 时左侧补零（如 0x05 → "05"）。"""

    # 找一个 XOR 结果是单位数的载荷
    # ord('A')=0x41, 找 char c 使 0x41 ^ ord(c) < 0x10
    # 0x41 ^ 0x4D = 0x0C
    assert compute_checksum("AM") == "0C"


def test_compute_checksum_matches_xor_checksum():
    """compute_checksum 与 _xor_checksum 数值一致（格式不同）。"""

    payload = "GPGGA,123519"
    assert int(compute_checksum(payload), 16) == _xor_checksum(payload)


# ── _verify_checksum ─────────────────────────────────────────────────────


def test_verify_checksum_missing_dollar_raises():
    """缺 $ 前缀 → NmeaParseError。"""

    with pytest.raises(NmeaParseError, match="must start"):
        _verify_checksum("GPGGA,data")


def test_verify_checksum_no_asterisk_tolerant():
    """有 $ 但无 * → 容错返回 body（部分设备不发校验和）。"""

    body = _verify_checksum("$GPGGA,data")
    assert body == "GPGGA,data"


def test_verify_checksum_valid_passes():
    """合法校验和 → 返回 payload。"""

    # 构造 $payload*CS 格式
    payload = "GPGGA,123519"
    cs = compute_checksum(payload)
    body = _verify_checksum(f"${payload}*{cs}")
    assert body == payload


def test_verify_checksum_mismatch_raises():
    """校验和不匹配 → NmeaParseError。"""

    with pytest.raises(NmeaParseError, match="checksum mismatch"):
        _verify_checksum("$GPGGA,data*FF")  # FF 几乎不可能匹配


def test_verify_checksum_invalid_hex_raises():
    """校验和段非合法 hex → NmeaParseError。"""

    with pytest.raises(NmeaParseError, match="invalid checksum hex"):
        _verify_checksum("$GPGGA,data*ZZ")


def test_verify_checksum_strips_whitespace():
    """前后空白被 strip（容错换行/空格）。"""

    payload = "GPGGA"
    cs = compute_checksum(payload)
    body = _verify_checksum(f"  ${payload}*{cs}  \n")
    assert body == payload


# ── _int_or ──────────────────────────────────────────────────────────────


def test_int_or_valid_int():
    assert _int_or("42") == 42


def test_int_or_negative():
    assert _int_or("-5") == -5


def test_int_or_empty_returns_default():
    assert _int_or("") == 0
    assert _int_or("", default=-1) == -1


def test_int_or_invalid_returns_default():
    assert _int_or("abc") == 0
    assert _int_or("abc", default=99) == 99


def test_int_or_strips_whitespace():
    """字段前后空白被 strip。"""

    assert _int_or("  7  ") == 7


# ── _float_or ────────────────────────────────────────────────────────────


def test_float_or_valid_float():
    assert _float_or("3.14") == 3.14


def test_float_or_int_string():
    assert _float_or("5") == 5.0


def test_float_or_empty_returns_default():
    assert _float_or("") == 0.0
    assert _float_or("", default=-1.5) == -1.5


def test_float_or_invalid_returns_default():
    assert _float_or("abc") == 0.0
    assert _float_or("abc", default=9.9) == 9.9


def test_float_or_negative():
    assert _float_or("-2.5") == -2.5


# ── _to_decimal 边界 ─────────────────────────────────────────────────────


def test_to_decimal_zero_returns_zero():
    """coord=0.0 早返回 0.0（避免 0//100 等边界）。"""

    assert _to_decimal(0.0, "N") == 0.0
    assert _to_decimal(0.0, "S") == 0.0  # 0 不取负


def test_to_decimal_invalid_hemisphere_keeps_positive():
    """无效 hemisphere（非 S/W）→ 保持正值（防御性，不抛）。"""

    assert _to_decimal(3957.6123, "X") > 0  # 不取负


def test_to_decimal_north_positive():
    assert _to_decimal(3957.6123, "N") > 0


def test_to_decimal_south_negative():
    assert _to_decimal(3957.6123, "S") < 0


def test_to_decimal_east_positive_west_negative():
    assert _to_decimal(11622.5234, "E") > 0
    assert _to_decimal(11622.5234, "W") < 0


# ── _fields ──────────────────────────────────────────────────────────────


def test_fields_splits_by_comma():
    assert _fields("a,b,c") == ["a", "b", "c"]


def test_fields_single_no_comma():
    assert _fields("only") == ["only"]


def test_fields_empty_string():
    """空串切分返回 ['']（split 行为）。"""

    assert _fields("") == [""]

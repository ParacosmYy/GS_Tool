"""OTA 协议纯 helper 单元测试（zmodem _hex_byte + xmodem build_eot/cancel + crc16 init）。

补强 test_ota.py / test_ota_protocols.py 未直接断言的边角 helper：
- _hex_byte（zmodem）：0-255 范围格式化为两位大写 hex ASCII。
- build_eot / build_cancel（xmodem）：单字节 EOT / 双字节 CAN。
- is_crc_valid（zmodem static）：CRC 校验 True/False。
- crc16_ccitt init 参数：非零初始值影响结果 + 链式调用累加。
- ZModem 常量：ZRINIT/ZFILE/ZDATA/ZEOF + ZPAD/ZDLE/ZHEX 控制字节。
- parse_hex_frame：无 marker 返回 None / 截断帧返回 None。
"""

from __future__ import annotations

from embeddebug.serial_station.ota.xmodem import (
    ACK,
    CAN,
    C,
    EOT,
    NAK,
    SOH,
    STX,
    XModemTransfer,
    crc16_ccitt,
)
from embeddebug.serial_station.ota.zmodem import (
    CR,
    LF,
    ZDLE,
    ZFILE,
    ZHEX,
    ZPAD,
    ZRINIT,
    ZDATA,
    ZEOF,
    XON,
    ZModemTransfer,
    _hex_byte,
)


# ── _hex_byte（zmodem 私有 helper） ──────────────────────────────────────


def test_hex_byte_zero():
    """0 → "00"。"""

    assert _hex_byte(0) == b"00"


def test_hex_byte_max_ff():
    """255 → "FF"（大写）。"""

    assert _hex_byte(0xFF) == b"FF"


def test_hex_byte_single_digit_padded():
    """单位数（如 10）左侧补零 → "0A"。"""

    assert _hex_byte(10) == b"0A"
    assert _hex_byte(15) == b"0F"


def test_hex_byte_masks_above_ff():
    """值 > 0xFF 被 mask 到 0xFF（& 0xFF）。"""

    assert _hex_byte(0x1FF) == b"FF"
    assert _hex_byte(0x100) == b"00"


def test_hex_byte_two_digit_upper():
    """两位数大写 hex（如 0xAB）。"""

    assert _hex_byte(0xAB) == b"AB"
    assert _hex_byte(0x5A) == b"5A"


# ── build_eot / build_cancel（xmodem） ───────────────────────────────────


def _make_xmodem(use_crc: bool = True):
    """构造 XModemTransfer（默认 CRC 模式）。"""

    from embeddebug.serial_station.ota.config import OtaConfig

    return XModemTransfer(OtaConfig(file_path="dummy.bin", use_crc=use_crc))


def test_build_eot_returns_single_eot_byte():
    """build_eot → bytes([EOT]) 单字节。"""

    tx = _make_xmodem()
    assert tx.build_eot() == bytes([EOT])


def test_build_cancel_returns_double_can():
    """build_cancel → bytes([CAN, CAN]) 双字节取消。"""

    tx = _make_xmodem()
    assert tx.build_cancel() == bytes([CAN, CAN])


def test_build_eot_is_one_byte():
    """EOT 帧恰好 1 字节。"""

    assert len(_make_xmodem().build_eot()) == 1


def test_build_cancel_is_two_bytes():
    """取消帧恰好 2 字节。"""

    assert len(_make_xmodem().build_cancel()) == 2


# ── parse_ack 全分支（此前 test_ota 仅测部分） ──────────────────────────


def test_parse_ack_all_known_bytes():
    """parse_ack 覆盖 ack/nak/can/c 四个已知 + unknown。"""

    assert XModemTransfer.parse_ack(ACK) == "ack"
    assert XModemTransfer.parse_ack(NAK) == "nak"
    assert XModemTransfer.parse_ack(CAN) == "can"
    assert XModemTransfer.parse_ack(C) == "c"


def test_parse_ack_unknown_byte():
    """未知字节 → "unknown"。"""

    assert XModemTransfer.parse_ack(0x00) == "unknown"
    assert XModemTransfer.parse_ack(0xFF) == "unknown"


# ── crc16_ccitt init 参数 ────────────────────────────────────────────────


def test_crc16_init_zero_default():
    """默认 init=0x0000 与显式传 0 一致。"""

    assert crc16_ccitt(b"test") == crc16_ccitt(b"test", init=0x0000)


def test_crc16_init_nonzero_changes_result():
    """非零 init 改变结果（不同于默认 0）。"""

    data = b"test"
    default = crc16_ccitt(data)
    with_init = crc16_ccitt(data, init=0xFFFF)
    assert default != with_init


def test_crc16_chained_init_accumulates():
    """分两段计算（init 传递）= 整体计算。"""

    full = crc16_ccitt(b"hello world")
    partial = crc16_ccitt(b"hello ")
    chained = crc16_ccitt(b"world", init=partial)
    assert full == chained


def test_crc16_empty_returns_init():
    """空输入 → 返回 init 值（无字节处理）。"""

    assert crc16_ccitt(b"") == 0x0000
    assert crc16_ccitt(b"", init=0x1234) == 0x1234


# ── is_crc_valid（zmodem static） ────────────────────────────────────────


def test_is_crc_valid_true_for_correct_crc():
    """正确 CRC → True。"""

    frame_type = ZRINIT
    data4 = b"\x00\x00\x00\x00"
    correct = crc16_ccitt(bytes([frame_type]) + data4)
    assert ZModemTransfer.is_crc_valid(frame_type, data4, correct) is True


def test_is_crc_valid_false_for_wrong_crc():
    """错误 CRC → False。"""

    frame_type = ZFILE
    data4 = b"\x01\x02\x03\x04"
    assert ZModemTransfer.is_crc_valid(frame_type, data4, 0x0000) is False


def test_is_crc_valid_distinguishes_frame_type():
    """不同 frame_type 产生不同 CRC（防 frame_type 被忽略）。"""

    data4 = b"\x00\x00\x00\x00"
    crc_a = crc16_ccitt(bytes([ZRINIT]) + data4)
    crc_b = crc16_ccitt(bytes([ZFILE]) + data4)
    assert crc_a != crc_b


# ── parse_hex_frame 边界 ────────────────────────────────────────────────


def test_parse_hex_frame_no_marker_returns_none():
    """无 ZDLE+ZHEX marker → None。"""

    zm = ZModemTransfer()
    assert zm.parse_hex_frame(b"garbage no marker") is None


def test_parse_hex_frame_truncated_returns_none():
    """有 marker 但 hex nibble 不足 14 个 → None。"""

    zm = ZModemTransfer()
    # marker + 仅 4 个 hex 字符（不足 14）
    frame = bytes([ZPAD, ZPAD, ZDLE, ZHEX]) + b"0102"
    assert zm.parse_hex_frame(frame) is None


# ── ZModem 常量契约 ─────────────────────────────────────────────────────


def test_zmodem_frame_type_constants_distinct():
    """4 个帧类型常量互不相同。"""

    types = {ZRINIT, ZFILE, ZDATA, ZEOF}
    assert len(types) == 4


def test_zmodem_control_byte_constants():
    """ZPAD/ZDLE/ZHEX/CR/LF/XON 控制字节值契约。"""

    assert ord("*") == ZPAD
    assert ord("A") == ZHEX
    assert ZDLE == 0x18
    assert CR == 0x0D
    assert LF == 0x0A
    assert XON == 0x11


def test_xmodem_control_byte_constants():
    """SOH/STX/EOT/ACK/NAK/CAN/C 控制字节值契约。"""

    assert SOH == 0x01
    assert STX == 0x02
    assert EOT == 0x04
    assert ACK == 0x06
    assert NAK == 0x15
    assert CAN == 0x18
    assert C == 0x43

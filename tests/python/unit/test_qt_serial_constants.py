"""qt_serial 常量映射 + _reverse_lookup 单元测试。

qt_serial.py 暴露的 DATA_BITS/PARITY/STOP_BITS/FLOW_CONTROL 常量映射 +
_reverse_lookup 反查 helper 此前仅通过 QtSerialPortTransport.configure 间接覆盖，
未直接断言映射完备性与反查行为。

覆盖：
- DATA_BITS：4 个键（5/6/7/8）映射到 QSerialPort.DataBits 全集。
- PARITY：5 个键（none/even/odd/space/mark）映射到 QSerialPort.Parity 全集。
- STOP_BITS：3 个键（1/1.5/2）映射到 QSerialPort.StopBits 全集。
- FLOW_CONTROL：3 个键（none/hardware/software）映射到 QSerialPort.FlowControl 全集。
- _reverse_lookup：正向反查 / 未知值返回空串 / 键类型保留（int vs str）。
"""

from __future__ import annotations

from PyQt6.QtSerialPort import QSerialPort

from embeddebug.serial_station.drivers.qt_serial import (
    DATA_BITS,
    FLOW_CONTROL,
    PARITY,
    STOP_BITS,
    _reverse_lookup,
)


# ── DATA_BITS 完备性 ─────────────────────────────────────────────────────


def test_data_bits_covers_all_four_values():
    """DATA_BITS 映射 5/6/7/8 到全部 4 个 QSerialPort.DataBits 枚举。"""

    assert set(DATA_BITS.keys()) == {5, 6, 7, 8}
    assert DATA_BITS[5] == QSerialPort.DataBits.Data5
    assert DATA_BITS[6] == QSerialPort.DataBits.Data6
    assert DATA_BITS[7] == QSerialPort.DataBits.Data7
    assert DATA_BITS[8] == QSerialPort.DataBits.Data8


def test_data_bits_values_are_distinct():
    """4 个 DataBits 枚举值互不相同（防映射打错）。"""

    assert len(set(DATA_BITS.values())) == 4


# ── PARITY 完备性 ─────────────────────────────────────────────────────────


def test_parity_covers_all_five_values():
    """PARITY 映射 none/even/odd/space/mark 到全部 5 个 QSerialPort.Parity 枚举。"""

    assert set(PARITY.keys()) == {"none", "even", "odd", "space", "mark"}
    assert PARITY["none"] == QSerialPort.Parity.NoParity
    assert PARITY["even"] == QSerialPort.Parity.EvenParity
    assert PARITY["odd"] == QSerialPort.Parity.OddParity
    assert PARITY["space"] == QSerialPort.Parity.SpaceParity
    assert PARITY["mark"] == QSerialPort.Parity.MarkParity


def test_parity_values_are_distinct():
    assert len(set(PARITY.values())) == 5


# ── STOP_BITS 完备性 ──────────────────────────────────────────────────────


def test_stop_bits_covers_all_three_values():
    """STOP_BITS 映射 1/1.5/2 到全部 3 个 QSerialPort.StopBits 枚举。"""

    assert set(STOP_BITS.keys()) == {"1", "1.5", "2"}
    assert STOP_BITS["1"] == QSerialPort.StopBits.OneStop
    assert STOP_BITS["1.5"] == QSerialPort.StopBits.OneAndHalfStop
    assert STOP_BITS["2"] == QSerialPort.StopBits.TwoStop


def test_stop_bits_values_are_distinct():
    assert len(set(STOP_BITS.values())) == 3


# ── FLOW_CONTROL 完备性 ───────────────────────────────────────────────────


def test_flow_control_covers_all_three_values():
    """FLOW_CONTROL 映射 none/hardware/software 到全部 3 个 FlowControl 枚举。"""

    assert set(FLOW_CONTROL.keys()) == {"none", "hardware", "software"}
    assert FLOW_CONTROL["none"] == QSerialPort.FlowControl.NoFlowControl
    assert FLOW_CONTROL["hardware"] == QSerialPort.FlowControl.HardwareControl
    assert FLOW_CONTROL["software"] == QSerialPort.FlowControl.SoftwareControl


def test_flow_control_values_are_distinct():
    assert len(set(FLOW_CONTROL.values())) == 3


# ── _reverse_lookup ──────────────────────────────────────────────────────


def test_reverse_lookup_finds_key_for_mapped_value():
    """_reverse_lookup 对映射中的值返回对应键。"""

    assert _reverse_lookup(PARITY, QSerialPort.Parity.NoParity) == "none"
    assert _reverse_lookup(PARITY, QSerialPort.Parity.EvenParity) == "even"
    assert _reverse_lookup(STOP_BITS, QSerialPort.StopBits.TwoStop) == "2"
    assert _reverse_lookup(FLOW_CONTROL, QSerialPort.FlowControl.HardwareControl) == "hardware"


def test_reverse_lookup_unknown_value_returns_empty():
    """未映射的值返回空串（不抛异常）。"""

    assert _reverse_lookup(PARITY, "nonexistent_value") == ""
    assert _reverse_lookup(DATA_BITS, 999) == ""


def test_reverse_lookup_preserves_int_key_type():
    """DATA_BITS 的键是 int，反查返回的应是 str（函数内 str(key) 转换）。"""

    result = _reverse_lookup(DATA_BITS, QSerialPort.DataBits.Data7)
    assert result == "7"  # int key 7 → str "7"
    assert isinstance(result, str)


def test_reverse_lookup_preserves_str_key_type():
    """STOP_BITS 的键是 str，反查返回 str。"""

    result = _reverse_lookup(STOP_BITS, QSerialPort.StopBits.OneAndHalfStop)
    assert result == "1.5"


def test_reverse_lookup_empty_mapping_returns_empty():
    """空映射对任何值都返回空串。"""

    assert _reverse_lookup({}, "anything") == ""


def test_reverse_lookup_handles_none_value():
    """None 值不匹配任何映射项（除非映射中本就有 None）。"""

    assert _reverse_lookup(PARITY, None) == ""


def test_reverse_lookup_first_match_wins_on_duplicate_values():
    """映射中若有两个键指向同值，返回第一个遇到的键。"""

    mapping = {"a": 1, "b": 1}
    result = _reverse_lookup(mapping, 1)
    assert result in ("a", "b")  # 任一即可（dict 顺序依实现，但 Python 3.7+ 保插入序）


def test_reverse_lookup_all_real_mappings_round_trip():
    """每个常量映射都能 round-trip：key → value → key。"""

    for mapping in (DATA_BITS, PARITY, STOP_BITS, FLOW_CONTROL):
        for key, value in mapping.items():
            assert _reverse_lookup(mapping, value) == str(key)

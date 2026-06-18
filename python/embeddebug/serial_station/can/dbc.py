"""最小 DBC(数据库) 解析与信号物理值解码，纯 Python 无外部依赖。"""

from __future__ import annotations

from dataclasses import dataclass, field

CAN_FD_MAX_DLC = 64


@dataclass(frozen=True)
class DbcSignal:
    """单个信号定义。"""

    name: str
    start_bit: int
    bit_length: int
    is_little_endian: bool
    factor: float = 1.0
    offset: float = 0.0
    unit: str = ""


@dataclass
class DbcMessage:
    """一条 CAN 报文及其下属信号。"""

    id: int
    name: str
    dlc: int
    is_extended: bool = False
    signals: list[DbcSignal] = field(default_factory=list)

    def signal(self, name: str) -> DbcSignal:
        """按名查找信号。"""
        for sig in self.signals:
            if sig.name == name:
                return sig
        raise KeyError(f"未知信号: {name}")


@dataclass
class DbcDatabase:
    """DBC 数据库：按 id 索引报文集合。"""

    messages: dict[int, DbcMessage] = field(default_factory=dict)

    def message(self, can_id: int) -> DbcMessage:
        """按 CAN id 取报文。"""
        return self.messages[can_id]

    @classmethod
    def parse(cls, text: str) -> "DbcDatabase":
        """解析最小 DBC 文本：仅识别 BO_ 与 SG_ 两类定义行。"""
        db = cls()
        current: DbcMessage | None = None
        for raw_line in text.splitlines():
            line = raw_line.strip()
            if line.startswith("BO_ "):
                current = _parse_bo(line)
                db.messages[current.id] = current
            elif line.startswith("SG_ ") and current is not None:
                current.signals.append(_parse_sg(line))
        return db


def _parse_bo(line: str) -> DbcMessage:
    """解析 BO_ 报文定义行。"""
    parts = line.split()
    msg_id = int(parts[1])
    name_dlc = parts[2]
    name = name_dlc.split(":")[0] if ":" in name_dlc else name_dlc
    dlc = 8
    if ":" in name_dlc:
        rest = name_dlc.split(":", 1)[1]
        dlc = int(rest) if rest.isdigit() else int(parts[3])
    else:
        dlc = int(parts[3]) if len(parts) > 3 and parts[3].isdigit() else 8
    is_extended = msg_id > 0x7FF
    return DbcMessage(id=msg_id, name=name, dlc=dlc, is_extended=is_extended)


def _parse_sg(line: str) -> DbcSignal:
    """解析 SG_ 信号定义行。"""
    head, _, tail = line.partition(":")
    name = head.split()[1].strip()
    body = tail.strip()
    layout, _, rest = body.partition("(")
    layout = layout.strip()
    start_bit = int(layout.split("|")[0])
    len_endian = layout.split("|")[1]
    bit_length = int(len_endian.split("@")[0])
    endian_flag = len_endian.split("@")[1][0]
    is_little_endian = endian_flag == "1"
    factor, offset = 1.0, 0.0
    try:
        inner = rest.split(")")[0].strip().strip("()")
        nums = [x.strip() for x in inner.split(",")]
        factor = float(nums[0])
        offset = float(nums[1]) if len(nums) > 1 else 0.0
    except (ValueError, IndexError):
        factor, offset = 1.0, 0.0
    unit = ""
    if '"' in body:
        unit = body.split('"')[1]
    return DbcSignal(
        name=name,
        start_bit=start_bit,
        bit_length=bit_length,
        is_little_endian=is_little_endian,
        factor=factor,
        offset=offset,
        unit=unit,
    )


def decode_signal(raw_bytes: bytes, signal: DbcSignal) -> float:
    """从原始报文字节解码出信号的物理值。"""
    if signal.bit_length <= 0 or len(raw_bytes) == 0:
        return 0.0
    if signal.is_little_endian:
        raw = _extract_intel(raw_bytes, signal.start_bit, signal.bit_length)
    else:
        raw = _extract_motorola(raw_bytes, signal.start_bit, signal.bit_length)
    return raw * signal.factor + signal.offset


def _extract_intel(data: bytes, start_bit: int, bit_length: int) -> int:
    """Intel 小端提取。"""
    value = 0
    for index, byte in enumerate(data):
        value |= byte << (index * 8)
    return (value >> start_bit) & ((1 << bit_length) - 1)


def _extract_motorola(data: bytes, start_bit: int, bit_length: int) -> int:
    """Motorola 大端提取（简化模型）。"""
    result = 0
    for i in range(bit_length):
        byte_index = (start_bit - i) // 8
        if byte_index < 0 or byte_index >= len(data):
            continue
        bit_pos = 7 - ((start_bit - i) % 8)
        bit = (data[byte_index] >> bit_pos) & 1
        result = (result << 1) | bit
    return result

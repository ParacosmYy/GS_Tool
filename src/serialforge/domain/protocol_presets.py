"""Bounded built-in protocol presets independent of transports and Qt."""

from __future__ import annotations

from dataclasses import dataclass

from .errors import ConfigurationError
from .protocols import ChecksumKind, FramingKind, ProtocolConfig
from .timing import ModbusRtuTiming

MAX_PROTOCOL_PRESETS = 16
MAX_PROTOCOL_PRESET_KEY_CHARS = 64
MAX_PROTOCOL_PRESET_LABEL_CHARS = 96
MAX_PROTOCOL_PRESET_DESCRIPTION_CHARS = 256


@dataclass(frozen=True, slots=True)
class ProtocolPreset:
    """An immutable editor preset; applying it remains an explicit UI action."""

    key: str
    label: str
    description: str
    config: ProtocolConfig

    def __post_init__(self) -> None:
        if (
            not isinstance(self.key, str)
            or not self.key
            or len(self.key) > MAX_PROTOCOL_PRESET_KEY_CHARS
            or not self.key.isascii()
            or any(char not in "abcdefghijklmnopqrstuvwxyz0123456789_-" for char in self.key)
        ):
            raise ConfigurationError("协议 preset key 必须是有界 ASCII 标识符。")
        if (
            not isinstance(self.label, str)
            or not self.label.strip()
            or len(self.label) > MAX_PROTOCOL_PRESET_LABEL_CHARS
        ):
            raise ConfigurationError("协议 preset label 不能为空且必须有界。")
        if (
            not isinstance(self.description, str)
            or not self.description.strip()
            or len(self.description) > MAX_PROTOCOL_PRESET_DESCRIPTION_CHARS
        ):
            raise ConfigurationError("协议 preset description 不能为空且必须有界。")
        if not isinstance(self.config, ProtocolConfig):
            raise ConfigurationError("协议 preset config 类型无效。")


def _validate_catalog(catalog: tuple[ProtocolPreset, ...]) -> tuple[ProtocolPreset, ...]:
    """Validate the bounded built-in catalog once at import time."""

    if not 1 <= len(catalog) <= MAX_PROTOCOL_PRESETS:
        raise ConfigurationError(f"协议 preset 数量必须在 1 到 {MAX_PROTOCOL_PRESETS} 项之间。")
    keys = tuple(item.key for item in catalog)
    if len(set(keys)) != len(keys):
        raise ConfigurationError("协议 preset key 不能重复。")
    configs = tuple(item.config for item in catalog)
    if len(set(configs)) != len(configs):
        raise ConfigurationError("协议 preset config 不能重复。")
    return catalog


_BUILTIN_PROTOCOL_PRESETS = _validate_catalog(
    (
        ProtocolPreset(
            key="raw",
            label="Raw bytes",
            description="每个接收 chunk 作为一帧；不做 framing 和 checksum。",
            config=ProtocolConfig(),
        ),
        ProtocolPreset(
            key="line",
            label="Line (LF/CRLF)",
            description="按 LF 分帧，并兼容去除行尾 CR；适合文本行协议。",
            config=ProtocolConfig(framing=FramingKind.LINE),
        ),
        ProtocolPreset(
            key="delimiter_aa55",
            label="Delimiter AA 55",
            description="以 AA 55 作为分隔符；分隔符不包含在 frame payload 中。",
            config=ProtocolConfig(framing=FramingKind.DELIMITER, delimiter=b"\xaa\x55"),
        ),
        ProtocolPreset(
            key="length_u8_le",
            label="Length U8 LE",
            description="1 字节 little-endian payload 长度；不附带 checksum。",
            config=ProtocolConfig(
                framing=FramingKind.LENGTH_PREFIXED,
                length_bytes=1,
                byteorder="little",
            ),
        ),
        ProtocolPreset(
            key="length_u16_le_crc16",
            label="Length U16 LE + CRC16",
            description="2 字节 little-endian payload 长度，末尾 CRC16/Modbus little-endian。",
            config=ProtocolConfig(
                framing=FramingKind.LENGTH_PREFIXED,
                checksum=ChecksumKind.CRC16_MODBUS,
                length_bytes=2,
                byteorder="little",
                checksum_byteorder="little",
            ),
        ),
        ProtocolPreset(
            key="nmea0183_line",
            label="NMEA 0183 · Line + XOR",
            description="按 LF/CRLF 分帧，校验 $ 与 * 之间的 XOR 并移除 *HH 后缀。",
            config=ProtocolConfig(
                framing=FramingKind.LINE,
                checksum=ChecksumKind.NMEA0183,
            ),
        ),
        ProtocolPreset(
            key="mavlink_stream",
            label="MAVLink v1/v2 · Stream",
            description=(
                "按 0xFE/0xFD、长度字段和 v2 signature flag 从噪声流重同步；"
                "CRC_EXTRA 仍由显式 profile 校验。"
            ),
            config=ProtocolConfig(
                framing=FramingKind.MAVLINK_STREAM,
                max_frame_bytes=280,
            ),
        ),
        ProtocolPreset(
            key="modbus_rtu_timed",
            label="Modbus RTU · Host-gap",
            description=(
                "按 UART 参数计算 t1.5/t3.5，并使用主机 read gap 作为边界观察；"
                "不是 per-byte wire timestamp，适合先做诊断验证。"
            ),
            config=ProtocolConfig(
                framing=FramingKind.MODBUS_RTU_TIMED,
                max_frame_bytes=256,
                modbus_timing=ModbusRtuTiming(),
            ),
        ),
    )
)


def builtin_protocol_presets() -> tuple[ProtocolPreset, ...]:
    """Return the bounded immutable catalog of generic built-in presets."""

    return _BUILTIN_PROTOCOL_PRESETS


def preset_for_config(config: ProtocolConfig) -> ProtocolPreset | None:
    """Return the matching built-in preset, or ``None`` for custom settings."""

    if not isinstance(config, ProtocolConfig):
        raise ConfigurationError("协议 preset lookup 只接受 ProtocolConfig。")
    return next(
        (preset for preset in _BUILTIN_PROTOCOL_PRESETS if preset.config == config),
        None,
    )

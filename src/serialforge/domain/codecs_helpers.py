"""Shared bounded helpers for component codec implementations."""

from __future__ import annotations

import json
import math

from .codecs_config import (
    MAX_JSON_DEPTH,
    MAX_JSON_NODES,
    MAX_JSON_STRING_BYTES,
    ComponentCodecConfig,
    JsonCodecConfig,
    JsonFieldSpec,
    JsonValueKind,
    MavlinkCodecConfig,
    MavlinkFieldSource,
    MavlinkFieldSpec,
    ModbusRtuCodecConfig,
    ModbusRtuFieldSource,
    ModbusRtuFieldSpec,
    TlvCodecConfig,
)
from .components import (
    MAX_COMPONENT_DISPLAY_LENGTH,
    MAX_COMPONENT_FIELD_BYTES,
    ComponentFieldValue,
    ComponentFrameRow,
    ValueKind,
)
from .errors import ConfigurationError
from .mavlink import MavlinkFrame, MavlinkVersion
from .modbus import ModbusRtuFrame
from .protocols import DecodedFrame, FrameStatus, ProtocolSource


def _modbus_value_kind(source: ModbusRtuFieldSource) -> ValueKind:
    if source is ModbusRtuFieldSource.IS_EXCEPTION:
        return ValueKind.BOOLEAN
    if source is ModbusRtuFieldSource.DATA_HEX:
        return ValueKind.HEX
    return ValueKind.UINT


def _modbus_field(
    decoded: ModbusRtuFrame,
    spec: ModbusRtuFieldSpec,
) -> tuple[bytes, str, str | int | bool | None] | None:
    source = spec.source
    if source is ModbusRtuFieldSource.ADDRESS:
        if decoded.address is None:
            return None
        label = "broadcast (0)" if decoded.address == 0 else str(decoded.address)
        return bytes((decoded.address,)), label, decoded.address
    if source is ModbusRtuFieldSource.FUNCTION:
        if decoded.function is None:
            return None
        return bytes((decoded.function,)), f"0x{decoded.function:02X}", decoded.function
    if source is ModbusRtuFieldSource.BASE_FUNCTION:
        if decoded.base_function is None:
            return None
        return (
            bytes((decoded.base_function,)),
            f"0x{decoded.base_function:02X}",
            decoded.base_function,
        )
    if source is ModbusRtuFieldSource.IS_EXCEPTION:
        is_exception = decoded.is_exception
        if is_exception is None:
            return None
        raw = b"\x01" if is_exception else b"\x00"
        return raw, "true" if is_exception else "false", is_exception
    if decoded.status in {FrameStatus.INCOMPLETE, FrameStatus.OVERSIZE}:
        return None
    if source is ModbusRtuFieldSource.DATA_LENGTH:
        length = len(decoded.data)
        return length.to_bytes(2, "little"), str(length), length
    if source is ModbusRtuFieldSource.DATA_HEX:
        return decoded.data, decoded.data.hex(" ").upper() or "<empty>", None
    if source is ModbusRtuFieldSource.EXCEPTION_CODE:
        if decoded.is_exception is not True or not decoded.data:
            return None
        code = decoded.data[0]
        return bytes((code,)), f"0x{code:02X}", code
    if source is ModbusRtuFieldSource.RECEIVED_CRC:
        if decoded.received_crc is None:
            return None
        return (
            decoded.received_crc.to_bytes(2, "little"),
            f"0x{decoded.received_crc:04X}",
            decoded.received_crc,
        )
    if source is ModbusRtuFieldSource.CALCULATED_CRC:
        if decoded.calculated_crc is None:
            return None
        return (
            decoded.calculated_crc.to_bytes(2, "little"),
            f"0x{decoded.calculated_crc:04X}",
            decoded.calculated_crc,
        )
    raise ConfigurationError("未知 Modbus RTU 字段 source。")


def _mavlink_value_kind(source: MavlinkFieldSource) -> ValueKind:
    if source is MavlinkFieldSource.VERSION:
        return ValueKind.STRING
    if source is MavlinkFieldSource.SIGNED:
        return ValueKind.BOOLEAN
    if source in {MavlinkFieldSource.PAYLOAD_HEX, MavlinkFieldSource.SIGNATURE_HEX}:
        return ValueKind.HEX
    return ValueKind.UINT


def _mavlink_field(
    decoded: MavlinkFrame,
    spec: MavlinkFieldSpec,
) -> tuple[bytes, str, str | int | bool | None] | None:
    source = spec.source
    if source is MavlinkFieldSource.VERSION:
        if decoded.version is None:
            return None
        raw = decoded.version.value.encode("ascii")
        return raw, decoded.version.value, decoded.version.value
    if source is MavlinkFieldSource.PAYLOAD_LENGTH:
        if decoded.payload_length is None:
            return None
        return bytes((decoded.payload_length,)), str(decoded.payload_length), decoded.payload_length
    if source is MavlinkFieldSource.INCOMPAT_FLAGS:
        if decoded.version is MavlinkVersion.V1:
            return b"", "<n/a>", None
        if decoded.incompat_flags is None:
            return None
        return (
            bytes((decoded.incompat_flags,)),
            f"0x{decoded.incompat_flags:02X}",
            decoded.incompat_flags,
        )
    if source is MavlinkFieldSource.COMPAT_FLAGS:
        if decoded.version is MavlinkVersion.V1:
            return b"", "<n/a>", None
        if decoded.compat_flags is None:
            return None
        return bytes((decoded.compat_flags,)), f"0x{decoded.compat_flags:02X}", decoded.compat_flags
    if source is MavlinkFieldSource.SEQUENCE:
        if decoded.sequence is None:
            return None
        return bytes((decoded.sequence,)), str(decoded.sequence), decoded.sequence
    if source is MavlinkFieldSource.SYSTEM_ID:
        if decoded.system_id is None:
            return None
        return bytes((decoded.system_id,)), str(decoded.system_id), decoded.system_id
    if source is MavlinkFieldSource.COMPONENT_ID:
        if decoded.component_id is None:
            return None
        return bytes((decoded.component_id,)), str(decoded.component_id), decoded.component_id
    if source is MavlinkFieldSource.MESSAGE_ID:
        if decoded.message_id is None:
            return None
        raw = decoded.message_id.to_bytes(3, "little")
        return raw, f"0x{decoded.message_id:06X}", decoded.message_id
    if source is MavlinkFieldSource.SIGNED:
        if decoded.signed is None:
            return None
        return (
            b"\x01" if decoded.signed else b"\x00",
            "true" if decoded.signed else "false",
            decoded.signed,
        )
    if source is MavlinkFieldSource.CRC_EXTRA:
        if decoded.crc_extra is None:
            return None
        return bytes((decoded.crc_extra,)), str(decoded.crc_extra), decoded.crc_extra
    if source is MavlinkFieldSource.RECEIVED_CRC:
        if decoded.received_crc is None:
            return None
        return (
            decoded.received_crc.to_bytes(2, "little"),
            f"0x{decoded.received_crc:04X}",
            decoded.received_crc,
        )
    if source is MavlinkFieldSource.CALCULATED_CRC:
        if decoded.calculated_crc is None:
            return None
        return (
            decoded.calculated_crc.to_bytes(2, "little"),
            f"0x{decoded.calculated_crc:04X}",
            decoded.calculated_crc,
        )
    if source is MavlinkFieldSource.PAYLOAD_HEX:
        if decoded.version is None:
            return None
        return decoded.message_payload, decoded.message_payload.hex(" ").upper() or "<empty>", None
    if source is MavlinkFieldSource.SIGNATURE_HEX:
        if decoded.version is None:
            return None
        if decoded.version is MavlinkVersion.V1:
            return b"", "<none>", None
        if decoded.signed is None:
            return None
        return decoded.signature, decoded.signature.hex(" ").upper() or "<none>", None
    raise ConfigurationError("未知 MAVLink 字段 source。")


def _row(
    frame: DecodedFrame,
    source: ProtocolSource,
    occurred_at: float,
    fields: tuple[ComponentFieldValue, ...],
    *,
    error: str | None,
    codec_error: str | None,
    status: FrameStatus | None = None,
) -> ComponentFrameRow:
    payload = frame.payload[:512]
    payload_hex = payload.hex(" ").upper()
    if len(frame.payload) > 512:
        payload_hex += " …"
    return ComponentFrameRow(
        source=source,
        sequence=frame.sequence,
        status=frame.status if status is None else status,
        payload_length=len(frame.payload),
        payload_hex=payload_hex,
        fields=fields,
        error=error,
        codec_error=codec_error,
        occurred_at=occurred_at,
    )


def _component_error(
    name: str,
    kind: ValueKind,
    error: str,
    *,
    raw: bytes = b"",
) -> ComponentFieldValue:
    return ComponentFieldValue(
        name=name,
        kind=kind,
        raw=raw[:MAX_COMPONENT_FIELD_BYTES],
        display=(
            raw[:MAX_COMPONENT_DISPLAY_LENGTH].decode("utf-8", errors="replace")
            if raw
            else "<错误>"
        ),
        error=error[:MAX_COMPONENT_DISPLAY_LENGTH],
    )


def _require_codec(
    configuration: ComponentCodecConfig,
    expected: (
        type[JsonCodecConfig]
        | type[TlvCodecConfig]
        | type[ModbusRtuCodecConfig]
        | type[MavlinkCodecConfig]
    ),
) -> JsonCodecConfig | TlvCodecConfig | ModbusRtuCodecConfig | MavlinkCodecConfig:
    if not isinstance(configuration, ComponentCodecConfig) or not isinstance(
        configuration.codec, expected
    ):
        raise ConfigurationError("组件 codec 配置与实现不匹配。")
    return configuration.codec


def _validate_json_tree(value: object, *, depth: int = 0, nodes: list[int] | None = None) -> None:
    counter = nodes if nodes is not None else [0]
    counter[0] += 1
    if counter[0] > MAX_JSON_NODES:
        raise ValueError(f"JSON 节点数超过 {MAX_JSON_NODES} 上限")
    if depth > MAX_JSON_DEPTH:
        raise ValueError(f"JSON 嵌套深度超过 {MAX_JSON_DEPTH} 上限")
    if isinstance(value, str):
        if len(value.encode("utf-8")) > MAX_JSON_STRING_BYTES:
            raise ValueError(f"JSON 字符串超过 {MAX_JSON_STRING_BYTES} 字节上限")
    elif isinstance(value, dict):
        for key, child in value.items():
            if len(key.encode("utf-8")) > MAX_JSON_STRING_BYTES:
                raise ValueError("JSON key 超过长度上限")
            _validate_json_tree(child, depth=depth + 1, nodes=counter)
    elif isinstance(value, list):
        for child in value:
            _validate_json_tree(child, depth=depth + 1, nodes=counter)
    elif isinstance(value, float) and not math.isfinite(value):
        raise ValueError("JSON number 必须是有限数字")


def _matches_json_kind(value: object, kind: JsonValueKind) -> bool:
    if kind is JsonValueKind.STRING:
        return isinstance(value, str)
    if kind is JsonValueKind.NUMBER:
        return isinstance(value, (int, float)) and not isinstance(value, bool)
    if kind is JsonValueKind.BOOLEAN:
        return isinstance(value, bool)
    return True


def _json_value_kind(kind: JsonValueKind) -> ValueKind:
    return {
        JsonValueKind.STRING: ValueKind.STRING,
        JsonValueKind.NUMBER: ValueKind.NUMBER,
        JsonValueKind.BOOLEAN: ValueKind.BOOLEAN,
        JsonValueKind.JSON: ValueKind.JSON,
    }[kind]


def _json_scalar_value(value: object) -> str | int | float | bool | None:
    if isinstance(value, (str, int, float, bool)) and not (
        isinstance(value, float) and not math.isfinite(value)
    ):
        return value
    return None


def _bounded_json_raw(value: object) -> bytes:
    text = json.dumps(value, ensure_ascii=False, separators=(",", ":"), allow_nan=False)
    return text.encode("utf-8")


def _format_json_value(value: object, spec: JsonFieldSpec) -> str:
    if spec.value_kind is JsonValueKind.STRING:
        return str(value)[:MAX_COMPONENT_DISPLAY_LENGTH]
    if spec.value_kind is JsonValueKind.BOOLEAN:
        return "true" if value else "false"
    if spec.value_kind is JsonValueKind.JSON:
        return json.dumps(value, ensure_ascii=False, separators=(",", ":"))[
            :MAX_COMPONENT_DISPLAY_LENGTH
        ]
    number = value * spec.scale  # type: ignore[operator]
    if isinstance(number, float) and not math.isfinite(number):
        raise ValueError("JSON scale 结果必须是有限数字")
    if isinstance(value, int) and spec.scale == 1.0:
        text = str(value)
    else:
        text = f"{number:.6g}"
    return f"{text} {spec.unit}".strip()


def _bounded_error(prefix: str, exc: BaseException) -> str:
    detail = str(exc).strip().replace("\r", " ").replace("\n", " ")
    return f"{prefix}：{detail[:240]}。"


def _append_error(current: str | None, message: str) -> str:
    if current is None:
        return message
    return f"{current} {message}"

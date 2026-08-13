"""Bounded, transport-neutral component profiles and binary field decoding."""

from __future__ import annotations

import json
import math
import struct
from dataclasses import dataclass
from enum import StrEnum
from typing import Final

from .errors import ConfigurationError
from .protocols import DecodedFrame, FrameStatus, ProtocolSource

MAX_COMPONENT_PROFILE_BYTES: Final = 64 * 1024
MAX_COMPONENT_FIELDS: Final = 32
MAX_COMPONENT_FIELD_NAME: Final = 64
MAX_COMPONENT_FIELD_BYTES: Final = 1_024
MAX_COMPONENT_UNIT_LENGTH: Final = 32
MAX_COMPONENT_PREVIEW_BYTES: Final = 512
MAX_COMPONENT_DISPLAY_LENGTH: Final = 4_096
MAX_COMPONENT_ROWS: Final = 256

type ScalarValue = str | int | float | bool | None


class FieldKind(StrEnum):
    """Safe built-in field representations; profiles never execute code."""

    HEX = "hex"
    UTF8 = "utf8"
    UINT = "uint"
    INT = "int"
    FLOAT32 = "float32"


class ValueKind(StrEnum):
    """Stable display kinds shared by binary and structured codecs."""

    HEX = "hex"
    UTF8 = "utf8"
    UINT = "uint"
    INT = "int"
    FLOAT32 = "float32"
    STRING = "string"
    NUMBER = "number"
    BOOLEAN = "boolean"
    JSON = "json"


@dataclass(frozen=True, slots=True)
class FieldSpec:
    """One bounded fixed-offset field in a component profile."""

    name: str
    offset: int
    length: int
    kind: FieldKind = FieldKind.HEX
    byteorder: str = "little"
    scale: float = 1.0
    unit: str = ""

    def __post_init__(self) -> None:
        if not isinstance(self.name, str):
            raise ConfigurationError("组件字段 name 必须是字符串。")
        name = self.name.strip()
        if not name or len(name) > MAX_COMPONENT_FIELD_NAME:
            raise ConfigurationError(f"组件字段名称必须为 1 到 {MAX_COMPONENT_FIELD_NAME} 个字符。")
        if isinstance(self.offset, bool) or not isinstance(self.offset, int) or self.offset < 0:
            raise ConfigurationError("组件字段 offset 必须是非负整数。")
        if (
            isinstance(self.length, bool)
            or not isinstance(self.length, int)
            or not 1 <= self.length <= MAX_COMPONENT_FIELD_BYTES
        ):
            raise ConfigurationError(
                f"组件字段 length 必须在 1 到 {MAX_COMPONENT_FIELD_BYTES} 之间。"
            )
        if not isinstance(self.kind, FieldKind):
            raise ConfigurationError("组件字段 kind 必须使用 FieldKind。")
        if not isinstance(self.byteorder, str) or self.byteorder not in {"little", "big"}:
            raise ConfigurationError("组件字段 byteorder 必须是 little 或 big。")
        if (
            isinstance(self.scale, bool)
            or not isinstance(self.scale, (int, float))
            or not math.isfinite(float(self.scale))
        ):
            raise ConfigurationError("组件字段 scale 必须是有限数字。")
        if not isinstance(self.unit, str):
            raise ConfigurationError("组件字段 unit 必须是字符串。")
        unit = self.unit.strip()
        if len(unit) > MAX_COMPONENT_UNIT_LENGTH:
            raise ConfigurationError(f"组件字段 unit 不能超过 {MAX_COMPONENT_UNIT_LENGTH} 个字符。")
        if self.kind in {FieldKind.UINT, FieldKind.INT} and self.length not in {1, 2, 4, 8}:
            raise ConfigurationError("整数组件字段 length 只能是 1、2、4 或 8。")
        if self.kind is FieldKind.FLOAT32 and self.length != 4:
            raise ConfigurationError("FLOAT32 组件字段 length 必须是 4。")
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "unit", unit)
        object.__setattr__(self, "scale", float(self.scale))


@dataclass(frozen=True, slots=True)
class ComponentProfile:
    """Versioned declarative profile for a bounded component view."""

    name: str = "Raw frame"
    schema_version: int = 1
    fields: tuple[FieldSpec, ...] = ()

    def __post_init__(self) -> None:
        if not isinstance(self.name, str):
            raise ConfigurationError("组件 profile name 必须是字符串。")
        name = self.name.strip()
        if not name or len(name) > MAX_COMPONENT_FIELD_NAME:
            raise ConfigurationError("组件 profile 名称长度无效。")
        if (
            isinstance(self.schema_version, bool)
            or not isinstance(self.schema_version, int)
            or self.schema_version != 1
        ):
            raise ConfigurationError("不支持的组件 profile schema 版本。")
        fields = tuple(self.fields)
        if len(fields) > MAX_COMPONENT_FIELDS:
            raise ConfigurationError(f"组件 profile 字段数不能超过 {MAX_COMPONENT_FIELDS}。")
        if any(not isinstance(field, FieldSpec) for field in fields):
            raise ConfigurationError("组件 profile fields 必须使用 FieldSpec。")
        names = [field.name.casefold() for field in fields]
        if len(names) != len(set(names)):
            raise ConfigurationError("组件 profile 字段名称不能重复。")
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "fields", fields)


@dataclass(frozen=True, slots=True)
class ComponentFieldValue:
    """One safe field result; malformed fields stay visible as an error value."""

    name: str
    kind: ValueKind
    raw: bytes
    display: str
    error: str | None = None
    value: ScalarValue = None

    def __post_init__(self) -> None:
        if isinstance(self.kind, FieldKind):
            object.__setattr__(self, "kind", ValueKind(self.kind.value))
        elif not isinstance(self.kind, ValueKind):
            raise ConfigurationError("组件字段结果 kind 类型无效。")
        if not isinstance(self.raw, (bytes, bytearray, memoryview)):
            raise ConfigurationError("组件字段结果 raw 必须是 bytes-like。")
        raw = bytes(self.raw)
        if len(raw) > MAX_COMPONENT_FIELD_BYTES:
            raise ConfigurationError("组件字段结果 raw 超过上限。")
        display = str(self.display)
        if len(display) > MAX_COMPONENT_DISPLAY_LENGTH:
            raise ConfigurationError("组件字段结果 display 超过上限。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("组件字段结果 error 不能为空字符串。")
        if self.value is not None and not isinstance(self.value, (str, int, float, bool)):
            raise ConfigurationError("组件字段结果 value 必须是有限 scalar。")
        if isinstance(self.value, float) and not math.isfinite(self.value):
            raise ConfigurationError("组件字段结果 value 必须是有限 scalar。")
        if (
            isinstance(self.value, str)
            and len(self.value.encode("utf-8")) > MAX_COMPONENT_DISPLAY_LENGTH
        ):
            raise ConfigurationError("组件字段结果 value 超过显示上限。")
        object.__setattr__(self, "raw", raw)
        object.__setattr__(self, "display", display)


@dataclass(frozen=True, slots=True)
class ComponentFrameRow:
    """Bounded table row derived from one decoded protocol frame."""

    source: ProtocolSource
    sequence: int
    status: FrameStatus
    payload_length: int
    payload_hex: str
    fields: tuple[ComponentFieldValue, ...] = ()
    error: str | None = None
    codec_error: str | None = None
    occurred_at: float = 0.0

    def __post_init__(self) -> None:
        if not isinstance(self.source, ProtocolSource):
            raise ConfigurationError("组件 frame source 类型无效。")
        if (
            isinstance(self.sequence, bool)
            or not isinstance(self.sequence, int)
            or self.sequence < 1
        ):
            raise ConfigurationError("组件 frame sequence 必须是正整数。")
        if not isinstance(self.status, FrameStatus):
            raise ConfigurationError("组件 frame status 类型无效。")
        if isinstance(self.payload_length, bool) or not 0 <= self.payload_length <= 65_536:
            raise ConfigurationError("组件 frame payload_length 超过范围。")
        fields = tuple(self.fields)
        if len(fields) > MAX_COMPONENT_FIELDS:
            raise ConfigurationError("组件 frame fields 超过上限。")
        if any(not isinstance(field, ComponentFieldValue) for field in fields):
            raise ConfigurationError("组件 frame fields 类型无效。")
        payload_hex = str(self.payload_hex)
        if len(payload_hex) > MAX_COMPONENT_PREVIEW_BYTES * 2 + 3:
            raise ConfigurationError("组件 frame payload_hex 超过预览上限。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("组件 frame error 不能为空字符串。")
        if self.codec_error is not None and not self.codec_error.strip():
            raise ConfigurationError("组件 frame codec_error 不能为空字符串。")
        object.__setattr__(self, "fields", fields)
        object.__setattr__(self, "payload_hex", payload_hex)


@dataclass(frozen=True, slots=True)
class ComponentStats:
    """Bounded component worker counters."""

    frames_in: int = 0
    rows_out: int = 0
    field_errors: int = 0
    codec_errors: int = 0
    dropped_frames: int = 0
    buffered_bytes: int = 0


def default_component_profile() -> ComponentProfile:
    """Return the safe built-in profile that exposes frame metadata and raw Hex."""

    return ComponentProfile()


class ComponentProfileCodec:
    """Parse/dump a strict JSON profile without expressions or dynamic imports."""

    _PROFILE_KEYS = {"name", "schema_version", "fields"}
    _FIELD_KEYS = {"name", "offset", "length", "kind", "byteorder", "scale", "unit"}

    @classmethod
    def loads(cls, text: str) -> ComponentProfile:
        if not isinstance(text, str):
            raise ConfigurationError("组件 profile 内容必须是文本。")
        if len(text.encode("utf-8")) > MAX_COMPONENT_PROFILE_BYTES:
            raise ConfigurationError("组件 profile 超过 64 KiB 上限。")
        try:
            value = json.loads(text)
        except json.JSONDecodeError as exc:
            raise ConfigurationError(f"组件 profile JSON 无效：{exc.msg}。") from exc
        if not isinstance(value, dict) or set(value) - cls._PROFILE_KEYS:
            raise ConfigurationError("组件 profile 顶层字段不受支持。")
        fields_value = value.get("fields", [])
        if not isinstance(fields_value, list):
            raise ConfigurationError("组件 profile fields 必须是数组。")
        fields: list[FieldSpec] = []
        for item in fields_value:
            if not isinstance(item, dict) or set(item) - cls._FIELD_KEYS:
                raise ConfigurationError("组件 profile 字段定义不受支持。")
            try:
                fields.append(
                    FieldSpec(
                        name=item["name"],
                        offset=item["offset"],
                        length=item["length"],
                        kind=FieldKind(item.get("kind", FieldKind.HEX)),
                        byteorder=item.get("byteorder", "little"),
                        scale=item.get("scale", 1.0),
                        unit=item.get("unit", ""),
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("组件 profile 字段定义无效。", detail=str(exc)) from exc
        return ComponentProfile(
            name=value.get("name", "Raw frame"),
            schema_version=value.get("schema_version", 1),
            fields=tuple(fields),
        )

    @staticmethod
    def dumps(profile: ComponentProfile) -> str:
        if not isinstance(profile, ComponentProfile):
            raise ConfigurationError("只能序列化 ComponentProfile。")
        value = {
            "name": profile.name,
            "schema_version": profile.schema_version,
            "fields": [
                {
                    "name": field.name,
                    "offset": field.offset,
                    "length": field.length,
                    "kind": field.kind.value,
                    "byteorder": field.byteorder,
                    "scale": field.scale,
                    "unit": field.unit,
                }
                for field in profile.fields
            ],
        }
        text = json.dumps(value, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
        if len(text.encode("utf-8")) > MAX_COMPONENT_PROFILE_BYTES:
            raise ConfigurationError("组件 profile 序列化结果超过 64 KiB 上限。")
        return text


class BinaryComponentCodec:
    """Decode fixed fields with explicit bounds and no user-defined execution."""

    def decode(
        self,
        frame: DecodedFrame,
        profile: ComponentProfile,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        if not isinstance(frame, DecodedFrame) or not isinstance(profile, ComponentProfile):
            raise ConfigurationError("组件 codec 输入类型无效。")
        if not isinstance(source, ProtocolSource):
            raise ConfigurationError("组件 codec source 类型无效。")
        values: list[ComponentFieldValue] = []
        field_errors = 0
        for spec in profile.fields:
            end = spec.offset + spec.length
            raw = frame.payload[spec.offset : end]
            if len(raw) != spec.length:
                field_errors += 1
                values.append(
                    ComponentFieldValue(
                        name=spec.name,
                        kind=ValueKind(spec.kind.value),
                        raw=raw,
                        display="<缺少字节>",
                        error="字段超出 frame payload。",
                    )
                )
                continue
            try:
                value = self._decode_value(raw, spec)
                display = self._format_value(raw, spec)
                error = None
            except (UnicodeDecodeError, struct.error, ValueError, OverflowError) as exc:
                field_errors += 1
                display = raw.hex(" ").upper()
                error = f"字段解码失败：{type(exc).__name__}。"
                value = None
            values.append(
                ComponentFieldValue(
                    name=spec.name,
                    kind=ValueKind(spec.kind.value),
                    raw=raw,
                    display=display,
                    error=error,
                    value=value,
                )
            )
        payload = frame.payload[:MAX_COMPONENT_PREVIEW_BYTES]
        payload_hex = payload.hex(" ").upper()
        if len(frame.payload) > MAX_COMPONENT_PREVIEW_BYTES:
            payload_hex += " …"
        error = frame.error
        if field_errors and error is None:
            error = f"{field_errors} 个字段解码失败。"
        return ComponentFrameRow(
            source=source,
            sequence=frame.sequence,
            status=frame.status,
            payload_length=len(frame.payload),
            payload_hex=payload_hex,
            fields=tuple(values),
            error=error,
            occurred_at=occurred_at,
        )

    @staticmethod
    def _format_value(raw: bytes, spec: FieldSpec) -> str:
        value = BinaryComponentCodec._decode_value(raw, spec)
        if spec.kind is FieldKind.HEX:
            return raw.hex(" ").upper()
        if spec.kind is FieldKind.UTF8:
            return str(value)
        return _format_scaled(value, spec)

    @staticmethod
    def _decode_value(raw: bytes, spec: FieldSpec) -> ScalarValue:
        if spec.kind is FieldKind.HEX:
            return None
        if spec.kind is FieldKind.UTF8:
            return raw.decode("utf-8")
        if spec.kind is FieldKind.UINT:
            value = int.from_bytes(raw, spec.byteorder, signed=False)
            return _format_scaled(value, spec)
        if spec.kind is FieldKind.INT:
            value = int.from_bytes(raw, spec.byteorder, signed=True)
            return _format_scaled(value, spec)
        prefix = "<" if spec.byteorder == "little" else ">"
        value = struct.unpack(f"{prefix}f", raw)[0]
        if not math.isfinite(value):
            raise ValueError("FLOAT32 必须是有限数字")
        return value


def _format_scaled(value: int | float, spec: FieldSpec) -> str:
    scaled = value * spec.scale
    if isinstance(scaled, float) and not math.isfinite(scaled):
        raise ValueError("scale 结果必须是有限数字")
    if spec.scale == 1.0 and isinstance(value, int):
        text = str(value)
    else:
        text = f"{scaled:.6g}"
    return f"{text} {spec.unit}".strip()


__all__ = [
    "BinaryComponentCodec",
    "ComponentFieldValue",
    "ComponentFrameRow",
    "ComponentProfile",
    "ComponentProfileCodec",
    "ComponentStats",
    "FieldKind",
    "FieldSpec",
    "ScalarValue",
    "ValueKind",
    "default_component_profile",
]

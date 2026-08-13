"""Versioned, declarative component codecs for structured RX payloads.

This module deliberately stays transport- and Qt-free.  A codec consumes one
already-framed ``DecodedFrame`` and produces a bounded derived row.  The raw
payload and transport events remain owned by the existing session/recorder
paths.
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from enum import StrEnum
from typing import Final

from .components import (
    MAX_COMPONENT_FIELD_BYTES,
    MAX_COMPONENT_FIELDS,
    MAX_COMPONENT_PROFILE_BYTES,
    MAX_COMPONENT_UNIT_LENGTH,
    ComponentProfile,
    ComponentProfileCodec,
    FieldKind,
)
from .errors import ConfigurationError

MAX_CODEC_PATH_BYTES: Final = 512
MAX_CODEC_PATH_SEGMENTS: Final = 16
MAX_JSON_DEPTH: Final = 16
MAX_JSON_NODES: Final = 512
MAX_JSON_STRING_BYTES: Final = 4_096
MAX_TLV_ITEMS: Final = 128
MAX_TLV_TAG: Final = 0xFFFF_FFFF


class ComponentCodecKind(StrEnum):
    """Built-in codec identifiers; registration remains explicit and static."""

    JSON = "json"
    TLV = "tlv"
    MODBUS_RTU = "modbus_rtu"
    MAVLINK = "mavlink"


class JsonValueKind(StrEnum):
    """Allowed JSON value types for a field binding."""

    STRING = "string"
    NUMBER = "number"
    BOOLEAN = "boolean"
    JSON = "json"


@dataclass(frozen=True, slots=True)
class JsonFieldSpec:
    """One bounded JSON Pointer binding."""

    name: str
    path: tuple[str, ...]
    value_kind: JsonValueKind = JsonValueKind.STRING
    scale: float = 1.0
    unit: str = ""

    def __post_init__(self) -> None:
        _validate_name(self.name, "JSON 字段")
        if not isinstance(self.path, tuple):
            raise ConfigurationError("JSON path 必须使用不可变 tuple。")
        path = tuple(self.path)
        if len(path) > MAX_CODEC_PATH_SEGMENTS:
            raise ConfigurationError(f"JSON path 段数不能超过 {MAX_CODEC_PATH_SEGMENTS}。")
        if any(not isinstance(segment, str) for segment in path):
            raise ConfigurationError("JSON path 段必须是字符串。")
        if any(len(segment) > 128 for segment in path):
            raise ConfigurationError("JSON path 单段长度超过上限。")
        if any(segment in {"*", "-"} for segment in path):
            raise ConfigurationError("JSON path 不支持 wildcard 或 append 段。")
        if len(_encode_pointer(path)) > MAX_CODEC_PATH_BYTES:
            raise ConfigurationError(f"JSON path 不能超过 {MAX_CODEC_PATH_BYTES} 字节。")
        if not isinstance(self.value_kind, JsonValueKind):
            raise ConfigurationError("JSON 字段 value_kind 类型无效。")
        scale = _validate_scale(self.scale, "JSON 字段 scale")
        unit = _validate_unit(self.unit, "JSON 字段 unit")
        if self.value_kind is not JsonValueKind.NUMBER and scale != 1.0:
            raise ConfigurationError("只有 JSON number 字段允许 scale。")
        if self.value_kind is not JsonValueKind.NUMBER and unit:
            raise ConfigurationError("只有 JSON number 字段允许 unit。")
        object.__setattr__(self, "name", self.name.strip())
        object.__setattr__(self, "path", path)
        object.__setattr__(self, "scale", scale)
        object.__setattr__(self, "unit", unit)

    @property
    def path_text(self) -> str:
        """Return the canonical, non-executable JSON Pointer text."""

        return _encode_pointer(self.path)


@dataclass(frozen=True, slots=True)
class TlvFieldSpec:
    """One field selected by tag and explicit zero-based occurrence."""

    name: str
    tag: int
    kind: FieldKind = FieldKind.HEX
    length: int | None = None
    occurrence: int = 0
    byteorder: str = "little"
    scale: float = 1.0
    unit: str = ""

    def __post_init__(self) -> None:
        _validate_name(self.name, "TLV 字段")
        if (
            isinstance(self.tag, bool)
            or not isinstance(self.tag, int)
            or not 0 <= self.tag <= MAX_TLV_TAG
        ):
            raise ConfigurationError(f"TLV tag 必须在 0 到 {MAX_TLV_TAG} 之间。")
        if not isinstance(self.kind, FieldKind):
            raise ConfigurationError("TLV 字段 kind 必须使用 FieldKind。")
        if self.length is not None and (
            isinstance(self.length, bool)
            or not isinstance(self.length, int)
            or not 1 <= self.length <= MAX_COMPONENT_FIELD_BYTES
        ):
            raise ConfigurationError(
                f"TLV 字段 length 必须为空或在 1 到 {MAX_COMPONENT_FIELD_BYTES} 之间。"
            )
        if (
            isinstance(self.occurrence, bool)
            or not isinstance(self.occurrence, int)
            or not 0 <= self.occurrence < MAX_COMPONENT_FIELDS
        ):
            raise ConfigurationError("TLV 字段 occurrence 必须是有限非负整数。")
        if not isinstance(self.byteorder, str) or self.byteorder not in {"little", "big"}:
            raise ConfigurationError("TLV 字段 byteorder 必须是 little 或 big。")
        scale = _validate_scale(self.scale, "TLV 字段 scale")
        unit = _validate_unit(self.unit, "TLV 字段 unit")
        if self.kind in {FieldKind.UINT, FieldKind.INT}:
            if self.length not in {1, 2, 4, 8}:
                raise ConfigurationError("TLV 整数字段 length 只能是 1、2、4 或 8。")
        if self.kind is FieldKind.FLOAT32 and self.length != 4:
            raise ConfigurationError("TLV FLOAT32 字段 length 必须是 4。")
        object.__setattr__(self, "name", self.name.strip())
        object.__setattr__(self, "scale", scale)
        object.__setattr__(self, "unit", unit)


@dataclass(frozen=True, slots=True)
class JsonCodecConfig:
    """Version 1 JSON codec settings."""

    version: int = 1
    encoding: str = "utf-8"
    fields: tuple[JsonFieldSpec, ...] = ()

    def __post_init__(self) -> None:
        if isinstance(self.version, bool) or not isinstance(self.version, int) or self.version != 1:
            raise ConfigurationError("不支持的 JSON codec 版本。")
        if self.encoding != "utf-8":
            raise ConfigurationError("JSON codec 当前只支持 utf-8。")
        fields = tuple(self.fields)
        _validate_fields(fields, JsonFieldSpec, "JSON codec")
        object.__setattr__(self, "fields", fields)


@dataclass(frozen=True, slots=True)
class TlvCodecConfig:
    """Version 1 flat TLV codec settings.

    Wire layout is always ``tag | length | value``.  Length counts only the
    value bytes; nested TLVs, padding, and resynchronisation are intentionally
    outside this first dialect.
    """

    version: int = 1
    type_bytes: int = 1
    length_bytes: int = 1
    byteorder: str = "big"
    unknown_tag_policy: str = "ignore"
    fields: tuple[TlvFieldSpec, ...] = ()

    def __post_init__(self) -> None:
        if isinstance(self.version, bool) or not isinstance(self.version, int) or self.version != 1:
            raise ConfigurationError("不支持的 TLV codec 版本。")
        if (
            isinstance(self.type_bytes, bool)
            or not isinstance(self.type_bytes, int)
            or isinstance(self.length_bytes, bool)
            or not isinstance(self.length_bytes, int)
            or self.type_bytes not in {1, 2, 4}
            or self.length_bytes not in {1, 2, 4}
        ):
            raise ConfigurationError("TLV type_bytes/length_bytes 只能是 1、2 或 4。")
        if self.type_bytes == 1 and any(field.tag > 0xFF for field in self.fields):
            raise ConfigurationError("TLV tag 超出 type_bytes 表示范围。")
        if self.type_bytes == 2 and any(field.tag > 0xFFFF for field in self.fields):
            raise ConfigurationError("TLV tag 超出 type_bytes 表示范围。")
        if not isinstance(self.byteorder, str) or self.byteorder not in {"little", "big"}:
            raise ConfigurationError("TLV byteorder 必须是 little 或 big。")
        if self.unknown_tag_policy not in {"ignore", "error"}:
            raise ConfigurationError("TLV unknown_tag_policy 只能是 ignore 或 error。")
        fields = tuple(self.fields)
        _validate_fields(fields, TlvFieldSpec, "TLV codec")
        keys = [(field.tag, field.occurrence) for field in fields]
        if len(keys) != len(set(keys)):
            raise ConfigurationError("TLV 字段 tag+occurrence 不能重复。")
        object.__setattr__(self, "fields", fields)


class ModbusRtuFieldSource(StrEnum):
    """Safe, fixed Modbus RTU ADU values exposed to the component table."""

    ADDRESS = "address"
    FUNCTION = "function"
    BASE_FUNCTION = "base_function"
    IS_EXCEPTION = "is_exception"
    DATA_LENGTH = "data_length"
    DATA_HEX = "data_hex"
    EXCEPTION_CODE = "exception_code"
    RECEIVED_CRC = "received_crc"
    CALCULATED_CRC = "calculated_crc"


@dataclass(frozen=True, slots=True)
class ModbusRtuFieldSpec:
    """One declarative binding to a bounded Modbus RTU ADU field."""

    name: str
    source: ModbusRtuFieldSource

    def __post_init__(self) -> None:
        _validate_name(self.name, "Modbus RTU 字段")
        if not isinstance(self.source, ModbusRtuFieldSource):
            raise ConfigurationError("Modbus RTU 字段 source 类型无效。")
        object.__setattr__(self, "name", self.name.strip())


def _default_modbus_rtu_fields() -> tuple[ModbusRtuFieldSpec, ...]:
    return (
        ModbusRtuFieldSpec("address", ModbusRtuFieldSource.ADDRESS),
        ModbusRtuFieldSpec("function", ModbusRtuFieldSource.FUNCTION),
        ModbusRtuFieldSpec("base_function", ModbusRtuFieldSource.BASE_FUNCTION),
        ModbusRtuFieldSpec("is_exception", ModbusRtuFieldSource.IS_EXCEPTION),
        ModbusRtuFieldSpec("data_length", ModbusRtuFieldSource.DATA_LENGTH),
        ModbusRtuFieldSpec("data", ModbusRtuFieldSource.DATA_HEX),
        ModbusRtuFieldSpec("exception_code", ModbusRtuFieldSource.EXCEPTION_CODE),
        ModbusRtuFieldSpec("received_crc", ModbusRtuFieldSource.RECEIVED_CRC),
        ModbusRtuFieldSpec("calculated_crc", ModbusRtuFieldSource.CALCULATED_CRC),
    )


@dataclass(frozen=True, slots=True)
class ModbusRtuCodecConfig:
    """Version 1 profile for one already-framed Modbus RTU ADU."""

    version: int = 1
    fields: tuple[ModbusRtuFieldSpec, ...] = ()

    def __post_init__(self) -> None:
        if isinstance(self.version, bool) or not isinstance(self.version, int) or self.version != 1:
            raise ConfigurationError("不支持的 Modbus RTU codec 版本。")
        fields = tuple(self.fields) or _default_modbus_rtu_fields()
        _validate_fields(fields, ModbusRtuFieldSpec, "Modbus RTU codec")
        if len(fields) > 16:
            raise ConfigurationError("Modbus RTU codec 字段数不能超过 16。")
        object.__setattr__(self, "fields", fields)


class MavlinkVersionPolicy(StrEnum):
    """Allowed MAVLink wire version for one component profile."""

    AUTO = "auto"
    V1 = "v1"
    V2 = "v2"


@dataclass(frozen=True, slots=True)
class MavlinkCrcExtraSpec:
    """One explicit message-id to CRC_EXTRA binding."""

    message_id: int
    extra: int

    def __post_init__(self) -> None:
        if (
            isinstance(self.message_id, bool)
            or not isinstance(self.message_id, int)
            or not 0 <= self.message_id <= 0xFF_FFFF
        ):
            raise ConfigurationError("MAVLink message_id 必须在 0 到 16777215 之间。")
        if (
            isinstance(self.extra, bool)
            or not isinstance(self.extra, int)
            or not 0 <= self.extra <= 0xFF
        ):
            raise ConfigurationError("MAVLink CRC_EXTRA 必须在 0 到 255 之间。")


class MavlinkFieldSource(StrEnum):
    """Safe MAVLink packet metadata exposed to the component table."""

    VERSION = "version"
    PAYLOAD_LENGTH = "payload_length"
    INCOMPAT_FLAGS = "incompat_flags"
    COMPAT_FLAGS = "compat_flags"
    SEQUENCE = "sequence"
    SYSTEM_ID = "system_id"
    COMPONENT_ID = "component_id"
    MESSAGE_ID = "message_id"
    SIGNED = "signed"
    CRC_EXTRA = "crc_extra"
    RECEIVED_CRC = "received_crc"
    CALCULATED_CRC = "calculated_crc"
    PAYLOAD_HEX = "payload_hex"
    SIGNATURE_HEX = "signature_hex"


@dataclass(frozen=True, slots=True)
class MavlinkFieldSpec:
    """One declarative binding to a bounded MAVLink packet field."""

    name: str
    source: MavlinkFieldSource

    def __post_init__(self) -> None:
        _validate_name(self.name, "MAVLink 字段")
        if not isinstance(self.source, MavlinkFieldSource):
            raise ConfigurationError("MAVLink 字段 source 类型无效。")
        object.__setattr__(self, "name", self.name.strip())


def _default_mavlink_fields() -> tuple[MavlinkFieldSpec, ...]:
    return (
        MavlinkFieldSpec("version", MavlinkFieldSource.VERSION),
        MavlinkFieldSpec("payload_length", MavlinkFieldSource.PAYLOAD_LENGTH),
        MavlinkFieldSpec("incompat_flags", MavlinkFieldSource.INCOMPAT_FLAGS),
        MavlinkFieldSpec("compat_flags", MavlinkFieldSource.COMPAT_FLAGS),
        MavlinkFieldSpec("sequence", MavlinkFieldSource.SEQUENCE),
        MavlinkFieldSpec("system_id", MavlinkFieldSource.SYSTEM_ID),
        MavlinkFieldSpec("component_id", MavlinkFieldSource.COMPONENT_ID),
        MavlinkFieldSpec("message_id", MavlinkFieldSource.MESSAGE_ID),
        MavlinkFieldSpec("signed", MavlinkFieldSource.SIGNED),
        MavlinkFieldSpec("crc_extra", MavlinkFieldSource.CRC_EXTRA),
        MavlinkFieldSpec("received_crc", MavlinkFieldSource.RECEIVED_CRC),
        MavlinkFieldSpec("calculated_crc", MavlinkFieldSource.CALCULATED_CRC),
        MavlinkFieldSpec("payload", MavlinkFieldSource.PAYLOAD_HEX),
        MavlinkFieldSpec("signature", MavlinkFieldSource.SIGNATURE_HEX),
    )


@dataclass(frozen=True, slots=True)
class MavlinkCodecConfig:
    """Version 1 bounded MAVLink v1/v2 RX profile."""

    version: int = 1
    version_policy: MavlinkVersionPolicy = MavlinkVersionPolicy.AUTO
    dialect: str = "custom"
    mapping_source: str = "user supplied CRC_EXTRA mapping"
    mapping_revision: str = "unversioned"
    crc_extra: tuple[MavlinkCrcExtraSpec, ...] = ()
    fields: tuple[MavlinkFieldSpec, ...] = ()

    def __post_init__(self) -> None:
        if isinstance(self.version, bool) or not isinstance(self.version, int) or self.version != 1:
            raise ConfigurationError("不支持的 MAVLink codec 版本。")
        if not isinstance(self.version_policy, MavlinkVersionPolicy):
            raise ConfigurationError("MAVLink version_policy 类型无效。")
        _validate_name(self.dialect, "MAVLink dialect")
        _validate_name(self.mapping_source, "MAVLink mapping_source")
        _validate_name(self.mapping_revision, "MAVLink mapping_revision")
        mapping = tuple(self.crc_extra)
        if len(mapping) > 512:
            raise ConfigurationError("MAVLink CRC_EXTRA mapping 不能超过 512 项。")
        if any(not isinstance(item, MavlinkCrcExtraSpec) for item in mapping):
            raise ConfigurationError("MAVLink CRC_EXTRA mapping 类型无效。")
        message_ids = [item.message_id for item in mapping]
        if len(message_ids) != len(set(message_ids)):
            raise ConfigurationError("MAVLink CRC_EXTRA message_id 不能重复。")
        fields = tuple(self.fields) or _default_mavlink_fields()
        _validate_fields(fields, MavlinkFieldSpec, "MAVLink codec")
        if len(fields) > 16:
            raise ConfigurationError("MAVLink codec 字段数不能超过 16。")
        object.__setattr__(self, "dialect", self.dialect.strip())
        object.__setattr__(self, "mapping_source", self.mapping_source.strip())
        object.__setattr__(self, "mapping_revision", self.mapping_revision.strip())
        object.__setattr__(self, "crc_extra", mapping)
        object.__setattr__(self, "fields", fields)

    def extra_for(self, message_id: int | None) -> int | None:
        if message_id is None:
            return None
        for item in self.crc_extra:
            if item.message_id == message_id:
                return item.extra
        return None


@dataclass(frozen=True, slots=True)
class ComponentCodecConfig:
    """Schema v2 wrapper that atomically owns one codec dialect."""

    name: str = "Structured frame"
    schema_version: int = 2
    codec: JsonCodecConfig | TlvCodecConfig | ModbusRtuCodecConfig | MavlinkCodecConfig = field(
        default_factory=JsonCodecConfig
    )

    def __post_init__(self) -> None:
        _validate_name(self.name, "组件 codec")
        if (
            isinstance(self.schema_version, bool)
            or not isinstance(self.schema_version, int)
            or self.schema_version != 2
        ):
            raise ConfigurationError("不支持的组件 codec schema 版本。")
        if not isinstance(
            self.codec,
            (JsonCodecConfig, TlvCodecConfig, ModbusRtuCodecConfig, MavlinkCodecConfig),
        ):
            raise ConfigurationError("组件 codec 配置类型无效。")
        object.__setattr__(self, "name", self.name.strip())

    @property
    def codec_kind(self) -> ComponentCodecKind:
        if isinstance(self.codec, JsonCodecConfig):
            return ComponentCodecKind.JSON
        if isinstance(self.codec, TlvCodecConfig):
            return ComponentCodecKind.TLV
        if isinstance(self.codec, ModbusRtuCodecConfig):
            return ComponentCodecKind.MODBUS_RTU
        return ComponentCodecKind.MAVLINK

    @property
    def fields(
        self,
    ) -> tuple[JsonFieldSpec | TlvFieldSpec | ModbusRtuFieldSpec | MavlinkFieldSpec, ...]:
        return self.codec.fields


type ComponentConfiguration = ComponentProfile | ComponentCodecConfig


class ComponentConfigurationCodec:
    """Strict loader/dumper for legacy v1 and structured schema v2."""

    @classmethod
    def loads(cls, text: str) -> ComponentConfiguration:
        if not isinstance(text, str):
            raise ConfigurationError("组件配置内容必须是文本。")
        if len(text.encode("utf-8")) > MAX_COMPONENT_PROFILE_BYTES:
            raise ConfigurationError("组件配置超过 64 KiB 上限。")
        try:
            value = json.loads(text)
        except json.JSONDecodeError as exc:
            raise ConfigurationError(f"组件配置 JSON 无效：{exc.msg}。") from exc
        if not isinstance(value, dict):
            raise ConfigurationError("组件配置顶层必须是对象。")
        schema_version = value.get("schema_version", 1)
        if isinstance(schema_version, bool) or not isinstance(schema_version, int):
            raise ConfigurationError("组件配置 schema_version 必须是整数。")
        if schema_version == 1:
            return ComponentProfileCodec.loads(text)
        if schema_version != 2:
            raise ConfigurationError("不支持的组件配置 schema 版本。")
        try:
            value = json.loads(
                text,
                object_pairs_hook=_reject_duplicate_keys,
                parse_constant=_reject_constant,
            )
        except (json.JSONDecodeError, ValueError) as exc:
            raise ConfigurationError(f"组件 codec v2 JSON 无效：{exc}。") from exc
        if not isinstance(value, dict):
            raise ConfigurationError("组件 codec v2 顶层必须是对象。")
        if set(value) != {"name", "schema_version", "codec"}:
            raise ConfigurationError("组件 codec v2 顶层字段不受支持。")
        codec_value = value["codec"]
        if not isinstance(codec_value, dict) or "kind" not in codec_value:
            raise ConfigurationError("组件 codec v2 必须声明 codec.kind。")
        kind = codec_value["kind"]
        try:
            codec_kind = ComponentCodecKind(kind)
        except (TypeError, ValueError) as exc:
            raise ConfigurationError("组件 codec kind 不受支持。") from exc
        try:
            if codec_kind is ComponentCodecKind.JSON:
                codec = cls._load_json_codec(codec_value)
            elif codec_kind is ComponentCodecKind.TLV:
                codec = cls._load_tlv_codec(codec_value)
            elif codec_kind is ComponentCodecKind.MODBUS_RTU:
                codec = cls._load_modbus_rtu_codec(codec_value)
            else:
                codec = cls._load_mavlink_codec(codec_value)
            return ComponentCodecConfig(name=value["name"], codec=codec)
        except ConfigurationError:
            raise
        except (KeyError, TypeError, ValueError) as exc:
            raise ConfigurationError("组件 codec v2 配置无效。", detail=str(exc)) from exc

    @staticmethod
    def dumps(configuration: ComponentConfiguration) -> str:
        if isinstance(configuration, ComponentProfile):
            return ComponentProfileCodec.dumps(configuration)
        if not isinstance(configuration, ComponentCodecConfig):
            raise ConfigurationError("只能序列化组件 profile 或 codec 配置。")
        codec_value: dict[str, object]
        if isinstance(configuration.codec, JsonCodecConfig):
            codec_value = {
                "kind": ComponentCodecKind.JSON.value,
                "version": configuration.codec.version,
                "encoding": configuration.codec.encoding,
                "fields": [
                    {
                        "name": field.name,
                        "path": field.path_text,
                        "value_kind": field.value_kind.value,
                        "scale": field.scale,
                        "unit": field.unit,
                    }
                    for field in configuration.codec.fields
                ],
            }
        elif isinstance(configuration.codec, TlvCodecConfig):
            codec_value = {
                "kind": ComponentCodecKind.TLV.value,
                "version": configuration.codec.version,
                "type_bytes": configuration.codec.type_bytes,
                "length_bytes": configuration.codec.length_bytes,
                "byteorder": configuration.codec.byteorder,
                "unknown_tag_policy": configuration.codec.unknown_tag_policy,
                "fields": [
                    {
                        "name": field.name,
                        "tag": field.tag,
                        "kind": field.kind.value,
                        "length": field.length,
                        "occurrence": field.occurrence,
                        "byteorder": field.byteorder,
                        "scale": field.scale,
                        "unit": field.unit,
                    }
                    for field in configuration.codec.fields
                ],
            }
        elif isinstance(configuration.codec, ModbusRtuCodecConfig):
            codec_value = {
                "kind": ComponentCodecKind.MODBUS_RTU.value,
                "version": configuration.codec.version,
                "fields": [
                    {"name": field.name, "source": field.source.value}
                    for field in configuration.codec.fields
                ],
            }
        else:
            codec_value = {
                "kind": ComponentCodecKind.MAVLINK.value,
                "version": configuration.codec.version,
                "version_policy": configuration.codec.version_policy.value,
                "dialect": configuration.codec.dialect,
                "mapping_source": configuration.codec.mapping_source,
                "mapping_revision": configuration.codec.mapping_revision,
                "crc_extra": [
                    {"message_id": item.message_id, "extra": item.extra}
                    for item in configuration.codec.crc_extra
                ],
                "fields": [
                    {"name": field.name, "source": field.source.value}
                    for field in configuration.codec.fields
                ],
            }
        text = (
            json.dumps(
                {
                    "name": configuration.name,
                    "schema_version": configuration.schema_version,
                    "codec": codec_value,
                },
                ensure_ascii=False,
                indent=2,
                sort_keys=True,
            )
            + "\n"
        )
        if len(text.encode("utf-8")) > MAX_COMPONENT_PROFILE_BYTES:
            raise ConfigurationError("组件 codec 序列化结果超过 64 KiB 上限。")
        return text

    @staticmethod
    def _load_json_codec(value: dict[str, object]) -> JsonCodecConfig:
        allowed = {"kind", "version", "encoding", "fields"}
        if set(value) - allowed:
            raise ConfigurationError("JSON codec 字段不受支持。")
        raw_fields = value.get("fields", [])
        if not isinstance(raw_fields, list):
            raise ConfigurationError("JSON codec fields 必须是数组。")
        fields: list[JsonFieldSpec] = []
        for raw in raw_fields:
            if not isinstance(raw, dict):
                raise ConfigurationError("JSON codec 字段定义必须是对象。")
            if set(raw) - {"name", "path", "value_kind", "scale", "unit"}:
                raise ConfigurationError("JSON codec 字段定义不受支持。")
            path_value = raw.get("path")
            try:
                path = _decode_pointer(path_value)
                fields.append(
                    JsonFieldSpec(
                        name=raw["name"],
                        path=path,
                        value_kind=JsonValueKind(raw.get("value_kind", "string")),
                        scale=raw.get("scale", 1.0),
                        unit=raw.get("unit", ""),
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("JSON codec 字段定义无效。", detail=str(exc)) from exc
        return JsonCodecConfig(
            version=value.get("version", 1),
            encoding=value.get("encoding", "utf-8"),
            fields=tuple(fields),
        )

    @staticmethod
    def _load_tlv_codec(value: dict[str, object]) -> TlvCodecConfig:
        allowed = {
            "kind",
            "version",
            "type_bytes",
            "length_bytes",
            "byteorder",
            "unknown_tag_policy",
            "fields",
        }
        if set(value) - allowed:
            raise ConfigurationError("TLV codec 字段不受支持。")
        raw_fields = value.get("fields", [])
        if not isinstance(raw_fields, list):
            raise ConfigurationError("TLV codec fields 必须是数组。")
        fields: list[TlvFieldSpec] = []
        for raw in raw_fields:
            if not isinstance(raw, dict):
                raise ConfigurationError("TLV codec 字段定义必须是对象。")
            if set(raw) - {
                "name",
                "tag",
                "kind",
                "length",
                "occurrence",
                "byteorder",
                "scale",
                "unit",
            }:
                raise ConfigurationError("TLV codec 字段定义不受支持。")
            try:
                fields.append(
                    TlvFieldSpec(
                        name=raw["name"],
                        tag=raw["tag"],
                        kind=FieldKind(raw.get("kind", FieldKind.HEX)),
                        length=raw.get("length"),
                        occurrence=raw.get("occurrence", 0),
                        byteorder=raw.get("byteorder", "little"),
                        scale=raw.get("scale", 1.0),
                        unit=raw.get("unit", ""),
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("TLV codec 字段定义无效。", detail=str(exc)) from exc
        return TlvCodecConfig(
            version=value.get("version", 1),
            type_bytes=value.get("type_bytes", 1),
            length_bytes=value.get("length_bytes", 1),
            byteorder=value.get("byteorder", "big"),
            unknown_tag_policy=value.get("unknown_tag_policy", "ignore"),
            fields=tuple(fields),
        )

    @staticmethod
    def _load_modbus_rtu_codec(value: dict[str, object]) -> ModbusRtuCodecConfig:
        allowed = {"kind", "version", "fields"}
        if set(value) - allowed:
            raise ConfigurationError("Modbus RTU codec 字段不受支持。")
        raw_fields = value.get("fields", [])
        if not isinstance(raw_fields, list):
            raise ConfigurationError("Modbus RTU codec fields 必须是数组。")
        fields: list[ModbusRtuFieldSpec] = []
        for raw in raw_fields:
            if not isinstance(raw, dict) or set(raw) - {"name", "source"}:
                raise ConfigurationError("Modbus RTU 字段定义不受支持。")
            try:
                fields.append(
                    ModbusRtuFieldSpec(
                        name=raw["name"],
                        source=ModbusRtuFieldSource(raw["source"]),
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("Modbus RTU 字段定义无效。", detail=str(exc)) from exc
        return ModbusRtuCodecConfig(
            version=value.get("version", 1),
            fields=tuple(fields),
        )

    @staticmethod
    def _load_mavlink_codec(value: dict[str, object]) -> MavlinkCodecConfig:
        allowed = {
            "kind",
            "version",
            "version_policy",
            "dialect",
            "mapping_source",
            "mapping_revision",
            "crc_extra",
            "fields",
        }
        if set(value) - allowed:
            raise ConfigurationError("MAVLink codec 字段不受支持。")
        raw_mapping = value.get("crc_extra", [])
        if not isinstance(raw_mapping, list):
            raise ConfigurationError("MAVLink crc_extra 必须是数组。")
        mapping: list[MavlinkCrcExtraSpec] = []
        for raw in raw_mapping:
            if not isinstance(raw, dict) or set(raw) - {"message_id", "extra"}:
                raise ConfigurationError("MAVLink CRC_EXTRA 定义不受支持。")
            try:
                mapping.append(
                    MavlinkCrcExtraSpec(
                        message_id=raw["message_id"],
                        extra=raw["extra"],
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("MAVLink CRC_EXTRA 定义无效。", detail=str(exc)) from exc
        raw_fields = value.get("fields", [])
        if not isinstance(raw_fields, list):
            raise ConfigurationError("MAVLink codec fields 必须是数组。")
        fields: list[MavlinkFieldSpec] = []
        for raw in raw_fields:
            if not isinstance(raw, dict) or set(raw) - {"name", "source"}:
                raise ConfigurationError("MAVLink 字段定义不受支持。")
            try:
                fields.append(
                    MavlinkFieldSpec(
                        name=raw["name"],
                        source=MavlinkFieldSource(raw["source"]),
                    )
                )
            except (KeyError, TypeError, ValueError) as exc:
                raise ConfigurationError("MAVLink 字段定义无效。", detail=str(exc)) from exc
        return MavlinkCodecConfig(
            version=value.get("version", 1),
            version_policy=MavlinkVersionPolicy(value.get("version_policy", "auto")),
            dialect=value.get("dialect", "custom"),
            mapping_source=value.get("mapping_source", "user supplied CRC_EXTRA mapping"),
            mapping_revision=value.get("mapping_revision", "unversioned"),
            crc_extra=tuple(mapping),
            fields=tuple(fields),
        )


def _validate_fields(fields: tuple[object, ...], expected: type[object], label: str) -> None:
    if len(fields) > MAX_COMPONENT_FIELDS:
        raise ConfigurationError(f"{label} 字段数不能超过 {MAX_COMPONENT_FIELDS}。")
    if any(not isinstance(field, expected) for field in fields):
        raise ConfigurationError(f"{label} 字段类型无效。")
    names = [field.name.casefold() for field in fields]
    if len(names) != len(set(names)):
        raise ConfigurationError(f"{label} 字段名称不能重复。")


def _validate_name(value: object, label: str) -> None:
    if not isinstance(value, str) or not value.strip() or len(value.strip()) > 64:
        raise ConfigurationError(f"{label}名称必须为 1 到 64 个字符。")


def _validate_scale(value: object, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ConfigurationError(f"{label}必须是数字。")
    result = float(value)
    if not math.isfinite(result):
        raise ConfigurationError(f"{label}必须是有限数字。")
    return result


def _validate_unit(value: object, label: str) -> str:
    if not isinstance(value, str):
        raise ConfigurationError(f"{label}必须是字符串。")
    result = value.strip()
    if len(result) > MAX_COMPONENT_UNIT_LENGTH:
        raise ConfigurationError(f"{label}不能超过 {MAX_COMPONENT_UNIT_LENGTH} 个字符。")
    return result


def _decode_pointer(value: object) -> tuple[str, ...]:
    if not isinstance(value, str):
        raise ConfigurationError("JSON path 必须是 JSON Pointer 字符串。")
    if value == "":
        return ()
    if not value.startswith("/"):
        raise ConfigurationError("JSON path 必须以 / 开始，或使用空字符串表示根。")
    if len(value.encode("utf-8")) > MAX_CODEC_PATH_BYTES:
        raise ConfigurationError(f"JSON path 不能超过 {MAX_CODEC_PATH_BYTES} 字节。")
    segments: list[str] = []
    for encoded in value[1:].split("/"):
        result: list[str] = []
        index = 0
        while index < len(encoded):
            char = encoded[index]
            if char != "~":
                result.append(char)
                index += 1
                continue
            if index + 1 >= len(encoded) or encoded[index + 1] not in {"0", "1"}:
                raise ConfigurationError("JSON path 包含无效的 ~ 转义。")
            result.append("~" if encoded[index + 1] == "0" else "/")
            index += 2
        segments.append("".join(result))
    return tuple(segments)


def _encode_pointer(path: tuple[str, ...]) -> str:
    if not path:
        return ""
    return "/" + "/".join(segment.replace("~", "~0").replace("/", "~1") for segment in path)


def _resolve_pointer(value: object, path: tuple[str, ...]) -> tuple[bool, object | None]:
    current = value
    for segment in path:
        if isinstance(current, dict):
            if segment not in current:
                return False, None
            current = current[segment]
            continue
        if isinstance(current, list):
            if not segment.isdecimal() or (len(segment) > 1 and segment.startswith("0")):
                return False, None
            index = int(segment)
            if index >= len(current):
                return False, None
            current = current[index]
            continue
        return False, None
    return True, current


def _reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"重复 JSON key：{key}")
        result[key] = value
    return result


def _reject_constant(value: str) -> None:
    raise ValueError(f"JSON 不允许特殊数字：{value}")

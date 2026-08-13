"""Built-in component codec implementations."""

from __future__ import annotations

import json
import struct

from .codecs_config import (
    MAX_COMPONENT_FIELD_BYTES,
    MAX_TLV_ITEMS,
    ComponentCodecConfig,
    JsonCodecConfig,
    MavlinkCodecConfig,
    MavlinkVersionPolicy,
    ModbusRtuCodecConfig,
    TlvCodecConfig,
    _reject_constant,
    _reject_duplicate_keys,
    _resolve_pointer,
)
from .codecs_helpers import (
    _append_error,
    _bounded_error,
    _bounded_json_raw,
    _component_error,
    _format_json_value,
    _json_scalar_value,
    _json_value_kind,
    _matches_json_kind,
    _mavlink_field,
    _mavlink_value_kind,
    _modbus_field,
    _modbus_value_kind,
    _require_codec,
    _row,
    _validate_json_tree,
)
from .components import (
    BinaryComponentCodec,
    ComponentFieldValue,
    ComponentFrameRow,
    FieldSpec,
    ValueKind,
)
from .mavlink import decode_mavlink
from .modbus import decode_modbus_rtu
from .protocols import DecodedFrame, FrameStatus, ProtocolSource


class JsonComponentCodec:
    """Decode strict UTF-8 JSON with bounded JSON Pointer bindings."""

    def decode(
        self,
        frame: DecodedFrame,
        configuration: ComponentCodecConfig,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        config = _require_codec(configuration, JsonCodecConfig)
        value: object | None = None
        payload_error: str | None = None
        try:
            text = frame.payload.decode(config.encoding)
            value = json.loads(
                text,
                object_pairs_hook=_reject_duplicate_keys,
                parse_constant=_reject_constant,
            )
            _validate_json_tree(value)
        except (UnicodeDecodeError, json.JSONDecodeError, ValueError, RecursionError) as exc:
            payload_error = _bounded_error("JSON payload 解码失败", exc)

        values: list[ComponentFieldValue] = []
        field_errors = 0
        for spec in config.fields:
            if payload_error is not None:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        _json_value_kind(spec.value_kind),
                        payload_error,
                    )
                )
                continue
            found, selected = _resolve_pointer(value, spec.path)
            if not found:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        _json_value_kind(spec.value_kind),
                        f"JSON path 不存在：{spec.path_text}。",
                    )
                )
                continue
            if not _matches_json_kind(selected, spec.value_kind):
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        _json_value_kind(spec.value_kind),
                        f"JSON 值类型与 {spec.value_kind.value} 不匹配。",
                        raw=_bounded_json_raw(selected),
                    )
                )
                continue
            raw = _bounded_json_raw(selected)
            if len(raw) > MAX_COMPONENT_FIELD_BYTES:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        _json_value_kind(spec.value_kind),
                        "JSON 字段 canonical 值超过 1 KiB。",
                        raw=raw[:MAX_COMPONENT_FIELD_BYTES],
                    )
                )
                continue
            try:
                display = _format_json_value(selected, spec)
            except (TypeError, ValueError, OverflowError) as exc:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        _json_value_kind(spec.value_kind),
                        _bounded_error("JSON 字段格式化失败", exc),
                        raw=raw,
                    )
                )
                continue
            values.append(
                ComponentFieldValue(
                    name=spec.name,
                    kind=_json_value_kind(spec.value_kind),
                    raw=raw,
                    display=display,
                    value=_json_scalar_value(selected),
                )
            )
        error = frame.error
        if field_errors and error is None:
            error = f"{field_errors} 个 JSON 字段解码失败。"
        return _row(
            frame,
            source,
            occurred_at,
            tuple(values),
            error=error,
            codec_error=payload_error,
        )


class TlvComponentCodec:
    """Decode the flat ``tag | length | value`` TLV dialect."""

    def decode(
        self,
        frame: DecodedFrame,
        configuration: ComponentCodecConfig,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        config = _require_codec(configuration, TlvCodecConfig)
        items: dict[int, list[bytes]] = {}
        cursor = 0
        payload_error: str | None = None
        item_count = 0
        header_size = config.type_bytes + config.length_bytes
        while cursor < len(frame.payload):
            if len(frame.payload) - cursor < header_size:
                payload_error = "TLV 尾部不足以读取完整 tag/length。"
                break
            tag_start = cursor
            tag = int.from_bytes(
                frame.payload[tag_start : tag_start + config.type_bytes], config.byteorder
            )
            cursor += config.type_bytes
            length = int.from_bytes(
                frame.payload[cursor : cursor + config.length_bytes], config.byteorder
            )
            cursor += config.length_bytes
            if length > len(frame.payload) - cursor:
                payload_error = f"TLV tag {tag} 的 length 超出剩余 payload。"
                break
            value = bytes(frame.payload[cursor : cursor + length])
            cursor += length
            item_count += 1
            if item_count > MAX_TLV_ITEMS:
                payload_error = f"TLV 项数超过 {MAX_TLV_ITEMS} 上限。"
                break
            items.setdefault(tag, []).append(value)

        known_tags = {field.tag for field in config.fields}
        if config.unknown_tag_policy == "error":
            unknown = sorted(tag for tag in items if tag not in known_tags)
            if unknown:
                payload_error = _append_error(
                    payload_error,
                    "未知 TLV tag：" + ", ".join(str(tag) for tag in unknown) + "。",
                )
        for tag, values_for_tag in items.items():
            if len(values_for_tag) > 1:
                requested = {field.occurrence for field in config.fields if field.tag == tag}
                if not set(range(len(values_for_tag))).issubset(requested):
                    payload_error = _append_error(
                        payload_error,
                        f"TLV tag {tag} 重复；请为每个 occurrence 显式声明字段。",
                    )

        values: list[ComponentFieldValue] = []
        field_errors = 0
        for spec in config.fields:
            candidates = items.get(spec.tag, [])
            if spec.occurrence >= len(candidates):
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        ValueKind(spec.kind.value),
                        f"未找到 TLV tag {spec.tag} occurrence {spec.occurrence}。",
                    )
                )
                continue
            raw = candidates[spec.occurrence]
            bounded_raw = raw[:MAX_COMPONENT_FIELD_BYTES]
            if len(raw) > MAX_COMPONENT_FIELD_BYTES:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        ValueKind(spec.kind.value),
                        "TLV value 超过 1 KiB。",
                        raw=bounded_raw,
                    )
                )
                continue
            if spec.length is not None and len(raw) != spec.length:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        ValueKind(spec.kind.value),
                        f"TLV value 长度 {len(raw)} 与期望 {spec.length} 不符。",
                        raw=raw,
                    )
                )
                continue
            try:
                field_spec = FieldSpec(
                    name=spec.name,
                    offset=0,
                    length=len(raw),
                    kind=spec.kind,
                    byteorder=spec.byteorder,
                    scale=spec.scale,
                    unit=spec.unit,
                )
                display = BinaryComponentCodec._format_value(raw, field_spec)
                value = BinaryComponentCodec._decode_value(raw, field_spec)
            except (UnicodeDecodeError, ValueError, OverflowError, struct.error) as exc:
                field_errors += 1
                values.append(
                    _component_error(
                        spec.name,
                        ValueKind(spec.kind.value),
                        _bounded_error("TLV 字段解码失败", exc),
                        raw=raw,
                    )
                )
                continue
            values.append(
                ComponentFieldValue(
                    name=spec.name,
                    kind=ValueKind(spec.kind.value),
                    raw=raw,
                    display=display,
                    value=value,
                )
            )
        error = frame.error
        if field_errors and error is None:
            error = f"{field_errors} 个 TLV 字段解码失败。"
        return _row(
            frame,
            source,
            occurred_at,
            tuple(values),
            error=error,
            codec_error=payload_error,
        )


class ModbusRtuComponentCodec:
    """Project one already-framed Modbus RTU ADU into bounded table fields."""

    def decode(
        self,
        frame: DecodedFrame,
        configuration: ComponentCodecConfig,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        config = _require_codec(configuration, ModbusRtuCodecConfig)
        decoded = decode_modbus_rtu(frame.payload)
        values: list[ComponentFieldValue] = []
        error = _append_error(frame.error, decoded.error)
        field_error = error if error is not None else None
        for spec in config.fields:
            field = _modbus_field(decoded, spec)
            if field is None:
                values.append(
                    _component_error(
                        spec.name,
                        _modbus_value_kind(spec.source),
                        decoded.error or "Modbus RTU 字段不可用。",
                    )
                )
                continue
            raw, display, value = field
            values.append(
                ComponentFieldValue(
                    name=spec.name,
                    kind=_modbus_value_kind(spec.source),
                    raw=raw,
                    display=display,
                    error=field_error,
                    value=value,
                )
            )
        status = (
            decoded.status
            if frame.status in {FrameStatus.VALID, FrameStatus.UNVERIFIED}
            else frame.status
        )
        return _row(
            frame,
            source,
            occurred_at,
            tuple(values),
            error=error,
            codec_error=None,
            status=status,
        )


class MavlinkComponentCodec:
    """Project one already-framed MAVLink v1/v2 packet into table fields."""

    def decode(
        self,
        frame: DecodedFrame,
        configuration: ComponentCodecConfig,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        config = _require_codec(configuration, MavlinkCodecConfig)
        decoded = decode_mavlink(frame.payload)
        if decoded.message_id is not None:
            decoded = decode_mavlink(
                frame.payload,
                crc_extra=config.extra_for(decoded.message_id),
            )
        policy_error: str | None = None
        if (
            decoded.version is not None
            and config.version_policy is not MavlinkVersionPolicy.AUTO
            and decoded.version.value != config.version_policy.value
        ):
            policy_error = (
                f"MAVLink packet version {decoded.version.value} 与 profile policy "
                f"{config.version_policy.value} 不匹配。"
            )
        error = _append_error(frame.error, decoded.error)
        error = _append_error(error, policy_error)
        status = (
            decoded.status
            if frame.status in {FrameStatus.VALID, FrameStatus.UNVERIFIED}
            else frame.status
        )
        if policy_error is not None and status is FrameStatus.VALID:
            status = FrameStatus.INVALID_FORMAT
        field_error = error if status is not FrameStatus.VALID else None
        values: list[ComponentFieldValue] = []
        for spec in config.fields:
            field = _mavlink_field(decoded, spec)
            if field is None:
                values.append(
                    _component_error(
                        spec.name,
                        _mavlink_value_kind(spec.source),
                        error or "MAVLink 字段不可用。",
                    )
                )
                continue
            raw, display, value = field
            values.append(
                ComponentFieldValue(
                    name=spec.name,
                    kind=_mavlink_value_kind(spec.source),
                    raw=raw,
                    display=display,
                    error=field_error,
                    value=value,
                )
            )
        return _row(
            frame,
            source,
            occurred_at,
            tuple(values),
            error=error,
            codec_error=None,
            status=status,
        )

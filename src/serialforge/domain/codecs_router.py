"""Explicit component codec registry."""

from __future__ import annotations

from .codecs_config import (
    ComponentCodecConfig,
    ComponentConfiguration,
    ComponentProfile,
    JsonCodecConfig,
    ModbusRtuCodecConfig,
    TlvCodecConfig,
)
from .codecs_decoders import (
    JsonComponentCodec,
    MavlinkComponentCodec,
    ModbusRtuComponentCodec,
    TlvComponentCodec,
)
from .components import BinaryComponentCodec, ComponentFrameRow
from .errors import ConfigurationError
from .protocols import DecodedFrame, ProtocolSource


class ComponentCodecRouter:
    """Explicit built-in registry; future codecs add a typed registration."""

    def __init__(self) -> None:
        self._json = JsonComponentCodec()
        self._tlv = TlvComponentCodec()
        self._modbus_rtu = ModbusRtuComponentCodec()
        self._mavlink = MavlinkComponentCodec()
        self._binary = BinaryComponentCodec()

    def decode(
        self,
        frame: DecodedFrame,
        configuration: ComponentConfiguration,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        if isinstance(configuration, ComponentProfile):
            return self._binary.decode(frame, configuration, source, occurred_at)
        if not isinstance(configuration, ComponentCodecConfig):
            raise ConfigurationError("组件 codec 配置类型无效。")
        if isinstance(configuration.codec, JsonCodecConfig):
            return self._json.decode(frame, configuration, source, occurred_at)
        if isinstance(configuration.codec, TlvCodecConfig):
            return self._tlv.decode(frame, configuration, source, occurred_at)
        if isinstance(configuration.codec, ModbusRtuCodecConfig):
            return self._modbus_rtu.decode(frame, configuration, source, occurred_at)
        return self._mavlink.decode(frame, configuration, source, occurred_at)

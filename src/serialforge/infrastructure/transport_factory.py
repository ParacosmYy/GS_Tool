"""Route typed transport configurations to their isolated adapters."""

from __future__ import annotations

from ..domain.errors import ConfigurationError
from ..domain.models import (
    RttTransportConfig,
    TcpTransportConfig,
    TransportConfig,
    UartTransportConfig,
    UdpTransportConfig,
)
from ..domain.ports import TransportFactoryPort, TransportPort
from .network_transport import NetworkTransportFactory
from .rtt_transport import RttTransportFactory
from .serial_transport import SerialTransportFactory


class RoutingTransportFactory(TransportFactoryPort):
    """Keep the composition root small while preserving adapter boundaries."""

    def __init__(self) -> None:
        self._serial = SerialTransportFactory()
        self._network = NetworkTransportFactory()
        self._rtt = RttTransportFactory()

    def create(self, config: TransportConfig) -> TransportPort:
        """Create."""
        if isinstance(config, UartTransportConfig):
            return self._serial.create(config)
        if isinstance(config, (TcpTransportConfig, UdpTransportConfig)):
            return self._network.create(config)
        if isinstance(config, RttTransportConfig):
            return self._rtt.create(config)
        raise ConfigurationError("未注册的传输配置类型。")

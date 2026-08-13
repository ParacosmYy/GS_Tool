"""Factory for the standard-library TCP and UDP adapters."""

from __future__ import annotations

from ..domain.errors import ConfigurationError
from ..domain.models import TcpTransportConfig, TransportConfig, UdpTransportConfig
from ..domain.ports import DatagramTransportPort, StreamTransportPort, TransportFactoryPort
from .tcp_transport import TcpTransport
from .udp_transport import UdpTransport


class NetworkTransportFactory(TransportFactoryPort):
    """Create one explicitly configured TCP client or UDP unicast adapter."""

    def create(self, config: TransportConfig) -> StreamTransportPort | DatagramTransportPort:
        """Create."""
        if isinstance(config, TcpTransportConfig):
            return TcpTransport(config)
        if isinstance(config, UdpTransportConfig):
            return UdpTransport(config)
        raise ConfigurationError("网络 factory 只接受 TCP 或 UDP 配置。")

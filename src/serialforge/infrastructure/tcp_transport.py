"""Standard-library TCP client adapter for the bounded stream port."""

from __future__ import annotations

from ..domain.models import TcpTransportConfig
from .socket_stream_transport import SocketStreamTransport


class TcpTransport(SocketStreamTransport):
    """A single-peer TCP client whose blocking calls stay on a session worker."""

    def __init__(self, config: TcpTransportConfig) -> None:
        super().__init__(config, adapter_name="TCP")

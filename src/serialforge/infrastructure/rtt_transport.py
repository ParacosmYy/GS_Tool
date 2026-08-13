"""Attach-only J-Link RTT Telnet adapter.

The adapter speaks the documented local TCP/Telnet bridge exposed by an
already-running SEGGER debug tool. It intentionally does not load a SEGGER
DLL, start a vendor process, or perform probe/memory/debug operations.
"""

from __future__ import annotations

from ..domain.models import RttTransportConfig
from .socket_stream_transport import SocketStreamTransport


def _rtt_channel_config(channel: int) -> bytes:
    """Select one RTT channel during SEGGER's short post-connect window."""

    return f"$$SEGGER_TELNET_ConfigStr=RTTCh;{channel}$$".encode("ascii")


class RttTransport(SocketStreamTransport):
    """A bounded byte stream backed by an existing RTT Telnet endpoint."""

    def __init__(self, config: RttTransportConfig) -> None:
        super().__init__(
            config,
            adapter_name="J-Link RTT",
            connect_preamble=_rtt_channel_config(config.channel),
        )


class RttTransportFactory:
    """Create RTT adapters without opening sockets on the caller's thread."""

    def create(self, config: RttTransportConfig) -> RttTransport:
        """Create."""
        return RttTransport(config)

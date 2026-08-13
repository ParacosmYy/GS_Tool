"""Standard-library UDP unicast adapter preserving packet and peer semantics."""

from __future__ import annotations

import socket
from threading import Lock

from ..domain.errors import (
    ConfigurationError,
    DatagramPeerRejectedError,
    DatagramTooLargeError,
    TransportCloseError,
    TransportNotOpenError,
    TransportOpenError,
    TransportReadError,
    TransportWriteError,
)
from ..domain.models import (
    MAX_UDP_DATAGRAM_BYTES,
    Datagram,
    DatagramDropReason,
    DatagramReadResult,
    DatagramSend,
    Endpoint,
    PeerAddress,
    TransportKind,
    UdpTransportConfig,
)
from ..domain.ports import DatagramTransportPort


class UdpTransport(DatagramTransportPort):
    """A single-socket UDP unicast adapter with explicit source filtering."""

    def __init__(self, config: UdpTransportConfig) -> None:
        self._config = config
        self._socket: socket.socket | None = None
        self._remote_addresses: tuple[tuple[str, int], ...] = ()
        self._bound_address: tuple[str, int] | None = None
        self._lock = Lock()

    @property
    def kind(self) -> TransportKind:
        """Kind."""
        return TransportKind.UDP_DATAGRAM

    @property
    def endpoint(self) -> Endpoint:
        """Endpoint."""
        if self._bound_address is None:
            return self._config.endpoint
        local_host, local_port = self._bound_address
        address = f"{local_host}:{local_port}->{self._config.remote_peer.display}"
        return Endpoint(
            transport=TransportKind.UDP_DATAGRAM,
            address=address,
            label=f"UDP {self._config.remote_peer.display}",
            identity=self._config.endpoint_identity,
            metadata=(
                ("local_host", local_host),
                ("local_port", str(local_port)),
                ("remote_host", self._config.remote_peer.host),
                ("remote_port", str(self._config.remote_peer.port)),
                ("max_datagram_size", str(self._config.max_datagram_size)),
            ),
        )

    def open(self) -> None:
        """Open."""
        with self._lock:
            if self._socket is not None:
                raise TransportOpenError("UDP socket 已经打开。")

        handle: socket.socket | None = None
        try:
            remote_addresses = self._resolve(self._config.remote_peer)
            handle = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            handle.settimeout(self._config.read_timeout)
            handle.bind((self._config.local_host, self._config.local_port))
        except socket.gaierror as exc:
            self._close_quietly(handle)
            raise TransportOpenError(
                "无法解析 UDP peer 或本地绑定地址。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        except OSError as exc:
            self._close_quietly(handle)
            raise TransportOpenError(
                "无法打开 UDP socket 或执行本地 bind。",
                detail=(
                    f"local={self._config.local_host}:{self._config.local_port}; "
                    f"remote={self._config.remote_peer.display}; "
                    f"{type(exc).__name__}: {exc}"
                ),
            ) from exc

        with self._lock:
            self._socket = handle
            self._remote_addresses = remote_addresses
            bound = handle.getsockname()
            self._bound_address = (str(bound[0]), int(bound[1]))

    def receive(self, max_bytes: int) -> DatagramReadResult:
        """Receive."""
        if max_bytes <= 0:
            raise ConfigurationError("UDP receive max_bytes 必须为正数。")
        handle = self._require_open()
        try:
            # Always receive the IPv4 hard maximum so an oversized packet is
            # rejected as a whole rather than truncated by a smaller buffer.
            payload, address = handle.recvfrom(MAX_UDP_DATAGRAM_BYTES)
        except TimeoutError:
            return DatagramReadResult.timeout()
        except OSError as exc:
            raise TransportReadError(
                "读取 UDP datagram 失败。",
                detail=(
                    f"local={self._config.local_host}:{self._config.local_port}; "
                    f"{type(exc).__name__}: {exc}"
                ),
            ) from exc

        peer = PeerAddress(str(address[0]), int(address[1]))
        if not self._is_authorized(peer):
            return DatagramReadResult.dropped(peer, DatagramDropReason.UNAUTHORIZED_PEER)
        if len(payload) > self._config.max_datagram_size:
            return DatagramReadResult.dropped(peer, DatagramDropReason.PAYLOAD_TOO_LARGE)
        return DatagramReadResult.data(Datagram(peer=peer, payload=payload))

    def send(self, write: DatagramSend) -> None:
        """Send."""
        if write.peer != self._config.remote_peer:
            raise DatagramPeerRejectedError(
                detail=(
                    f"configured={self._config.remote_peer.display}; requested={write.peer.display}"
                )
            )
        if len(write.payload) > self._config.max_datagram_size:
            raise DatagramTooLargeError(
                detail=(
                    f"size={len(write.payload)}; limit={self._config.max_datagram_size}; "
                    f"peer={write.peer.display}"
                )
            )

        handle = self._require_open()
        remote_address = self._remote_addresses[0] if self._remote_addresses else None
        if remote_address is None:
            raise TransportNotOpenError("UDP remote peer 尚未解析。")
        try:
            handle.settimeout(self._config.write_timeout)
            written = handle.sendto(write.payload, remote_address)
        except TimeoutError as exc:
            raise TransportWriteError(
                "UDP 写入超时。",
                detail=f"peer={write.peer.display}; timeout={self._config.write_timeout}",
            ) from exc
        except OSError as exc:
            raise TransportWriteError(
                "发送 UDP datagram 失败。",
                detail=f"peer={write.peer.display}; {type(exc).__name__}: {exc}",
            ) from exc
        finally:
            try:
                handle.settimeout(self._config.read_timeout)
            except OSError:
                pass
        if written != len(write.payload):
            raise TransportWriteError(
                "UDP datagram 写入未完成。",
                detail=f"written={written}; expected={len(write.payload)}",
            )

    def close(self) -> None:
        """Close."""
        with self._lock:
            handle = self._socket
            self._socket = None
            self._remote_addresses = ()
            self._bound_address = None
        if handle is None:
            return
        try:
            handle.close()
        except OSError as exc:
            raise TransportCloseError(
                "关闭 UDP socket 失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

    def _require_open(self) -> socket.socket:
        """Require open."""
        with self._lock:
            handle = self._socket
        if handle is None:
            raise TransportNotOpenError()
        return handle

    def _is_authorized(self, peer: PeerAddress) -> bool:
        """Is authorized."""
        return any(
            peer.host == address[0] and peer.port == address[1]
            for address in self._remote_addresses
        )

    @staticmethod
    def _resolve(peer: PeerAddress) -> tuple[tuple[str, int], ...]:
        """Resolve."""
        infos = socket.getaddrinfo(
            peer.host,
            peer.port,
            socket.AF_INET,
            socket.SOCK_DGRAM,
        )
        addresses: list[tuple[str, int]] = []
        for _family, _socktype, _proto, _canonname, sockaddr in infos:
            address = (str(sockaddr[0]), int(sockaddr[1]))
            if address not in addresses:
                addresses.append(address)
        if not addresses:
            raise socket.gaierror(f"no IPv4 address for {peer.display}")
        return tuple(addresses)

    @staticmethod
    def _close_quietly(handle: socket.socket | None) -> None:
        """Close quietly."""
        if handle is None:
            return
        try:
            handle.close()
        except OSError:
            pass

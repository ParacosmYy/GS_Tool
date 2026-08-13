"""Route session orchestration to the transport-specific manager."""

from __future__ import annotations

import time
from threading import Lock

from ..domain.errors import SessionBusyError, SessionNotFoundError
from ..domain.models import (
    BleGattTransportConfig,
    SessionId,
    SessionSnapshot,
    SessionState,
    TcpServerTransportConfig,
    TransportConfig,
)
from ..domain.ports import SessionPort, TransportWrite


class RoutingSessionManager(SessionPort):
    """Keep M2a sessions and the TCP Server manager isolated behind one port."""

    def __init__(
        self,
        client_manager: SessionPort,
        server_manager: SessionPort,
        ble_manager: SessionPort,
    ) -> None:
        self._client_manager = client_manager
        self._server_manager = server_manager
        self._ble_manager = ble_manager
        self._active: SessionPort | None = None
        self._last_snapshot: SessionSnapshot | None = None
        self._lock = Lock()

    def open(self, config: TransportConfig) -> SessionId:
        """Open."""
        with self._lock:
            if self._active is not None:
                snapshot = self._active.snapshot()
                if snapshot is None or snapshot.state in {
                    SessionState.OPENING,
                    SessionState.OPEN,
                    SessionState.CLOSING,
                }:
                    raise SessionBusyError()
                self._last_snapshot = snapshot
                self._active = None
            manager = (
                self._server_manager
                if isinstance(config, TcpServerTransportConfig)
                else self._ble_manager
                if isinstance(config, BleGattTransportConfig)
                else self._client_manager
            )
            session_id = manager.open(config)
            self._active = manager
            self._last_snapshot = manager.snapshot()
            return session_id

    def close(self, session_id: SessionId) -> None:
        """Close."""
        with self._lock:
            manager = self._require_manager(session_id)
            manager.close(session_id)

    def send(self, session_id: SessionId, write: TransportWrite) -> None:
        """Send."""
        with self._lock:
            manager = self._require_manager(session_id)
            manager.send(session_id, write)

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Shutdown."""
        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        started = time.monotonic()
        managers = (self._client_manager, self._server_manager, self._ble_manager)
        for manager in managers:
            remaining = (
                None if timeout is None else max(0.0, timeout - (time.monotonic() - started))
            )
            manager.shutdown(remaining)

    def snapshot(self) -> SessionSnapshot | None:
        """Snapshot."""
        with self._lock:
            if self._active is not None:
                snapshot = self._active.snapshot()
                if snapshot is not None:
                    self._last_snapshot = snapshot
                return snapshot or self._last_snapshot
            return self._last_snapshot

    def _require_manager(self, session_id: SessionId) -> SessionPort:
        """Require manager."""
        manager = self._active
        if manager is None:
            raise SessionNotFoundError()
        snapshot = manager.snapshot()
        if snapshot is None or snapshot.session_id != session_id:
            raise SessionNotFoundError()
        return manager

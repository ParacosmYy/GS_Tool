"""Bounded Qt worker jobs used by the session ViewModel facade.

These jobs own only the blocking discovery calls and their queued result
signals.  Session state, cancellation ownership, error projection, and the
global thread pool remain with ``SessionViewModel``.
"""

from __future__ import annotations

from threading import Event

from ..domain.errors import ErrorCode, ErrorInfo, SerialForgeError
from ..domain.models import BleGattDiscoveryConfig
from ..domain.ports import BleGattDiscoveryPort, EndpointDiscoveryPort
from .qt import QObject, QRunnable, Signal, Slot


class _DiscoverySignals(QObject):
    completed = Signal(object)
    failed = Signal(object)


class _DiscoveryJob(QRunnable):
    """Run a blocking endpoint enumeration away from the Qt thread."""

    def __init__(self, discovery: EndpointDiscoveryPort, cancelled: Event) -> None:
        super().__init__()
        self.discovery = discovery
        self.cancelled = cancelled
        self.signals = _DiscoverySignals()

    @Slot()
    def run(self) -> None:
        """Run."""
        try:
            endpoints = self.discovery.discover()
            if not self.cancelled.is_set():
                self.signals.completed.emit(endpoints)
        except SerialForgeError as exc:
            if not self.cancelled.is_set():
                self.signals.failed.emit(exc.info)
        except Exception as exc:
            if not self.cancelled.is_set():
                self.signals.failed.emit(
                    ErrorInfo(
                        code=ErrorCode.UNKNOWN,
                        message="端口枚举发生未处理错误。",
                        recoverable=True,
                        detail=f"{type(exc).__name__}: {exc}",
                    )
                )


class _BleScanSignals(QObject):
    completed = Signal(object)
    failed = Signal(object)


class _BleScanJob(QRunnable):
    """Run one bounded BLE scan away from the Qt thread."""

    def __init__(
        self,
        discovery: BleGattDiscoveryPort,
        config: BleGattDiscoveryConfig,
        cancelled: Event,
    ) -> None:
        super().__init__()
        self.discovery = discovery
        self.config = config
        self.cancelled = cancelled
        self.signals = _BleScanSignals()

    @Slot()
    def run(self) -> None:
        """Run."""
        try:
            devices = self.discovery.discover(self.config)
            if not self.cancelled.is_set():
                self.signals.completed.emit(devices)
        except SerialForgeError as exc:
            if not self.cancelled.is_set():
                self.signals.failed.emit(exc.info)
        except Exception as exc:
            if not self.cancelled.is_set():
                self.signals.failed.emit(
                    ErrorInfo(
                        code=ErrorCode.UNKNOWN,
                        message="BLE 扫描发生未处理错误。",
                        recoverable=True,
                        detail=f"{type(exc).__name__}: {exc}",
                    )
                )


__all__ = ["_BleScanJob", "_DiscoveryJob"]

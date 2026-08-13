"""pyserial-backed UART adapter; pyserial stays isolated in infrastructure."""

from __future__ import annotations

from importlib import import_module
from threading import Lock
from typing import Any

from ..domain.errors import (
    ConfigurationError,
    TransportCloseError,
    TransportDependencyError,
    TransportDiscoveryError,
    TransportNotOpenError,
    TransportOpenError,
    TransportReadError,
    TransportWriteError,
)
from ..domain.models import (
    Endpoint,
    StreamReadResult,
    StreamWrite,
    TransportConfig,
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
    UartTransportConfig,
)
from ..domain.ports import EndpointDiscoveryPort, StreamTransportPort, TransportFactoryPort


def _load_pyserial() -> Any:
    """Load the optional adapter dependency only when UART is used."""

    try:
        import serial
    except ModuleNotFoundError as exc:
        raise TransportDependencyError(
            "UART 适配器需要安装 pyserial。",
            detail=str(exc),
        ) from exc
    return serial


class SerialTransportFactory(TransportFactoryPort):
    """Create UART adapters without importing or opening pyserial handles."""

    def create(self, config: TransportConfig) -> StreamTransportPort:
        """Create."""
        if not isinstance(config, UartTransportConfig):
            raise ConfigurationError("当前组合根只注册了 UART transport。")
        return SerialTransport(config)


class SerialPortDiscovery(EndpointDiscoveryPort):
    """Enumerate UART endpoints through pyserial's optional list-port API."""

    def discover(self) -> tuple[Endpoint, ...]:
        """Discover."""
        try:
            list_ports = import_module("serial.tools.list_ports")
        except ImportError as exc:
            raise TransportDependencyError(
                "UART 枚举需要安装 pyserial。",
                detail=str(exc),
            ) from exc

        try:
            ports = list_ports.comports()
        except Exception as exc:
            raise TransportDiscoveryError(
                "无法枚举 Windows 串口。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

        endpoints: list[Endpoint] = []
        for port in ports:
            device = str(getattr(port, "device", "")).strip()
            if not device:
                continue
            metadata = tuple(
                sorted(
                    (key, str(value).strip())
                    for key in (
                        "name",
                        "manufacturer",
                        "product",
                        "serial_number",
                        "vid",
                        "pid",
                        "location",
                        "interface",
                        "hwid",
                    )
                    if (value := getattr(port, key, None)) not in (None, "")
                )
            )
            serial_number = dict(metadata).get("serial_number")
            location = dict(metadata).get("location")
            if serial_number:
                identity = f"uart:serial:{serial_number}"
            elif location:
                identity = f"uart:location:{location}"
            else:
                identity = f"uart:address:{device}"
            endpoints.append(
                Endpoint(
                    transport=TransportKind.UART,
                    address=device,
                    label=str(getattr(port, "description", "") or device),
                    identity=identity,
                    metadata=metadata,
                )
            )
        return tuple(endpoints)


class SerialTransport(StreamTransportPort):
    """A blocking UART adapter intended to run only inside SessionManager."""

    def __init__(self, config: UartTransportConfig) -> None:
        self._config = config
        self._serial: Any | None = None
        self._lock = Lock()

    @property
    def kind(self) -> TransportKind:
        """Kind."""
        return TransportKind.UART

    @property
    def endpoint(self) -> Endpoint:
        """Endpoint."""
        return self._config.endpoint

    @property
    def peer(self) -> None:
        """UART has no network peer metadata."""

        return None

    def open(self) -> None:
        """Open."""
        serial = _load_pyserial()
        with self._lock:
            if self._serial is not None:
                raise TransportOpenError("UART 端口已经打开。")

        handle: Any | None = None
        try:
            serial_kwargs: dict[str, Any] = {
                "port": self._config.port,
                "baudrate": self._config.baud_rate,
                "bytesize": self._bytesize(serial),
                "parity": self._parity(serial),
                "stopbits": self._stopbits(serial),
                "timeout": self._config.read_timeout,
                "write_timeout": self._config.write_timeout,
                "inter_byte_timeout": self._config.inter_byte_timeout,
                "xonxoff": self._config.flow_control == UartFlowControl.XON_XOFF,
                "rtscts": self._config.flow_control == UartFlowControl.RTS_CTS,
                "dsrdtr": self._config.flow_control == UartFlowControl.DSR_DTR,
            }
            if self._config.exclusive is not None:
                serial_kwargs["exclusive"] = self._config.exclusive
            handle = serial.Serial(**serial_kwargs)
            handle.dtr = self._config.dtr
            handle.rts = self._config.rts
        except Exception as exc:
            if handle is not None:
                try:
                    handle.close()
                except Exception:
                    pass
            raise TransportOpenError(
                "无法打开 UART 端口。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

        with self._lock:
            self._serial = handle

    def receive(self, max_bytes: int) -> StreamReadResult:
        """Receive."""
        if max_bytes <= 0:
            raise ConfigurationError("UART receive max_bytes 必须为正数。")
        handle = self._require_open()
        try:
            payload = handle.read(min(max_bytes, self._config.read_chunk_size))
        except Exception as exc:
            raise TransportReadError(
                "读取 UART 数据失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        payload = bytes(payload)
        return StreamReadResult.timeout() if not payload else StreamReadResult.data(payload)

    def send(self, write: StreamWrite) -> None:
        """Send."""
        handle = self._require_open()
        try:
            written = handle.write(write.payload)
        except Exception as exc:
            raise TransportWriteError(
                "写入 UART 数据失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc
        if written != len(write.payload):
            raise TransportWriteError(
                "UART 写入未完成。",
                detail=f"written={written}, expected={len(write.payload)}",
            )

    def close(self) -> None:
        """Close."""
        with self._lock:
            handle = self._serial
            self._serial = None
        if handle is None:
            return
        try:
            handle.close()
        except Exception as exc:
            raise TransportCloseError(
                "关闭 UART 端口失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

    def _require_open(self) -> Any:
        """Require open."""
        with self._lock:
            handle = self._serial
        if handle is None:
            raise TransportNotOpenError()
        return handle

    def _bytesize(self, serial: Any) -> Any:
        """Bytesize."""
        return {
            5: serial.FIVEBITS,
            6: serial.SIXBITS,
            7: serial.SEVENBITS,
            8: serial.EIGHTBITS,
        }[self._config.data_bits]

    def _parity(self, serial: Any) -> Any:
        """Parity."""
        return {
            UartParity.NONE: serial.PARITY_NONE,
            UartParity.ODD: serial.PARITY_ODD,
            UartParity.EVEN: serial.PARITY_EVEN,
            UartParity.MARK: serial.PARITY_MARK,
            UartParity.SPACE: serial.PARITY_SPACE,
        }[self._config.parity]

    def _stopbits(self, serial: Any) -> Any:
        """Stopbits."""
        return {
            UartStopBits.ONE: serial.STOPBITS_ONE,
            UartStopBits.ONE_POINT_FIVE: serial.STOPBITS_ONE_POINT_FIVE,
            UartStopBits.TWO: serial.STOPBITS_TWO,
        }[self._config.stop_bits]

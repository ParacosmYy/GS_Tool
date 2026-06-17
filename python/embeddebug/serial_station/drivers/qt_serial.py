"""PyQt6 QSerialPort adapter for the Python Serial Station lane."""

from __future__ import annotations

from PyQt6.QtCore import QIODeviceBase
from PyQt6.QtSerialPort import QSerialPort, QSerialPortInfo

from embeddebug.serial_station.drivers.base import BytesCallback, ErrorCallback, SerialPortConfig, SerialTransport


DATA_BITS = {
    5: QSerialPort.DataBits.Data5,
    6: QSerialPort.DataBits.Data6,
    7: QSerialPort.DataBits.Data7,
    8: QSerialPort.DataBits.Data8,
}
PARITY = {
    "none": QSerialPort.Parity.NoParity,
    "even": QSerialPort.Parity.EvenParity,
    "odd": QSerialPort.Parity.OddParity,
    "space": QSerialPort.Parity.SpaceParity,
    "mark": QSerialPort.Parity.MarkParity,
}
STOP_BITS = {
    "1": QSerialPort.StopBits.OneStop,
    "1.5": QSerialPort.StopBits.OneAndHalfStop,
    "2": QSerialPort.StopBits.TwoStop,
}
FLOW_CONTROL = {
    "none": QSerialPort.FlowControl.NoFlowControl,
    "hardware": QSerialPort.FlowControl.HardwareControl,
    "software": QSerialPort.FlowControl.SoftwareControl,
}


class QtSerialPortTransport(SerialTransport):
    """Thin QSerialPort adapter with callback-based byte delivery."""

    def __init__(self, port: QSerialPort | None = None) -> None:
        self._port = port or QSerialPort()
        self._config: SerialPortConfig | None = None
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._port.readyRead.connect(self._handle_ready_read)
        self._port.errorOccurred.connect(self._handle_error)

    @property
    def config(self) -> SerialPortConfig | None:
        return self._config

    @property
    def port_name(self) -> str:
        return self._port.portName()

    @property
    def baud_rate(self) -> int:
        return int(self._port.baudRate())

    @property
    def data_bits(self) -> int:
        return int(self._port.dataBits().name.removeprefix("Data"))

    @property
    def parity(self) -> str:
        return _reverse_lookup(PARITY, self._port.parity())

    @property
    def stop_bits(self) -> str:
        return _reverse_lookup(STOP_BITS, self._port.stopBits())

    @property
    def flow_control(self) -> str:
        return _reverse_lookup(FLOW_CONTROL, self._port.flowControl())

    @property
    def is_open(self) -> bool:
        return self._port.isOpen()

    @staticmethod
    def available_ports() -> list[str]:
        return [port.portName() for port in QSerialPortInfo.availablePorts()]

    def configure(self, config: SerialPortConfig) -> None:
        self._config = config
        self._port.setPortName(config.port_name)
        self._port.setBaudRate(config.baud_rate)
        self._port.setDataBits(DATA_BITS[config.data_bits])
        self._port.setParity(PARITY[config.parity])
        self._port.setStopBits(STOP_BITS[config.stop_bits])
        self._port.setFlowControl(FLOW_CONTROL[config.flow_control])

    def open(self, config: SerialPortConfig) -> bool:
        self.configure(config)
        ok = self._port.open(QIODeviceBase.OpenModeFlag.ReadWrite)
        if not ok:
            self._emit_error(self._port.errorString() or "open_failed")
        return bool(ok)

    def close(self) -> None:
        self._port.close()

    def write(self, data: bytes) -> int:
        if not self._port.isOpen():
            self._emit_error("transport_not_open")
            return 0
        return int(self._port.write(bytes(data)))

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def _handle_ready_read(self) -> None:
        payload = bytes(self._port.readAll())
        if not payload:
            return
        for callback in list(self._bytes_callbacks):
            callback(payload)

    def _handle_error(self, error: QSerialPort.SerialPortError) -> None:
        if error == QSerialPort.SerialPortError.NoError:
            return
        self._emit_error(self._port.errorString() or error.name)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)


def _reverse_lookup(mapping: dict[str, object] | dict[int, object], value: object) -> str:
    for key, mapped_value in mapping.items():
        if mapped_value == value:
            return str(key)
    return ""

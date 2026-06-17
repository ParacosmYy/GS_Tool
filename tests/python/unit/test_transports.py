from __future__ import annotations

from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    QtSerialPortTransport,
    SerialPortConfig,
)


def test_fake_transport_open_write_inject_and_close():
    received: list[bytes] = []
    transport = FakeSerialTransport()
    transport.on_bytes_received(received.append)

    assert not transport.is_open

    transport.open(SerialPortConfig(port_name="loopback", baud_rate=115200))
    written = transport.write(b"ping")
    transport.inject_rx(b"pong")
    transport.close()

    assert written == 4
    assert transport.written == [b"ping"]
    assert received == [b"pong"]
    assert not transport.is_open


def test_fake_transport_reports_errors_for_closed_write_and_scripted_open_failure():
    errors: list[str] = []
    transport = FakeSerialTransport(open_error="denied")
    transport.on_error(errors.append)

    assert not transport.open(SerialPortConfig(port_name="COM404"))

    closed = FakeSerialTransport()
    closed.on_error(errors.append)
    assert closed.write(b"data") == 0

    assert errors == ["denied", "transport_not_open"]


def test_qt_serial_transport_configures_port_without_opening(qtbot):
    transport = QtSerialPortTransport()
    config = SerialPortConfig(port_name="COM_TEST", baud_rate=57600)

    transport.configure(config)

    assert transport.config == config
    assert transport.port_name == "COM_TEST"
    assert transport.baud_rate == 57600
    assert not transport.is_open
    assert isinstance(QtSerialPortTransport.available_ports(), list)


def test_qt_serial_transport_configures_frame_settings(qtbot):
    transport = QtSerialPortTransport()
    config = SerialPortConfig(
        port_name="COM_TEST",
        baud_rate=38400,
        data_bits=7,
        parity="even",
        stop_bits="2",
        flow_control="hardware",
    )

    transport.configure(config)

    assert transport.config == config
    assert transport.data_bits == 7
    assert transport.parity == "even"
    assert transport.stop_bits == "2"
    assert transport.flow_control == "hardware"

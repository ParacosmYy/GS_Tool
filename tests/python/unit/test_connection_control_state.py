from __future__ import annotations

from embeddebug.serial_station.ui.connection_control_state import set_connection_control_state


class Button:
    def __init__(self) -> None:
        self.enabled = False

    def setEnabled(self, enabled: bool) -> None:
        self.enabled = enabled


class Host:
    def __init__(self) -> None:
        self._connect_button = Button()
        self._connect_serial_button = Button()
        self._connect_tcp_button = Button()
        self._connect_udp_button = Button()
        self._disconnect_button = Button()


def test_set_connection_control_state_enables_available_connectors_when_disconnected():
    host = Host()

    set_connection_control_state(host, connected=False, has_serial_ports=True)

    assert host._connect_button.enabled
    assert host._connect_serial_button.enabled
    assert host._connect_tcp_button.enabled
    assert host._connect_udp_button.enabled
    assert not host._disconnect_button.enabled


def test_set_connection_control_state_disables_connectors_when_connected():
    host = Host()

    set_connection_control_state(host, connected=True, has_serial_ports=True)

    assert not host._connect_button.enabled
    assert not host._connect_serial_button.enabled
    assert not host._connect_tcp_button.enabled
    assert not host._connect_udp_button.enabled
    assert host._disconnect_button.enabled


def test_set_connection_control_state_disables_serial_connector_without_ports():
    host = Host()

    set_connection_control_state(host, connected=False, has_serial_ports=False)

    assert host._connect_button.enabled
    assert not host._connect_serial_button.enabled
    assert host._connect_tcp_button.enabled
    assert host._connect_udp_button.enabled
    assert not host._disconnect_button.enabled

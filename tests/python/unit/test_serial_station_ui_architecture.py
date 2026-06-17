from __future__ import annotations

import importlib
import inspect

from embeddebug.serial_station.ui import (
    connection_actions,
    main_window,
    session_actions,
    tcp_controls,
)


def test_tcp_connection_action_lives_with_connection_actions():
    assert hasattr(connection_actions, "connect_tcp")
    assert not hasattr(tcp_controls, "connect_tcp_from_controls")

    source = inspect.getsource(tcp_controls)
    assert "connect_tcp_result" not in source
    assert "_status_label.setText" not in source


def test_disconnect_action_lives_with_connection_actions():
    assert hasattr(connection_actions, "disconnect")

    source = inspect.getsource(main_window.SerialStationMainWindow._disconnect)
    assert "connection_actions.disconnect(self)" in source
    assert "_controller.disconnect" not in source


def test_serial_port_refresh_action_lives_with_connection_actions():
    assert hasattr(connection_actions, "refresh_serial_ports")

    source = inspect.getsource(main_window.SerialStationMainWindow._refresh_serial_ports)
    assert "connection_actions.refresh_serial_ports(self)" in source
    assert "_controller.available_serial_ports" not in source
    assert "_refresh_port_combo" not in source


def test_clear_log_action_lives_with_session_actions():
    assert hasattr(session_actions, "clear_log")

    source = inspect.getsource(main_window.SerialStationMainWindow._clear_log)
    assert "session_actions.clear_log(self)" in source
    assert "_controller.clear_log" not in source


def test_protocol_selection_action_lives_with_protocol_actions():
    protocol_actions = importlib.import_module("embeddebug.serial_station.ui.protocol_actions")

    assert hasattr(protocol_actions, "select_protocol")

    source = inspect.getsource(main_window.SerialStationMainWindow._set_protocol)
    assert "protocol_actions.select_protocol(self, name)" in source
    assert "_controller.set_protocol" not in source

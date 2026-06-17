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


def test_connection_control_state_actions_live_with_connection_actions():
    assert hasattr(connection_actions, "set_connected_controls")
    assert hasattr(connection_actions, "has_serial_ports")

    controls_source = inspect.getsource(main_window.SerialStationMainWindow._set_connected_controls)
    ports_source = inspect.getsource(main_window.SerialStationMainWindow._has_serial_ports)
    assert "connection_actions.set_connected_controls(self, connected)" in controls_source
    assert "connection_actions.has_serial_ports(self)" in ports_source
    assert "_connect_button.setEnabled" not in controls_source
    assert "_port_combo.currentText" not in ports_source


def test_command_history_actions_live_with_connection_actions():
    assert hasattr(connection_actions, "refresh_command_history")
    assert hasattr(connection_actions, "select_command_history")

    refresh_source = inspect.getsource(main_window.SerialStationMainWindow._refresh_command_history)
    select_source = inspect.getsource(main_window.SerialStationMainWindow._select_command_history)
    assert "connection_actions.refresh_command_history(self)" in refresh_source
    assert "connection_actions.select_command_history(self, text)" in select_source
    assert "_controller.command_history" not in refresh_source
    assert "_send_edit.setText" not in select_source


def test_clear_log_action_lives_with_session_actions():
    assert hasattr(session_actions, "clear_log")

    source = inspect.getsource(main_window.SerialStationMainWindow._clear_log)
    assert "session_actions.clear_log(self)" in source
    assert "_controller.clear_log" not in source


def test_profile_control_apply_action_lives_with_session_actions():
    assert hasattr(session_actions, "apply_profile_controls")

    source = inspect.getsource(main_window.SerialStationMainWindow._apply_profile_controls)
    module_source = inspect.getsource(main_window)
    assert "session_actions.apply_profile_controls(self, profile)" in source
    assert "_select_combo_value" not in source
    assert "apply_tcp_profile_controls" not in module_source


def test_log_display_actions_live_with_log_actions():
    log_actions = importlib.import_module("embeddebug.serial_station.ui.log_actions")

    assert hasattr(log_actions, "append_log_entry")
    assert hasattr(log_actions, "render_log_entries")
    assert hasattr(log_actions, "update_log_stats")

    append_source = inspect.getsource(main_window.SerialStationMainWindow._append_log_entry)
    render_source = inspect.getsource(main_window.SerialStationMainWindow._render_log_entries)
    stats_source = inspect.getsource(main_window.SerialStationMainWindow._update_log_stats)
    assert "log_actions.append_log_entry(self, entry)" in append_source
    assert "log_actions.render_log_entries(self)" in render_source
    assert "log_actions.update_log_stats(self)" in stats_source
    assert "_controller.entries" not in render_source
    assert "_log_stats_label.setText" not in stats_source


def test_protocol_selection_action_lives_with_protocol_actions():
    protocol_actions = importlib.import_module("embeddebug.serial_station.ui.protocol_actions")

    assert hasattr(protocol_actions, "select_protocol")

    source = inspect.getsource(main_window.SerialStationMainWindow._set_protocol)
    assert "protocol_actions.select_protocol(self, name)" in source
    assert "_controller.set_protocol" not in source

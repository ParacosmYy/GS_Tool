from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.drivers import QtSerialPortTransport, TcpClientTransport
from embeddebug.shared import OperationResult


def test_pyqt_mvp_fake_connect_send_receive_and_clear(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    history_combo = window.findChild(QComboBox, "serialStationCommandHistoryCombo")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    clear_button = window.findChild(QPushButton, "serialStationClearButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert connect_button is not None
    assert send_edit is not None
    assert history_combo is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert clear_button is not None
    assert log_view is not None
    assert status_label is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    assert "Connected" in status_label.text()

    send_edit.setText("ping")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    assert history_combo.currentText() == "ping"

    inject_edit.setText("pong")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX ping" in log_view.toPlainText()
        and "RX pong" in log_view.toPlainText(),
        timeout=1000,
    )

    qtbot.mouseClick(clear_button, Qt.MouseButton.LeftButton)
    assert log_view.toPlainText() == ""


def test_pyqt_mvp_command_history_refills_send_edit(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    history_combo = window.findChild(QComboBox, "serialStationCommandHistoryCombo")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert history_combo is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("first")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    send_edit.setText("second")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    send_edit.clear()

    history_combo.setCurrentText("first")

    assert send_edit.text() == "first"


def test_pyqt_mvp_send_failure_shows_result_message(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert send_edit is not None
    assert send_button is not None
    assert status_label is not None

    send_edit.setText("before-open")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)

    assert "Send failed: Open a transport before sending" in status_label.text()


def test_pyqt_mvp_fake_connection_failure_shows_result_message(qtbot, monkeypatch):
    monkeypatch.setattr(
        "embeddebug.serial_station.controllers.SerialWorkbenchController.connect_fake_result",
        lambda self: OperationResult.failure("transport_open_failed", "fake denied"),
    )
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert connect_button is not None
    assert status_label is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)

    assert "Connection failed: fake denied" in status_label.text()


def test_pyqt_mvp_filters_log_by_direction(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert log_filter_combo is not None
    assert log_view is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("tx-only")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("rx-only")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX tx-only" in log_view.toPlainText()
        and "RX rx-only" in log_view.toPlainText(),
        timeout=1000,
    )

    log_filter_combo.setCurrentText("TX")
    assert "TX tx-only" in log_view.toPlainText()
    assert "RX rx-only" not in log_view.toPlainText()

    log_filter_combo.setCurrentText("RX")
    assert "TX tx-only" not in log_view.toPlainText()
    assert "RX rx-only" in log_view.toPlainText()

    log_filter_combo.setCurrentText("All")
    assert "TX tx-only" in log_view.toPlainText()
    assert "RX rx-only" in log_view.toPlainText()


def test_pyqt_mvp_filters_log_by_search_text(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_search_edit = window.findChild(QLineEdit, "serialStationLogSearchEdit")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert log_filter_combo is not None
    assert log_search_edit is not None
    assert log_view is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("alpha-command")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("beta-reply")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX alpha-command" in log_view.toPlainText()
        and "RX beta-reply" in log_view.toPlainText(),
        timeout=1000,
    )

    log_search_edit.setText("beta")
    assert "TX alpha-command" not in log_view.toPlainText()
    assert "RX beta-reply" in log_view.toPlainText()

    log_filter_combo.setCurrentText("TX")
    assert log_view.toPlainText() == ""

    log_search_edit.clear()
    assert "TX alpha-command" in log_view.toPlainText()
    assert "RX beta-reply" not in log_view.toPlainText()


def test_pyqt_mvp_log_stats_follow_entries_and_filters(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    clear_button = window.findChild(QPushButton, "serialStationClearButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_stats_label = window.findChild(QLabel, "serialStationLogStatsLabel")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert clear_button is not None
    assert log_filter_combo is not None
    assert log_stats_label is not None
    assert log_stats_label.text() == "Visible 0 / Total 0 | TX 0 | RX 0"

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("stat-tx")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("stat-rx")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(lambda: "Visible 2 / Total 2" in log_stats_label.text(), timeout=1000)
    assert "TX 1" in log_stats_label.text()
    assert "RX 1" in log_stats_label.text()

    log_filter_combo.setCurrentText("TX")
    assert log_stats_label.text() == "Visible 1 / Total 2 | TX 1 | RX 1"

    qtbot.mouseClick(clear_button, Qt.MouseButton.LeftButton)
    assert log_stats_label.text() == "Visible 0 / Total 0 | TX 0 | RX 0"


def test_pyqt_mvp_shortcuts_send_clear_and_refresh(qtbot, monkeypatch):
    port_snapshots = iter([[], ["COM_SHORTCUT"]])
    monkeypatch.setattr(
        QtSerialPortTransport,
        "available_ports",
        staticmethod(lambda: next(port_snapshots)),
    )

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    port_combo = window.findChild(QComboBox, "serialStationPortCombo")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert connect_button is not None
    assert send_edit is not None
    assert log_view is not None
    assert port_combo is not None
    assert status_label is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("shortcut")
    qtbot.keyClick(send_edit, Qt.Key.Key_Return, modifier=Qt.KeyboardModifier.ControlModifier)
    qtbot.waitUntil(lambda: "TX shortcut" in log_view.toPlainText(), timeout=1000)

    qtbot.keyClick(window, Qt.Key.Key_L, modifier=Qt.KeyboardModifier.ControlModifier)
    assert log_view.toPlainText() == ""

    qtbot.keyClick(window, Qt.Key.Key_R, modifier=Qt.KeyboardModifier.ControlModifier)
    assert port_combo.currentText() == "COM_SHORTCUT"
    assert "Serial ports refreshed" in status_label.text()


def test_pyqt_mvp_connects_selected_serial_port(qtbot, monkeypatch):
    monkeypatch.setattr(QtSerialPortTransport, "available_ports", staticmethod(lambda: ["COM_TEST"]))
    opened_configs = []
    monkeypatch.setattr(
        QtSerialPortTransport,
        "open",
        lambda self, config: opened_configs.append(config) or True,
    )

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    port_combo = window.findChild(QComboBox, "serialStationPortCombo")
    baud_combo = window.findChild(QComboBox, "serialStationBaudCombo")
    data_bits_combo = window.findChild(QComboBox, "serialStationDataBitsCombo")
    parity_combo = window.findChild(QComboBox, "serialStationParityCombo")
    stop_bits_combo = window.findChild(QComboBox, "serialStationStopBitsCombo")
    flow_control_combo = window.findChild(QComboBox, "serialStationFlowControlCombo")
    connect_serial_button = window.findChild(QPushButton, "serialStationConnectSerialButton")
    disconnect_button = window.findChild(QPushButton, "serialStationDisconnectButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert port_combo is not None
    assert baud_combo is not None
    assert data_bits_combo is not None
    assert parity_combo is not None
    assert stop_bits_combo is not None
    assert flow_control_combo is not None
    assert connect_serial_button is not None
    assert disconnect_button is not None
    assert status_label is not None

    assert port_combo.itemText(0) == "COM_TEST"
    baud_combo.setCurrentText("57600")
    data_bits_combo.setCurrentText("7")
    parity_combo.setCurrentText("Even")
    stop_bits_combo.setCurrentText("2")
    flow_control_combo.setCurrentText("Hardware")
    qtbot.mouseClick(connect_serial_button, Qt.MouseButton.LeftButton)

    assert opened_configs[-1].data_bits == 7
    assert opened_configs[-1].parity == "even"
    assert opened_configs[-1].stop_bits == "2"
    assert opened_configs[-1].flow_control == "hardware"
    assert "Connected to COM_TEST" in status_label.text()
    assert connect_serial_button.isEnabled() is False
    assert disconnect_button.isEnabled() is True


def test_pyqt_mvp_connects_tcp_endpoint_and_profiles(qtbot, monkeypatch, tmp_path):
    opened_configs = []

    def open_tcp(self, config):
        self._config = config
        opened_configs.append(config)
        return True

    monkeypatch.setattr(
        TcpClientTransport,
        "open",
        open_tcp,
    )

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    tcp_host_edit = window.findChild(QLineEdit, "serialStationTcpHostEdit")
    tcp_port_edit = window.findChild(QLineEdit, "serialStationTcpPortEdit")
    connect_tcp_button = window.findChild(QPushButton, "serialStationConnectTcpButton")
    disconnect_button = window.findChild(QPushButton, "serialStationDisconnectButton")
    profile_path_edit = window.findChild(QLineEdit, "serialStationProfilePathEdit")
    profile_name_edit = window.findChild(QLineEdit, "serialStationProfileNameEdit")
    save_profile_button = window.findChild(QPushButton, "serialStationSaveProfileButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        tcp_host_edit,
        tcp_port_edit,
        connect_tcp_button,
        disconnect_button,
        profile_path_edit,
        profile_name_edit,
        save_profile_button,
        status_label,
    ]:
        assert widget is not None

    tcp_host_edit.setText("127.0.0.1")
    tcp_port_edit.setText("19002")
    qtbot.mouseClick(connect_tcp_button, Qt.MouseButton.LeftButton)

    assert opened_configs[-1].port_name == "127.0.0.1:19002"
    assert "Connected to TCP 127.0.0.1:19002" in status_label.text()
    assert connect_tcp_button.isEnabled() is False
    assert disconnect_button.isEnabled() is True

    profile_path = tmp_path / "tcp-profile.json"
    profile_path_edit.setText(str(profile_path))
    profile_name_edit.setText("tcp-profile")
    qtbot.mouseClick(save_profile_button, Qt.MouseButton.LeftButton)

    profile_text = profile_path.read_text(encoding="utf-8")
    assert '"mode": "tcp"' in profile_text
    assert '"portName": "127.0.0.1:19002"' in profile_text


def test_pyqt_mvp_tcp_connection_failure_shows_result_message(qtbot, monkeypatch):
    monkeypatch.setattr(TcpClientTransport, "open", lambda self, config: False)

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    tcp_host_edit = window.findChild(QLineEdit, "serialStationTcpHostEdit")
    tcp_port_edit = window.findChild(QLineEdit, "serialStationTcpPortEdit")
    connect_tcp_button = window.findChild(QPushButton, "serialStationConnectTcpButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert tcp_host_edit is not None
    assert tcp_port_edit is not None
    assert connect_tcp_button is not None
    assert status_label is not None

    tcp_host_edit.setText("127.0.0.1")
    tcp_port_edit.setText("19003")
    qtbot.mouseClick(connect_tcp_button, Qt.MouseButton.LeftButton)

    assert "Connection failed: Failed to open tcp transport" in status_label.text()


def test_pyqt_mvp_refreshes_serial_ports_without_restart(qtbot, monkeypatch):
    port_snapshots = iter([[], ["COM_REFRESHED"]])
    monkeypatch.setattr(
        QtSerialPortTransport,
        "available_ports",
        staticmethod(lambda: next(port_snapshots)),
    )

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    port_combo = window.findChild(QComboBox, "serialStationPortCombo")
    refresh_button = window.findChild(QPushButton, "serialStationRefreshPortsButton")
    connect_serial_button = window.findChild(QPushButton, "serialStationConnectSerialButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert port_combo is not None
    assert refresh_button is not None
    assert connect_serial_button is not None
    assert status_label is not None
    assert port_combo.currentText() == "No serial ports"
    assert connect_serial_button.isEnabled() is False

    qtbot.mouseClick(refresh_button, Qt.MouseButton.LeftButton)

    assert port_combo.currentText() == "COM_REFRESHED"
    assert connect_serial_button.isEnabled() is True
    assert "Serial ports refreshed" in status_label.text()

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.drivers import QtSerialPortTransport, TcpClientTransport


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

    monkeypatch.setattr(TcpClientTransport, "open", open_tcp)

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


def test_pyqt_mvp_inject_failure_shows_result_message(qtbot, monkeypatch):
    def open_tcp(self, config):
        self._config = config
        return True

    monkeypatch.setattr(TcpClientTransport, "open", open_tcp)

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    tcp_host_edit = window.findChild(QLineEdit, "serialStationTcpHostEdit")
    tcp_port_edit = window.findChild(QLineEdit, "serialStationTcpPortEdit")
    connect_tcp_button = window.findChild(QPushButton, "serialStationConnectTcpButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        tcp_host_edit,
        tcp_port_edit,
        connect_tcp_button,
        inject_edit,
        inject_button,
        status_label,
    ]:
        assert widget is not None

    tcp_host_edit.setText("127.0.0.1")
    tcp_port_edit.setText("19004")
    qtbot.mouseClick(connect_tcp_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("not-fake")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    assert "Inject failed: Fake RX injection requires fake transport" in status_label.text()


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

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QLineEdit, QPushButton

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.drivers import UdpDatagramTransport


def test_pyqt_serial_station_connects_udp_endpoint_and_profiles(qtbot, monkeypatch, tmp_path):
    opened_configs = []

    def open_udp(self, config):
        self._config = config
        opened_configs.append(config)
        return True

    monkeypatch.setattr(UdpDatagramTransport, "open", open_udp)

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    udp_host_edit = window.findChild(QLineEdit, "serialStationUdpHostEdit")
    udp_port_edit = window.findChild(QLineEdit, "serialStationUdpPortEdit")
    connect_udp_button = window.findChild(QPushButton, "serialStationConnectUdpButton")
    disconnect_button = window.findChild(QPushButton, "serialStationDisconnectButton")
    profile_path_edit = window.findChild(QLineEdit, "serialStationProfilePathEdit")
    profile_name_edit = window.findChild(QLineEdit, "serialStationProfileNameEdit")
    save_profile_button = window.findChild(QPushButton, "serialStationSaveProfileButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        udp_host_edit,
        udp_port_edit,
        connect_udp_button,
        disconnect_button,
        profile_path_edit,
        profile_name_edit,
        save_profile_button,
        status_label,
    ]:
        assert widget is not None

    udp_host_edit.setText("127.0.0.1")
    udp_port_edit.setText("19005")
    qtbot.mouseClick(connect_udp_button, Qt.MouseButton.LeftButton)

    assert opened_configs[-1].port_name == "127.0.0.1:19005"
    assert "Connected to UDP 127.0.0.1:19005" in status_label.text()
    assert connect_udp_button.isEnabled() is False
    assert disconnect_button.isEnabled() is True

    profile_path = tmp_path / "udp-profile.json"
    profile_path_edit.setText(str(profile_path))
    profile_name_edit.setText("udp-profile")
    qtbot.mouseClick(save_profile_button, Qt.MouseButton.LeftButton)

    profile_text = profile_path.read_text(encoding="utf-8")
    assert '"mode": "udp"' in profile_text
    assert '"portName": "127.0.0.1:19005"' in profile_text

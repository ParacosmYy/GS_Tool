from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtNetwork import QHostAddress, QUdpSocket
from PyQt6.QtWidgets import QLabel, QLineEdit, QPlainTextEdit, QPushButton

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
    assert "local " in status_label.text()
    assert connect_udp_button.isEnabled() is False
    assert disconnect_button.isEnabled() is True

    profile_path = tmp_path / "udp-profile.json"
    profile_path_edit.setText(str(profile_path))
    profile_name_edit.setText("udp-profile")
    qtbot.mouseClick(save_profile_button, Qt.MouseButton.LeftButton)

    profile_text = profile_path.read_text(encoding="utf-8")
    assert '"mode": "udp"' in profile_text
    assert '"portName": "127.0.0.1:19005"' in profile_text


def test_pyqt_serial_station_udp_loopback_send_and_receive(qtbot):
    peer = QUdpSocket()
    assert peer.bind(QHostAddress(QHostAddress.SpecialAddress.LocalHost), 0)

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    udp_host_edit = window.findChild(QLineEdit, "serialStationUdpHostEdit")
    udp_port_edit = window.findChild(QLineEdit, "serialStationUdpPortEdit")
    connect_udp_button = window.findChild(QPushButton, "serialStationConnectUdpButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        udp_host_edit,
        udp_port_edit,
        connect_udp_button,
        send_edit,
        send_button,
        log_view,
        status_label,
    ]:
        assert widget is not None

    udp_host_edit.setText("127.0.0.1")
    udp_port_edit.setText(str(peer.localPort()))
    qtbot.mouseClick(connect_udp_button, Qt.MouseButton.LeftButton)

    status_text = status_label.text()
    assert "Connected to UDP 127.0.0.1" in status_text
    ui_udp_port = int(status_text.rsplit("local ", maxsplit=1)[-1])
    assert ui_udp_port > 0

    send_edit.setText("udp-ping")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(lambda: peer.hasPendingDatagrams(), timeout=1000)
    datagram = peer.receiveDatagram()
    assert bytes(datagram.data()) == b"udp-ping"
    assert "TX udp-ping" in log_view.toPlainText()

    assert peer.writeDatagram(
        b"udp-pong",
        QHostAddress.SpecialAddress.LocalHost,
        ui_udp_port,
    ) == len(b"udp-pong")
    qtbot.waitUntil(lambda: "RX udp-pong" in log_view.toPlainText(), timeout=1000)

    peer.close()

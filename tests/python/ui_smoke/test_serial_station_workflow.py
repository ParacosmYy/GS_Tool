from __future__ import annotations

import os
import json

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window
from embeddebug.serial_station.drivers import QtSerialPortTransport


def test_pyqt_workflow_exports_replays_and_loads_profile(qtbot, tmp_path):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    clear_button = window.findChild(QPushButton, "serialStationClearButton")
    log_path_edit = window.findChild(QLineEdit, "serialStationLogPathEdit")
    export_button = window.findChild(QPushButton, "serialStationExportLogButton")
    replay_button = window.findChild(QPushButton, "serialStationReplayLogButton")
    profile_path_edit = window.findChild(QLineEdit, "serialStationProfilePathEdit")
    profile_name_edit = window.findChild(QLineEdit, "serialStationProfileNameEdit")
    save_profile_button = window.findChild(QPushButton, "serialStationSaveProfileButton")
    load_profile_button = window.findChild(QPushButton, "serialStationLoadProfileButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")
    profile_label = window.findChild(QLabel, "serialStationProfileLabel")

    for widget in [
        connect_button,
        send_edit,
        send_button,
        inject_edit,
        inject_button,
        clear_button,
        log_path_edit,
        export_button,
        replay_button,
        profile_path_edit,
        profile_name_edit,
        save_profile_button,
        load_profile_button,
        log_view,
        status_label,
        profile_label,
    ]:
        assert widget is not None

    log_path = tmp_path / "session.jsonl"
    profile_path = tmp_path / "profile.json"

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("ping")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("pong")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    log_path_edit.setText(str(log_path))
    qtbot.mouseClick(export_button, Qt.MouseButton.LeftButton)
    assert log_path.is_file()
    assert "Saved log" in status_label.text()

    qtbot.mouseClick(clear_button, Qt.MouseButton.LeftButton)
    assert log_view.toPlainText() == ""

    qtbot.mouseClick(replay_button, Qt.MouseButton.LeftButton)
    qtbot.waitUntil(lambda: "TX ping" in log_view.toPlainText(), timeout=1000)
    assert "RX pong" in log_view.toPlainText()

    profile_path_edit.setText(str(profile_path))
    profile_name_edit.setText("bench-profile")
    qtbot.mouseClick(save_profile_button, Qt.MouseButton.LeftButton)
    assert profile_path.is_file()

    profile_name_edit.setText("")
    qtbot.mouseClick(load_profile_button, Qt.MouseButton.LeftButton)

    assert "bench-profile" in profile_label.text()
    assert "Loaded profile" in status_label.text()


def test_pyqt_workflow_loads_profile_into_serial_controls(qtbot, tmp_path, monkeypatch):
    monkeypatch.setattr(QtSerialPortTransport, "available_ports", staticmethod(lambda: ["COM_A"]))
    profile_path = tmp_path / "profile.json"
    profile_path.write_text(
        json.dumps(
            {
                "name": "serial-profile",
                "transport": {
                    "mode": "serial",
                    "portName": "COM_PROFILE",
                    "baudRate": 38400,
                    "dataBits": 7,
                    "parity": "even",
                    "stopBits": "2",
                    "flowControl": "software",
                    "connected": False,
                },
                "protocol": "fire_water",
                "commandHistory": ["status?", "reset"],
            }
        ),
        encoding="utf-8",
    )

    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    profile_path_edit = window.findChild(QLineEdit, "serialStationProfilePathEdit")
    load_profile_button = window.findChild(QPushButton, "serialStationLoadProfileButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    protocol_combo = window.findChild(QComboBox, "serialStationProtocolCombo")
    port_combo = window.findChild(QComboBox, "serialStationPortCombo")
    baud_combo = window.findChild(QComboBox, "serialStationBaudCombo")
    data_bits_combo = window.findChild(QComboBox, "serialStationDataBitsCombo")
    parity_combo = window.findChild(QComboBox, "serialStationParityCombo")
    stop_bits_combo = window.findChild(QComboBox, "serialStationStopBitsCombo")
    flow_control_combo = window.findChild(QComboBox, "serialStationFlowControlCombo")
    history_combo = window.findChild(QComboBox, "serialStationCommandHistoryCombo")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        profile_path_edit,
        load_profile_button,
        log_view,
        protocol_combo,
        port_combo,
        baud_combo,
        data_bits_combo,
        parity_combo,
        stop_bits_combo,
        flow_control_combo,
        history_combo,
        send_edit,
        status_label,
    ]:
        assert widget is not None

    profile_path_edit.setText(str(profile_path))
    qtbot.mouseClick(load_profile_button, Qt.MouseButton.LeftButton)

    assert protocol_combo.currentText() == "fire_water"
    assert port_combo.currentText() == "COM_PROFILE"
    assert baud_combo.currentText() == "38400"
    assert data_bits_combo.currentText() == "7"
    assert parity_combo.currentText() == "Even"
    assert stop_bits_combo.currentText() == "2"
    assert flow_control_combo.currentText() == "Software"
    assert history_combo.currentText() == "reset"
    history_combo.setCurrentText("status?")
    assert send_edit.text() == "status?"
    assert "Loaded profile" in status_label.text()
    assert "System profile loaded: serial-profile" in log_view.toPlainText()


def test_pyqt_workflow_replay_failure_preserves_visible_log(qtbot, tmp_path):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    log_path_edit = window.findChild(QLineEdit, "serialStationLogPathEdit")
    replay_button = window.findChild(QPushButton, "serialStationReplayLogButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        connect_button,
        send_edit,
        send_button,
        log_path_edit,
        replay_button,
        log_view,
        status_label,
    ]:
        assert widget is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("keep-me")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    assert "TX keep-me" in log_view.toPlainText()

    log_path_edit.setText(str(tmp_path / "missing-session.jsonl"))
    qtbot.mouseClick(replay_button, Qt.MouseButton.LeftButton)

    assert "TX keep-me" in log_view.toPlainText()
    assert "Replay failed" in status_label.text()


def test_pyqt_workflow_profile_load_failure_preserves_current_profile(qtbot, tmp_path):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    profile_path_edit = window.findChild(QLineEdit, "serialStationProfilePathEdit")
    profile_name_edit = window.findChild(QLineEdit, "serialStationProfileNameEdit")
    load_profile_button = window.findChild(QPushButton, "serialStationLoadProfileButton")
    profile_label = window.findChild(QLabel, "serialStationProfileLabel")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    for widget in [
        profile_path_edit,
        profile_name_edit,
        load_profile_button,
        profile_label,
        status_label,
    ]:
        assert widget is not None

    profile_name_edit.setText("existing-profile")
    profile_label.setText("Profile: existing-profile")
    profile_path_edit.setText(str(tmp_path / "missing-profile.json"))
    qtbot.mouseClick(load_profile_button, Qt.MouseButton.LeftButton)

    assert profile_name_edit.text() == "existing-profile"
    assert profile_label.text() == "Profile: existing-profile"
    assert "Load profile failed" in status_label.text()

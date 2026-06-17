from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window


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

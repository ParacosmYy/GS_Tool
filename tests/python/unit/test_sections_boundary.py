"""command_section build_send_row + sections build_*_row/footer 边界测试。

补强 test_animations_integrations / test_serial_station_ui_architecture 未直接断言的边角。
"""

from __future__ import annotations

import logging
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from PyQt6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.ui.command_section import build_send_row
class _SendHost:
    """模拟 CommandSectionHost（send_row 最小协议）。"""

    def __init__(self):
        self._send_text_called = False
        self._select_history_called = False

    def tr(self, text: str) -> str:
        return text

    def _send_text(self) -> None:
        self._send_text_called = True

    def _select_command_history(self, text: str) -> None:
        self._select_history_called = text

def test_build_send_row_returns_hboxlayout(qtbot):
    """build_send_row 返回 QHBoxLayout。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_send_row(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_send_row_creates_send_edit(qtbot):
    """send_row 创建 _send_edit（objectName=serialStationSendEdit）。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_send_row(owner, root)
    assert hasattr(owner, "_send_edit")
    assert owner._send_edit.objectName() == "serialStationSendEdit"


def test_build_send_row_creates_history_combo(qtbot):
    """send_row 创建 _command_history_combo。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_send_row(owner, root)
    assert hasattr(owner, "_command_history_combo")
    assert owner._command_history_combo.objectName() == "serialStationCommandHistoryCombo"


def test_build_send_row_creates_send_button(qtbot):
    """send_row 创建 _send_button。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_send_row(owner, root)
    assert hasattr(owner, "_send_button")
    assert isinstance(owner._send_button, QPushButton)


def test_build_send_row_logs_scale_press_install_failure(qtbot, monkeypatch, caplog):
    """发送按钮微交互安装失败不阻断 UI 构建，但必须留下 debug 日志。"""

    from embeddebug.serial_station.ui import micro_interactions

    def fail_install(button):
        del button
        raise RuntimeError("effect unavailable")

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    monkeypatch.setattr(micro_interactions, "install_scale_press", fail_install)
    caplog.set_level(logging.DEBUG, logger="embeddebug.serial_station.ui.command_section")

    build_send_row(owner, root)

    assert isinstance(owner._send_button, QPushButton)
    assert "install send button scale press failed" in caplog.text


def test_build_send_row_send_edit_is_lineedit(qtbot):
    """_send_edit 是 QLineEdit。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_send_row(owner, root)
    assert isinstance(owner._send_edit, QLineEdit)

class _SectionsHost:
    """模拟 SerialStationSectionsHost（最小协议：tr + 所有回调 + 属性）。"""

    def __init__(self):
        self._send_edit = QLineEdit()
        self._command_history_combo = None
        self._center_log_card = QWidget()
        self._center_waveform_card = QWidget()

    def tr(self, text: str) -> str:
        return text

    def _export_log(self): pass
    def _replay_log(self): pass
    def _save_profile(self): pass
    def _load_profile(self): pass
    def _clear_log(self): pass
    def _refresh_serial_ports(self): pass
    def _has_serial_ports(self): return True
    def _connect_fake(self): pass
    def _connect_serial(self): pass
    def _connect_tcp(self): pass
    def _connect_udp(self): pass
    def _disconnect(self): pass
    def _send_text(self): pass
    def _select_command_history(self, text): pass
    def _inject_received(self): pass
    def _render_log_entries(self): pass
    def _update_log_stats(self): pass


def test_build_inject_row_no_crash(qtbot):
    """build_inject_row 不崩溃（创建 inject_edit + button）。"""

    from embeddebug.serial_station.ui.sections import build_inject_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_inject_row(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_log_row_no_crash(qtbot):
    """build_log_row 不崩溃（创建 export/replay buttons）。"""

    from embeddebug.serial_station.ui.sections import build_log_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_log_row(owner, root)
    assert isinstance(row, QVBoxLayout)


def test_build_log_row_creates_filter_and_search_controls(qtbot):
    from embeddebug.serial_station.ui.sections import build_log_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)

    assert isinstance(owner._log_filter_combo, QComboBox)
    assert owner._log_filter_combo.objectName() == "serialStationLogFilterCombo"
    assert owner._log_filter_combo.currentText() == "All"
    assert owner._log_filter_combo.count() == 5
    assert isinstance(owner._log_search_edit, QLineEdit)
    assert owner._log_search_edit.objectName() == "serialStationLogSearchEdit"
    assert owner._log_search_edit.placeholderText() != ""


def test_build_log_row_creates_path_stats_and_banner(qtbot):
    from embeddebug.serial_station.ui.sections import build_log_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)

    assert isinstance(owner._log_path_edit, QLineEdit)
    assert owner._log_path_edit.objectName() == "serialStationLogPathEdit"
    assert isinstance(owner._log_stats_label, QLabel)
    assert owner._log_stats_label.objectName() == "serialStationLogStatsLabel"
    assert owner._log_info_banner is not None


def test_build_log_row_creates_action_buttons(qtbot):
    from embeddebug.serial_station.ui.sections import build_log_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)

    assert isinstance(owner._export_log_button, QPushButton)
    assert owner._export_log_button.objectName() == "serialStationExportLogButton"
    assert isinstance(owner._replay_log_button, QPushButton)
    assert owner._replay_log_button.objectName() == "serialStationReplayLogButton"


def test_build_profile_row_no_crash(qtbot):
    """build_profile_row 不崩溃（创建 save/load buttons）。"""

    from embeddebug.serial_station.ui.sections import build_profile_row

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_profile_row(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_footer_no_crash(qtbot):
    """build_footer 不崩溃（创建 clear button）。"""

    from embeddebug.serial_station.ui.sections import build_footer

    owner = _SectionsHost()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_footer(owner, root)
    assert isinstance(row, QHBoxLayout)

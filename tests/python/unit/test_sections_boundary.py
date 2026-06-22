"""command_section build_send_row + sections build_*_row/footer 边界测试。

补强 test_animations_integrations / test_serial_station_ui_architecture 未直接断言的边角：
- build_send_row：返回 QHBoxLayout + 创建 send_edit + command_history_combo + objectName。
- build_inject_row：返回 QHBoxLayout + inject_edit + inject_button。
- build_log_row：返回 QVBoxLayout + export/replay buttons。
- build_profile_row：返回 QHBoxLayout + save/load buttons。
- build_footer：返回 QHBoxLayout + clear button。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from PyQt6.QtWidgets import QHBoxLayout, QLineEdit, QPushButton, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.command_section import build_send_row


# ── Fake hosts ────────────────────────────────────────────────────────


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


# ── build_send_row ────────────────────────────────────────────────────


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


def test_build_send_row_send_edit_is_lineedit(qtbot):
    """_send_edit 是 QLineEdit。"""

    owner = _SendHost()
    root = QWidget()
    qtbot.addWidget(root)
    build_send_row(owner, root)
    assert isinstance(owner._send_edit, QLineEdit)


# ── sections build_*_row / footer ─────────────────────────────────────


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

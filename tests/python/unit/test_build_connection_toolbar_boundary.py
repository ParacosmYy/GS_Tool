"""build_connection_toolbar 整体装配边界测试。

test_animations_integrations 仅集成调用；本文件覆盖 build_connection_toolbar
装配后所有 owner 属性 + objectName 契约。

覆盖：
1. build_connection_toolbar 返回 QHBoxLayout。
2. owner._protocol_combo objectName + 初始 raw_data + 3 项。
3. owner._port_combo / _refresh_ports_button objectName。
4. 5 个 serial config combos objectName。
5. connect/disconnect buttons objectName + disconnect 初始 disabled。
6. connect_serial disabled 当无串口。
7. tcp/udp endpoint controls 附加 + 默认 endpoint。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QHBoxLayout, QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar


def _make_controller():
    controller = MagicMock()
    controller.available_protocols.return_value = ["raw_data", "fire_water", "just_float"]
    controller.available_serial_ports.return_value = []
    controller.is_connected = False
    return controller


def _make_owner_and_root(qtbot):
    """构造 owner + root（root 经 qtbot 保持存活）。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    owner._controller = _make_controller()
    owner._set_protocol = MagicMock()
    owner._refresh_serial_ports = MagicMock()
    owner._has_serial_ports = MagicMock(return_value=False)
    owner._connect_fake = MagicMock()
    owner._connect_serial = MagicMock()
    owner._connect_tcp = MagicMock()
    owner._connect_udp = MagicMock()
    owner._disconnect = MagicMock()
    owner._profile_label = QWidget(root)
    owner._status_label = QWidget(root)
    return owner, root


# ── build_connection_toolbar 返回 ────────────────────────────────
def test_build_returns_hboxlayout(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    toolbar = build_connection_toolbar(owner, owner._controller, root)
    assert isinstance(toolbar, QHBoxLayout)


# ── protocol_combo ───────────────────────────────────────────────
def test_protocol_combo_objectname(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._protocol_combo.objectName() == "serialStationProtocolCombo"


def test_protocol_combo_initial_raw_data(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._protocol_combo.currentText() == "raw_data"


def test_protocol_combo_has_three_items(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._protocol_combo.count() == 3


# ── port_combo + refresh ─────────────────────────────────────────
def test_port_combo_objectname(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._port_combo.objectName() == "serialStationPortCombo"


def test_refresh_ports_button_objectname(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._refresh_ports_button.objectName() == "serialStationRefreshPortsButton"


# ── serial config combos ─────────────────────────────────────────
def test_all_serial_config_combos_present(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    for attr, name in (
        ("_baud_combo", "serialStationBaudCombo"),
        ("_data_bits_combo", "serialStationDataBitsCombo"),
        ("_parity_combo", "serialStationParityCombo"),
        ("_stop_bits_combo", "serialStationStopBitsCombo"),
        ("_flow_control_combo", "serialStationFlowControlCombo"),
    ):
        assert getattr(owner, attr).objectName() == name


# ── connect buttons ──────────────────────────────────────────────
def test_connect_button_objectname(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._connect_button.objectName() == "serialStationConnectButton"
    assert owner._connect_serial_button.objectName() == "serialStationConnectSerialButton"


def test_disconnect_button_objectname_and_initial_disabled(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._disconnect_button.objectName() == "serialStationDisconnectButton"
    assert owner._disconnect_button.isEnabled() is False


def test_connect_serial_button_disabled_when_no_ports(qtbot):
    """_has_serial_ports 返回 False → connect_serial_button disabled。"""

    owner, root = _make_owner_and_root(qtbot)
    owner._has_serial_ports = MagicMock(return_value=False)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._connect_serial_button.isEnabled() is False


# ── tcp/udp endpoint controls ────────────────────────────────────
def test_tcp_controls_attached(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert isinstance(owner._tcp_host_edit, QLineEdit)
    assert isinstance(owner._tcp_port_edit, QLineEdit)
    assert isinstance(owner._connect_tcp_button, QPushButton)


def test_udp_controls_attached(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert isinstance(owner._udp_host_edit, QLineEdit)
    assert isinstance(owner._udp_port_edit, QLineEdit)
    assert isinstance(owner._connect_udp_button, QPushButton)


def test_tcp_default_endpoint(qtbot):
    owner, root = _make_owner_and_root(qtbot)
    build_connection_toolbar(owner, owner._controller, root)
    assert owner._tcp_host_edit.text() == "127.0.0.1"
    assert owner._tcp_port_edit.text() == "19000"

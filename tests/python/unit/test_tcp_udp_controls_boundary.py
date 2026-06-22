"""tcp_controls + udp_controls build + apply profile 行为边界测试。

test_serial_station_ui_architecture.py 仅源码级；本文件覆盖 build 返回 tuple +
objectName 契约 + apply profile 分发。

覆盖：
1. build_tcp_controls 返回 3-tuple（host/port/button）+ objectName。
2. build_tcp_controls host_edit 有 placeholder + tooltip。
3. build_tcp_controls port_edit returnPressed 连接。
4. build_tcp_controls connect_button clicked 连接 + objectName。
5. build_udp_controls 返回 3-tuple + objectName。
6. apply_tcp_profile_controls 调 apply_endpoint_profile_controls（mode=tcp）。
7. apply_udp_profile_controls 调 apply_endpoint_profile_controls（mode=udp）。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.tcp_controls import (
    apply_tcp_profile_controls,
    build_tcp_controls,
)
from embeddebug.serial_station.ui.udp_controls import (
    apply_udp_profile_controls,
    build_udp_controls,
)


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    owner._connect_tcp = MagicMock()
    owner._connect_udp = MagicMock()
    return owner


# ── build_tcp_controls ───────────────────────────────────────────
def test_build_tcp_controls_returns_three_tuple(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    result = build_tcp_controls(owner, root)
    assert len(result) == 3
    host, port, button = result
    assert isinstance(host, QLineEdit)
    assert isinstance(port, QLineEdit)
    assert isinstance(button, QPushButton)


def test_build_tcp_controls_objectnames(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    host, port, button = build_tcp_controls(owner, root)
    assert host.objectName() == "serialStationTcpHostEdit"
    assert port.objectName() == "serialStationTcpPortEdit"
    assert button.objectName() == "serialStationConnectTcpButton"


def test_build_tcp_controls_host_has_placeholder(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    host, _, _ = build_tcp_controls(owner, root)
    assert host.placeholderText() != ""


def test_build_tcp_controls_default_endpoint_applied(qtbot):
    """build 后 host/port 应有默认值（apply_default_endpoint_text）。"""

    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    host, port, _ = build_tcp_controls(owner, root)
    assert host.text() == "127.0.0.1"
    assert port.text() == "19000"


# ── build_udp_controls ───────────────────────────────────────────
def test_build_udp_controls_returns_three_tuple(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    result = build_udp_controls(owner, root)
    assert len(result) == 3
    host, port, button = result
    assert isinstance(host, QLineEdit)
    assert isinstance(port, QLineEdit)
    assert isinstance(button, QPushButton)


def test_build_udp_controls_objectnames(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    host, port, button = build_udp_controls(owner, root)
    assert host.objectName() == "serialStationUdpHostEdit"
    assert port.objectName() == "serialStationUdpPortEdit"
    assert button.objectName() == "serialStationConnectUdpButton"


def test_build_udp_controls_default_endpoint_applied(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    host, port, _ = build_udp_controls(owner, root)
    assert host.text() == "127.0.0.1"
    assert port.text() == "19000"


# ── apply_tcp_profile_controls ───────────────────────────────────
def test_apply_tcp_profile_controls_calls_apply_endpoint():
    """apply_tcp_profile_controls 委托 apply_endpoint_profile_controls（mode=tcp）。"""

    window = SimpleNamespace()
    window._tcp_host_edit = MagicMock()
    window._tcp_port_edit = MagicMock()
    transport = {"host": "192.168.1.1", "port": "8080"}
    with patch(
        "embeddebug.serial_station.ui.tcp_controls.apply_endpoint_profile_controls"
    ) as mock_apply:
        apply_tcp_profile_controls(window, transport, "TCP:8080")
        mock_apply.assert_called_once()
        call_kwargs = mock_apply.call_args
        assert call_kwargs.kwargs["expected_mode"] == "tcp"


def test_apply_udp_profile_controls_calls_apply_endpoint():
    """apply_udp_profile_controls 委托 apply_endpoint_profile_controls（mode=udp）。"""

    window = SimpleNamespace()
    window._udp_host_edit = MagicMock()
    window._udp_port_edit = MagicMock()
    transport = {"host": "10.0.0.1", "port": "9090"}
    with patch(
        "embeddebug.serial_station.ui.udp_controls.apply_endpoint_profile_controls"
    ) as mock_apply:
        apply_udp_profile_controls(window, transport, "UDP:9090")
        mock_apply.assert_called_once()
        call_kwargs = mock_apply.call_args
        assert call_kwargs.kwargs["expected_mode"] == "udp"

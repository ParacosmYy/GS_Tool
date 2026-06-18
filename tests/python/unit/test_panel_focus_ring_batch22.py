"""Batch 22 测试：域面板输入控件 focus_ring 接入。

覆盖：
1. apply_panel_focus_rings 给输入控件装 focus ring（返回装饰数 > 0）。
2. 跳过只读控件（isReadOnly True 不装）。
3. 无输入控件返回 0。
4. 5 域面板（OTA/CAN/BLE/RTT/Automation）build 后接入 apply_panel_focus_rings（源码断言）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLineEdit, QWidget

from embeddebug.serial_station.ui.panels._focus_ring import apply_panel_focus_rings


# ── apply_panel_focus_rings 行为 ───────────────────────────────────
def test_applies_to_line_edits(qtbot):
    """含 QLineEdit 的 root 应装 focus ring（返回 >= 1）。"""

    root = QWidget()
    qtbot.addWidget(root)
    QLineEdit(root)  # 可聚焦输入
    count = apply_panel_focus_rings(root)
    assert count >= 1


def test_skips_readonly_widgets(qtbot):
    """只读 QLineEdit 应被跳过（focus ring 对只读无意义）。"""

    root = QWidget()
    qtbot.addWidget(root)
    ro = QLineEdit(root)
    ro.setReadOnly(True)
    rw = QLineEdit(root)
    rw.setReadOnly(False)
    count = apply_panel_focus_rings(root)
    # 只装了非只读那一个。
    assert count == 1


def test_no_inputs_returns_zero(qtbot):
    """无输入控件的 root 返回 0。"""

    root = QWidget()
    qtbot.addWidget(root)
    assert apply_panel_focus_rings(root) == 0


def test_focus_ring_installs_graphics_effect(qtbot):
    """装 focus ring 后控件应有 graphicsEffect（_FocusRingFilter 挂载）。"""

    root = QWidget()
    qtbot.addWidget(root)
    edit = QLineEdit(root)
    apply_panel_focus_rings(root)
    # install_focus_ring 会 setGraphicsEffect + installEventFilter。
    assert edit.graphicsEffect() is not None


# ── 5 域面板源码接入断言 ───────────────────────────────────────────
def test_ota_panel_wires_focus_ring():
    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    assert "apply_panel_focus_rings" in inspect.getsource(OtaPanel)


def test_can_panel_wires_focus_ring():
    from embeddebug.serial_station.ui.panels.can_panel import CanPanel

    assert "apply_panel_focus_rings" in inspect.getsource(CanPanel)


def test_ble_panel_wires_focus_ring():
    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    assert "apply_panel_focus_rings" in inspect.getsource(BlePanel)


def test_rtt_panel_wires_focus_ring():
    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    assert "apply_panel_focus_rings" in inspect.getsource(RttPanel)


def test_automation_panel_wires_focus_ring():
    from embeddebug.serial_station.ui.panels.automation_panel import AutomationPanel

    assert "apply_panel_focus_rings" in inspect.getsource(AutomationPanel)


# ── helper 存在性 ──────────────────────────────────────────────────
def test_focus_ring_helper_module_exists():
    from embeddebug.serial_station.ui.panels import _focus_ring

    assert callable(_focus_ring.apply_panel_focus_rings)

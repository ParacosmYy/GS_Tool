"""B49-5 ui_smoke: 连接按钮 ProgressRing 加载态 + 端口刷新复用。

覆盖：
- set_button_loading(True)：按钮禁用、文字清空、ProgressRing 子控件显示。
- set_button_loading(False)：按钮启用、文字恢复、ProgressRing 隐藏。
- _set_loading 委托：connection_actions._set_loading 等价 set_button_loading。
- None 安全：button=None 不抛异常。
- ProgressRing indeterminate：setIndeterminate(True) 启动旋转动画。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QPushButton
from embeddebug.serial_station.ui.connection_actions import _set_loading
from embeddebug.serial_station.ui.connection_loading import set_button_loading


def _make_button(qtbot, text: str = "Connect Fake") -> QPushButton:
    btn = QPushButton(text)
    btn.setObjectName("serialStationConnectButton")
    btn.resize(120, 32)
    qtbot.addWidget(btn)
    btn.show()
    qtbot.waitUntil(lambda: btn.isVisible(), timeout=1000)
    return btn


def test_set_button_loading_true_embeds_ring(qtbot):
    """set_button_loading(True) 在按钮内嵌 ProgressRing 并清空文字。"""

    btn = _make_button(qtbot, "Connect Fake")
    set_button_loading(btn, True)

    assert not btn.isEnabled()
    assert btn.text() == ""
    ring = getattr(btn, "_loading_ring", None)
    assert ring is not None
    assert not ring.isHidden()
    assert ring.isIndeterminate()


def test_set_button_loading_false_restores(qtbot):
    """set_button_loading(False) 恢复文字、启用按钮、隐藏 ring。"""

    btn = _make_button(qtbot, "Connect Serial")
    set_button_loading(btn, True)
    set_button_loading(btn, False)

    assert btn.isEnabled()
    assert btn.text() == "Connect Serial"
    ring = btn._loading_ring
    assert ring.isHidden()


def test_set_button_loading_none_button_safe(qtbot):
    """button=None 安全跳过（对齐原 _set_loading 契约）。"""

    set_button_loading(None, True)  # 不应抛异常
    set_button_loading(None, False)


def test_set_loading_delegates_to_set_button_loading(qtbot):
    """connection_actions._set_loading 委托到 set_button_loading（B49-5 升级）。"""

    btn = _make_button(qtbot, "Connect TCP")
    _set_loading(btn, True)
    assert not btn.isEnabled()
    assert btn.text() == ""
    assert not btn._loading_ring.isHidden()

    _set_loading(btn, False)
    assert btn.isEnabled()
    assert btn.text() == "Connect TCP"
    assert btn._loading_ring.isHidden()


def test_set_button_loading_reuses_ring_across_cycles(qtbot):
    """多次 loading 循环复用同一个 ProgressRing（不重复创建）。"""

    btn = _make_button(qtbot, "Connect UDP")
    set_button_loading(btn, True)
    ring1 = btn._loading_ring

    set_button_loading(btn, False)
    set_button_loading(btn, True)
    ring2 = btn._loading_ring

    assert ring1 is ring2, "ProgressRing should be reused, not recreated"


def test_set_button_loading_ring_positioned_within_button(qtbot):
    """ProgressRing 居中放置在按钮内（move 后坐标在按钮 rect 内）。"""

    btn = _make_button(qtbot)
    set_button_loading(btn, True)
    ring = btn._loading_ring

    # ring 的几何应在按钮 rect 内（居中）。
    assert 0 <= ring.x() <= btn.width()
    assert 0 <= ring.y() <= btn.height()
    assert ring.width() < btn.width()

"""TopBar 组件单元测试。

验证 TopBar：
1. 以正确 objectName 创建，品牌区控件可达。
2. 复用 owner 的 _status_label / _profile_label（reparent，不改 objectName），
   保持 findChild 与 action 模块契约。
3. 标签缺失时静默跳过（调用顺序鲁棒性）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame, QLabel, QWidget

from embeddebug.serial_station.ui.main_window import SerialStationMainWindow
from embeddebug.serial_station.ui.top_bar import build_top_bar


class _StubOwner:
    """最小宿主：仅持有 tr + 可选的状态/Profile 标签。"""

    def __init__(self, root: QWidget, *, with_labels: bool = True) -> None:
        self._root = root
        if with_labels:
            self._status_label = QLabel("Disconnected", root)
            self._status_label.setObjectName("serialStationStatusLabel")
            self._profile_label = QLabel("Profile: unsaved", root)
            self._profile_label.setObjectName("serialStationProfileLabel")

    def tr(self, source_text: str) -> str:
        return source_text


def test_build_top_bar_creates_frame_with_objectname(qtbot):
    root = QWidget()
    qtbot.addWidget(root)
    owner = _StubOwner(root)
    bar = build_top_bar(owner, root)
    qtbot.addWidget(bar)
    assert isinstance(bar, QFrame)
    assert bar.objectName() == "serialStationTopBar"


def test_top_bar_exposes_brand_widgets(qtbot):
    root = QWidget()
    qtbot.addWidget(root)
    owner = _StubOwner(root)
    bar = build_top_bar(owner, root)
    qtbot.addWidget(bar)
    assert bar.findChild(QFrame, "serialStationBrandChip") is not None
    assert bar.findChild(QLabel, "serialStationBrandName") is not None
    assert bar.findChild(QLabel, "serialStationBrandTagline") is not None


def test_top_bar_repurposes_status_and_profile_labels(qtbot):
    """状态/Profile 标签被 reparent 到 TopBar，objectName 与文本逻辑不变。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _StubOwner(root)
    bar = build_top_bar(owner, root)
    qtbot.addWidget(bar)

    status = bar.findChild(QLabel, "serialStationStatusLabel")
    profile = bar.findChild(QLabel, "serialStationProfileLabel")
    assert status is owner._status_label
    assert profile is owner._profile_label
    assert status.text() == "Disconnected"
    assert profile.text() == "Profile: unsaved"


def test_top_bar_skips_missing_labels_without_error(qtbot):
    """标签未创建时不应抛异常（调用顺序鲁棒性）。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _StubOwner(root, with_labels=False)
    bar = build_top_bar(owner, root)
    qtbot.addWidget(bar)
    assert bar.objectName() == "serialStationTopBar"


def test_main_window_top_bar_preserves_critical_labels(qtbot):
    """完整窗口装配后，TopBar 上的状态/Profile 标签仍可 findChild（契约不破）。"""

    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    assert window.findChild(QFrame, "serialStationTopBar") is not None
    # 9 核心之一：statusLabel 必须仍可达（被 reparent 到 TopBar 后）。
    assert window.findChild(QLabel, "serialStationStatusLabel") is not None
    assert window.findChild(QLabel, "serialStationProfileLabel") is not None

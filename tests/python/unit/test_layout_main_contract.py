"""layout_main 三区分栏装配契约单元测试。

Batch 23 抽出的三栏 Shell 装配模块（assemble_three_zone），用 mock layout builders
避免重组件树，验证 splitter + 三区 + owner 属性契约：

- splitter：objectName / handleWidth=4 / childrenCollapsible=False / count=3。
- 三区：LeftZone minWidth=300 / CenterZone minWidth=380 / RightZone minWidth=260。
- owner 属性：_center_log_card / _center_waveform_card / _waveform_preview 注入。
- 三区 objectName 契约（QSS 覆盖）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QHBoxLayout, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.layout_main import assemble_three_zone


def _make_owner() -> SimpleNamespace:
    """构造最小 LayoutMainHost 替身（只需 tr 方法）。"""

    return SimpleNamespace(tr=lambda source_text: source_text)


def _make_layouts() -> tuple:
    """构造 6 个空 layout（toolbar/send/inject/log/profile/footer）。"""

    return (
        QVBoxLayout(), QHBoxLayout(), QHBoxLayout(),
        QHBoxLayout(), QHBoxLayout(), QHBoxLayout(),
    )


def test_assemble_three_zone_returns_splitter(qtbot):
    """assemble_three_zone 返回 QSplitter 实例。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _make_owner()
    splitter = assemble_three_zone(owner, root, *_make_layouts())
    qtbot.addWidget(splitter)
    from PyQt6.QtWidgets import QSplitter
    assert isinstance(splitter, QSplitter)


def test_assemble_three_zone_splitter_objectname(qtbot):
    """splitter objectName = serialStationMainSplitter（QSS 契约）。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.objectName() == "serialStationMainSplitter"


def test_assemble_three_zone_handle_width_is_four(qtbot):
    """handleWidth=4（Batch 46 加宽改善拖拽可用性）。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.handleWidth() == 4


def test_assemble_three_zone_children_not_collapsible(qtbot):
    """setChildrenCollapsible(False) 防误拖把分区整列吞掉。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.childrenCollapsible() is False


def test_assemble_three_zone_has_three_widgets(qtbot):
    """splitter 含 3 个 zone widget（左/中/右）。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.count() == 3


def test_assemble_three_zone_zone_objectnames(qtbot):
    """三区 objectName 契约（QSS zones_section 覆盖）。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.widget(0).objectName() == "serialStationLeftZone"
    assert splitter.widget(1).objectName() == "serialStationCenterZone"
    assert splitter.widget(2).objectName() == "serialStationRightZone"


def test_assemble_three_zone_minimum_widths(qtbot):
    """三区最小宽度：左 300 / 中 380 / 右 260（防窄栏截断横向控件）。"""

    root = QWidget()
    qtbot.addWidget(root)
    splitter = assemble_three_zone(_make_owner(), root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert splitter.widget(0).minimumWidth() == 300
    assert splitter.widget(1).minimumWidth() == 380
    assert splitter.widget(2).minimumWidth() == 260


def test_assemble_three_zone_injects_owner_center_cards(qtbot):
    """owner._center_log_card / _center_waveform_card 被注入（供 sections 后续挂 log_view）。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _make_owner()
    splitter = assemble_three_zone(owner, root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert hasattr(owner, "_center_log_card")
    assert hasattr(owner, "_center_waveform_card")
    from PyQt6.QtWidgets import QFrame
    assert isinstance(owner._center_log_card, QFrame)
    assert isinstance(owner._center_waveform_card, QFrame)


def test_assemble_three_zone_injects_waveform_preview(qtbot):
    """owner._waveform_preview 被注入（中区波形卡内嵌 SerialWaveformPreview）。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _make_owner()
    splitter = assemble_three_zone(owner, root, *_make_layouts())
    qtbot.addWidget(splitter)
    assert hasattr(owner, "_waveform_preview")


def test_assemble_three_zone_center_cards_have_minimum_height(qtbot):
    """中区 log/waveform 卡有最小高度（防压扁，保证可读）。"""

    root = QWidget()
    qtbot.addWidget(root)
    owner = _make_owner()
    splitter = assemble_three_zone(owner, root, *_make_layouts())
    qtbot.addWidget(splitter)
    # waveform_card minHeight=160, log_card minHeight=200。
    assert owner._center_waveform_card.minimumHeight() == 160
    assert owner._center_log_card.minimumHeight() == 200

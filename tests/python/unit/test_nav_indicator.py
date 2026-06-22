"""NavIndicator 单元测试 — NavRail 激活项滑动指示条。

直接 import 子模块（不依赖 animations/__init__.py 的 re-export，避免环境回退）。
不依赖 AppShell 装配（AppShell 接线由 Batch 115 在稳定文件系统下完成）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QRect
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.nav_indicator import (
    NavIndicator,
    _INDICATOR_HEIGHT_RATIO,
    _INDICATOR_WIDTH_PX,
)
from embeddebug.serial_station.ui.animations.reduced_motion import ReducedMotionState


def _reset_reduced_motion():
    ReducedMotionState.reset_for_tests()


def test_construction_defaults(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    qtbot.addWidget(indicator)
    assert indicator.objectName() == "serialStationNavIndicator"
    assert indicator.isVisible() is False
    assert indicator.indicatorOpacity == 0.0


def test_indicator_rect_for_geometry(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    button_rect = QRect(0, 10, 40, 40)
    rect = NavIndicator._indicator_rect_for(button_rect)
    expected_h = int(40 * _INDICATOR_HEIGHT_RATIO)
    expected_y = 10 + (40 - expected_h) // 2
    assert rect.width() == _INDICATOR_WIDTH_PX
    assert rect.height() == expected_h
    assert rect.y() == expected_y
    assert rect.x() == 0


def test_move_to_first_call_fades_in(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.move_to(QRect(0, 10, 40, 40), animate=True)
    # offscreen 平台 isVisible() 依赖父级 show；用 opacity 增长作为淡入启动的稳健信号。
    qtbot.wait(60)
    assert indicator.indicatorOpacity > 0.0


def test_move_to_animate_creates_slide(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    # 首次 move_to 触发淡入（opacity 从 0 增长）。
    indicator.move_to(QRect(0, 10, 40, 40), animate=True)
    qtbot.wait(60)
    assert indicator.indicatorOpacity > 0.0
    # 第二次 move_to（已可见态）：应创建 slide 动画。
    indicator.move_to(QRect(0, 60, 40, 40), animate=True)
    assert indicator._slide_anim is not None


def test_move_to_reduced_motion_no_slide(qtbot):
    _reset_reduced_motion()
    ReducedMotionState.current().set_enabled(True)
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.move_to(QRect(0, 10, 40, 40), animate=True)
    qtbot.wait(60)
    indicator.move_to(QRect(0, 60, 40, 40), animate=True)
    assert indicator._slide_anim is None


def test_move_to_no_animate_jumps(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.move_to(QRect(0, 10, 40, 40), animate=True)
    qtbot.wait(60)
    indicator.move_to(QRect(0, 60, 40, 40), animate=False)
    assert indicator._slide_anim is None


def test_indicator_opacity_clamp(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.indicatorOpacity = -0.5
    assert indicator.indicatorOpacity == 0.0
    indicator.indicatorOpacity = 1.5
    assert indicator.indicatorOpacity == 1.0
    indicator.indicatorOpacity = 0.7
    assert 0.69 < indicator.indicatorOpacity < 0.71


def test_stop_animations_clears_refs(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.move_to(QRect(0, 10, 40, 40), animate=True)
    qtbot.wait(60)
    indicator.move_to(QRect(0, 60, 40, 40), animate=True)
    indicator.stop_animations()
    assert indicator._slide_anim is None
    assert indicator._fade_anim is None


def test_paint_event_no_crash_when_zero_opacity(qtbot):
    _reset_reduced_motion()
    parent = QWidget()
    qtbot.addWidget(parent)
    indicator = NavIndicator(parent)
    indicator.resize(_INDICATOR_WIDTH_PX, 24)
    indicator.indicatorOpacity = 0.0
    indicator.setVisible(True)
    indicator.update()
    qtbot.wait(20)
    assert indicator.indicatorOpacity == 0.0

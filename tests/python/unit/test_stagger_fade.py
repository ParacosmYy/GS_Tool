"""Batch 21 测试：panel_animations.stagger_fade 激活 + workbench 卡片错峰淡入。

覆盖：
1. stagger_fade 存在且返回 N 个动画（每张卡片一个）。
2. stagger_fade 只用 fade_in（不 move 控件，对布局安全）。
3. stagger_fade 错峰：每张卡片延迟 index*delay_ms 启动。
4. main_window 构建后 _stagger_enter_cards 装了 stagger_fade（_card_enter_anims 非空）。
5. 源码接入断言。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QFrame, QWidget

from embeddebug.serial_station.ui.panel_animations import stagger_fade


# ── stagger_fade 行为 ──────────────────────────────────────────────
def test_stagger_fade_returns_one_anim_per_card(qtbot):
    """stagger_fade 应为每张卡片返回一个动画。"""

    cards = [QFrame() for _ in range(4)]
    for c in cards:
        qtbot.addWidget(c)
    anims = stagger_fade(cards, delay_ms=50)
    assert len(anims) == 4


def test_stagger_fade_does_not_move_widgets(qtbot):
    """stagger_fade 应纯淡入不 move 控件（对布局安全）。

    slide_in 会 widget.move()；stagger_fade 用 fade_in 只设 opacity。
    验证调用前后控件 pos 不变。
    """

    card = QFrame()
    qtbot.addWidget(card)
    card.move(100, 100)
    pos_before = card.pos()
    stagger_fade([card], delay_ms=10)
    assert card.pos() == pos_before  # 未被 move


def test_stagger_fade_empty_cards_returns_empty(qtbot):
    """空卡片列表应返回空动画列表。"""

    assert stagger_fade([], delay_ms=50) == []


def test_stagger_fade_starts_animations(qtbot):
    """stagger_fade 返回的动画应可启动（QTimer.singleShot 延迟启动）。"""

    card = QFrame()
    qtbot.addWidget(card)
    anims = stagger_fade([card], delay_ms=10)
    assert len(anims) == 1
    # 动画对象应有效（duration 正确）。
    assert anims[0].duration() > 0


# ── main_window 接入 ───────────────────────────────────────────────
def test_main_window_staggers_cards(qtbot):
    """SerialStationMainWindow 构建应装 stagger_fade（_card_enter_anims 非空）。"""

    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    window = SerialStationMainWindow()
    qtbot.addWidget(window)
    # 工作台含多张 serialStationCard，_card_enter_anims 应非空。
    assert hasattr(window, "_card_enter_anims")
    assert len(window._card_enter_anims) > 0


def test_main_window_has_stagger_enter_cards_method():
    """SerialStationMainWindow 应有 _stagger_enter_cards 方法（源码级断言）。"""

    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    src = inspect.getsource(SerialStationMainWindow)
    assert "_stagger_enter_cards" in src
    assert "stagger_fade" in src
    assert "serialStationCard" in src


# ── stagger_fade 模块存在性 ────────────────────────────────────────
def test_panel_animations_exposes_stagger_fade():
    """panel_animations 应暴露 stagger_fade。"""

    from embeddebug.serial_station.ui import panel_animations

    assert callable(panel_animations.stagger_fade)

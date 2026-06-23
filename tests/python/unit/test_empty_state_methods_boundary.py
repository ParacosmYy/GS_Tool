"""EmptyStateWidget set_description/show_with_fade/hide_with_fade/_stop_inflight 边界测试。

test_widgets_polish 覆盖基础；本文件补 set_description + fade duration + _stop_inflight。

覆盖：
1. set_title 更新标题文本。
2. set_description 更新描述文本。
3. show_with_fade 默认 duration 不崩。
4. show_with_fade 自定义 duration 不崩。
5. hide_with_fade 默认 duration 不崩。
6. hide_with_fade 自定义 duration 不崩。
7. _stop_inflight_fade 不崩。
8. 连续 show_with_fade 不崩（停旧动画）。
9. 连续 hide_with_fade 不崩。
10. set_title 后 set_description 独立更新。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.widgets import EmptyStateWidget


def _make_widget(qtbot):
    w = EmptyStateWidget(
        icon_name="inbox",
        title="初始标题",
        description="初始描述",
    )
    qtbot.addWidget(w)
    return w


# ── set_title / set_description ──────────────────────────────────
def test_set_title_updates(qtbot):
    w = _make_widget(qtbot)
    w.set_title("新标题")
    assert w._title_label.text() == "新标题"


def test_set_description_updates(qtbot):
    w = _make_widget(qtbot)
    w.set_description("新描述")
    assert w._desc_label.text() == "新描述"


def test_set_title_and_description_independent(qtbot):
    """set_title 后 set_description 独立更新（不互相干扰）。"""

    w = _make_widget(qtbot)
    w.set_title("标题A")
    w.set_description("描述B")
    assert w._title_label.text() == "标题A"
    assert w._desc_label.text() == "描述B"


# ── show_with_fade ───────────────────────────────────────────────
def test_show_with_fade_default_duration(qtbot):
    w = _make_widget(qtbot)
    w.show_with_fade()  # 默认 duration 不崩


def test_show_with_fade_custom_duration(qtbot):
    w = _make_widget(qtbot)
    w.show_with_fade(duration=500)  # 自定义不崩


def test_show_with_fade_consecutive(qtbot):
    """连续 show_with_fade 停旧动画不崩。"""

    w = _make_widget(qtbot)
    w.show_with_fade()
    w.show_with_fade()  # 再次


# ── hide_with_fade ───────────────────────────────────────────────
def test_hide_with_fade_default_duration(qtbot):
    w = _make_widget(qtbot)
    w.hide_with_fade()  # 默认不崩


def test_hide_with_fade_custom_duration(qtbot):
    w = _make_widget(qtbot)
    w.hide_with_fade(duration=300)  # 自定义不崩


def test_hide_with_fade_consecutive(qtbot):
    """连续 hide_with_fade 不崩。"""

    w = _make_widget(qtbot)
    w.hide_with_fade()
    w.hide_with_fade()


# ── _stop_inflight_fade ──────────────────────────────────────────
def test_stop_inflight_fade_no_crash(qtbot):
    """_stop_inflight_fade 不崩（无进行中动画时安全）。"""

    w = _make_widget(qtbot)
    w._stop_inflight_fade()


def test_stop_inflight_fade_after_show(qtbot):
    """show_with_fade 后 _stop_inflight_fade 停止动画不崩。"""

    w = _make_widget(qtbot)
    w.show_with_fade()
    w._stop_inflight_fade()

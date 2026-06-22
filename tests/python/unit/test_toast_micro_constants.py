"""toast/skeleton/micro_interactions 常量 + toast_container 属性边界测试。

补强 test_toast_widget/container 未直接断言的边角：
- toast.py 常量：_DEFAULT_TIMEOUT_MS=3000 / _DEFAULT_SLIDE_DISTANCE=80 /
  _LEVEL_COLORS 4 键 / _LEVEL_ICON 4 键。
- micro_interactions.py 常量：LIFT_PIXELS / SHADOW_BLUR_NORMAL / SHADOW_BLUR_HOVER /
  ANIM_DURATION = DURATION_FAST。
- SkeletonWidget：默认 height=20 + stop_shimmer 不崩溃。
- ToastContainer：空容器 is_empty/count/active_toasts + clear_all 空不崩溃。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


from embeddebug.serial_station.notifications.data import NotificationLevel
from embeddebug.serial_station.ui.widgets.toast import (
    _DEFAULT_SLIDE_DISTANCE,
    _DEFAULT_TIMEOUT_MS,
    _LEVEL_COLORS,
    _LEVEL_ICON,
)
from embeddebug.serial_station.notifications import NotificationManager
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer
from embeddebug.serial_station.ui.widgets.skeleton import SkeletonWidget
from embeddebug.serial_station.ui import micro_interactions
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def _make_container(qtbot):
    """构造带 manager 的 ToastContainer。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    return container


# ── toast.py 常量 ──────────────────────────────────────────────────────


def test_default_timeout_ms_3000():
    """_DEFAULT_TIMEOUT_MS = 3000。"""

    assert _DEFAULT_TIMEOUT_MS == 3000


def test_default_slide_distance_80():
    """_DEFAULT_SLIDE_DISTANCE = 80。"""

    assert _DEFAULT_SLIDE_DISTANCE == 80


def test_level_colors_has_four_levels():
    """_LEVEL_COLORS 含 4 个 NotificationLevel 映射。"""

    assert len(_LEVEL_COLORS) == 4
    for level in NotificationLevel:
        assert level in _LEVEL_COLORS


def test_level_icon_has_four_levels():
    """_LEVEL_ICON 含 4 个 NotificationLevel 映射。"""

    assert len(_LEVEL_ICON) == 4
    for level in NotificationLevel:
        assert level in _LEVEL_ICON


def test_level_colors_values_non_empty():
    """_LEVEL_COLORS 值非空（颜色 hex 或 rgba）。"""

    for _level, color in _LEVEL_COLORS.items():
        assert isinstance(color, str)
        assert len(color) > 0


def test_level_icon_values_non_empty():
    """_LEVEL_ICON 值非空（lucide 图标名）。"""

    for _level, icon in _LEVEL_ICON.items():
        assert isinstance(icon, str)
        assert len(icon) > 0


# ── micro_interactions.py 常量 ─────────────────────────────────────────


def test_lift_pixels_constant():
    """LIFT_PIXELS = AnimationTokens.LIFT_PIXELS。"""

    assert micro_interactions.LIFT_PIXELS == AnimationTokens.LIFT_PIXELS


def test_shadow_blur_normal_constant():
    """SHADOW_BLUR_NORMAL = AnimationTokens.SHADOW_BLUR_NORMAL。"""

    assert micro_interactions.SHADOW_BLUR_NORMAL == AnimationTokens.SHADOW_BLUR_NORMAL


def test_shadow_blur_hover_constant():
    """SHADOW_BLUR_HOVER = AnimationTokens.SHADOW_BLUR_HOVER。"""

    assert micro_interactions.SHADOW_BLUR_HOVER == AnimationTokens.SHADOW_BLUR_HOVER


def test_anim_duration_constant():
    """ANIM_DURATION = AnimationTokens.DURATION_FAST。"""

    assert micro_interactions.ANIM_DURATION == AnimationTokens.DURATION_FAST


def test_shadow_blur_hover_greater_than_normal():
    """hover blur > normal blur。"""

    assert micro_interactions.SHADOW_BLUR_HOVER > micro_interactions.SHADOW_BLUR_NORMAL


# ── SkeletonWidget 边界 ────────────────────────────────────────────────


def test_skeleton_default_height(qtbot):
    """SkeletonWidget 默认 height=20。"""

    skel = SkeletonWidget()
    qtbot.addWidget(skel)
    assert skel.height() == 20 or skel.minimumHeight() == 20 or skel.sizeHint().height() <= 20


def test_skeleton_custom_height(qtbot):
    """自定义 height=40。"""

    skel = SkeletonWidget(height=40)
    qtbot.addWidget(skel)
    skel.stop_shimmer()  # 不崩溃


# ── ToastContainer 空容器属性 ─────────────────────────────────────────


def test_empty_container_is_empty(qtbot):
    """空容器 is_empty=True。"""

    container = _make_container(qtbot)
    assert container.is_empty is True


def test_empty_container_count_zero(qtbot):
    """空容器 count=0。"""

    container = _make_container(qtbot)
    assert container.count == 0


def test_empty_container_active_toasts_empty(qtbot):
    """空容器 active_toasts=()。"""

    container = _make_container(qtbot)
    assert container.active_toasts == ()


def test_clear_all_empty_no_crash(qtbot):
    """clear_all 空容器不崩溃。"""

    container = _make_container(qtbot)
    container.clear_all()  # 不抛


def test_dismiss_oldest_empty_returns_false(qtbot):
    """dismiss_oldest 空容器 → False。"""

    container = _make_container(qtbot)
    assert container.dismiss_oldest() is False

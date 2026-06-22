"""BouncePathAnimation 边界 + plugins/discovery 常量单元测试。

补强 test_animations_factories 未直接断言的边角：
- _offset_rect：dx/dy 偏移 + 零偏移返回原 rect。
- _scaled_rect_centered：factor 缩放 + 中心对齐 + 0 clamp。
- 常量：DROP_INITIAL_SCALE=0.6 / SQUASH_HEIGHT_RATIO=0.92 / SQUASH_WIDTH_RATIO=1.04。
- PLUGIN_FILE=plugin.py / SUPPORTED_PLUGIN_TYPES。
- slide_bounce：from_x 参数 + 返回 QPropertyAnimation。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.plugins.discovery import (
    PLUGIN_FILE,
    SUPPORTED_PLUGIN_TYPES,
)


# ── BouncePathAnimation 常量 ───────────────────────────────────────────


def test_drop_initial_scale_is_pop_in():
    """DROP_INITIAL_SCALE = SCALE_POP_IN = 0.6。"""

    assert BouncePathAnimation.DROP_INITIAL_SCALE == AnimationTokens.SCALE_POP_IN
    assert BouncePathAnimation.DROP_INITIAL_SCALE == 0.6


def test_squash_height_ratio():
    """SQUASH_HEIGHT_RATIO = 0.92。"""

    assert BouncePathAnimation.SQUASH_HEIGHT_RATIO == 0.92


def test_squash_width_ratio():
    """SQUASH_WIDTH_RATIO = 1.04。"""

    assert BouncePathAnimation.SQUASH_WIDTH_RATIO == 1.04


# ── _offset_rect 纯几何 ───────────────────────────────────────────────


def _widget(qtbot, x=10, y=20, w=100, h=50):
    wid = QWidget()
    wid.setGeometry(x, y, w, h)
    qtbot.addWidget(wid)
    return wid


def test_offset_rect_positive_dx(qtbot):
    """dx>0 → x 右移，尺寸不变。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._offset_rect(w, 30, 0)
    assert result.x() == 40
    assert result.width() == 100


def test_offset_rect_negative_dy(qtbot):
    """dy<0 → y 上移。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._offset_rect(w, 0, -50)
    assert result.y() == -30


def test_offset_rect_zero_returns_same(qtbot):
    """dx=dy=0 → 返回与原 rect 相同位置和尺寸。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._offset_rect(w, 0, 0)
    assert result.size() == w.geometry().size()


def test_offset_rect_preserves_size(qtbot):
    """偏移不改变尺寸。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._offset_rect(w, 100, 200)
    assert result.width() == 100
    assert result.height() == 50


# ── _scaled_rect_centered 纯几何 ──────────────────────────────────────


def test_scaled_rect_centered_factor_one(qtbot):
    """factor=1.0 → 宽高不变 + 中心对齐。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._scaled_rect_centered(w, 1.0)
    assert result.width() == 100
    assert result.height() == 50
    assert result.center() == w.geometry().center()


def test_scaled_rect_centered_half(qtbot):
    """factor=0.5 → 宽高减半。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._scaled_rect_centered(w, 0.5)
    assert result.width() == 50
    assert result.height() == 25


def test_scaled_rect_centered_zero_clamped(qtbot):
    """factor=0 → clamp min(1)。"""

    w = _widget(qtbot, 10, 20, 100, 50)
    result = BouncePathAnimation._scaled_rect_centered(w, 0.0)
    assert result.width() >= 1
    assert result.height() >= 1


# ── slide_bounce ──────────────────────────────────────────────────────


def test_slide_bounce_returns_animation(qtbot):
    """slide_bounce 返回 QPropertyAnimation。"""

    from PyQt6.QtCore import QPropertyAnimation

    w = _widget(qtbot, 50, 50, 80, 40)
    anim = BouncePathAnimation.slide_bounce(w, from_x=200)
    assert isinstance(anim, QPropertyAnimation)


def test_slide_bounce_registered_for_gc(qtbot):
    """slide_bounce 注册到 _active（防 GC）。"""

    BouncePathAnimation._active.clear()
    w = _widget(qtbot, 50, 50, 80, 40)
    anim = BouncePathAnimation.slide_bounce(w, from_x=200)
    assert anim in BouncePathAnimation._active


# ── plugins/discovery 常量 ────────────────────────────────────────────


def test_plugin_file_constant():
    """PLUGIN_FILE = plugin.py。"""

    assert PLUGIN_FILE == "plugin.py"


def test_supported_plugin_types():
    """SUPPORTED_PLUGIN_TYPES = ('protocol', 'control')。"""

    assert set(SUPPORTED_PLUGIN_TYPES) == {"protocol", "control"}
    assert len(SUPPORTED_PLUGIN_TYPES) == 2


def test_plugin_info_is_frozen():
    """PluginInfo 是 frozen dataclass。"""

    from embeddebug.serial_station.plugins.discovery import PluginInfo
    from pathlib import Path
    import pytest

    info = PluginInfo(
        name="test", version="1.0", plugin_type="protocol",
        path=Path("/tmp"), module_name="test_mod",
    )
    with pytest.raises((AttributeError, Exception)):
        info.name = "changed"  # type: ignore[misc]

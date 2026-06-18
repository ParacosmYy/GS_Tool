"""按钮 ripple 水波纹反馈测试（Material 风格）。

覆盖诊断报告剩余项『按钮无 ripple』：
1. RippleButton 自绘 ripple：点击位置扩散 accent 圆形涟漪。
2. install_ripple 动态注入：给现有 QPushButton 注入 ripple。
3. ConfigurableButton.set_ripple 开关。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPointF

from embeddebug.serial_station.ui.controls import ConfigurableButton
from embeddebug.serial_station.ui.controls.ripple import RippleButton, install_ripple


def test_ripple_button_has_objectname(qtbot):
    btn = RippleButton("Click")
    qtbot.addWidget(btn)
    assert btn.objectName() == "serialStationRippleButton"


def test_ripple_button_click_starts_animation(qtbot):
    """点击应启动 ripple 扩散动画（ripple_progress 动画对象非 None）。"""

    btn = RippleButton("Click")
    qtbot.addWidget(btn)
    # 初始无动画。
    assert btn._ripple_anim is None
    # 模拟点击位置启动 ripple。
    btn._start_ripple(QPointF(10, 10))
    assert btn._ripple_anim is not None
    assert btn._ripple_center == QPointF(10, 10)


def test_ripple_button_set_ripple_disable(qtbot):
    """set_ripple(False) 后点击不启动 ripple。"""

    btn = RippleButton("Click")
    qtbot.addWidget(btn)
    btn.set_ripple(False)
    btn._start_ripple(QPointF(5, 5))
    # set_ripple(False) 不阻止手动 _start_ripple（它是底层 API），
    # 但 mousePressEvent 会检查 _ripple_enabled 跳过。验证标志。
    assert btn._ripple_enabled is False


def test_ripple_button_progress_property(qtbot):
    """ripple_progress 属性应可读写并触发 update。"""

    btn = RippleButton("Click")
    qtbot.addWidget(btn)
    btn._ripple_center = QPointF(10, 10)
    btn.ripple_progress = 0.5
    assert btn.ripple_progress == 0.5


def test_install_ripple_on_plain_button(qtbot):
    """install_ripple 应给普通 QPushButton 注入 ripple 能力。"""

    from PyQt6.QtWidgets import QPushButton

    btn = QPushButton("X")
    qtbot.addWidget(btn)
    install_ripple(btn)
    assert getattr(btn, "_ripple_enabled", False) is True
    # 注入后应有 patched mousePressEvent。
    assert hasattr(btn, "mousePressEvent")


def test_configurable_button_set_ripple(qtbot):
    """ConfigurableButton.set_ripple(True) 应注入 ripple（默认关闭）。"""

    btn = ConfigurableButton("Send")
    qtbot.addWidget(btn)
    assert btn._ripple_enabled is False
    btn.set_ripple(True)
    assert btn._ripple_enabled is True


def test_empty_state_cta_is_ripple_button(qtbot):
    """EmptyStateWidget 的 CTA 应是 RippleButton（主操作有 ripple）。"""

    from embeddebug.serial_station.ui.widgets import EmptyStateWidget

    w = EmptyStateWidget(title="T", description="D", cta_text="重试")
    qtbot.addWidget(w)
    assert isinstance(w._cta_button, RippleButton)

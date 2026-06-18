"""动画 token：统一时长与缓动曲线，全应用一致。"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve


class AnimationTokens:
    """动画时长（毫秒）与缓动曲线 token。

    所有动画工厂引用本类的常量，保证视觉节奏统一。
    """

    # 时长
    DURATION_INSTANT = 100
    DURATION_FAST = 180
    DURATION_NORMAL = 280
    DURATION_SLOW = 400
    DURATION_SLOWER = 600

    # 缓动曲线
    EASE_OUT = QEasingCurve.Type.OutCubic
    EASE_IN = QEasingCurve.Type.InCubic
    EASE_IN_OUT = QEasingCurve.Type.InOutCubic
    EASE_OUT_BACK = QEasingCurve.Type.OutBack
    EASE_OUT_BOUNCE = QEasingCurve.Type.OutBounce
    EASE_OUT_ELASTIC = QEasingCurve.Type.OutElastic
    LINEAR = QEasingCurve.Type.Linear

    # 缩放比例
    SCALE_PRESSED = 0.92
    SCALE_HOVER = 1.05
    SCALE_NORMAL = 1.0

    # 折叠
    COLLAPSED_HEIGHT = 0

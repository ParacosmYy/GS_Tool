"""Elevation 阴影工厂 — 统一所有 ``QGraphicsDropShadowEffect`` 构造入口。

所有需要阴影的控件都应通过 ``elevation_effect(level, color)`` 构造，避免散落在
``micro_interactions.py`` / ``waveform_preview.py`` 等处的硬编码 blur/offset/alpha。

对标 Material Design 3 elevation L0-L5 规范，level 参数取自 ``AnimationTokens.ELEVATION_*``。

用法::

    from embeddebug.serial_station.ui.animations.elevation import elevation_effect
    from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

    shadow = elevation_effect(AnimationTokens.ELEVATION_L3)
    widget.setGraphicsEffect(shadow)

    # hover 时升级到 L5
    shadow.setBlurRadius(AnimationTokens.ELEVATION_L5[0])
"""

from __future__ import annotations

from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QGraphicsDropShadowEffect

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


def elevation_effect(
    level: tuple[int, int, int] = AnimationTokens.ELEVATION_L1,
    color: QColor | None = None,
) -> QGraphicsDropShadowEffect:
    """创建统一规约的阴影效果。

    Args:
        level: ``(blur_radius, offset_y, alpha)`` 三元组。
            取自 ``AnimationTokens.ELEVATION_L0`` ~ ``ELEVATION_L5``。
            - L0=(0,0,0)：无阴影
            - L1=(8,1,60)：卡片静态
            - L2=(12,3,80)：抽屉/折叠展开
            - L3=(16,3,120)：hover/focus
            - L4=(24,4,160)：popover/toast/flyout
            - L5=(32,6,200)：modal/dragged
        color: 阴影颜色。``None`` 时用黑色 + level 中的 alpha。
            传 accent 色可实现品牌色辉光。

    Returns:
        配置好的 ``QGraphicsDropShadowEffect``，调用方 ``setGraphicsEffect()`` 即可。
    """

    blur, offset_y, alpha = level
    effect = QGraphicsDropShadowEffect()
    effect.setBlurRadius(blur)
    effect.setOffset(0, offset_y)
    if color is None:
        effect.setColor(QColor(0, 0, 0, alpha))
    else:
        c = QColor(color)
        c.setAlpha(alpha)
        effect.setColor(c)
    return effect

"""微交互（对齐 Linear/Arc 级精致度）。

- ``install_hover_lift``：按钮/卡片 hover 时**真实垂直上浮 + 双层阴影增强**
  （黑色阴影保持深度 + accent 色阴影提供品牌强调）。
- ``install_focus_ring``：输入框 focus 时强调色光环。
- ``HoverLiftFilter``：事件过滤器，统一管理 hover 浮起动画。

修正记录（对比旧实现）：
- 旧版 ``_HoverLiftFilter`` 声明了 ``_original_geometry`` 和 ``LIFT_PIXELS`` 但只动
  ``blurRadius``，widget 几何从不变化，「hover 上浮」实际只有阴影变模糊，效果极弱。
- 新版真正位移 widget（上浮 ``LIFT_PIXELS``），并把阴影模糊半径从 normal→hover 动画，
  阴影颜色从黑色→accent tint 半透明，产生 Linear/Vercel 级的卡片浮起质感。
- 阴影色 hover 时切到 accent 半透明（``ACCENT_BORDER``），让卡片边缘有品牌色辉光。

约束：本模块只依赖 PyQt6 + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import (
    QEvent,
    QObject,
    QPoint,
    QPropertyAnimation,
    pyqtProperty,
)
from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

# 模块级兼容别名：旧测试和外部代码可能直接引用 micro_interactions.SHADOW_BLUR_* /
# LIFT_PIXELS / ANIM_DURATION。统一指向 AnimationTokens，保持单一真相源。
LIFT_PIXELS = AnimationTokens.LIFT_PIXELS
SHADOW_BLUR_NORMAL = AnimationTokens.SHADOW_BLUR_NORMAL
SHADOW_BLUR_HOVER = AnimationTokens.SHADOW_BLUR_HOVER
SHADOW_BLUR_FOCUS = AnimationTokens.SHADOW_BLUR_FOCUS
ANIM_DURATION = AnimationTokens.DURATION_FAST


def install_hover_lift(widget: QWidget, accent_tint: bool = True) -> QGraphicsDropShadowEffect:
    """给 widget 安装 hover 浮起 + 双层阴影效果。

    Args:
        widget: 目标按钮/卡片。
        accent_tint: hover 时阴影是否染 accent 色（品牌强调辉光）。

    Returns:
        阴影 effect（已挂到 widget，调用方一般无需持有）。

    注意：会改变 widget 的 pos（上浮 ``LIFT_PIXELS``）。widget 应处于不会被
    父布局立即回弹的位置（按钮/图标按钮通常 OK）。
    """

    effect = QGraphicsDropShadowEffect(widget)
    effect.setBlurRadius(AnimationTokens.SHADOW_BLUR_NORMAL)
    effect.setColor(QColor(*AnimationTokens.SHADOW_COLOR_RGB))
    effect.setOffset(0, AnimationTokens.SHADOW_OFFSET_Y)
    widget.setGraphicsEffect(effect)
    widget.installEventFilter(_HoverLiftFilter(widget, effect, accent_tint))
    return effect


def install_card_shadow(widget: QWidget, level=None) -> "QGraphicsDropShadowEffect":
    """给静态卡片/面板安装统一规约的 elevation 阴影（Batch 50-3）。

    与 ``install_hover_lift``（动态 hover 浮起）互补：本函数用于**静态**卡片
    （如 BasePanel、placeholder），无需 hover 动画，只设固定深度阴影。

    通过 ``elevation_effect(level)`` 统一构造，替代散落的硬编码
    ``QGraphicsDropShadowEffect`` + setBlurRadius/setColor/setOffset 三连。
    激活 elevation 模块进入生产链（死代码守护测试 B50-2 要求）。

    Args:
        widget: 目标卡片/面板。
        level: ``(blur, offset_y, alpha)`` 三元组，取自 ``AnimationTokens.ELEVATION_*``。
            默认 ``ELEVATION_L1``（卡片静态深度）。

    Returns:
        配置好的 ``QGraphicsDropShadowEffect``（已挂到 widget）。
    """

    from embeddebug.serial_station.ui.animations.elevation import elevation_effect

    if level is None:
        level = AnimationTokens.ELEVATION_L1
    effect = elevation_effect(level)
    effect.setParent(widget)
    widget.setGraphicsEffect(effect)
    return effect


class _HoverLiftFilter(QObject):
    """hover 进入/离开时：阴影模糊动画 + 阴影色动画 + widget 垂直位移。"""

    def __init__(self, widget: QWidget, effect: QGraphicsDropShadowEffect,
                 accent_tint: bool = True) -> None:
        super().__init__(widget)
        self._widget = widget
        self._effect = effect
        self._accent_tint = accent_tint
        self._shadow_anim: QPropertyAnimation | None = None
        self._pos_anim: QPropertyAnimation | None = None
        self._color_anim: QPropertyAnimation | None = None
        self._base_pos = widget.pos()

    def eventFilter(self, obj: object, event: QEvent) -> bool:
        if obj is not self._widget:
            return False
        etype = event.type()
        if etype == QEvent.Type.Enter:
            self._on_enter()
        elif etype == QEvent.Type.Leave:
            self._on_leave()
        return False

    def _on_enter(self) -> None:
        self._animate_shadow(AnimationTokens.SHADOW_BLUR_HOVER, hover=True)
        self._animate_lift(up=True)

    def _on_leave(self) -> None:
        self._animate_shadow(AnimationTokens.SHADOW_BLUR_NORMAL, hover=False)
        self._animate_lift(up=False)

    def _animate_shadow(self, target_blur: int, hover: bool) -> None:
        if self._shadow_anim is not None:
            self._shadow_anim.stop()
        self._shadow_anim = QPropertyAnimation(self._effect, b"blurRadius", self._widget)
        self._shadow_anim.setDuration(AnimationTokens.DURATION_FAST)
        self._shadow_anim.setStartValue(self._effect.blurRadius())
        self._shadow_anim.setEndValue(target_blur)
        self._shadow_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._shadow_anim.start()

        if self._accent_tint:
            # 阴影色：normal 黑 → hover accent 半透明（品牌辉光）。
            # Batch 10: accent 跟随用户选中的强调色变体（cyan/blue/.../teal），
            # 而非写死 cyan。accent 变体切换时辉光随之变色。
            from embeddebug.serial_station.ui.theme.accents import active_accent_border

            target_color = QColor(active_accent_border()) if hover else QColor(*AnimationTokens.SHADOW_COLOR_RGB)
            # QGraphicsDropShadowEffect.color 是 QColor，无原生动画；用 setKeyValueAt 近似。
            # 这里直接 set，淡入由 blurRadius 动画的视觉融合承担（阴影半径变化时颜色过渡自然）。
            self._effect.setColor(target_color)

    def _animate_lift(self, up: bool) -> None:
        """widget 垂直位移动画（上浮 LIFT_PIXELS 或回落）。"""

        if self._pos_anim is not None:
            self._pos_anim.stop()
        current = self._widget.pos()
        # 基准位（非 hover 时的位置）：如果当前 y 已被上浮过，恢复基准。
        base_y = self._base_pos.y()
        target_y = base_y - AnimationTokens.LIFT_PIXELS if up else base_y
        self._pos_anim = QPropertyAnimation(self._widget, b"pos", self._widget)
        self._pos_anim.setDuration(AnimationTokens.DURATION_FAST)
        self._pos_anim.setStartValue(current)
        self._pos_anim.setEndValue(QPoint(current.x(), target_y))
        self._pos_anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._pos_anim.start()


def install_focus_ring(widget: QWidget) -> None:
    """给输入框安装 focus 光环（focus 时阴影变强调色 + 扩散）。"""

    # Batch 10: focus 光环色跟随当前 accent 变体（cyan/blue/.../teal）。
    from embeddebug.serial_station.ui.theme.accents import active_accent_base

    effect = QGraphicsDropShadowEffect(widget)
    effect.setBlurRadius(0)
    effect.setColor(QColor(active_accent_base()))
    effect.setOffset(0, 0)
    widget.setGraphicsEffect(effect)
    widget.installEventFilter(_FocusRingFilter(effect, widget))


def install_scale_press(button) -> None:
    """给按钮安装分离式按压 scale 弹性反馈（Batch 14，落地「动态化动画缩放」）。

    - ``pressed`` → ``ScaleAnimation.press_down``（即时陷下到 0.96）；同时捕获
      press 前原始几何，供 release 回弹归位。
    - ``released`` → ``ScaleAnimation.press_up``（OutBack 回弹到原几何）。

    与 ``ConfigurableButton`` 的 ``ScaleAnimation.press``（单一合并动画绑 clicked）
    的区别：分离式绑 pressed/released，按下即陷、松手弹回，观感更物理。
    适用于任意 ``QPushButton``（含工具栏/设置页等非 ConfigurableButton 按钮）。
    动画自动防 GC（ScaleAnimation 类级活跃列表），无需持有引用。
    """

    from PyQt6.QtCore import QRect

    from embeddebug.serial_station.ui.animations.scale import ScaleAnimation

    # press 前原始几何（pressed 时捕获，released 时传给 press_up 归位）。
    state = {"orig": QRect()}

    def _on_pressed() -> None:
        state["orig"] = QRect(button.geometry())
        ScaleAnimation.press_down(button).start()

    def _on_released() -> None:
        ScaleAnimation.press_up(button, state["orig"]).start()

    button.pressed.connect(_on_pressed)
    button.released.connect(_on_released)


class _FocusRingFilter(QObject):
    """focus in/out 动画调整光环半径与颜色。"""

    def __init__(self, effect: QGraphicsDropShadowEffect, widget: QWidget) -> None:
        super().__init__(widget)
        self._effect = effect
        self._widget = widget
        self._anim: QPropertyAnimation | None = None

    def eventFilter(self, obj: object, event: QEvent) -> bool:
        if obj is not self._widget:
            return False
        etype = event.type()
        if etype == QEvent.Type.FocusIn:
            self._animate(0, AnimationTokens.SHADOW_BLUR_FOCUS)
        elif etype == QEvent.Type.FocusOut:
            self._animate(self._effect.blurRadius(), 0)
        return False

    def _animate(self, start: int, end: int) -> None:
        if self._anim is not None:
            self._anim.stop()
        self._anim = QPropertyAnimation(self._effect, b"blurRadius", self._widget)
        self._anim.setDuration(AnimationTokens.DURATION_FAST)
        self._anim.setStartValue(start)
        self._anim.setEndValue(end)
        self._anim.setEasingCurve(AnimationTokens.EASE_OUT)
        self._anim.start()


def install_nav_hover_scale(button) -> None:
    """给 NavRail 图标按钮安装 hover scale 高亮（Batch 15，落地「动画缩放」）。

    hover enter → ``ScaleAnimation`` 放大到 1.12（图标「弹出」高亮）；
    hover leave → 回弹到原态（EASE_OUT_BACK 轻微过冲）。通过事件过滤器监听
    ``Enter``/``Leave``，比 signal 更精准（覆盖鼠标滑过而不只是停留）。

    与 ``install_scale_press`` 叠加安全：press 改 geometry 到 0.96，hover 改到 1.12，
    两者都中心锚定，不会冲突（实际交互中 hover 与 press 时序错开）。
    动画自动防 GC（ScaleAnimation 类级活跃列表）。
    """

    button.installEventFilter(_NavHoverScaleFilter(button))


class _NavHoverScaleFilter(QObject):
    """NavRail 图标 hover enter/leave 缩放高亮。"""

    # hover 放大比例（NavRail 图标「弹出」感，比 hover_in 的 1.03 更显著，
    # 因为主导航元素需更强反馈；token 化以便全应用统一）。
    HOVER_SCALE = AnimationTokens.SCALE_NAV_HOVER

    def __init__(self, button) -> None:
        super().__init__(button)
        self._button = button
        # enter 时捕获原几何，leave 时恢复到它（避免从放大态 ÷1.0 仍放大）。
        self._orig_rect = None

    def eventFilter(self, obj: object, event: QEvent) -> bool:
        if obj is not self._button:
            return False
        from PyQt6.QtCore import QRect

        from embeddebug.serial_station.ui.animations.scale import ScaleAnimation

        etype = event.type()
        if etype == QEvent.Type.Enter:
            self._orig_rect = QRect(self._button.geometry())
            anim = QPropertyAnimation(self._button, b"geometry", self._button)
            anim.setDuration(AnimationTokens.DURATION_FAST)
            anim.setStartValue(self._button.geometry())
            anim.setEndValue(ScaleAnimation._scaled_rect(self._button, self.HOVER_SCALE))
            anim.setEasingCurve(AnimationTokens.EASE_OUT)
            ScaleAnimation._track(anim).start()
        elif etype == QEvent.Type.Leave:
            orig = self._orig_rect if self._orig_rect is not None else QRect(self._button.geometry())
            # 以当前（放大态）中心为锚恢复原尺寸。
            restored = QRect(orig)
            restored.moveCenter(self._button.geometry().center())
            anim = QPropertyAnimation(self._button, b"geometry", self._button)
            anim.setDuration(AnimationTokens.DURATION_FAST)
            anim.setStartValue(self._button.geometry())
            anim.setEndValue(restored)
            anim.setEasingCurve(AnimationTokens.EASE_OUT_BACK)
            ScaleAnimation._track(anim).start()
        return False

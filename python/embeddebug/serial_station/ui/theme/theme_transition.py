"""主题切换过渡动画 —— 整窗 windowOpacity 暗淡 + 回亮交叉。

切换主题/强调色时，把整窗透明度从 1.0 暗淡到 ~0.6，**在最低点换 QSS**，
再回亮到 1.0。视觉上是一次柔和的「呼吸」，掩盖 QSS 硬切的闪烁，观感优于
瞬时 ``setStyleSheet``。

为什么用 ``windowOpacity`` 而非 ``QGraphicsOpacityEffect``：

- ``QGraphicsOpacityEffect`` 作用于单个 ``QWidget``，挂到顶层窗口时 Qt 已知会
  glitch（重绘撕裂、子控件闪烁、某些平台无效）。
- ``windowOpacity`` 是 OS 级窗口合成属性，对整个顶层窗及其全部子控件统一生效，
  稳定无 glitch，是 macOS / VS Code / Linear 等生产级 App 主题切换的标准做法。
- ``QApplication`` 原生暴露 ``windowOpacity`` 可动画属性，无需额外 effect 控件。

节奏对齐 ``AnimationTokens``：暗淡段用 ``EASE_IN``（自然加速下沉），回亮段用
``EASE_OUT``（自然减速回升），总时长 ~300ms（暗 120 + 亮 180，与
``DURATION_NORMAL=240`` 同量级，略偏慢以容纳换 QSS 的耗时）。

GC 防护采用 ``animations.scale`` 的 ``_active``/``_track``/``_discard`` 类级
活跃列表范式（动画完成自动移除），调用方无需持有引用。

约束：本模块只依赖 PyQt6 + animations.tokens，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import (
    QPropertyAnimation,
    QSequentialAnimationGroup,
)
from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens

# 暗淡到的不透明度谷值（0.6：足够暗以掩盖 QSS 切换，又不至于全黑丢失定位）。
OPACITY_DIP = 0.6

# 暗淡段 / 回亮段时长（毫秒）。总 ~300ms，对齐 DURATION_NORMAL 量级。
DIP_MS = 120   # 暗下去（EASE_IN）
RISE_MS = 180  # 亮回来（EASE_OUT，略慢，视觉收束感）


class ThemeTransition:
    """整窗 windowOpacity 过渡动画工厂（类级活跃列表防 GC）。

    用法（theme_switcher 在换主题时调用）::

        ThemeTransition.run(app, lambda: app.setStyleSheet(new_qss))

    ``apply_fn`` 在窗口暗到 OPACITY_DIP 的瞬间被调用（此时做 QSS 硬切，
    用户视觉已被暗度掩盖）。
    """

    # 持有进行中的过渡动画引用，防止 Python 侧 GC（完成自动移除）。
    _active: list[QSequentialAnimationGroup] = []

    @classmethod
    def _track(cls, group: QSequentialAnimationGroup) -> QSequentialAnimationGroup:
        cls._active.append(group)
        group.finished.connect(lambda: cls._discard(group))
        return group

    @classmethod
    def _discard(cls, group: QSequentialAnimationGroup) -> None:
        try:
            cls._active.remove(group)
        except ValueError:
            pass

    @staticmethod
    def is_running() -> bool:
        """是否有过渡动画在进行（用于防重入）。"""

        return bool(ThemeTransition._active)

    @classmethod
    def run(
        cls,
        app: QApplication,
        apply_fn: Callable[[], None],
        *,
        dip_ms: int = DIP_MS,
        rise_ms: int = RISE_MS,
    ) -> QSequentialAnimationGroup | None:
        """运行一次「暗淡 → 换 QSS → 回亮」过渡。

        Args:
            app: 目标 ``QApplication``（动画其 ``windowOpacity``）。
            apply_fn: 在透明度谷值时调用的回调（执行真正的 QSS/accent 切换）。
            dip_ms: 暗淡段时长。
            rise_ms: 回亮段时长。

        Returns:
            启动的动画组（已 ``start()``，自动防 GC）；若已有过渡在进行则返回
            ``None``（防重入：上一次未完成时不打断，直接跳过本次过渡，由调用方
            决定是否同步执行 ``apply_fn``）。
        """

        if cls.is_running():
            return None  # 防重入：上一次过渡未完成。

        # 段 1：1.0 → OPACITY_DIP（EASE_IN，自然加速下沉）。
        dip = QPropertyAnimation(app, b"windowOpacity")
        dip.setDuration(dip_ms)
        dip.setStartValue(1.0)
        dip.setEndValue(OPACITY_DIP)
        dip.setEasingCurve(AnimationTokens.EASE_IN)

        # 段 2：零长暂停 —— 在谷值点同步执行 apply_fn（换 QSS）。
        # 用 QPauseAnimation(0) + valueChanged 不可靠（0 长 pause 不触发回调），
        # 改用 dip.finished 直连 apply_fn，保证在「暗到位、尚未回亮」的瞬间切换。
        dip.finished.connect(apply_fn)

        # 段 3：OPACITY_DIP → 1.0（EASE_OUT，自然减速回升）。
        rise = QPropertyAnimation(app, b"windowOpacity")
        rise.setDuration(rise_ms)
        rise.setStartValue(OPACITY_DIP)
        rise.setEndValue(1.0)
        rise.setEasingCurve(AnimationTokens.EASE_OUT)

        group = QSequentialAnimationGroup(app)
        group.addAnimation(dip)
        group.addAnimation(rise)

        cls._track(group)
        group.start()
        return group


def transition_theme(app: QApplication, apply_fn: Callable[[], None]) -> bool:
    """便捷入口：运行一次主题过渡，返回是否真正启动了动画。

    若上一次过渡仍在进行（防重入），返回 ``False`` —— 此时调用方应**同步执行**
    ``apply_fn``（不带动画地换 QSS），避免用户操作丢失。
    """

    group = ThemeTransition.run(app, apply_fn)
    if group is None:
        # 防重入：仍要保证主题被切换（只是没动画）。
        apply_fn()
        return False
    return True


__all__ = [
    "DIP_MS",
    "OPACITY_DIP",
    "RISE_MS",
    "ThemeTransition",
    "transition_theme",
]

"""折叠展开动画：菜单隐藏/显示、侧边栏折叠。

通过动画修改 ``maximumHeight`` 实现 JAccordion 风格的折叠展开。
时长引用 ``AnimationTokens``。
"""

from __future__ import annotations

from PyQt6.QtCore import QPropertyAnimation, pyqtSignal
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.tokens import AnimationTokens


class CollapseAnimation:
    """高度折叠/展开动画工厂。

    所有方法返回的动画自动注册到类级活跃列表防 GC，完成自动移除。
    """

    _active: list = []

    @classmethod
    def _track(cls, anim: QPropertyAnimation) -> QPropertyAnimation:
        cls._active.append(anim)
        anim.finished.connect(lambda: cls._discard(anim))
        return anim

    @classmethod
    def _discard(cls, anim: QPropertyAnimation) -> None:
        try:
            cls._active.remove(anim)
        except ValueError:
            pass

    @staticmethod
    def expand(widget: QWidget, target_height: int) -> QPropertyAnimation:
        """展开：从当前高度动画到 target_height。"""

        anim = QPropertyAnimation(widget, b"maximumHeight", widget)
        anim.setDuration(AnimationTokens.DURATION_NORMAL)
        anim.setStartValue(widget.maximumHeight())
        anim.setEndValue(target_height)
        anim.setEasingCurve(AnimationTokens.EASE_OUT)
        return CollapseAnimation._track(anim)

    @staticmethod
    def collapse(widget: QWidget) -> QPropertyAnimation:
        """折叠：动画到 0 高度。"""

        anim = QPropertyAnimation(widget, b"maximumHeight", widget)
        anim.setDuration(AnimationTokens.DURATION_FAST)
        anim.setStartValue(widget.maximumHeight())
        anim.setEndValue(0)
        anim.setEasingCurve(AnimationTokens.EASE_IN)
        return CollapseAnimation._track(anim)

    @staticmethod
    def stop(widget: QWidget) -> None:
        """停止 widget 上正在进行的折叠动画。"""

        for anim in list(CollapseAnimation._active):
            if anim.parent() is widget:
                anim.stop()
                CollapseAnimation._discard(anim)


class CollapsiblePanel(QWidget):
    """可折叠面板：点击标题栏展开/收起内容区。

    用法：
        panel = CollapsiblePanel(title="设置")
        panel.set_content(my_settings_widget)
        panel.toggle()  # 切换展开/折叠
    """

    toggled = pyqtSignal(bool)

    def __init__(self, title: str = "", parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationCollapsiblePanel")
        self._expanded = True
        self._target_height = 200
        self._title = title

    def set_target_height(self, height: int) -> None:
        """设置展开后的目标高度。"""

        self._target_height = max(1, int(height))

    @property
    def is_expanded(self) -> bool:
        return self._expanded

    def toggle(self) -> None:
        """切换展开/折叠状态并播放动画。"""

        if self._expanded:
            self.collapse()
        else:
            self.expand()

    def expand(self) -> None:
        """展开。"""

        self._expanded = True
        self.show()
        anim = CollapseAnimation.expand(self, self._target_height)
        anim.start()
        self.toggled.emit(True)

    def collapse(self) -> None:
        """折叠。"""

        self._expanded = False
        anim = CollapseAnimation.collapse(self)
        anim.start()
        self.toggled.emit(False)

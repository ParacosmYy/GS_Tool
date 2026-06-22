"""动画控制器：统一管理多个动画的生命周期与组合。"""

from __future__ import annotations

from PyQt6.QtCore import QParallelAnimationGroup, QSequentialAnimationGroup, QObject



class AnimationController(QObject):
    """统一管理控件的动画序列。

    支持串行/并行组合，自动持有动画对象防止 GC，stop 清理全部。
    """

    def __init__(self, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._animations: list = []
        self._groups: list = []

    def add(self, animation) -> None:
        """注册一个动画（防 GC）。"""
        self._animations.append(animation)

    def play_sequential(self, animations: list) -> QSequentialAnimationGroup:
        """串行播放多个动画。"""
        group = QSequentialAnimationGroup(self)
        for anim in animations:
            group.addAnimation(anim)
        self._groups.append(group)
        group.start()
        return group

    def play_parallel(self, animations: list) -> QParallelAnimationGroup:
        """并行播放多个动画。"""
        group = QParallelAnimationGroup(self)
        for anim in animations:
            group.addAnimation(anim)
        self._groups.append(group)
        group.start()
        return group

    def stop_all(self) -> None:
        """停止并清理所有动画。"""
        for group in self._groups:
            group.stop()
        for anim in self._animations:
            anim.stop()
        self._animations.clear()
        self._groups.clear()

    @property
    def active_count(self) -> int:
        """正在运行的动画数量。"""
        return sum(1 for a in self._animations if a.state() == a.State.Running)

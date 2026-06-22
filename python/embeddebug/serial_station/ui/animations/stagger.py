"""错峰入场编排器（StaggerCoordinator）。

把列表/卡片依次入场的散落 QTimer.singleShot 集中到可复用工厂，对标
Linear / Vercel 列表项 stagger 入场。用 AnimationController 统一持有所有
stagger 动画 + pending timers，cancel() 一次性停掉，安全配合页面切换。

约束：只依赖 PyQt6 + 标准库 + 本仓动画基础设施。
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.animations.controller import AnimationController
from embeddebug.serial_station.ui.animations.fade import FadeTransition
from embeddebug.serial_station.ui.animations.reduced_motion import ReducedMotionState
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.panel_animations import slide_in


_STAGGER_SLIDE_PX = 12


@dataclass
class StaggerHandle:
    """一次 stagger 入场的句柄（持有动画 + pending timers + 完成回调）。"""

    controller: AnimationController
    timers: list = field(default_factory=list)
    remaining: int = 0
    on_all_finished: object = None

    def cancel(self):
        for timer in self.timers:
            try:
                timer.stop()
            except Exception:
                pass
        self.timers.clear()
        self.controller.stop_all()
        self.remaining = 0

    def _mark_one_finished(self):
        if self.remaining > 0:
            self.remaining -= 1
        if self.remaining == 0 and self.on_all_finished is not None:
            try:
                self.on_all_finished()
            except Exception:
                pass


class StaggerCoordinator:
    """错峰入场编排工厂（无实例状态，全部静态方法）。"""

    @staticmethod
    def stagger_in(items, step_ms=AnimationTokens.STAGGER_STEP_MS,
                   slide_px=_STAGGER_SLIDE_PX, parent=None,
                   on_all_finished=None):
        state = ReducedMotionState.current()
        effective_step = state.scaled_stagger_step(step_ms)
        effective_slide = 0 if state.should_skip_path_motion() else slide_px

        handle = StaggerHandle(
            controller=AnimationController(parent),
            remaining=len(items),
            on_all_finished=on_all_finished,
        )

        if not items:
            handle.remaining = 0
            if on_all_finished is not None:
                try:
                    on_all_finished()
                except Exception:
                    pass
            return handle

        for index, widget in enumerate(items):
            delay_ms = index * effective_step

            def _start(w=widget):
                StaggerCoordinator._animate_one(handle, w, effective_slide)

            if delay_ms <= 0:
                _start()
            else:
                timer = QTimer(parent)
                timer.setSingleShot(True)
                timer.setInterval(delay_ms)
                timer.timeout.connect(_start)
                handle.timers.append(timer)
                timer.start()

        return handle

    @staticmethod
    def _animate_one(handle, widget, slide_px):
        try:
            fade = FadeTransition.fade_in(widget)
            fade.finished.connect(handle._mark_one_finished)
            handle.controller.add(fade)
            fade.start()
            if slide_px > 0:
                slide_anim = slide_in(widget)
                slide_anim.setDuration(AnimationTokens.DURATION_NORMAL)
                handle.controller.add(slide_anim)
                slide_anim.start()
        except RuntimeError:
            handle._mark_one_finished()


def stagger_fade_safe(items, step_ms=AnimationTokens.STAGGER_STEP_MS, parent=None):
    """便捷封装：纯淡入错峰（无上滑），对动态布局最安全。"""

    return StaggerCoordinator.stagger_in(items, step_ms=step_ms, slide_px=0, parent=parent)

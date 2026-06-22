"""Reduced-motion 全局开关（可访问性 / 前庭敏感用户）。

对标 CSS prefers-reduced-motion 与 macOS「减少动态效果」系统设置：
当用户开启 reduced motion 后，全应用动画应降级为瞬时或极短（opacity/位置
直接到位，无缓动回弹、无弹跳、无路径位移），避免给前庭敏感用户带来眩晕感。

接入方式（统一真相源）：
1. ReducedMotionState 是单例状态机（current() 拿到唯一实例）。
2. UI 在「设置页 / 命令面板 / 快捷键」调用 set_enabled(True/False) 切换。
3. 动画工厂在读 AnimationTokens 时长/曲线之前统一过一道
   ReducedMotionState.apply(duration, easing)。
4. enabled_changed 信号供 AppShell / StatusBar / 命令面板刷新 UI 文案。

约束：只依赖 PyQt6 + 标准库；不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtCore import QEasingCurve, QObject, pyqtSignal


_REDUCED_DURATION_MS = 100
_REDUCED_EASING = QEasingCurve.Type.Linear


class ReducedMotionState(QObject):
    """全局 reduced motion 偏好状态机（单例）。"""

    _instance = None

    enabled_changed = pyqtSignal(bool)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._enabled = False

    @classmethod
    def current(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def reset_for_tests(cls):
        prev = cls._instance
        if prev is not None:
            try:
                prev.deleteLater()
            except Exception:
                pass
        cls._instance = cls()
        return cls._instance

    @property
    def enabled(self):
        return self._enabled

    def set_enabled(self, enabled):
        enabled = bool(enabled)
        if enabled == self._enabled:
            return
        self._enabled = enabled
        self.enabled_changed.emit(enabled)

    def toggle(self):
        self.set_enabled(not self._enabled)
        return self._enabled

    def apply_duration(self, duration_ms):
        if not self._enabled:
            return int(duration_ms)
        return min(int(duration_ms), _REDUCED_DURATION_MS)

    def apply_easing(self, easing):
        if not self._enabled:
            return easing
        return _REDUCED_EASING

    def should_skip_path_motion(self):
        return self._enabled

    def should_skip_stagger(self):
        return self._enabled

    def scaled_stagger_step(self, base_step_ms):
        if not self._enabled:
            return int(base_step_ms)
        return 0


def reduced_motion_enabled():
    return ReducedMotionState.current().enabled


def apply_reduced_duration(duration_ms):
    return ReducedMotionState.current().apply_duration(duration_ms)


def apply_reduced_easing(easing):
    return ReducedMotionState.current().apply_easing(easing)

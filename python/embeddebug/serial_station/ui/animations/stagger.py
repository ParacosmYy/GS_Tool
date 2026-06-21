"""错峰协调器：将 N 个动画以固定间隔依次启动，实现级联入场（Linear/Vercel 卡片瀑布）。

用途：仪表盘上 6 个卡片面板需要逐个淡入/弹入，而非同时闪现——
``StaggerCoordinator`` 以 60ms 步长把每张卡片的入场动画错开，形成「瀑布」视觉。
也对应 VOFA+ 的 widget canvas reveal：每条通道曲线依次铺开，而非一次性铺满，
让用户视觉先抓住第 1 张卡片再扩散到其余，降低首次进入仪表盘的认知负荷。

与 ``AnimationController`` 的差异（互补不替代）：
- ``AnimationController`` 负责**并行/串行组合**多个动画的生命周期管理
  （``play_sequential`` / ``play_parallel`` / ``stop_all``）。
- ``StaggerCoordinator`` 负责**时序错峰**：注册 N 个工厂，按 ``step_ms`` 延迟
  依次启动，并在最后一个动画 ``finished`` 时统一发出 ``finished`` 信号。

GC 安全范式（与 ``BouncePathAnimation`` 一致）：
- 类级 ``_active`` 列表持有进行中的协调器引用，防止 Python 侧 GC 导致 QTimer
  与动画被提前回收。
- ``_track`` 在 ``start()`` 时注册协调器；``finished`` / ``cancel()`` 后由
  ``_discard`` 自动移除，调用方无需手动持有引用。

用法示例::

    coord = StaggerCoordinator(step_ms=60)
    for card in cards:
        coord.add(lambda c=card: FadeTransition.fade_in(c))
    coord.finished.connect(on_all_done)
    coord.start()  # 自动防 GC，60ms 步长依次淡入

注意：调用方应在闭包参数中预绑定 widget（``lambda c=card:``），避免循环变量
延迟绑定导致所有动画作用在最后一个卡片上。
"""

from __future__ import annotations

from typing import Callable, List, Tuple

from PyQt6.QtCore import QAbstractAnimation, QObject, QTimer, pyqtSignal


class StaggerCoordinator(QObject):
    """错峰动画协调器：以固定步长依次启动 N 个动画工厂。

    - ``add(factory)`` 仅注册，不立即调用工厂。
    - ``start()`` 按 ``i * step_ms`` 间隔为每个工厂调度 QTimer，到时调用工厂、
      持有返回的动画并立即 ``.start()``。
    - 最后一个动画（即最晚启动）的 ``finished`` 触发本协调器的 ``finished``。
    - ``cancel()`` 停止全部在途定时器与已启动动画，发出 ``cancelled``。

    与 ``AnimationController`` 互补：前者管「时序错峰」，后者管「组合生命周期」。
    """

    # 持有进行中的协调器引用，防止 Python 侧 GC（完成或取消后自动从列表移除）。
    _active: List["StaggerCoordinator"] = []

    # 全部错峰动画完成时发出（由最后一个动画的 finished 触发）。
    finished = pyqtSignal()
    # 被 cancel() 取消时发出。
    cancelled = pyqtSignal()

    def __init__(self, step_ms: int = 60, parent: QObject | None = None) -> None:
        """初始化协调器。

        Args:
            step_ms: 每个动画相对前一个的启动延迟（毫秒）。60ms 是 Linear 风格
                默认——足够明显让用户感知到「瀑布」节奏，又不至于拖沓。
            parent: 可选 Qt 父对象，用于父子 ownership。若提供，父对象会持有
                本协调器；否则依赖类级 ``_active`` 防 GC。
        """
        super().__init__(parent)
        self._step_ms: int = step_ms
        # 工厂列表：(factory, started_flag)。flag 标记该工厂是否已被调用过，
        # 便于 cancel 时区分「未启动的 pending」与「已启动需停止」。
        self._factories: List[Tuple[Callable[[], QAbstractAnimation], bool]] = []
        # 在途 QTimer 引用（cancel 时需要 stop）。
        self._timers: List[QTimer] = []
        # 已启动的动画引用（cancel 时需要 stop，亦防 GC）。
        self._anims: List[QAbstractAnimation] = []
        self._started: bool = False

    # ── GC 防护（与 BouncePathAnimation 同范式）─────────────────────────
    @classmethod
    def _track(cls, coord: "StaggerCoordinator") -> "StaggerCoordinator":
        """注册协调器到活跃列表，完成或取消后由 ``_discard`` 自动移除。

        Args:
            coord: 待防护的 StaggerCoordinator。

        Returns:
            传入的 coord（链式调用约定）。
        """
        cls._active.append(coord)
        return coord

    @classmethod
    def _discard(cls, coord: "StaggerCoordinator") -> None:
        """从活跃列表移除协调器（防内存泄漏，幂等）。

        Args:
            coord: 已完成或已取消的 StaggerCoordinator。
        """
        try:
            cls._active.remove(coord)
        except ValueError:
            # 协调器可能已被显式清理，幂等忽略。
            pass

    # ── 注册 ──────────────────────────────────────────────────────────
    def add(self, factory: Callable[[], QAbstractAnimation]) -> None:
        """注册一个动画工厂。

        工厂在 ``start()`` 时按错峰间隔被调用，返回 ``QAbstractAnimation`` 并立即
        ``.start()``。调用方应在闭包中预绑定 widget / property / duration，
        协调器本身不关心动画具体操作哪个目标。

        Args:
            factory: 无参 callable，返回**未启动**的 ``QAbstractAnimation``。
                协调器会在适当时机调用并 start。
        """
        self._factories.append((factory, False))

    # ── 启动 ──────────────────────────────────────────────────────────
    def start(self) -> None:
        """按 ``step_ms`` 间隔依次启动所有注册的动画。

        第 i 个动画在 ``i * step_ms`` 毫秒后由 QTimer 触发工厂调用并 start。
        最后一个动画的 ``finished`` 连接到 ``_on_all_finished``（绑定方法，非闭包），
        用于发出本协调器的 ``finished`` 信号并从 ``_active`` 移除。

        幂等：重复调用 ``start()`` 不会再次调度（``_started`` 守卫）。
        """
        if self._started:
            return
        self._started = True
        StaggerCoordinator._track(self)

        last_index = len(self._factories) - 1
        for i in range(len(self._factories)):
            timer = QTimer(self)
            timer.setSingleShot(True)
            timer.timeout.connect(self._make_runner(i, i == last_index))
            self._timers.append(timer)
            timer.start(i * self._step_ms)

    def _make_runner(self, index: int, is_last: bool) -> Callable[[], None]:
        """构造 QTimer.timeout 回调：调用工厂、持有动画、start、必要时连 finished。

        Args:
            index: 工厂在 ``_factories`` 中的下标。
            is_last: 是否为最后一个动画——若是，连接其 finished 到 _on_all_finished。

        Returns:
            无参 callable，由 QTimer.timeout 在适当时机触发。
        """

        def _run() -> None:
            factory, _ = self._factories[index]
            anim = factory()
            # 标记该工厂已启动（便于 cancel 区分 pending vs running）。
            self._factories[index] = (factory, True)
            self._anims.append(anim)
            if is_last:
                anim.finished.connect(self._on_all_finished)
            anim.start()

        return _run

    def _on_all_finished(self) -> None:
        """最后一个动画完成时：发出 finished 并从 _active 移除（绑定方法）。"""
        self.finished.emit()
        StaggerCoordinator._discard(self)

    # ── 取消 ──────────────────────────────────────────────────────────
    def cancel(self) -> None:
        """停止全部在途定时器与已启动动画，发出 cancelled 并从 _active 移除。

        - 对每个 QTimer 调用 ``stop()``：未触发的工厂将不再被调用。
        - 对每个已启动且仍在 Running 状态的动画调用 ``stop()``。
        - 清空 ``_timers`` / ``_anims`` 引用，便于 Python GC。
        - 从 ``_active`` 移除本协调器。

        幂等：多次调用安全（``_discard`` 与 ``stop`` 均幂等）。
        """
        for t in self._timers:
            t.stop()
        for a in self._anims:
            if a.state() == QAbstractAnimation.State.Running:
                a.stop()
        self._timers.clear()
        self._anims.clear()
        StaggerCoordinator._discard(self)
        self.cancelled.emit()

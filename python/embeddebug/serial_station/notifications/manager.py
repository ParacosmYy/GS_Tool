"""通知管理器：队列调度、自动清除与历史归档。"""

from __future__ import annotations

import itertools

from PyQt6.QtCore import QObject, QTimer, pyqtSignal

from embeddebug.serial_station.notifications.data import NotificationData, NotificationLevel


class NotificationManager(QObject):
    """非模态通知（toast）调度中心。"""

    notification_added = pyqtSignal(object)
    notification_removed = pyqtSignal(object)

    def __init__(self, max_visible: int = 5, max_history: int = 100, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._max_visible = max(1, int(max_visible))
        self._max_history = max(1, int(max_history))
        self._queue: list[NotificationData] = []
        self._history: list[NotificationData] = []
        self._timers: dict[int, QTimer] = {}
        self._uid_seq = itertools.count(1)

    @property
    def max_visible(self) -> int:
        return self._max_visible

    def queue(self) -> list[NotificationData]:
        return list(self._queue)

    def history(self) -> list[NotificationData]:
        return list(self._history)

    def has_pending(self) -> bool:
        return bool(self._queue)

    def show(self, level: NotificationLevel, title: str, message: str, timeout_ms: int = 3000) -> NotificationData:
        """入队一条通知并启动自动清除定时器。"""
        data = NotificationData.create(level, title, message, timeout_ms)
        data.uid = next(self._uid_seq)
        self._queue.append(data)
        self._archive(data)
        while len(self._queue) > self._max_visible:
            self._evict_oldest()
        self.notification_added.emit(data)
        if timeout_ms > 0:
            self._start_timer(data)
        return data

    def dismiss(self, uid: int) -> bool:
        data = next((d for d in self._queue if d.uid == uid), None)
        if data is None:
            return False
        self._remove(data)
        return True

    def clear(self) -> None:
        for data in list(self._queue):
            self._remove(data)

    def clear_history(self) -> None:
        self._history.clear()

    def _evict_oldest(self) -> None:
        target = next((d for d in self._queue if d.timeout_ms > 0), self._queue[0] if self._queue else None)
        if target is not None:
            self._remove(target)

    def _remove(self, data: NotificationData) -> None:
        if data not in self._queue:
            return
        self._queue.remove(data)
        timer = self._timers.pop(data.uid, None)
        if timer is not None:
            timer.stop()
            timer.deleteLater()
        self.notification_removed.emit(data)

    def _start_timer(self, data: NotificationData) -> None:
        timer = QTimer(self)
        timer.setSingleShot(True)
        timer.setInterval(max(1, data.timeout_ms))
        timer.timeout.connect(lambda d=data: self._on_timeout(d))
        self._timers[data.uid] = timer
        timer.start()

    def _on_timeout(self, data: NotificationData) -> None:
        if data in self._queue:
            self._remove(data)

    def _archive(self, data: NotificationData) -> None:
        self._history.append(data)
        overflow = len(self._history) - self._max_history
        if overflow > 0:
            del self._history[:overflow]

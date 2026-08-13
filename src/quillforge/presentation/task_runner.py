"""Qt worker boundary for slow application operations."""

from collections.abc import Callable

from PyQt6.QtCore import QObject, QRunnable, Qt, QThreadPool, pyqtSignal, pyqtSlot


class _TaskSignals(QObject):
    completed = pyqtSignal(object)


class _TaskTerminationError(RuntimeError):
    """Normalize non-Exception worker termination for existing failure ports."""

    def __init__(self, cause: BaseException) -> None:
        self.cause = cause
        detail = str(cause)
        suffix = f": {detail}" if detail else ""
        super().__init__(f"Worker operation terminated with {type(cause).__name__}{suffix}")


class _Task(QRunnable):
    def __init__(
        self,
        operation: Callable[[], object],
        operation_id: int,
        on_success: Callable[[object, int], None],
        on_failure: Callable[[Exception, int], None],
    ) -> None:
        super().__init__()
        # Queued completion signals need the Python task payload to remain
        # alive until the UI thread has consumed it.
        self.setAutoDelete(False)
        self.operation = operation
        self.operation_id = operation_id
        self.on_success = on_success
        self.on_failure = on_failure
        self.result: object | None = None
        self.error: Exception | None = None
        self.signals = _TaskSignals()

    @pyqtSlot()
    def run(self) -> None:
        try:
            self.result = self.operation()
        except Exception as error:
            self.error = error
        except BaseException as error:
            self.error = _TaskTerminationError(error)
        finally:
            self.signals.completed.emit(self)


class TaskRunner(QObject):
    """Submit bounded work and marshal completion callbacks back to the UI owner."""

    pending_changed = pyqtSignal(int)

    def __init__(self, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._pool = QThreadPool(self)
        self._tasks: set[_Task] = set()

    @property
    def pending_count(self) -> int:
        """Return UI-thread-observed tasks retained through queued completion."""
        return len(self._tasks)

    def has_pending_work(self) -> bool:
        """Report whether worker work or queued completion delivery remains."""
        return bool(self._tasks)

    def submit(
        self,
        operation: Callable[[], object],
        operation_id: int,
        on_success: Callable[[object, int], None],
        on_failure: Callable[[Exception, int], None],
    ) -> None:
        """Run one operation and retain its task until the queued result is delivered."""
        task = _Task(operation, operation_id, on_success, on_failure)
        self._tasks.add(task)
        self.pending_changed.emit(len(self._tasks))
        try:
            task.signals.completed.connect(
                self._deliver_completion,
                Qt.ConnectionType.QueuedConnection,
            )
            self._pool.start(task)
        except Exception:
            self._release_task(task)
            raise

    def _release_task(self, task: _Task) -> None:
        """Release one retained task and emit only for an actual state change."""
        if task not in self._tasks:
            return
        self._tasks.remove(task)
        self.pending_changed.emit(len(self._tasks))

    @pyqtSlot(object)
    def _deliver_completion(self, task: _Task) -> None:
        """Deliver one result and release it only after its callback returns."""
        try:
            if task.error is None:
                task.on_success(task.result, task.operation_id)
            else:
                task.on_failure(task.error, task.operation_id)
        finally:
            self._release_task(task)

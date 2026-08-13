"""Qt-free worker boundary contracts shared by presentation coordinators."""

from __future__ import annotations

from collections.abc import Callable
from typing import Protocol


class TaskSubmitter(Protocol):
    """Minimal worker boundary needed by presentation coordinators."""

    def submit(
        self,
        operation: Callable[[], object],
        operation_id: int,
        on_success: Callable[[object, int], None],
        on_failure: Callable[[Exception, int], None],
    ) -> None:
        """Submit work and deliver its result on the owning UI boundary."""

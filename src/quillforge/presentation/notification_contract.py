"""Qt-free notification contracts shared by presentation boundaries."""

from typing import Literal, Protocol

StatusMessageLevel = Literal["info", "success", "warning", "error"]


class NotificationSink(Protocol):
    """Typed notification port without a Qt or widget dependency."""

    def __call__(
        self,
        message: str,
        *,
        level: StatusMessageLevel = "info",
    ) -> None:
        """Project a short user-facing notification."""

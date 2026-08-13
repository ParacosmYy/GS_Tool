"""Application port for a diagnostic, process-isolated plugin host."""

from dataclasses import dataclass
from typing import Literal, Protocol

from .errors import ApplicationValidationError

PluginHostProbeState = Literal[
    "ready",
    "rejected",
    "timeout",
    "crashed",
    "protocol-error",
    "containment-error",
]
PluginHostContainmentState = Literal[
    "attached",
    "attached-after-start",
    "unsupported",
    "failed",
    "not-requested",
]


@dataclass(frozen=True, slots=True)
class PluginHostProbeResult:
    """Typed outcome of one host-process probe."""

    state: PluginHostProbeState
    detail: str
    launcher_pid: int | None = None
    reported_host_pid: int | None = None
    execution_enabled: bool = False
    stderr: str | None = None
    containment_state: PluginHostContainmentState = "not-requested"
    containment_detail: str | None = None
    containment_limits: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        for name in ("launcher_pid", "reported_host_pid"):
            value = getattr(self, name)
            if value is not None and (type(value) is not int or value < 1):
                raise ApplicationValidationError(f"{name} must be a positive integer when present")
        if type(self.execution_enabled) is not bool or self.execution_enabled:
            raise ApplicationValidationError(
                "Diagnostic plugin-host execution must remain disabled"
            )

    def summary(self) -> str:
        """Return a bounded user-facing diagnostic."""
        launcher = f"; launcher PID {self.launcher_pid}" if self.launcher_pid else ""
        reported = f"; reported host PID {self.reported_host_pid}" if self.reported_host_pid else ""
        execution = "enabled" if self.execution_enabled else "disabled"
        containment = f"; containment {self.containment_state}"
        if self.containment_detail:
            containment = f"{containment} ({self.containment_detail})"
        limits = f" [{', '.join(self.containment_limits)}]" if self.containment_limits else ""
        return (
            f"Plugin host {self.state}{launcher}{reported}{containment}{limits}; "
            f"external execution {execution}: {self.detail}"
        )


class PluginHostClient(Protocol):
    """Application-facing seam for a process host implementation."""

    def probe(self) -> PluginHostProbeResult:
        """Run one bounded diagnostic exchange without loading extension code."""

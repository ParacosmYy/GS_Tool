"""Subprocess adapter for the diagnostic plugin-host protocol."""

import os
import subprocess
import sys
from collections.abc import Sequence
from pathlib import Path
from queue import Empty, Queue
from threading import Thread
from typing import BinaryIO

from ..application.plugin_host import PluginHostProbeResult
from ..plugins.host_protocol import (
    PLUGIN_HOST_MAX_FRAME_BYTES,
    PluginHostProtocolError,
    decode_frame,
    encode_frame,
    make_probe_request,
    parse_error,
    parse_hello,
    parse_probe_result,
)
from .process_containment import (
    ContainedProcess,
    ProcessContainment,
    ProcessContainmentLaunchError,
    ProcessContainmentLease,
    default_process_containment,
)

DEFAULT_PLUGIN_HOST_TIMEOUT_SECONDS = 2.0
DEFAULT_PLUGIN_HOST_OUTPUT_BYTES = PLUGIN_HOST_MAX_FRAME_BYTES * 2


class SubprocessPluginHost:
    """Start one exact host command with shell disabled and bounded exchange."""

    def __init__(
        self,
        command: Sequence[str],
        *,
        timeout_seconds: float = DEFAULT_PLUGIN_HOST_TIMEOUT_SECONDS,
        max_output_bytes: int = DEFAULT_PLUGIN_HOST_OUTPUT_BYTES,
        containment: ProcessContainment | None = None,
    ) -> None:
        if not command or any(type(part) is not str or not part for part in command):
            raise ValueError("Plugin host command must contain non-empty strings")
        if timeout_seconds <= 0:
            raise ValueError("Plugin host timeout must be positive")
        if max_output_bytes < PLUGIN_HOST_MAX_FRAME_BYTES:
            raise ValueError("Plugin host output bound is too small")
        self._command = tuple(command)
        self._timeout_seconds = timeout_seconds
        self._max_output_bytes = max_output_bytes
        self._containment = (
            containment if containment is not None else default_process_containment()
        )

    def probe(self) -> PluginHostProbeResult:
        """Run one hello/probe exchange and map every child failure explicitly."""
        command = (*self._command, "--probe")
        creation_flags = getattr(subprocess, "CREATE_NO_WINDOW", 0) if os.name == "nt" else 0
        try:
            process, lease = self._start_process(command, creation_flags)
        except ProcessContainmentLaunchError as error:
            return PluginHostProbeResult(
                "containment-error",
                f"Containment adapter failed: {error.detail}",
                containment_state="failed",
                containment_detail=error.detail,
                containment_limits=error.limits,
            )
        except OSError as error:
            return PluginHostProbeResult("crashed", f"Host could not start: {error}")
        parent_pid = os.getpid()
        containment = lease.status
        if containment.state == "failed":
            _terminate_process(process)
            lease.close()
            return PluginHostProbeResult(
                "containment-error",
                "Host was not started because containment could not be attached",
                launcher_pid=process.pid,
                containment_state="failed",
                containment_detail=containment.detail,
                containment_limits=containment.limits,
            )

        try:
            try:
                stdout, stderr, timed_out = self._communicate_bounded(
                    process,
                    encode_frame(make_probe_request()),
                )
            except (OSError, ValueError) as error:
                return PluginHostProbeResult(
                    "crashed",
                    f"Host communication failed: {error}",
                    launcher_pid=process.pid,
                    containment_state=containment.state,
                    containment_detail=containment.detail,
                    containment_limits=containment.limits,
                )
            if timed_out:
                return PluginHostProbeResult(
                    "timeout",
                    f"Host exceeded {self._timeout_seconds:.3f}s",
                    launcher_pid=process.pid,
                    stderr=_bounded_text(stderr, self._max_output_bytes),
                    containment_state=containment.state,
                    containment_detail=containment.detail,
                    containment_limits=containment.limits,
                )

            if len(stdout) > self._max_output_bytes or len(stderr) > self._max_output_bytes:
                return PluginHostProbeResult(
                    "protocol-error",
                    f"Host output exceeds {self._max_output_bytes} bytes",
                    launcher_pid=process.pid,
                    stderr=_bounded_text(stderr, self._max_output_bytes),
                    containment_state=containment.state,
                    containment_detail=containment.detail,
                    containment_limits=containment.limits,
                )
            stderr_text = _bounded_text(stderr, self._max_output_bytes)
            if process.returncode != 0:
                detail = f"Host exited with code {process.returncode}"
                if stderr_text:
                    detail = f"{detail}: {stderr_text}"
                return PluginHostProbeResult(
                    "crashed",
                    detail,
                    launcher_pid=process.pid,
                    stderr=stderr_text,
                    containment_state=containment.state,
                    containment_detail=containment.detail,
                    containment_limits=containment.limits,
                )

            reported_host_pid: int | None = None
            try:
                frames = stdout.splitlines(keepends=True)
                if len(frames) != 2:
                    raise PluginHostProtocolError(
                        "Host response must contain hello and result frames"
                    )
                hello = parse_hello(decode_frame(frames[0]))
                if hello.host_pid == parent_pid:
                    raise PluginHostProtocolError(
                        "Host hello PID must differ from the main process"
                    )
                reported_host_pid = hello.host_pid
                second = decode_frame(frames[1])
                if second.get("kind") == "error":
                    detail = parse_error(second)
                    return PluginHostProbeResult(
                        "rejected",
                        detail,
                        launcher_pid=process.pid,
                        reported_host_pid=hello.host_pid,
                        execution_enabled=hello.execution_enabled,
                        stderr=stderr_text,
                        containment_state=containment.state,
                        containment_detail=containment.detail,
                        containment_limits=containment.limits,
                    )
                reply = parse_probe_result(second)
                if reply.host_pid != hello.host_pid:
                    raise PluginHostProtocolError("Host result PID differs from the hello PID")
            except (PluginHostProtocolError, UnicodeError) as error:
                return PluginHostProbeResult(
                    "protocol-error",
                    str(error),
                    launcher_pid=process.pid,
                    reported_host_pid=reported_host_pid,
                    stderr=stderr_text,
                    containment_state=containment.state,
                    containment_detail=containment.detail,
                    containment_limits=containment.limits,
                )
            return PluginHostProbeResult(
                reply.state if reply.state in {"ready", "rejected"} else "protocol-error",
                "Diagnostic handshake completed"
                if reply.state == "ready"
                else "Host rejected probe",
                launcher_pid=process.pid,
                reported_host_pid=reported_host_pid,
                execution_enabled=reply.execution_enabled,
                stderr=stderr_text,
                containment_state=containment.state,
                containment_detail=containment.detail,
                containment_limits=containment.limits,
            )
        finally:
            lease.close()
            _close_process(process)

    def _start_process(
        self,
        command: tuple[str, ...],
        creation_flags: int,
    ) -> tuple[ContainedProcess, ProcessContainmentLease]:
        launcher = getattr(self._containment, "launch", None)
        if callable(launcher):
            return launcher(command, creation_flags=creation_flags)
        try:
            process = subprocess.Popen(
                command,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=False,
                creationflags=creation_flags,
            )
        except OSError:
            raise
        try:
            lease = self._containment.attach(process)
        except (OSError, RuntimeError, ValueError) as error:
            _terminate_process(process)
            raise ProcessContainmentLaunchError(str(error), ()) from error
        return process, lease

    def _communicate_bounded(
        self,
        process: ContainedProcess,
        request: bytes,
    ) -> tuple[bytes, bytes, bool]:
        """Exchange one request while reader threads cap retained pipe bytes."""
        stdout_queue: Queue[bytes] = Queue(maxsize=1)
        stderr_queue: Queue[bytes] = Queue(maxsize=1)
        stdout_thread = Thread(
            target=_read_bounded,
            args=(process.stdout, self._max_output_bytes, stdout_queue),
            daemon=True,
        )
        stderr_thread = Thread(
            target=_read_bounded,
            args=(process.stderr, self._max_output_bytes, stderr_queue),
            daemon=True,
        )
        stdout_thread.start()
        stderr_thread.start()
        if process.stdin is not None:
            try:
                process.stdin.write(request)
                process.stdin.flush()
            except (BrokenPipeError, OSError, ValueError):
                pass
            finally:
                try:
                    process.stdin.close()
                except (OSError, ValueError):
                    pass
        timed_out = False
        communication_error: OSError | ValueError | None = None
        try:
            process.wait(timeout=self._timeout_seconds)
        except subprocess.TimeoutExpired:
            timed_out = True
            try:
                process.kill()
            except (OSError, ValueError):
                pass
            try:
                process.wait(timeout=1.0)
            except (OSError, ValueError, subprocess.TimeoutExpired):
                pass
        except (OSError, ValueError) as error:
            communication_error = error
            try:
                process.kill()
            except (OSError, ValueError):
                pass
            try:
                process.wait(timeout=1.0)
            except (OSError, ValueError, subprocess.TimeoutExpired):
                pass
        finally:
            if timed_out or communication_error is not None:
                for stream in (process.stdout, process.stderr):
                    if stream is not None:
                        try:
                            stream.close()
                        except (OSError, ValueError):
                            pass
            stdout_thread.join(timeout=1.0)
            stderr_thread.join(timeout=1.0)
        if communication_error is not None:
            raise communication_error
        return (
            _queue_value(stdout_queue),
            _queue_value(stderr_queue),
            timed_out,
        )


def default_plugin_host_command() -> tuple[str, ...]:
    """Build a fixed source or frozen command without a shell or user path."""
    executable = str(Path(sys.executable).resolve())
    if getattr(sys, "frozen", False):
        return executable, "--plugin-host"
    return executable, "-m", "quillforge", "--plugin-host"


def _bounded_text(payload: bytes, maximum: int) -> str | None:
    if not payload:
        return None
    return payload[:maximum].decode("utf-8", errors="replace")


def _read_bounded(
    stream: BinaryIO | None,
    maximum: int,
    target: Queue[bytes],
) -> None:
    """Read at most maximum+1 bytes and always publish one result."""
    if stream is None:
        target.put(b"")
        return
    try:
        payload = stream.read(maximum + 1)
    except (OSError, ValueError):
        payload = b""
    target.put(payload)


def _queue_value(target: Queue[bytes]) -> bytes:
    try:
        return target.get_nowait()
    except Empty:
        return b""


def _terminate_process(process: ContainedProcess) -> None:
    """Best-effort cleanup when containment cannot be established."""
    try:
        process.kill()
    except OSError:
        pass
    try:
        process.wait(timeout=1.0)
    except (OSError, subprocess.TimeoutExpired):
        pass
    for stream in (process.stdin, process.stdout, process.stderr):
        if stream is not None:
            try:
                stream.close()
            except (OSError, ValueError):
                pass


def _close_process(process: ContainedProcess) -> None:
    """Release process-specific native handles after the exchange is done."""
    closer = getattr(process, "close", None)
    if callable(closer):
        closer()
        return
    for stream in (process.stdin, process.stdout, process.stderr):
        if stream is not None:
            try:
                stream.close()
            except (OSError, ValueError):
                pass

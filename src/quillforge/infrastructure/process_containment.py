"""OS adapters for bounded child-process lifecycle and resource containment."""

from __future__ import annotations

import ctypes
import ctypes.wintypes as wintypes
import os
import subprocess
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from typing import BinaryIO, Literal, Protocol

ProcessContainmentState = Literal[
    "attached",
    "attached-after-start",
    "unsupported",
    "failed",
]

# One slot is reserved for a launcher/re-exec wrapper such as the uv-managed
# Windows Python trampoline; the second slot is the actual diagnostic host.
DEFAULT_PLUGIN_HOST_MAX_ACTIVE_PROCESSES = 2
DEFAULT_PLUGIN_HOST_PROCESS_MEMORY_BYTES = 256 * 1024 * 1024
_CREATE_SUSPENDED = 0x00000004


@dataclass(frozen=True, slots=True)
class ProcessContainmentStatus:
    """Immutable diagnostic state returned by a containment lease."""

    state: ProcessContainmentState
    detail: str
    limits: tuple[str, ...] = ()


class ProcessContainmentLease(Protocol):
    """Lifecycle handle held until the child-process exchange has ended."""

    @property
    def status(self) -> ProcessContainmentStatus:
        """Return the attachment state and configured limit labels."""

    def close(self) -> None:
        """Release the containment handle exactly once."""


class ContainedProcess(Protocol):
    """Minimal process surface shared by ``Popen`` and the Win32 launcher."""

    pid: int
    returncode: int | None
    stdin: BinaryIO | None
    stdout: BinaryIO | None
    stderr: BinaryIO | None

    def poll(self) -> int | None:
        """Return the exit code without blocking."""

    def wait(self, timeout: float | None = None) -> int:
        """Wait for process completion."""

    def kill(self) -> None:
        """Terminate the process."""


class ProcessContainment(Protocol):
    """Infrastructure seam for attaching a process to a containment policy."""

    def attach(self, process: ContainedProcess) -> ProcessContainmentLease:
        """Attach a child process and return its lifecycle lease."""


class ProcessContainmentLauncher(Protocol):
    """Optional creation-time launch seam implemented by the default adapters."""

    def launch(
        self,
        command: Sequence[str],
        *,
        creation_flags: int = 0,
    ) -> tuple[ContainedProcess, ProcessContainmentLease]:
        """Create a child and return it with its containment lease."""


class ProcessContainmentLaunchError(RuntimeError):
    """Raised when a child cannot be created with the requested containment."""

    def __init__(self, detail: str, limits: tuple[str, ...]) -> None:
        super().__init__(detail)
        self.detail = detail
        self.limits = limits


class UnsupportedProcessContainment:
    """Explicit fallback for platforms without the Windows Job Object adapter."""

    def __init__(self, detail: str = "Windows Job Object containment is unsupported") -> None:
        self._status = ProcessContainmentStatus("unsupported", detail)

    def attach(self, process: ContainedProcess) -> ProcessContainmentLease:
        del process
        return _StaticContainmentLease(self._status)

    def launch(
        self,
        command: Sequence[str],
        *,
        creation_flags: int = 0,
    ) -> tuple[ContainedProcess, ProcessContainmentLease]:
        process = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=False,
            creationflags=creation_flags,
        )
        return process, _StaticContainmentLease(self._status)


class WindowsJobObjectContainment:
    """Apply a bounded Windows Job Object policy to one child process."""

    def __init__(
        self,
        *,
        max_active_processes: int = DEFAULT_PLUGIN_HOST_MAX_ACTIVE_PROCESSES,
        process_memory_bytes: int = DEFAULT_PLUGIN_HOST_PROCESS_MEMORY_BYTES,
        api_factory: Callable[[], _Win32JobApi] | None = None,
    ) -> None:
        if max_active_processes < 1:
            raise ValueError("Job Object active-process limit must be positive")
        if process_memory_bytes < 1:
            raise ValueError("Job Object process-memory limit must be positive")
        self._max_active_processes = max_active_processes
        self._process_memory_bytes = process_memory_bytes
        self._api = (api_factory or _Win32JobApi)()

    def attach(self, process: ContainedProcess) -> ProcessContainmentLease:
        limits = self._limits()
        job_handle: int | None = None
        try:
            job_handle = self._api.create_job()
            self._api.configure_job(
                job_handle,
                max_active_processes=self._max_active_processes,
                process_memory_bytes=self._process_memory_bytes,
            )
            self._assign_existing_process(job_handle, process)
        except OSError as error:
            if job_handle is not None:
                self._api.close_handle(job_handle)
            return _StaticContainmentLease(ProcessContainmentStatus("failed", str(error), limits))
        assert job_handle is not None
        return _WindowsJobObjectLease(
            self._api,
            job_handle,
            ProcessContainmentStatus(
                "attached-after-start",
                "Windows Job Object attached after child start",
                limits,
            ),
        )

    def launch(
        self,
        command: Sequence[str],
        *,
        creation_flags: int = 0,
    ) -> tuple[ContainedProcess, ProcessContainmentLease]:
        """Bind the child before resuming its suspended initial thread."""
        limits = self._limits()
        job_handle: int | None = None
        process: subprocess.Popen[bytes] | None = None
        try:
            job_handle = self._api.create_job()
            self._api.configure_job(
                job_handle,
                max_active_processes=self._max_active_processes,
                process_memory_bytes=self._process_memory_bytes,
            )
            process = subprocess.Popen(
                command,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=False,
                creationflags=creation_flags | _CREATE_SUSPENDED,
            )
            self._assign_existing_process(job_handle, process)
            self._api.resume_process(process.pid)
        except (OSError, RuntimeError, ValueError) as error:
            if process is not None:
                _terminate_suspended_process(process)
            if job_handle is not None:
                self._api.close_handle(job_handle)
            raise ProcessContainmentLaunchError(str(error), limits) from error
        assert job_handle is not None
        lease = _WindowsJobObjectLease(
            self._api,
            job_handle,
            ProcessContainmentStatus(
                "attached",
                "Windows Job Object attached before child resume",
                limits,
            ),
        )
        assert process is not None
        return process, lease

    def _assign_existing_process(
        self,
        job_handle: int,
        process: ContainedProcess,
    ) -> None:
        process_handle = self._api.open_process(process.pid)
        try:
            self._api.assign_process(job_handle, process_handle)
        finally:
            self._api.close_handle(process_handle)

    def _limits(self) -> tuple[str, ...]:
        return (
            "kill-on-close",
            f"active-processes<={self._max_active_processes}",
            f"process-commit-memory<={self._process_memory_bytes}B",
        )


def default_process_containment() -> ProcessContainment:
    """Select the platform adapter without importing Windows APIs on other OSes."""
    if os.name == "nt":
        return WindowsJobObjectContainment()
    return UnsupportedProcessContainment()


class _StaticContainmentLease:
    def __init__(self, status: ProcessContainmentStatus) -> None:
        self._status = status

    @property
    def status(self) -> ProcessContainmentStatus:
        return self._status

    def close(self) -> None:
        return None


class _WindowsJobObjectLease:
    def __init__(
        self,
        api: _Win32JobApi,
        job_handle: int,
        status: ProcessContainmentStatus,
    ) -> None:
        self._api = api
        self._job_handle: int | None = job_handle
        self._status = status

    @property
    def status(self) -> ProcessContainmentStatus:
        return self._status

    def close(self) -> None:
        job_handle = self._job_handle
        self._job_handle = None
        if job_handle is not None:
            self._api.close_handle(job_handle)


class _Win32JobApi:
    """Small, typed ctypes wrapper for the documented Job Object calls."""

    def __init__(self) -> None:
        if os.name != "nt":
            raise OSError("Windows Job Object API is unavailable on this platform")
        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        self._create_job = kernel32.CreateJobObjectW
        self._create_job.argtypes = [wintypes.LPVOID, wintypes.LPCWSTR]
        self._create_job.restype = wintypes.HANDLE
        self._set_information = kernel32.SetInformationJobObject
        self._set_information.argtypes = [
            wintypes.HANDLE,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
        ]
        self._set_information.restype = wintypes.BOOL
        self._open_process = kernel32.OpenProcess
        self._open_process.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
        self._open_process.restype = wintypes.HANDLE
        self._assign_process = kernel32.AssignProcessToJobObject
        self._assign_process.argtypes = [wintypes.HANDLE, wintypes.HANDLE]
        self._assign_process.restype = wintypes.BOOL
        self._close_handle = kernel32.CloseHandle
        self._close_handle.argtypes = [wintypes.HANDLE]
        self._close_handle.restype = wintypes.BOOL
        self._create_snapshot = kernel32.CreateToolhelp32Snapshot
        self._create_snapshot.argtypes = [wintypes.DWORD, wintypes.DWORD]
        self._create_snapshot.restype = wintypes.HANDLE
        self._thread_first = kernel32.Thread32First
        self._thread_first.argtypes = [
            wintypes.HANDLE,
            ctypes.POINTER(_ThreadEntry32),
        ]
        self._thread_first.restype = wintypes.BOOL
        self._thread_next = kernel32.Thread32Next
        self._thread_next.argtypes = [
            wintypes.HANDLE,
            ctypes.POINTER(_ThreadEntry32),
        ]
        self._thread_next.restype = wintypes.BOOL
        self._open_thread = kernel32.OpenThread
        self._open_thread.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
        self._open_thread.restype = wintypes.HANDLE
        self._resume_thread = kernel32.ResumeThread
        self._resume_thread.argtypes = [wintypes.HANDLE]
        self._resume_thread.restype = wintypes.DWORD

    def create_job(self) -> int:
        handle = self._create_job(None, None)
        if not handle:
            raise _last_error("CreateJobObjectW")
        return int(handle.value)

    def configure_job(
        self,
        job_handle: int,
        *,
        max_active_processes: int,
        process_memory_bytes: int,
    ) -> None:
        information = _JobObjectExtendedLimitInformation()
        information.basic_limit_information.limit_flags = (
            _JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
            | _JOB_OBJECT_LIMIT_ACTIVE_PROCESS
            | _JOB_OBJECT_LIMIT_PROCESS_MEMORY
        )
        information.basic_limit_information.active_process_limit = max_active_processes
        information.process_memory_limit = process_memory_bytes
        if not self._set_information(
            job_handle,
            _JOB_OBJECT_EXTENDED_LIMIT_INFORMATION,
            ctypes.byref(information),
            ctypes.sizeof(information),
        ):
            raise _last_error("SetInformationJobObject")

    def open_process(self, pid: int) -> int:
        handle = self._open_process(
            _PROCESS_SET_QUOTA | _PROCESS_TERMINATE,
            False,
            pid,
        )
        if not handle:
            raise _last_error("OpenProcess")
        return int(handle.value)

    def assign_process(self, job_handle: int, process_handle: int) -> None:
        if not self._assign_process(job_handle, process_handle):
            raise _last_error("AssignProcessToJobObject")

    def close_handle(self, handle: int) -> None:
        self._close_handle(handle)

    def resume_process(self, pid: int) -> None:
        """Resume the one initial thread created by CREATE_SUSPENDED."""
        snapshot = self._create_snapshot(_TH32CS_SNAPTHREAD, 0)
        snapshot_value = ctypes.cast(snapshot, ctypes.c_void_p).value
        if snapshot_value in (None, _INVALID_HANDLE_VALUE):
            raise _last_error("CreateToolhelp32Snapshot")
        snapshot_handle = int(snapshot_value)
        entry = _ThreadEntry32()
        entry.size = ctypes.sizeof(_ThreadEntry32)
        thread_ids: list[int] = []
        try:
            has_thread = self._thread_first(snapshot_handle, ctypes.byref(entry))
            while has_thread:
                if entry.owner_process_id == pid:
                    thread_ids.append(int(entry.thread_id))
                has_thread = self._thread_next(snapshot_handle, ctypes.byref(entry))
        finally:
            self._close_handle(snapshot_handle)
        if len(thread_ids) != 1:
            raise OSError(f"Expected one initial thread for process {pid}, found {len(thread_ids)}")
        thread_handle = self._open_thread(_THREAD_SUSPEND_RESUME, False, thread_ids[0])
        if not thread_handle:
            raise _last_error("OpenThread")
        try:
            previous_count = self._resume_thread(thread_handle)
            if previous_count == 1:
                return
            if previous_count == _RESUME_THREAD_FAILED:
                raise _last_error("ResumeThread")
            raise OSError(f"Unexpected initial thread suspend count: {previous_count}")
        finally:
            self._close_handle(thread_handle)


class _JobObjectBasicLimitInformation(ctypes.Structure):
    _fields_ = [
        ("per_process_user_time_limit", ctypes.c_longlong),
        ("per_job_user_time_limit", ctypes.c_longlong),
        ("limit_flags", wintypes.DWORD),
        ("minimum_working_set_size", ctypes.c_size_t),
        ("maximum_working_set_size", ctypes.c_size_t),
        ("active_process_limit", wintypes.DWORD),
        ("affinity", ctypes.c_size_t),
        ("priority_class", wintypes.DWORD),
        ("scheduling_class", wintypes.DWORD),
    ]


class _IoCounters(ctypes.Structure):
    _fields_ = [
        ("read_operations", ctypes.c_ulonglong),
        ("write_operations", ctypes.c_ulonglong),
        ("other_operations", ctypes.c_ulonglong),
        ("read_bytes", ctypes.c_ulonglong),
        ("write_bytes", ctypes.c_ulonglong),
        ("other_bytes", ctypes.c_ulonglong),
    ]


class _JobObjectExtendedLimitInformation(ctypes.Structure):
    _fields_ = [
        ("basic_limit_information", _JobObjectBasicLimitInformation),
        ("io_info", _IoCounters),
        ("process_memory_limit", ctypes.c_size_t),
        ("job_memory_limit", ctypes.c_size_t),
        ("peak_process_memory_used", ctypes.c_size_t),
        ("peak_job_memory_used", ctypes.c_size_t),
    ]


class _ThreadEntry32(ctypes.Structure):
    _fields_ = [
        ("size", wintypes.DWORD),
        ("usage", wintypes.DWORD),
        ("thread_id", wintypes.DWORD),
        ("owner_process_id", wintypes.DWORD),
        ("base_priority", ctypes.c_long),
        ("delta_priority", ctypes.c_long),
        ("flags", wintypes.DWORD),
    ]


def _terminate_suspended_process(process: subprocess.Popen[bytes]) -> None:
    """Best-effort cleanup before a suspended child ever reaches user code."""
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


def _last_error(operation: str) -> OSError:
    error_code = ctypes.get_last_error()
    message = ctypes.FormatError(error_code).strip()[:256]
    return OSError(error_code, f"{operation} failed: {message}")


_JOB_OBJECT_EXTENDED_LIMIT_INFORMATION = 9
_JOB_OBJECT_LIMIT_ACTIVE_PROCESS = 0x00000008
_JOB_OBJECT_LIMIT_PROCESS_MEMORY = 0x00000100
_JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000
_PROCESS_TERMINATE = 0x0001
_PROCESS_SET_QUOTA = 0x0100
_TH32CS_SNAPTHREAD = 0x00000004
_THREAD_SUSPEND_RESUME = 0x0002
_RESUME_THREAD_FAILED = 0xFFFFFFFF
_INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value

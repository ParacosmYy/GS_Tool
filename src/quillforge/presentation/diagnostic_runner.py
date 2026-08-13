"""Explicit packaged-path diagnostics for recovery capture."""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
import platform
import sys
import tempfile
import time
from collections.abc import Sequence
from ctypes import wintypes
from datetime import UTC, datetime
from pathlib import Path
from threading import Thread
from typing import Any

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication

from ..application.recovery import RecoveryService
from ..domain.models import DocumentState
from ..infrastructure.file_store import FileDocumentStore
from ..infrastructure.recovery_channel import BoundedRecoveryChunkChannel
from ..infrastructure.recovery_store import JsonRecoverySnapshotStore
from .editor_widget import EditorWidget


class _ProcessMemoryCountersEx(ctypes.Structure):
    """Windows process counters used only for diagnostic observability."""

    _fields_ = [
        ("cb", ctypes.c_ulong),
        ("page_fault_count", ctypes.c_ulong),
        ("peak_working_set_size", ctypes.c_size_t),
        ("working_set_size", ctypes.c_size_t),
        ("quota_peak_paged_pool_usage", ctypes.c_size_t),
        ("quota_paged_pool_usage", ctypes.c_size_t),
        ("quota_peak_non_paged_pool_usage", ctypes.c_size_t),
        ("quota_non_paged_pool_usage", ctypes.c_size_t),
        ("pagefile_usage", ctypes.c_size_t),
        ("peak_pagefile_usage", ctypes.c_size_t),
        ("private_usage", ctypes.c_size_t),
    ]


_memory_kernel32 = None
_memory_get_info = None
if os.name == "nt":
    _memory_kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    _memory_kernel32.GetCurrentProcess.restype = wintypes.HANDLE
    _memory_get_info = ctypes.WinDLL("psapi", use_last_error=True).GetProcessMemoryInfo
    _memory_get_info.argtypes = [
        wintypes.HANDLE,
        ctypes.POINTER(_ProcessMemoryCountersEx),
        wintypes.DWORD,
    ]
    _memory_get_info.restype = wintypes.BOOL


def _process_memory() -> dict[str, int] | None:
    """Read current Windows process counters without a runtime dependency."""
    if _memory_kernel32 is None or _memory_get_info is None:
        return None
    counters = _ProcessMemoryCountersEx()
    counters.cb = ctypes.sizeof(counters)
    success = _memory_get_info(
        _memory_kernel32.GetCurrentProcess(),
        ctypes.byref(counters),
        counters.cb,
    )
    if not success:
        return None
    return {
        "working_set_bytes": counters.working_set_size,
        "peak_working_set_bytes": counters.peak_working_set_size,
        "private_bytes": counters.private_usage,
        "peak_pagefile_bytes": counters.peak_pagefile_usage,
    }


def _synthetic_text(input_bytes: int) -> str:
    """Create exact-size, UTF-8-safe text with realistic long lines."""
    if type(input_bytes) is not int or input_bytes < 1:
        raise ValueError("input_bytes must be a positive integer")
    line = ("x" * 1_048_576) + "\n"
    full_lines, remainder = divmod(input_bytes, len(line))
    text = line * full_lines
    if remainder:
        text += ("x" * (remainder - 1)) + "\n"
    return text


def _write_json_atomically(path: Path, payload: dict[str, Any]) -> None:
    """Persist a diagnostic report through an atomic same-directory replace."""
    path = path.resolve()
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as temporary:
            temporary_path = Path(temporary.name)
            json.dump(payload, temporary, ensure_ascii=False, indent=2)
            temporary.write("\n")
            temporary.flush()
            os.fsync(temporary.fileno())
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Run the explicit QuillForge packaged recovery-capture diagnostic."
    )
    parser.add_argument(
        "--diagnose-capture",
        action="store_true",
        required=True,
        help="Enable the non-default capture diagnostic path.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="JSON report path written atomically by the diagnostic.",
    )
    parser.add_argument(
        "--input-bytes",
        type=int,
        default=16 * 1_048_577,
        help="Exact UTF-8 workload size; defaults to about 16 MiB.",
    )
    return parser


def _max_memory(samples: list[dict[str, int]]) -> dict[str, int] | None:
    if not samples:
        return None
    keys = (
        "working_set_bytes",
        "peak_working_set_bytes",
        "private_bytes",
        "peak_pagefile_bytes",
    )
    return {key: max(sample[key] for sample in samples) for key in keys}


def _diagnostic_artifact_identity() -> dict[str, object] | None:
    """Bind a frozen capture report to the executable that produced it."""
    if not getattr(sys, "frozen", False):
        return None
    executable = Path(sys.executable).resolve()
    try:
        digest = hashlib.sha256()
        with executable.open("rb") as source:
            for chunk in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(chunk)
        return {
            "path": str(executable),
            "bytes": executable.stat().st_size,
            "sha256": digest.hexdigest().upper(),
        }
    except OSError as error:
        return {"path": str(executable), "error": str(error)}


def run_capture_diagnostic(argv: Sequence[str]) -> int:
    """Run the production capture session through a real Qt event loop."""
    arguments = _parser().parse_args(list(argv))
    output_path = arguments.output
    execution = "packaged-exe" if getattr(sys, "frozen", False) else "source-python"
    artifact = _diagnostic_artifact_identity()
    worker_thread: Thread | None = None
    channel: BoundedRecoveryChunkChannel | None = None
    temporary_directory: tempfile.TemporaryDirectory[str] | None = None
    primary_failure: BaseException | None = None
    try:
        text = _synthetic_text(arguments.input_bytes)
        expected_hash = hashlib.sha256(text.encode("utf-8")).hexdigest()
        output_path = output_path.resolve()
        output_path.parent.mkdir(parents=True, exist_ok=True)
        application = QApplication([])
        application.setApplicationName("QuillForge capture diagnostic")
        temporary_directory = tempfile.TemporaryDirectory(
            dir=output_path.parent,
            prefix=".quillforge-capture-",
        )
        recovery_root = Path(temporary_directory.name) / "recovery"
        recovery_store = JsonRecoverySnapshotStore(recovery_root)
        recovery = RecoveryService(
            recovery_store,
            FileDocumentStore(keep_backups=False),
            chunk_store=recovery_store,
        )
        recovery_state = DocumentState(document_id="packaged-diagnostic", dirty=True)
        snapshot_id = "packaged-diagnostic"
        channel = BoundedRecoveryChunkChannel(max_chunks=64, max_bytes=1_048_576)
        worker_errors: list[Exception] = []

        def consume_recovery() -> None:
            try:
                recovery.save_snapshot_chunks(
                    recovery_state,
                    channel.consume(),
                    snapshot_id=snapshot_id,
                )
            except Exception as error:  # pragma: no cover - diagnostic boundary
                worker_errors.append(error)

        worker_thread = Thread(target=consume_recovery, name="quillforge-diagnostic-worker")
        worker_thread.start()
        editor = EditorWidget()
        editor.set_text(text)
        session = editor.begin_text_capture(sink=channel.offer)
        heartbeat_gaps: list[float] = []
        memory_samples: list[dict[str, int]] = []
        last_heartbeat = [time.perf_counter()]
        final_progress = [session.progress()]
        slices = [0]
        failure: list[Exception] = []
        worker_finished = [False]

        def heartbeat_tick() -> None:
            now = time.perf_counter()
            heartbeat_gaps.append((now - last_heartbeat[0]) * 1000)
            last_heartbeat[0] = now
            memory = _process_memory()
            if memory is not None:
                memory_samples.append(memory)

        def capture_slice() -> None:
            try:
                if worker_errors:
                    raise worker_errors[0]
                previous_progress = session.progress()
                final_progress[0] = session.step(
                    budget_ms=8,
                    max_chunks=512,
                    characters_per_chunk=16_384,
                )
                slices[0] += 1
            except Exception as error:  # pragma: no cover - diagnostic boundary
                failure.append(error)
                channel.abort(error)
                session.cancel()
                QTimer.singleShot(0, wait_for_worker)
                return
            progress = final_progress[0]
            if session.is_terminal:
                channel.finish()
                QTimer.singleShot(0, wait_for_worker)
                return
            delay_ms = 1 if progress.captured_bytes == previous_progress.captured_bytes else 0
            QTimer.singleShot(delay_ms, capture_slice)

        def wait_for_worker() -> None:
            if worker_thread is not None and worker_thread.is_alive():
                QTimer.singleShot(1, wait_for_worker)
                return
            worker_finished[0] = True
            application.quit()

        def diagnostic_timeout() -> None:
            if worker_thread is not None and worker_thread.is_alive():
                timeout_error = TimeoutError("packaged capture diagnostic timed out")
                failure.append(timeout_error)
                channel.abort(timeout_error)
                session.cancel()
                application.quit()

        heartbeat_timer = QTimer()
        heartbeat_timer.setInterval(1)
        heartbeat_timer.timeout.connect(heartbeat_tick)
        memory = _process_memory()
        if memory is not None:
            memory_samples.append(memory)
        heartbeat_timer.start()
        started = time.perf_counter()
        QTimer.singleShot(0, capture_slice)
        QTimer.singleShot(120_000, diagnostic_timeout)
        application.exec()
        elapsed_ms = (time.perf_counter() - started) * 1000
        heartbeat_timer.stop()
        if worker_thread is not None:
            worker_thread.join(timeout=2)
        if worker_errors:
            failure.extend(worker_errors)

        progress = final_progress[0]
        snapshots = recovery_store.list_snapshots()
        snapshot = next((item for item in snapshots if item.snapshot_id == snapshot_id), None)
        round_trip = (
            not failure
            and progress.phase == "completed"
            and progress.captured_bytes == progress.total_bytes
            and snapshot is not None
            and snapshot.text == text
        )
        status = "completed" if round_trip and worker_finished[0] else "failed"
        report: dict[str, Any] = {
            "schema_version": "1.0",
            "diagnostic": "packaged-capture",
            "execution": execution,
            "status": status,
            "artifact": artifact,
            "created_at_utc": datetime.now(UTC).isoformat(),
            "workload": {
                "requested_input_bytes": arguments.input_bytes,
                "actual_input_bytes": len(text.encode("utf-8")),
                "input_sha256": expected_hash,
            },
            "capture": {
                "phase": progress.phase,
                "captured_bytes": progress.captured_bytes,
                "total_bytes": progress.total_bytes,
                "chunks": progress.chunks,
                "slices": slices[0],
                "round_trip": round_trip,
            },
            "handoff": {
                "phase": "committed" if round_trip else "failed",
                "round_trip": round_trip,
                "captured_bytes": progress.captured_bytes,
                "expected_bytes": len(text.encode("utf-8")),
                "queue_peak_chunks": channel.peak_queued_chunks,
                "queue_peak_bytes": channel.peak_queued_bytes,
                "queue_max_chunks": 64,
                "queue_max_bytes": 1_048_576,
                "channel_state": channel.state,
                "worker_terminal": worker_finished[0] and not worker_thread.is_alive(),
                "final_snapshot_files": len(snapshots),
                "temporary_files": len(list(recovery_root.glob("*.tmp"))),
            },
            "event_loop": {
                "elapsed_ms": round(elapsed_ms, 3),
                "heartbeat_samples": len(heartbeat_gaps),
                "heartbeat_max_gap_ms": round(max(heartbeat_gaps, default=0.0), 3),
            },
            "process": {
                "platform": platform.platform(),
                "memory_samples": len(memory_samples),
                "peak_memory": _max_memory(memory_samples),
            },
            "limits": [
                f"This is a {execution} path with the caller-selected Qt platform.",
                "It does not establish clean-machine or interactive non-offscreen behavior.",
                "It does not establish arbitrary larger-input, disk-full, power-loss, or "
                "hard memory-limit behavior.",
            ],
        }
        if failure:
            report["error"] = {
                "type": type(failure[0]).__name__,
                "message": str(failure[0]),
            }
        _write_json_atomically(output_path, report)
        return 0 if status == "completed" else 1
    except BaseException as error:  # pragma: no cover - diagnostic boundary
        primary_failure = error
        _write_json_atomically(
            output_path,
            {
                "schema_version": "1.0",
                "diagnostic": "packaged-capture",
                "execution": execution,
                "status": "failed",
                "artifact": artifact,
                "created_at_utc": datetime.now(UTC).isoformat(),
                "error": {"type": type(error).__name__, "message": str(error)},
            },
        )
        return 1
    finally:
        cleanup_failure: BaseException | None = None
        if worker_thread is not None and worker_thread.is_alive():
            try:
                if channel is not None:
                    channel.abort(RuntimeError("capture diagnostic cleanup requested"))
                worker_thread.join(timeout=2)
                if worker_thread.is_alive():
                    raise RuntimeError("capture diagnostic worker did not stop during cleanup")
            except BaseException as error:
                cleanup_failure = error
        if worker_thread is None or not worker_thread.is_alive():
            if temporary_directory is not None:
                try:
                    temporary_directory.cleanup()
                except BaseException as error:
                    if cleanup_failure is None:
                        cleanup_failure = error
        if primary_failure is None and cleanup_failure is not None:
            raise cleanup_failure


__all__ = ["run_capture_diagnostic"]

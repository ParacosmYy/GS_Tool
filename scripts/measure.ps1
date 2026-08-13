$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$previousLocation = (Get-Location).Path

$measureDirectory = Join-Path ([System.IO.Path]::GetTempPath()) (
    "QuillForge-measure-" + [guid]::NewGuid().ToString("N")
)
$previousMeasureDirectory = $env:QUILLFORGE_MEASURE_DIR
$previousQtPlatform = $env:QT_QPA_PLATFORM
$primaryFailure = $null

try {
    Set-Location -LiteralPath $root
    New-Item -ItemType Directory -Path $measureDirectory | Out-Null
    $env:QUILLFORGE_MEASURE_DIR = $measureDirectory
    $env:QT_QPA_PLATFORM = "offscreen"
    @'
import json
import hashlib
import ctypes
import os
import platform
import sys
import threading
import time
import tracemalloc
from ctypes import wintypes
from pathlib import Path

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication

from quillforge.application.commands import CommandRegistry
from quillforge.application.documents import DocumentService, OpenedDocument
from quillforge.application.events import EventBus
from quillforge.application.ports import RecoveryChannelAborted
from quillforge.application.recovery import RecoveryService
from quillforge.domain.models import DocumentState
from quillforge.infrastructure.file_store import FileDocumentStore
from quillforge.infrastructure.recovery_channel import BoundedRecoveryChunkChannel
from quillforge.infrastructure.recovery_store import JsonRecoverySnapshotStore
from quillforge.presentation.editor_widget import EditorWidget
from quillforge.presentation.main_window import MainWindow


def measured(operation):
    tracemalloc.start()
    started = time.perf_counter()
    result = operation()
    elapsed_ms = (time.perf_counter() - started) * 1000
    _current, peak_bytes = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    return result, {"elapsed_ms": round(elapsed_ms, 3), "python_peak_bytes": peak_bytes}


class _ProcessMemoryCountersEx(ctypes.Structure):
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


def process_memory():
    """Return Windows process memory counters without adding a runtime dependency."""
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


measure_root = Path(os.environ["QUILLFORGE_MEASURE_DIR"])
artifact_path = Path.cwd() / "QuillForge.exe"
store = FileDocumentStore(keep_backups=False)
small_text = "needle\n" * 150_000
long_line_text = ("x" * 1_048_576) + "\nneedle\n"
small_path = measure_root / "small.txt"
long_path = measure_root / "long-line.txt"
small_path.write_text(small_text, encoding="utf-8")
long_path.write_text(long_line_text, encoding="utf-8")

loaded_small, load_small = measured(lambda: store.load(small_path))
_loaded_long, load_long = measured(lambda: store.load(long_path))
documents = DocumentService(store)
save_state = DocumentState(
    document_id="measurement-document",
    path=small_path,
    encoding=loaded_small.encoding,
    line_ending=loaded_small.line_ending,
    dirty=True,
    revision=loaded_small.revision,
)
_saved, save_small = measured(
    lambda: documents.save_document(save_state, small_text, measure_root / "saved.txt")
)

application = QApplication.instance() or QApplication([])
search_editor = EditorWidget()
search_editor.set_text(small_text)
_found, search_latency = measured(
    lambda: search_editor.find_literal("needle", case_sensitive=True, forward=True)
)
replace_editor = EditorWidget()
replace_editor.set_text("needle\n" * 1_000)


def replace_cooperatively():
    session = replace_editor.begin_replace_all_literal(
        "needle",
        "replacement",
        case_sensitive=True,
        max_matches=10_000,
    )
    slices = 0
    progress = session.step(budget_ms=8, max_items=256)
    slices += 1
    while not session.is_terminal:
        progress = session.step(budget_ms=8, max_items=256)
        slices += 1
    return {"phase": progress.phase, "count": progress.count, "slices": slices}


replace_result, replace_latency = measured(replace_cooperatively)


def run_bounded_channel_probe():
    """Verify non-blocking capacity, finish drain, and abort wake-up semantics."""
    channel = BoundedRecoveryChunkChannel(max_chunks=2, max_bytes=32)
    first = "a" * 16
    second = "b" * 16
    accepted_first = channel.offer(first)
    accepted_second = channel.offer(second)
    rejected_full = not channel.offer("c")
    queued_before_consume = {
        "chunks": channel.queued_chunks,
        "bytes": channel.queued_bytes,
    }
    consumed = []
    consumer = threading.Thread(target=lambda: consumed.extend(channel.consume()))
    consumer.start()
    channel.finish()
    consumer.join(timeout=2)

    aborted = BoundedRecoveryChunkChannel(max_chunks=1, max_bytes=16)
    abort_errors = []

    def consume_aborted():
        try:
            tuple(aborted.consume())
        except RecoveryChannelAborted as error:
            abort_errors.append(error)

    abort_consumer = threading.Thread(target=consume_aborted)
    abort_consumer.start()
    abort_requested = aborted.abort(RuntimeError("measurement stale"))
    abort_consumer.join(timeout=2)
    return {
        "accepted_first": accepted_first,
        "accepted_second": accepted_second,
        "rejected_when_full": rejected_full,
        "queued_before_consume": queued_before_consume,
        "finish_state": channel.state,
        "consumer_stopped": not consumer.is_alive(),
        "round_trip": consumed == [first, second],
        "abort_requested": abort_requested,
        "abort_state": aborted.state,
        "abort_consumer_stopped": not abort_consumer.is_alive(),
        "abort_error_type": type(abort_errors[0]).__name__ if abort_errors else None,
    }


bounded_channel_result, bounded_channel_latency = measured(run_bounded_channel_probe)


def run_editor_safety_probes():
    """Record cancellation, limit safety, and stale-capture outcomes."""
    cancellation_text = "needle\n" * 5_000
    cancellation_editor = EditorWidget()
    cancellation_editor.set_text(cancellation_text)
    cancellation_session = cancellation_editor.begin_replace_all_literal(
        "needle",
        "replacement",
        case_sensitive=True,
        max_matches=10_000,
    )
    cancellation_started = False
    for _ in range(1_000):
        cancellation_session.step(budget_ms=1, max_items=32)
        if cancellation_editor.get_text() != cancellation_text:
            cancellation_started = True
            break
        if cancellation_session.is_terminal:
            break
    cancellation_before = cancellation_session.progress()
    cancellation_result = (
        cancellation_session.cancel()
        if not cancellation_session.is_terminal
        else cancellation_before
    )

    limit_text = "needle\n" * 11
    limit_editor = EditorWidget()
    limit_editor.set_text(limit_text)
    limit_session = limit_editor.begin_replace_all_literal(
        "needle",
        "replacement",
        case_sensitive=True,
        max_matches=10,
    )
    limit_progress = limit_session.step(budget_ms=8, max_items=256)
    while not limit_session.is_terminal:
        limit_progress = limit_session.step(budget_ms=8, max_items=256)

    stale_text = ("x" * 1_048_576 + "\n") * 16
    stale_root = measure_root / "stale-recovery"
    stale_store = JsonRecoverySnapshotStore(stale_root)
    stale_recovery = RecoveryService(stale_store, store, chunk_store=stale_store)
    stale_documents = DocumentService(store)
    stale_opened = stale_documents.new_document()
    stale_state = stale_documents.mark_dirty(stale_opened.state, True)
    stale_window = MainWindow(
        stale_documents,
        CommandRegistry(),
        EventBus(),
        recovery=stale_recovery,
        recovery_interval_ms=120_000,
        recovery_channel_factory=BoundedRecoveryChunkChannel,
    )
    stale_tab = stale_window._add_tab(OpenedDocument(stale_state, stale_text))
    stale_tab.editor.set_modified(True)
    application.processEvents()
    stale_window._autosave_recovery()
    stale_job = stale_window._recovery_capture_jobs.get(stale_state.document_id)
    if stale_job is None:
        raise RuntimeError("stale-capture probe did not start a recovery capture")
    stale_window._continue_recovery_capture(stale_state.document_id)
    stale_partial = stale_job.session.progress()
    stale_version_before = stale_tab.content_version
    stale_tab.editor.set_text("newer content")
    application.processEvents()
    stale_version_after = stale_tab.content_version
    stale_window._continue_recovery_capture(stale_state.document_id)
    stale_after = stale_job.session.progress()
    for _ in range(400):
        application.processEvents()
        if not stale_window._recovery_inflight_snapshots:
            break
        time.sleep(0.005)
    if stale_window._recovery_inflight_snapshots:
        raise RuntimeError("stale-capture worker did not reach a terminal state")
    stale_result = {
        "capture_started": True,
        "partial_before_edit": (
            stale_partial.captured_bytes > 0
            and stale_partial.captured_bytes < stale_partial.total_bytes
        ),
        "content_version_changed": stale_version_after > stale_version_before,
        "job_discarded": stale_state.document_id not in stale_window._recovery_capture_jobs,
        "session_phase": stale_after.phase,
        "inflight_cleared": stale_state.document_id not in stale_window._recovery_inflight,
        "worker_terminal": not stale_window._recovery_inflight_snapshots,
        "snapshot_files": len(list(stale_root.glob("*.qfrecovery"))),
        "temporary_files": len(list(stale_root.glob("*.tmp"))),
    }
    stale_window._recovery_timer.stop()
    stale_window.deleteLater()
    application.processEvents()

    return {
        "cancellation": {
            "phase_before_cancel": cancellation_before.phase,
            "phase_after_cancel": cancellation_result.phase,
            "mutated_before_cancel": cancellation_started,
            "rollback_succeeded": cancellation_session.rollback_succeeded,
            "text_restored": cancellation_editor.get_text() == cancellation_text,
        },
        "limit": {
            "phase": limit_progress.phase,
            "count": limit_progress.count,
            "limit": limit_progress.limit,
            "text_unchanged": limit_editor.get_text() == limit_text,
        },
        "stale_capture": stale_result,
    }


editor_safety_result, editor_safety_latency = measured(run_editor_safety_probes)


def run_bounded_recovery_handoff():
    """Run one production bounded capture through the worker and atomic store."""
    handoff_text = "recovery\n" * 250_000
    handoff_root = measure_root / "bounded-handoff"
    handoff_store = JsonRecoverySnapshotStore(handoff_root)
    handoff_recovery = RecoveryService(handoff_store, store, chunk_store=handoff_store)
    handoff_documents = DocumentService(store)
    handoff_opened = handoff_documents.new_document()
    handoff_state = handoff_documents.mark_dirty(handoff_opened.state, True)
    channels = []

    def channel_factory(max_chunks, max_bytes):
        channel = BoundedRecoveryChunkChannel(max_chunks, max_bytes)
        channels.append(channel)
        return channel

    handoff_window = MainWindow(
        handoff_documents,
        CommandRegistry(),
        EventBus(),
        recovery=handoff_recovery,
        recovery_interval_ms=120_000,
        recovery_channel_factory=channel_factory,
    )
    handoff_tab = handoff_window._add_tab(OpenedDocument(handoff_state, handoff_text))
    handoff_tab.editor.set_modified(True)
    application.processEvents()
    handoff_window._autosave_recovery()
    for _ in range(1_000):
        application.processEvents()
        if (
            not handoff_window._recovery_capture_jobs
            and not handoff_window._recovery_inflight
            and not handoff_window._recovery_inflight_snapshots
        ):
            break
        time.sleep(0.005)
    if handoff_window._recovery_inflight_snapshots:
        raise RuntimeError("bounded recovery handoff did not reach a terminal state")
    snapshots = handoff_store.list_snapshots()
    channel = channels[0] if channels else None
    if channel is None or len(snapshots) != 1:
        raise RuntimeError("bounded recovery handoff did not commit exactly one snapshot")
    snapshot = snapshots[0]
    result = {
        "phase": "committed",
        "round_trip": snapshot.text == handoff_text,
        "captured_bytes": len(snapshot.text.encode("utf-8")),
        "expected_bytes": len(handoff_text.encode("utf-8")),
        "queue_peak_chunks": channel.peak_queued_chunks,
        "queue_peak_bytes": channel.peak_queued_bytes,
        "queue_max_chunks": 64,
        "queue_max_bytes": 1_048_576,
        "channel_state": channel.state,
        "worker_terminal": not handoff_window._recovery_inflight_snapshots,
        "final_snapshot_files": len(list(handoff_root.glob("*.qfrecovery"))),
        "temporary_files": len(list(handoff_root.glob("*.tmp"))),
    }
    handoff_window._recovery_timer.stop()
    handoff_window.deleteLater()
    application.processEvents()
    return result


bounded_handoff_result, bounded_handoff_latency = measured(run_bounded_recovery_handoff)

capture_editor = EditorWidget()
capture_editor.set_text(small_text)
capture_session = capture_editor.begin_text_capture()


def capture_recovery_cooperatively():
    slices = 0
    progress = capture_session.step(
        budget_ms=8,
        max_chunks=512,
        characters_per_chunk=16_384,
    )
    slices += 1
    while not capture_session.is_terminal:
        progress = capture_session.step(
            budget_ms=8,
            max_chunks=512,
            characters_per_chunk=16_384,
        )
        slices += 1
    chunks = capture_session.chunks()
    return {
        "phase": progress.phase,
        "captured_bytes": progress.captured_bytes,
        "total_bytes": progress.total_bytes,
        "chunks": len(chunks),
        "slices": slices,
    }


capture_result, capture_latency = measured(capture_recovery_cooperatively)


def _line_ending_counts(text):
    crlf = text.count("\r\n")
    without_crlf = text.replace("\r\n", "")
    return {
        "CRLF": crlf,
        "LF": without_crlf.count("\n"),
        "CR": without_crlf.count("\r"),
    }


def run_capture_workload_matrix():
    """Compare capture behavior across size, encoding, EOL, and long-line shapes."""
    workloads = (
        ("ascii-lf-64k", "alpha\n" * 10_000, "utf-8", "LF"),
        ("ascii-crlf-256k", "beta\r\n" * 30_000, "utf-8", "CRLF"),
        ("unicode-lf-1m", "中文🙂\n" * 100_000, "utf-8", "LF"),
        ("long-line-lf-1m", "x" * 1_048_576 + "\n", "utf-8", "LF"),
    )
    results = []
    for name, source_text, encoding, requested_line_ending in workloads:
        editor = EditorWidget()
        editor.set_text(source_text)
        actual_text = editor.get_text()
        session = editor.begin_text_capture()
        slices = 0
        progress = session.step(
            budget_ms=8,
            max_chunks=512,
            characters_per_chunk=16_384,
        )
        slices += 1
        while not session.is_terminal:
            progress = session.step(
                budget_ms=8,
                max_chunks=512,
                characters_per_chunk=16_384,
            )
            slices += 1
        chunks = session.chunks()
        reconstructed = "".join(chunks)
        results.append(
            {
                "name": name,
                "encoding": encoding,
                "requested_line_ending": requested_line_ending,
                "actual_line_endings": _line_ending_counts(actual_text),
                "input_bytes": len(actual_text.encode("utf-8")),
                "input_characters": len(actual_text),
                "captured_bytes": progress.captured_bytes,
                "chunks": len(chunks),
                "slices": slices,
                "round_trip": reconstructed == actual_text,
            }
        )
        editor.deleteLater()
    return results


capture_matrix_result, capture_matrix_latency = measured(run_capture_workload_matrix)


def run_capture_event_loop_probe():
    """Run the real Qt timer handoff while sampling heartbeat gaps and process memory."""
    probe_text = ("x" * 1_048_576 + "\n") * 16
    probe_editor = EditorWidget()
    probe_editor.set_text(probe_text)
    probe_session = probe_editor.begin_text_capture()
    heartbeat_gaps = []
    memory_samples = []
    last_heartbeat = [time.perf_counter()]
    completed = [False]

    heartbeat = QTimer()

    def heartbeat_tick():
        now = time.perf_counter()
        heartbeat_gaps.append((now - last_heartbeat[0]) * 1000)
        last_heartbeat[0] = now
        memory = process_memory()
        if memory is not None:
            memory_samples.append(memory)

    heartbeat.timeout.connect(heartbeat_tick)

    def continue_capture():
        progress = probe_session.step(
            budget_ms=8,
            max_chunks=512,
            characters_per_chunk=16_384,
        )
        memory = process_memory()
        if memory is not None:
            memory_samples.append(memory)
        if progress.phase == "completed":
            completed[0] = True
            heartbeat.stop()
            application.quit()
            return
        QTimer.singleShot(0, continue_capture)

    heartbeat.start(1)
    QTimer.singleShot(0, continue_capture)
    QTimer.singleShot(120_000, application.quit)
    application.exec()
    if not completed[0]:
        raise RuntimeError("capture event-loop probe timed out")
    return {
        "phase": "completed",
        "input_bytes": len(probe_text.encode("utf-8")),
        "capture_chunks": len(probe_session.chunks()),
        "heartbeat_samples": len(heartbeat_gaps),
        "max_heartbeat_gap_ms": round(max(heartbeat_gaps), 3) if heartbeat_gaps else None,
        "memory_samples": len(memory_samples),
        "memory_peak": (
            {
                "working_set_bytes": max(item["working_set_bytes"] for item in memory_samples),
                "peak_working_set_bytes": max(
                    item["peak_working_set_bytes"] for item in memory_samples
                ),
                "private_bytes": max(item["private_bytes"] for item in memory_samples),
            }
            if memory_samples
            else None
        ),
    }


capture_probe_result, capture_probe_latency = measured(run_capture_event_loop_probe)

capture_chunks = capture_session.chunks()
recovery_store = JsonRecoverySnapshotStore(measure_root / "recovery")
recovery = RecoveryService(recovery_store, store, chunk_store=recovery_store)
_snapshot, recovery_timing = measured(
    lambda: recovery.save_snapshot(save_state, small_text, snapshot_id="measurement-snapshot")
)
_chunked_snapshot, chunked_recovery_timing = measured(
    lambda: recovery.save_snapshot_chunks(
        save_state,
        capture_chunks,
        snapshot_id="measurement-chunked-snapshot",
    )
)


def run_recovery_chunk_failure_probe():
    """Verify a mid-stream writer failure preserves the last valid snapshot."""
    failure_root = measure_root / "recovery-failure"
    failure_store = JsonRecoverySnapshotStore(failure_root)
    failure_recovery = RecoveryService(failure_store, store, chunk_store=failure_store)
    snapshot_id = "measurement-failure-preserved"
    failure_recovery.save_snapshot(save_state, "previous valid snapshot", snapshot_id=snapshot_id)

    def failing_chunks():
        yield "replacement chunk"
        raise RuntimeError("intentional measurement failure")

    error_type = None
    try:
        failure_recovery.save_snapshot_chunks(
            save_state,
            failing_chunks(),
            snapshot_id=snapshot_id,
        )
    except RuntimeError as error:
        error_type = type(error).__name__
    snapshots = failure_store.list_snapshots()
    preserved = next((item for item in snapshots if item.snapshot_id == snapshot_id), None)
    return {
        "failure_mode": "injected_mid_stream",
        "permission_or_disk_pressure": False,
        "error_type": error_type,
        "final_snapshot_preserved": preserved is not None and preserved.text == "previous valid snapshot",
        "final_snapshot_files": len(list(failure_root.glob("*.qfrecovery"))),
        "temporary_files": len(list(failure_root.glob("*.tmp"))),
        "source_dirty_after_failure": save_state.dirty,
        "retry_remains_eligible": save_state.dirty and error_type is not None,
    }


failure_probe_result, failure_probe_timing = measured(run_recovery_chunk_failure_probe)

report = {
    "tool": "scripts/measure.ps1",
    "python": sys.version.split()[0],
    "platform": platform.platform(),
    "execution": "source Python path with Qt offscreen; not a packaged-EXE performance run",
    "artifact": None,
    "reference_artifact": (
        {
            "path": str(artifact_path),
            "bytes": artifact_path.stat().st_size,
            "sha256": hashlib.sha256(artifact_path.read_bytes()).hexdigest(),
        }
        if artifact_path.exists()
        else None
    ),
    "scope": {
        "small_file_bytes": small_path.stat().st_size,
        "long_line_file_bytes": long_path.stat().st_size,
        "search_text_bytes": len(small_text.encode("utf-8")),
        "replace_match_count": 1_000,
        "editor_safety_probe_workloads": [
            "cooperative Replace All cancellation after mutation",
            "Replace All match-limit preflight",
            "stale recovery capture after content-version change",
        ],
        "recovery_channel_queue_chunks": 64,
        "recovery_channel_queue_bytes": 1_048_576,
        "recovery_capture_input_lines": 150_000,
        "recovery_capture_character_chunk": 16_384,
        "capture_workload_matrix": [
            "ascii-lf-64k",
            "ascii-crlf-256k",
            "unicode-lf-1m",
            "long-line-lf-1m",
        ],
        "capture_event_loop_probe_bytes": 16 * 1_048_577,
        "recovery_text_bytes": len(small_text.encode("utf-8")),
    },
    "measurements": {
        "file_load_small": load_small,
        "file_load_long_line": load_long,
        "file_save_small": save_small,
        "editor_find_literal": search_latency,
        "editor_replace_all_cooperative": replace_latency | {"result": replace_result},
        "bounded_recovery_channel": bounded_channel_latency | {"result": bounded_channel_result},
        "editor_safety_probes": editor_safety_latency | {"result": editor_safety_result},
        "bounded_recovery_handoff": bounded_handoff_latency | {"result": bounded_handoff_result},
        "editor_recovery_capture_cooperative": capture_latency | {"result": capture_result},
        "editor_recovery_capture_workload_matrix": capture_matrix_latency
        | {"result": capture_matrix_result},
        "editor_recovery_capture_event_loop_probe": capture_probe_latency
        | {"result": capture_probe_result},
        "recovery_snapshot_write": recovery_timing,
        "recovery_snapshot_chunk_write": chunked_recovery_timing,
        "recovery_chunk_write_failure_probe": failure_probe_timing
        | {"result": failure_probe_result},
    },
    "interpretation": {
        "claims": "machine baseline only; not a product support claim",
        "memory": "python_peak_bytes is the tracemalloc peak; memory_peak is a source/offscreen process working-set/private-bytes sample, not a product support limit",
        "unrun": [
            "interactive clean-machine Windows startup",
            "disk-full and permission pressure",
            "power-loss durability",
            "multi-window contention",
            "files larger than the measured inputs",
            "packaged-EXE capture performance",
        ],
    },
}
print(json.dumps(report, ensure_ascii=False, indent=2))
'@ | uv run python -
    if ($LASTEXITCODE -ne 0) {
        $primaryFailure = [System.Exception]::new(
            "uv run python measurement failed with exit code $LASTEXITCODE"
        )
        exit $LASTEXITCODE
    }
}
catch {
    $primaryFailure = $_
    throw
}
finally {
    $cleanupFailure = $null
    try {
        if (Test-Path -LiteralPath $measureDirectory) {
            Remove-Item -LiteralPath $measureDirectory -Recurse -Force
        }
    }
    catch {
        $cleanupFailure = $_
    }
    try {
        if ($null -eq $previousMeasureDirectory) {
            Remove-Item Env:QUILLFORGE_MEASURE_DIR -ErrorAction SilentlyContinue
        } else {
            $env:QUILLFORGE_MEASURE_DIR = $previousMeasureDirectory
        }
    }
    catch {
        if ($null -eq $cleanupFailure) {
            $cleanupFailure = $_
        }
    }
    try {
        if ($null -eq $previousQtPlatform) {
            Remove-Item Env:QT_QPA_PLATFORM -ErrorAction SilentlyContinue
        } else {
            $env:QT_QPA_PLATFORM = $previousQtPlatform
        }
    }
    catch {
        if ($null -eq $cleanupFailure) {
            $cleanupFailure = $_
        }
    }
    try {
        Set-Location -LiteralPath $previousLocation
    }
    catch {
        if ($null -eq $cleanupFailure) {
            $cleanupFailure = $_
        }
    }
    if ($null -eq $primaryFailure -and $null -ne $cleanupFailure) {
        throw $cleanupFailure
    }
}

param(
    [ValidateRange(2, 9)]
    [int]$RepeatCount = 3,
    [string]$OutputPath = "docs\performance\repeat-2026-08-09.json"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $root
$measureScript = Join-Path $PSScriptRoot "measure.ps1"
$reports = @()

for ($run = 1; $run -le $RepeatCount; $run++) {
    $rawReport = & $measureScript 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "Measurement run $run failed with exit code $LASTEXITCODE."
    }
    $reports += (($rawReport -join [Environment]::NewLine) | ConvertFrom-Json)
}

$artifactHashes = @($reports | ForEach-Object { $_.reference_artifact.sha256 } | Select-Object -Unique)
if ($artifactHashes.Count -ne 1) {
    throw "Repeated measurement runs used different artifact hashes."
}

foreach ($report in $reports) {
    $capture = $report.measurements.editor_recovery_capture_cooperative.result
    if ($capture.captured_bytes -ne $capture.total_bytes) {
        throw "A repeated capture run did not capture the full input."
    }
    foreach ($workload in @($report.measurements.editor_recovery_capture_workload_matrix.result)) {
        if ($workload.round_trip -ne $true) {
            throw "Workload round-trip failed in repeated measurement: $($workload.name)"
        }
    }
    if ($report.measurements.editor_recovery_capture_event_loop_probe.result.phase -ne "completed") {
        throw "The event-loop probe did not complete in a repeated measurement."
    }
    $safety = $report.measurements.editor_safety_probes.result
    $channel = $report.measurements.bounded_recovery_channel.result
    if (
        -not $channel.accepted_first -or
        -not $channel.accepted_second -or
        -not $channel.rejected_when_full -or
        $channel.queued_before_consume.chunks -ne 2 -or
        $channel.queued_before_consume.bytes -ne 32 -or
        $channel.finish_state -ne "finished" -or
        -not $channel.consumer_stopped -or
        -not $channel.round_trip -or
        -not $channel.abort_requested -or
        $channel.abort_state -ne "aborted" -or
        -not $channel.abort_consumer_stopped -or
        $channel.abort_error_type -ne "RecoveryChannelAborted"
    ) {
        throw "Bounded recovery channel probe did not preserve capacity, drain, or abort invariants."
    }
    $handoff = $report.measurements.bounded_recovery_handoff.result
    if (
        $handoff.phase -ne "committed" -or
        -not $handoff.round_trip -or
        $handoff.captured_bytes -ne $handoff.expected_bytes -or
        $handoff.queue_peak_chunks -gt $handoff.queue_max_chunks -or
        $handoff.queue_peak_bytes -gt $handoff.queue_max_bytes -or
        $handoff.channel_state -ne "finished" -or
        -not $handoff.worker_terminal -or
        $handoff.final_snapshot_files -ne 1 -or
        $handoff.temporary_files -ne 0
    ) {
        throw "Bounded recovery handoff did not commit a bounded, complete snapshot."
    }
    if (
        $safety.cancellation.phase_after_cancel -ne "cancelled" -or
        -not $safety.cancellation.mutated_before_cancel -or
        -not $safety.cancellation.rollback_succeeded -or
        -not $safety.cancellation.text_restored -or
        $safety.limit.phase -ne "limit_exceeded" -or
        -not $safety.limit.text_unchanged -or
        -not $safety.stale_capture.partial_before_edit -or
        -not $safety.stale_capture.content_version_changed -or
        -not $safety.stale_capture.job_discarded -or
        $safety.stale_capture.session_phase -ne "cancelled" -or
        -not $safety.stale_capture.inflight_cleared -or
        -not $safety.stale_capture.worker_terminal -or
        $safety.stale_capture.snapshot_files -ne 0 -or
        $safety.stale_capture.temporary_files -ne 0
    ) {
        throw "Editor safety probes did not preserve cancellation, limit, or stale-capture invariants."
    }
    $failure = $report.measurements.recovery_chunk_write_failure_probe.result
    if (
        -not $failure.final_snapshot_preserved -or
        $failure.temporary_files -ne 0 -or
        -not $failure.source_dirty_after_failure -or
        -not $failure.retry_remains_eligible
    ) {
        throw "Recovery failure cleanup did not preserve the last valid snapshot."
    }
}

$summary = [ordered]@{
    tool = "scripts/repeat_measure.ps1"
    repeat_count = $RepeatCount
    reference_artifact = $reports[0].reference_artifact
    runs = $reports
    interpretation = [ordered]@{
        claims = "Repeated source/offscreen machine baseline only; not a product support claim."
        unrun = @(
            "packaged-EXE capture performance",
            "clean interactive Windows startup",
            "disk-full and permission pressure",
            "power-loss durability",
            "multi-window contention",
            "files larger than the measured inputs"
        )
    }
}

$outputFile = Join-Path $root $OutputPath
$outputParent = Split-Path -Parent $outputFile
if (-not (Test-Path -LiteralPath $outputParent)) {
    New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
}
$summary | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $outputFile -Encoding utf8
Write-Output "Repeated measurement evidence: $outputFile"
Write-Output "Runs: $RepeatCount; reference artifact SHA-256: $($artifactHashes[0])"

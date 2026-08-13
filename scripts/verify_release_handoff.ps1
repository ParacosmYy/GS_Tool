param(
    [string]$OutputPath = "docs\release\handoff-2026-08-09.json"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $root

function Get-Identity([string]$Path) {
    $item = Get-Item -LiteralPath $Path
    return [ordered]@{
        path = $Path
        bytes = $item.Length
        sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash
    }
}

function ConvertFrom-JsonCompat([string]$Json) {
    if ($PSVersionTable.PSVersion.Major -ge 6) {
        return $Json | ConvertFrom-Json -DateKind String
    }
    return $Json | ConvertFrom-Json
}

$manifestPath = Join-Path $root "dist\QuillForge.release.json"
$rootExePath = Join-Path $root "QuillForge.exe"
$distExePath = Join-Path $root "dist\QuillForge.exe"
$noticePath = Join-Path $root "dist\NOTICE.md"
$releaseHandoffPath = Join-Path $root "docs\RELEASE_HANDOFF.md"
$supportRoutePath = Join-Path $root "docs\support\ISSUES.md"
$supportHandoffPath = Join-Path $root "docs\support\HANDOFF.md"
foreach ($requiredPath in @($manifestPath, $rootExePath, $distExePath, $noticePath, $releaseHandoffPath, $supportRoutePath, $supportHandoffPath)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required release handoff artifact is missing: $requiredPath"
    }
}

$manifest = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath)
$rootIdentity = Get-Identity "QuillForge.exe"
$distIdentity = Get-Identity "dist/QuillForge.exe"
$noticeIdentity = Get-Identity "dist/NOTICE.md"
$releaseHandoffText = Get-Content -Raw -LiteralPath $releaseHandoffPath
$releaseHandoffSizeText = $rootIdentity.bytes.ToString(
    "N0",
    [System.Globalization.CultureInfo]::InvariantCulture
)
$releaseHandoffIdentityMatch = (
    $releaseHandoffText.IndexOf(
        $rootIdentity.sha256,
        [System.StringComparison]::OrdinalIgnoreCase
    ) -ge 0 -and
    $releaseHandoffText.IndexOf($releaseHandoffSizeText, [System.StringComparison]::Ordinal) -ge 0 -and
    $releaseHandoffText.IndexOf($manifest.source_revision, [System.StringComparison]::Ordinal) -ge 0
)
$noticeInventoryExit = 0
& uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice dist/NOTICE.md
$noticeInventoryExit = $LASTEXITCODE
$versionInfo = (Get-Item -LiteralPath $rootExePath).VersionInfo
$expectedVersionResource = "$($manifest.version).0"
$expectedProcessApp = "process:$rootExePath"

$packagedReportPath = Join-Path $root "docs\performance\packaged-capture-2026-08-09.json"
$repeatReportPath = Join-Path $root "docs\performance\repeat-2026-08-09.json"
$interactiveReportPath = Join-Path $root "docs\performance\interactive-startup-2026-08-09.json"
$preflightReportPath = Join-Path $root "docs\performance\clean-machine-preflight-2026-08-09.json"
foreach ($requiredReport in @($packagedReportPath, $repeatReportPath, $interactiveReportPath, $preflightReportPath)) {
    if (-not (Test-Path -LiteralPath $requiredReport)) {
        throw "Required performance evidence is missing: $requiredReport"
    }
}
$packaged = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $packagedReportPath)
$repeat = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $repeatReportPath)
$interactive = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $interactiveReportPath)
$preflight = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $preflightReportPath)

$packagedRuns = @($packaged.runs)
$repeatRuns = @($repeat.runs)
$packagedComplete = @($packagedRuns | Where-Object {
    $_.report.status -eq "completed" -and
    $_.report.capture.captured_bytes -eq $_.report.capture.total_bytes -and
    $_.report.capture.round_trip -eq $true
}).Count
$repeatSafetyComplete = @($repeatRuns | Where-Object {
    $safety = $_.measurements.editor_safety_probes.result
    $failure = $_.measurements.recovery_chunk_write_failure_probe.result
    $safety.cancellation.phase_after_cancel -eq "cancelled" -and
    $safety.cancellation.rollback_succeeded -eq $true -and
    $safety.cancellation.text_restored -eq $true -and
    $safety.limit.phase -eq "limit_exceeded" -and
    $safety.limit.text_unchanged -eq $true -and
    $safety.stale_capture.job_discarded -eq $true -and
    $safety.stale_capture.session_phase -eq "cancelled" -and
    $safety.stale_capture.snapshot_files -eq 0 -and
    $safety.stale_capture.temporary_files -eq 0 -and
    $failure.final_snapshot_preserved -eq $true -and
    $failure.temporary_files -eq 0
}).Count

$checks = [ordered]@{
    root_dist_identity_match = (
        $rootIdentity.bytes -eq $distIdentity.bytes -and
        $rootIdentity.sha256.ToUpperInvariant() -eq $distIdentity.sha256.ToUpperInvariant()
    )
    manifest_artifact_match = (
        $manifest.artifact.bytes -eq $distIdentity.bytes -and
        $manifest.artifact.sha256.ToUpperInvariant() -eq $distIdentity.sha256.ToUpperInvariant()
    )
    manifest_root_copy_match = (
        $manifest.root_test_copy.bytes -eq $rootIdentity.bytes -and
        $manifest.root_test_copy.sha256.ToUpperInvariant() -eq $rootIdentity.sha256.ToUpperInvariant()
    )
    manifest_notice_match = (
        $manifest.notices[0].bytes -eq $noticeIdentity.bytes -and
        $manifest.notices[0].sha256.ToUpperInvariant() -eq $noticeIdentity.sha256.ToUpperInvariant()
    )
    human_handoff_identity_match = $releaseHandoffIdentityMatch
    notice_inventory_complete = ($noticeInventoryExit -eq 0)
    manifest_source_provenance_explicit = (
        $manifest.source_revision -is [string] -and
        $manifest.source_revision.StartsWith("tree-sha256:") -and
        $manifest.source_revision.Length -eq 76
    )
    file_associations_decision_explicit = (
        $null -ne $manifest.file_associations -and
        -not [string]::IsNullOrWhiteSpace($manifest.file_associations.status) -and
        -not [string]::IsNullOrWhiteSpace($manifest.file_associations.decision)
    )
    version_resource_match = (
        $versionInfo.FileVersion -eq $expectedVersionResource -and
        $versionInfo.ProductVersion -eq $expectedVersionResource
    )
    packaged_capture_runs_complete = ($packagedRuns.Count -eq 3 -and $packagedComplete -eq 3)
    packaged_report_artifact_match = (
        $packaged.artifact.bytes -eq $rootIdentity.bytes -and
        $packaged.artifact.sha256.ToUpperInvariant() -eq $rootIdentity.sha256.ToUpperInvariant()
    )
    source_repeat_safety_complete = ($repeatRuns.Count -eq 3 -and $repeatSafetyComplete -eq 3)
    support_owner_assigned = (-not [string]::IsNullOrWhiteSpace($manifest.support.owner))
    support_issue_route_declared = ($manifest.support.issue_route -eq "docs/support/ISSUES.md")
    support_issue_route_exists = (Test-Path -LiteralPath $supportRoutePath)
    support_handoff_packet_exists = (Test-Path -LiteralPath $supportHandoffPath)
    interactive_startup_report_consistent = (
        $interactive.startup.elapsed_seconds -ge $interactive.startup.threshold_seconds -and
        $interactive.startup.process_exact_path_verified -eq $true -and
        $interactive.startup.window.title -eq "QuillForge" -and
        $interactive.startup.window.app -eq $expectedProcessApp -and
        $interactive.artifact.sha256.ToUpperInvariant() -eq $rootIdentity.sha256.ToUpperInvariant() -and
        $interactive.startup.cleanup_exact_path_processes -eq $true -and
        $interactive.interpretation.clean_machine -eq $false
    )
    startup_preflight_report_consistent = (
        $preflight.status -eq "passed" -and
        $preflight.clean_machine -eq $false -and
        $preflight.startup.elapsed_seconds -ge $preflight.startup.threshold_seconds -and
        $preflight.startup.quillforge_window_found -eq $true -and
        $preflight.artifact.sha256.ToUpperInvariant() -eq $rootIdentity.sha256.ToUpperInvariant() -and
        $preflight.cleanup.exact_path_processes_removed -eq $true -and
        ($preflight.clean_machine -eq $true -or $preflight.clean_machine -eq $false)
    )
}

$mechanicalFailures = @($checks.GetEnumerator() | Where-Object { $_.Value -ne $true } | ForEach-Object Key)

$openGates = @(
    "signing:$($manifest.signing.status)",
    "installer:$($manifest.installer.status)",
    "update:$($manifest.update.status)",
    "file-associations:$($manifest.file_associations.status)",
    "support:$($manifest.support.status)",
    "clean-machine:environment_pending",
    "legal-notice-clearance:pending",
    "permission-and-disk-pressure:unrun",
    "hard-power-durability:unrun",
    "cross-machine-repeatability:unrun"
)
$payload = [ordered]@{
    schema_version = "1.0"
    generated_at_utc = [DateTime]::UtcNow.ToString("o")
    decision = "no-go"
    decision_reason = if ($mechanicalFailures.Count -gt 0) {
        "Mechanical handoff evidence is inconsistent; release gates remain open."
    } else {
        "Mechanical handoff evidence is consistent, but release gates remain open."
    }
    owner = "QuillForge project"
    artifact = $rootIdentity
    release_manifest = "dist/QuillForge.release.json"
    checks = $checks
    mechanical_failures = $mechanicalFailures
    evidence = [ordered]@{
        packaged_capture = "docs/performance/packaged-capture-2026-08-09.json"
        source_repeat = "docs/performance/repeat-2026-08-09.json"
        interactive_startup = "docs/performance/interactive-startup-2026-08-09.json"
        startup_preflight = "docs/performance/clean-machine-preflight-2026-08-09.json"
        notice = "dist/NOTICE.md"
        version = $versionInfo.FileVersion
        source_revision = $manifest.source_revision
        file_associations = $manifest.file_associations.status
        support_owner = $manifest.support.owner
        support_issue_route = $manifest.support.issue_route
        support_handoff_packet = "docs/support/HANDOFF.md"
        support_handoff_packet_exists = $checks.support_handoff_packet_exists
    }
    open_gates = $openGates
    support_boundary = "No large-file support range, native-memory ceiling, clean-machine guarantee, disk-pressure guarantee, or hard-power durability claim is approved."
}

$outputFile = Join-Path $root $OutputPath
$outputParent = Split-Path -Parent $outputFile
if (-not (Test-Path -LiteralPath $outputParent)) {
    New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
}
$payload | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $outputFile -Encoding utf8
Write-Output "Release handoff dossier: $outputFile"
Write-Output "Decision: $($payload.decision); open gates: $($openGates.Count)"
if ($mechanicalFailures.Count -gt 0) {
    throw "Release handoff evidence is inconsistent: $($mechanicalFailures -join ', ')"
}
if ($openGates.Count -gt 0) {
    throw "Release handoff gates remain open: $($openGates -join ', ')"
}

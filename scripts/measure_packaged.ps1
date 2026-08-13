param(
    [ValidateRange(1, 9)]
    [int]$RepeatCount = 3,
    [ValidateRange(1, 268435456)]
    [int]$InputBytes = 16 * 1048577,
    [string]$OutputPath = "docs\performance\packaged-capture-2026-08-09.json"
)

$ErrorActionPreference = "Stop"

function ConvertFrom-JsonCompat([string]$Json) {
    if ($PSVersionTable.PSVersion.Major -ge 6) {
        return $Json | ConvertFrom-Json -DateKind String
    }
    return $Json | ConvertFrom-Json
}

$root = Split-Path -Parent $PSScriptRoot
$previousLocation = (Get-Location).Path
$previousQtPlatform = $env:QT_QPA_PLATFORM
$primaryFailure = $null
$startedProcesses = @()
$temporaryDirectory = Join-Path ([System.IO.Path]::GetTempPath()) (
    "QuillForge-packaged-capture-" + [guid]::NewGuid().ToString("N")
)

try {
    Set-Location -LiteralPath $root
    $exePath = Join-Path $root "QuillForge.exe"
    if (-not (Test-Path -LiteralPath $exePath)) {
        throw "The root test EXE is missing: $exePath"
    }
    $resolvedExePath = [System.IO.Path]::GetFullPath($exePath)
    $artifactInfo = Get-Item -LiteralPath $exePath
    $artifactHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $exePath).Hash
    $releaseManifestPath = Join-Path $root "dist\QuillForge.release.json"
    if (-not (Test-Path -LiteralPath $releaseManifestPath)) {
        throw "The release manifest is missing: $releaseManifestPath"
    }
    $releaseManifest = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $releaseManifestPath)
    if (
        $releaseManifest.artifact.bytes -ne $artifactInfo.Length -or
        $releaseManifest.artifact.sha256.ToUpperInvariant() -ne $artifactHash.ToUpperInvariant() -or
        $releaseManifest.root_test_copy.bytes -ne $artifactInfo.Length -or
        $releaseManifest.root_test_copy.sha256.ToUpperInvariant() -ne $artifactHash.ToUpperInvariant()
    ) {
        throw "The root EXE is not the artifact recorded by the release manifest."
    }
    [System.IO.Directory]::CreateDirectory($temporaryDirectory) | Out-Null
    $env:QT_QPA_PLATFORM = "offscreen"
    $reports = @()
    for ($run = 1; $run -le $RepeatCount; $run++) {
        $runDirectory = Join-Path $temporaryDirectory ("run-{0}" -f $run)
        [System.IO.Directory]::CreateDirectory($runDirectory) | Out-Null
        $rawReportPath = Join-Path $runDirectory "capture.json"
        $quotedReportPath = '"' + $rawReportPath.Replace('"', '\"') + '"'
        $arguments = @(
            "--diagnose-capture",
            "--output", $quotedReportPath,
            "--input-bytes", "$InputBytes"
        )
        $process = Start-Process -FilePath $resolvedExePath -ArgumentList $arguments -PassThru -WindowStyle Hidden
        $startedProcesses += $process
        $deadline = (Get-Date).AddSeconds(120)
        while (-not $process.HasExited -or -not (Test-Path -LiteralPath $rawReportPath)) {
            if ((Get-Date) -gt $deadline) {
                $process.Refresh()
                if (-not $process.HasExited) {
                    $actualProcessPath = [System.IO.Path]::GetFullPath($process.MainModule.FileName)
                    if ($actualProcessPath -ne $resolvedExePath) {
                        throw "Refusing to stop an unexpected process: $actualProcessPath"
                    }
                    Stop-Process -Id $process.Id -Force
                }
                throw "Packaged capture run $run exceeded the 120 second timeout."
            }
            Start-Sleep -Milliseconds 100
            $process.Refresh()
        }
        $process.WaitForExit()
        if (-not (Test-Path -LiteralPath $rawReportPath)) {
            throw "Packaged capture run $run exited without a report. Exit code: $($process.ExitCode)"
        }
        $report = ConvertFrom-JsonCompat (Get-Content -Raw -Encoding UTF8 -LiteralPath $rawReportPath)
        $reportArtifact = $report.artifact
        if (
            $null -eq $reportArtifact -or
            $reportArtifact.bytes -ne $artifactInfo.Length -or
            $reportArtifact.sha256.ToUpperInvariant() -ne $artifactHash.ToUpperInvariant()
        ) {
            throw "Packaged capture run $run report is not bound to the producing EXE."
        }
        if ([System.IO.Path]::GetFullPath([string]$reportArtifact.path) -ne $resolvedExePath) {
            throw "Packaged capture run $run report names an unexpected executable."
        }
        if ($report.execution -ne "packaged-exe") {
            throw "Packaged capture run $run report has an unexpected execution path."
        }
        if ($process.ExitCode -ne 0 -or $report.status -ne "completed") {
            throw "Packaged capture run $run failed. Exit code: $($process.ExitCode); status: $($report.status)"
        }
        if (
            $report.capture.captured_bytes -ne $report.capture.total_bytes -or
            $report.capture.round_trip -ne $true -or
            $report.handoff.phase -ne "committed" -or
            $report.handoff.round_trip -ne $true -or
            $report.handoff.captured_bytes -ne $report.handoff.expected_bytes -or
            $report.handoff.queue_peak_chunks -gt $report.handoff.queue_max_chunks -or
            $report.handoff.queue_peak_bytes -gt $report.handoff.queue_max_bytes -or
            $report.handoff.channel_state -ne "finished" -or
            $report.handoff.worker_terminal -ne $true -or
            $report.handoff.final_snapshot_files -ne 1 -or
            $report.handoff.temporary_files -ne 0
        ) {
            throw "Packaged capture run $run did not complete a bounded full round trip."
        }
        $reports += [ordered]@{
            run = $run
            exit_code = $process.ExitCode
            executable = $resolvedExePath
            execution = "packaged EXE with Qt offscreen"
            artifact = [ordered]@{
                bytes = $artifactInfo.Length
                sha256 = $artifactHash
                release_manifest = "dist/QuillForge.release.json"
            }
            report = $report
        }
    }

    $postRunHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $exePath).Hash
    if ($postRunHash -ne $artifactHash) {
        throw "The root EXE changed during packaged measurement."
    }

    $summary = [ordered]@{
        schema_version = "1.0"
        tool = "scripts/measure_packaged.ps1"
        repeat_count = $RepeatCount
        input_bytes = $InputBytes
        artifact = [ordered]@{
            path = "QuillForge.exe"
            bytes = $artifactInfo.Length
            sha256 = $artifactHash
            release_manifest = "dist/QuillForge.release.json"
        }
        execution = "packaged EXE with QT_QPA_PLATFORM=offscreen"
        runs = $reports
        interpretation = [ordered]@{
            claims = "Packaged EXE capture path with Qt offscreen on the current Windows machine only."
            unrun = @(
                "clean-machine startup and capture",
                "interactive non-offscreen capture",
                "disk-full and permission pressure",
                "power-loss durability",
                "files larger than the measured input",
                "hard product memory ceiling"
            )
        }
    }

    $outputFile = Join-Path $root $OutputPath
    $outputParent = Split-Path -Parent $outputFile
    if (-not (Test-Path -LiteralPath $outputParent)) {
        New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
    }
    $summary | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $outputFile -Encoding utf8
    Write-Output "Packaged capture evidence: $outputFile"
    Write-Output "Runs: $RepeatCount; artifact SHA-256: $artifactHash"
}
catch {
    $primaryFailure = $_
    throw
}
finally {
    $cleanupFailure = $null
    foreach ($candidate in @($startedProcesses)) {
        try {
            $candidate.Refresh()
            if (-not $candidate.HasExited) {
                $actualProcessPath = [System.IO.Path]::GetFullPath($candidate.MainModule.FileName)
                if ($actualProcessPath -ne $resolvedExePath) {
                    throw "Refusing to stop an unexpected process: $actualProcessPath"
                }
                Stop-Process -Id $candidate.Id -Force
                if (-not $candidate.WaitForExit(2000)) {
                    throw "Packaged capture child process did not exit after cleanup."
                }
            }
        }
        catch {
            if ($null -eq $cleanupFailure) {
                $cleanupFailure = $_
            }
        }
    }
    try {
        if (Test-Path -LiteralPath $temporaryDirectory) {
            [System.IO.Directory]::Delete($temporaryDirectory, $true)
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

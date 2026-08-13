[CmdletBinding()]
param(
    [string]$ExecutablePath = ".\QuillForge.exe",
    [ValidateRange(1, 300)]
    [int]$StartupSeconds = 5,
    [string]$OutputPath = "docs\performance\interactive-startup-2026-08-09.json"
)

$ErrorActionPreference = "Stop"

$resolvedExecutable = (Resolve-Path -LiteralPath $ExecutablePath -ErrorAction Stop).Path
$artifact = Get-Item -LiteralPath $resolvedExecutable
$artifactHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $resolvedExecutable).Hash
$existing = @(Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -eq $resolvedExecutable })
if ($existing.Count -ne 0) {
    throw "Refusing interactive startup because the exact executable is already running: $($existing.ProcessId -join ',')"
}

$startedAt = [DateTime]::UtcNow
$launcher = $null
$processRecords = @()
$windowRecord = $null
$status = "failed"
$errorMessage = $null
$cleanupVerified = $false

function Get-ExactPathProcesses([string]$Path) {
    return @(
        Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -eq $Path }
    )
}

try {
    $launcher = Start-Process -FilePath $resolvedExecutable -PassThru
    $deadline = $startedAt.AddSeconds($StartupSeconds)
    do {
        Start-Sleep -Milliseconds 250
        $processRecords = @(Get-ExactPathProcesses $resolvedExecutable)
        foreach ($record in $processRecords) {
            try {
                $process = Get-Process -Id ([int]$record.ProcessId) -ErrorAction Stop
                if ($process.MainWindowTitle -eq "QuillForge") {
                    $windowRecord = [ordered]@{
                        app = "process:$resolvedExecutable"
                        title = $process.MainWindowTitle
                        window_id = [int64]$process.MainWindowHandle
                    }
                    break
                }
            }
            catch {
                continue
            }
        }
    } while ([DateTime]::UtcNow -lt $deadline)

    $elapsedSeconds = ([DateTime]::UtcNow - $startedAt).TotalSeconds
    if ($processRecords.Count -gt 0 -and $null -ne $windowRecord -and $elapsedSeconds -ge $StartupSeconds) {
        $status = "passed"
    }
    else {
        $errorMessage = "The QuillForge window was not verified for the configured threshold."
    }
}
catch {
    $errorMessage = $_.Exception.Message
    $elapsedSeconds = ([DateTime]::UtcNow - $startedAt).TotalSeconds
}
finally {
    $remainingBeforeCleanup = @(Get-ExactPathProcesses $resolvedExecutable)
    foreach ($record in $remainingBeforeCleanup) {
        Stop-Process -Id ([int]$record.ProcessId) -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Milliseconds 500
    $remainingAfterCleanup = @(Get-ExactPathProcesses $resolvedExecutable)
    $cleanupVerified = $remainingAfterCleanup.Count -eq 0
}

$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
$report = [ordered]@{
    schema_version = "1.0"
    tool = "scripts/verify_interactive_startup.ps1"
    generated_at_utc = [DateTime]::UtcNow.ToString("o")
    execution = "current interactive Windows desktop; not a clean-machine run"
    artifact = [ordered]@{
        path = "QuillForge.exe"
        resolved_path = $resolvedExecutable
        sha256 = $artifactHash
    }
    environment = [ordered]@{
        os = $os.Caption
        os_version = $os.Version
        architecture = $computer.SystemType
    }
    startup = [ordered]@{
        threshold_seconds = $StartupSeconds
        elapsed_seconds = [Math]::Round($elapsedSeconds, 3)
        process_exact_path_verified = ($processRecords.Count -gt 0)
        window = $windowRecord
        cleanup_exact_path_processes = $cleanupVerified
    }
    interpretation = [ordered]@{
        result = if ($status -eq "passed") { "current-machine-interactive-startup-passed" } else { "failed" }
        clean_machine = $false
        unrun = @(
            "clean Windows x64 desktop startup",
            "cross-machine repeatability",
            "interactive feature matrix beyond startup identity"
        )
    }
}

$outputFile = if ([System.IO.Path]::IsPathRooted($OutputPath)) {
    $OutputPath
}
else {
    Join-Path (Get-Location).Path $OutputPath
}
$outputParent = Split-Path -Parent $outputFile
if (-not (Test-Path -LiteralPath $outputParent)) {
    New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
}
$report | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $outputFile -Encoding utf8
Write-Output "Interactive startup evidence: $outputFile"

if ($status -ne "passed" -or -not $cleanupVerified) {
    throw "Interactive startup verification did not pass. See $outputFile"
}

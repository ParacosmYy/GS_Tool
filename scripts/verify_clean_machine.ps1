[CmdletBinding()]
param(
    [string]$ExecutablePath = ".\QuillForge.exe",
    [ValidateRange(1, 300)]
    [int]$StartupSeconds = 5,
    [string]$EnvironmentLabel = "unclassified-current-machine",
    [switch]$CleanMachineAttested,
    [string]$OutputPath = "docs\performance\clean-machine-preflight-2026-08-09.json"
)

$ErrorActionPreference = "Stop"

$resolvedExecutable = (Resolve-Path -LiteralPath $ExecutablePath -ErrorAction Stop).Path
$cleanMachine = $CleanMachineAttested.IsPresent
if ($cleanMachine -and $EnvironmentLabel -notmatch "^clean-windows-x64-") {
    throw "-CleanMachineAttested requires an EnvironmentLabel beginning with clean-windows-x64-."
}
$artifact = Get-Item -LiteralPath $resolvedExecutable
$artifactHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $resolvedExecutable).Hash
$versionInfo = $artifact.VersionInfo
$existing = @(Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -eq $resolvedExecutable })
if ($existing.Count -ne 0) {
    throw "Refusing startup preflight because the exact executable is already running: $($existing.ProcessId -join ',')"
}

$startedAt = [DateTime]::UtcNow
$launcher = $null
$status = "failed"
$errorMessage = $null
$windowRecords = @()
$processRecords = @()
$cleanupVerified = $false

try {
    $launcher = Start-Process -FilePath $resolvedExecutable -PassThru
    $deadline = $startedAt.AddSeconds($StartupSeconds)
    do {
        Start-Sleep -Milliseconds 250
        $processRecords = @(
            Get-CimInstance Win32_Process | Where-Object {
                $_.ExecutablePath -eq $resolvedExecutable
            } | ForEach-Object {
                [ordered]@{
                    process_id = [int]$_.ProcessId
                    parent_process_id = [int]$_.ParentProcessId
                    executable = $_.ExecutablePath
                }
            }
        )
        $windowRecords = @(
            foreach ($record in $processRecords) {
                try {
                    $process = Get-Process -Id ([int]$record.process_id) -ErrorAction Stop
                    [ordered]@{
                        process_id = [int]$record.process_id
                        title = $process.MainWindowTitle
                        executable = $record.executable
                    }
                }
                catch {
                    [ordered]@{
                        process_id = [int]$record.process_id
                        title = $null
                        executable = $record.executable
                    }
                }
            }
        )
    } while ([DateTime]::UtcNow -lt $deadline)

    $elapsedSeconds = ([DateTime]::UtcNow - $startedAt).TotalSeconds
    $alive = $processRecords.Count -gt 0
    $editorWindow = @($windowRecords | Where-Object { $_.title -eq "QuillForge" })
    $status = if ($alive -and $editorWindow.Count -gt 0 -and $elapsedSeconds -ge $StartupSeconds) {
        "passed"
    }
    else {
        "failed"
    }
}
catch {
    $errorMessage = $_.Exception.Message
    $elapsedSeconds = ([DateTime]::UtcNow - $startedAt).TotalSeconds
}
finally {
    $remainingBeforeCleanup = @(Get-CimInstance Win32_Process | Where-Object {
            $_.ExecutablePath -eq $resolvedExecutable
        })
    foreach ($record in $remainingBeforeCleanup) {
        Stop-Process -Id ([int]$record.ProcessId) -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Milliseconds 500
    $remainingAfterCleanup = @(Get-CimInstance Win32_Process | Where-Object {
            $_.ExecutablePath -eq $resolvedExecutable
        })
    $cleanupVerified = $remainingAfterCleanup.Count -eq 0
}

$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
$architectureSupported = $computer.SystemType -match "x64|AMD64|x86-64"
if (-not $architectureSupported -and $status -eq "passed") {
    $status = "failed"
    $errorMessage = "The startup process passed, but the environment is not Windows x64."
}
$attestationNote = if ($cleanMachine) {
    "Operator-attested clean Windows x64 environment; QA must retain the machine identity and review this report."
}
else {
    "No clean-machine attestation supplied; this report is a current/unclassified machine baseline."
}
$report = [ordered]@{
    schema_version = "1.0"
    tool = "scripts/verify_clean_machine.ps1"
    generated_at_utc = [DateTime]::UtcNow.ToString("o")
    status = $status
    environment_label = $EnvironmentLabel
    clean_machine = $cleanMachine
    clean_machine_attestation = $attestationNote
    artifact = [ordered]@{
        path = $resolvedExecutable
        bytes = $artifact.Length
        sha256 = $artifactHash
        file_version = $versionInfo.FileVersion
        product_version = $versionInfo.ProductVersion
    }
    environment = [ordered]@{
        os = $os.Caption
        os_version = $os.Version
        architecture = $computer.SystemType
        architecture_supported = $architectureSupported
        powershell = $PSVersionTable.PSVersion.ToString()
    }
    startup = [ordered]@{
        threshold_seconds = $StartupSeconds
        elapsed_seconds = [Math]::Round($elapsedSeconds, 3)
        launcher_pid = if ($null -ne $launcher) { $launcher.Id } else { $null }
        process_records = $processRecords
        window_records = $windowRecords
        quillforge_window_found = (@($windowRecords | Where-Object { $_.title -eq "QuillForge" }).Count -gt 0)
    }
    cleanup = [ordered]@{
        exact_path_processes_removed = $cleanupVerified
        remaining_exact_path_processes = @($remainingAfterCleanup | Select-Object -ExpandProperty ProcessId)
    }
    interpretation = [ordered]@{
        claims = "Interactive startup identity and lifetime on the labeled Windows machine only."
        unrun = @(
            "clean-machine attestation",
            "cross-machine repeatability",
            "interactive feature matrix beyond startup"
        )
    }
}
if ($null -ne $errorMessage) {
    $report.error = $errorMessage
}
$report.startup.architecture_supported = $architectureSupported

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
Write-Output "Startup preflight evidence: $outputFile"

if ($status -ne "passed" -or -not $cleanupVerified) {
    throw "Clean-machine startup preflight did not pass. See $outputFile"
}

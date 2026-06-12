param(
    [string]$RootDir = ".",
    [int]$TimeoutSeconds = 20
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    param([string]$Path)

    $resolved = Resolve-Path -LiteralPath $Path
    return $resolved.Path
}

function Get-EmbedDebugProcessIds {
    $processes = Get-Process -Name "EmbedDebug" -ErrorAction SilentlyContinue
    if (-not $processes) {
        return @()
    }
    return @($processes | Select-Object -ExpandProperty Id)
}

$root = Resolve-RepoRoot -Path $RootDir
$launcher = Join-Path $root "EmbedDebug.bat"
if (-not (Test-Path -LiteralPath $launcher)) {
    Write-Output "launch_result=FAILED reason=missing_launcher path=$launcher"
    exit 1
}

$beforeIds = Get-EmbedDebugProcessIds

try {
    Start-Process `
        -FilePath "cmd.exe" `
        -ArgumentList "/c", "EmbedDebug.bat" `
        -WorkingDirectory $root `
        -WindowStyle Hidden | Out-Null
} catch {
    Write-Output "launch_result=FAILED reason=start_process_error message=$($_.Exception.Message)"
    exit 1
}

$newProcess = $null
for ($i = 0; $i -lt $TimeoutSeconds; $i++) {
    Start-Sleep -Seconds 1
    $newProcess = Get-Process -Name "EmbedDebug" -ErrorAction SilentlyContinue |
        Where-Object { $beforeIds -notcontains $_.Id } |
        Select-Object -First 1
    if ($newProcess) {
        break
    }
}

if (-not $newProcess) {
    Write-Output "launch_result=FAILED reason=process_not_found timeout_seconds=$TimeoutSeconds"
    exit 1
}

Write-Output "launch_result=PASSED pid=$($newProcess.Id)"

try {
    $newProcess | Stop-Process -Force
} catch {
    Write-Output "cleanup_warning=failed_to_stop pid=$($newProcess.Id) message=$($_.Exception.Message)"
    exit 1
}

exit 0

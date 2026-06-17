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

$root = Resolve-RepoRoot -Path $RootDir
$launcher = Join-Path $root "EmbedDebug.bat"
if (-not (Test-Path -LiteralPath $launcher)) {
    Write-Output "launch_result=FAILED reason=missing_launcher path=$launcher"
    exit 1
}

try {
    $process = Start-Process `
        -FilePath "cmd.exe" `
        -ArgumentList "/c", "EmbedDebug.bat", "--smoke" `
        -WorkingDirectory $root `
        -WindowStyle Hidden `
        -Wait `
        -PassThru
} catch {
    Write-Output "launch_result=FAILED reason=start_process_error message=$($_.Exception.Message)"
    exit 1
}

if ($process.ExitCode -ne 0) {
    Write-Output "launch_result=FAILED reason=smoke_exit_code exit_code=$($process.ExitCode) timeout_seconds=$TimeoutSeconds"
    exit 1
}

Write-Output "launch_result=PASSED mode=python_pyqt_smoke"
exit 0

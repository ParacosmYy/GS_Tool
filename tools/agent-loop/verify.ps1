param(
    [switch]$RunSample
)

$ErrorActionPreference = "Stop"

function Write-Step([string]$Message) {
    Write-Host "[agent-loop] $Message"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..")
$sampleConfig = Join-Path $scriptDir "sample.embeddebug.json"

Write-Step "repo=$repoRoot"

$goCmd = Get-Command go.exe -ErrorAction SilentlyContinue
if (-not $goCmd) {
    Write-Host "agent_loop_go=UNAVAILABLE reason=go.exe_not_on_PATH"
    Write-Host "next_step=Install Go or add go.exe to PATH, then rerun tools\agent-loop\verify.ps1"
    exit 2
}

Write-Step "go=$($goCmd.Source)"
Push-Location $scriptDir
try {
    Write-Step "running go test ."
    go test .

    Write-Step "running dry-run sample"
    go run . -config $sampleConfig -dry-run

    if ($RunSample) {
        Write-Step "running sample config"
        go run . -config $sampleConfig
    } else {
        Write-Step "sample execution skipped; pass -RunSample to execute Doctor and launch probe"
    }

    Write-Host "agent_loop_verify=PASSED"
} finally {
    Pop-Location
}


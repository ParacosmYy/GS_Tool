param(
    [Parameter(Mandatory = $true)]
    [string]$RootDir,

    [string]$AppArguments = ""
)

$ErrorActionPreference = "Stop"

function Write-Info([string]$Message) { Write-Host "[INFO] $Message" }
function Write-Ok([string]$Message) { Write-Host "[OK] $Message" }
function Write-Fail([string]$Message) { Write-Host "[ERROR] $Message" }

function Resolve-CommandPath([string]$Name) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($cmd) { return $cmd.Source }
    return $null
}

$root = [System.IO.Path]::GetFullPath($RootDir)
$uv = Resolve-CommandPath "uv.exe"
if (-not $uv) {
    $uv = Resolve-CommandPath "uv"
}
if (-not $uv) {
    Write-Fail "uv was not found on PATH. Install uv or run from a uv-enabled terminal."
    exit 2
}

Write-Host "-------------------------------------------------"
Write-Host "EmbedDebug Python/PyQt Launcher"
Write-Host "Root : $root"
Write-Host "-------------------------------------------------"
Write-Info "start Python/PyQt application through uv run start-embeddebug."

Push-Location -LiteralPath $root
try {
    if ([string]::IsNullOrWhiteSpace($AppArguments)) {
        & $uv run start-embeddebug
    } else {
        & cmd.exe /c "uv run start-embeddebug $AppArguments"
    }
    $exitCode = $LASTEXITCODE
} finally {
    Pop-Location
}

if ($exitCode -ne 0) {
    Write-Fail "Python/PyQt launcher exited with code $exitCode."
    exit $exitCode
}

Write-Ok "Python/PyQt launcher completed."
exit 0

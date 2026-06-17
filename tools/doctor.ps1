param(
    [switch]$RunTests,
    [switch]$RunLaunch,
    [switch]$RunPackageDryRun
)

$ErrorActionPreference = "Stop"

$script:Failures = 0
$script:Warnings = 0

function Write-DoctorLine([string]$Level, [string]$Message) {
    Write-Host ("[{0}] {1}" -f $Level, $Message)
}

function Pass([string]$Message) { Write-DoctorLine "PASS" $Message }
function Skip([string]$Message) { Write-DoctorLine "SKIP" $Message }
function Warn([string]$Message) {
    $script:Warnings++
    Write-DoctorLine "WARN" $Message
}
function Fail([string]$Message) {
    $script:Failures++
    Write-DoctorLine "FAIL" $Message
}
function Info([string]$Message) { Write-DoctorLine "INFO" $Message }

function Resolve-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Resolve-CommandSource([string]$Name) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($cmd) { return $cmd.Source }
    return $null
}

function Check-Command([string]$Name, [bool]$Required) {
    $source = Resolve-CommandSource $Name
    if ($source) {
        Pass "$Name found: $source"
        return
    }

    if ($Required) {
        Fail "$Name is not available on PATH"
    } else {
        Warn "$Name is not available on PATH"
    }
}

function Check-ForbiddenBuildDirs([string]$Root) {
    $bad = Get-ChildItem -LiteralPath $Root -Force -Directory |
        Where-Object {
            $_.Name -in @("build2", "build-debug", "build-release") -or
            ($_.Name -like "*-build" -and $_.Name -ne "build")
        }

    if ($bad) {
        foreach ($item in $bad) {
            Fail "Forbidden build directory exists: $($item.FullName)"
        }
    } else {
        Pass "No forbidden parallel build directories found"
    }
}

function Invoke-CheckedCommand([string]$Label, [string]$FilePath, [string[]]$Arguments, [string]$WorkingDirectory) {
    Info "$Label"
    Push-Location -LiteralPath $WorkingDirectory
    try {
        & $FilePath @Arguments
        if ($LASTEXITCODE -eq 0) {
            Pass "$Label completed"
        } else {
            Fail "$Label failed with exit code $LASTEXITCODE"
        }
    } finally {
        Pop-Location
    }
}

$root = Resolve-RepoRoot
$uv = Resolve-CommandSource "uv.exe"
if (-not $uv) {
    $uv = Resolve-CommandSource "uv"
}

Write-Host "-------------------------------------------------"
Write-Host "EmbedDebug Python/PyQt Doctor"
Write-Host "Root : $root"
Write-Host "-------------------------------------------------"

if (Test-Path -LiteralPath (Join-Path $root "CLAUDE.md") -PathType Leaf) {
    Pass "Repo root detected"
} else {
    Fail "CLAUDE.md not found; run doctor from repository checkout"
}

Check-ForbiddenBuildDirs $root
Check-Command "uv.exe" $false
Check-Command "uv" $true

if (Test-Path -LiteralPath (Join-Path $root "pyproject.toml") -PathType Leaf) {
    Pass "pyproject.toml exists"
} else {
    Fail "pyproject.toml missing"
}

if (Test-Path -LiteralPath (Join-Path $root "uv.lock") -PathType Leaf) {
    Pass "uv.lock exists"
} else {
    Warn "uv.lock missing"
}

if (Test-Path -LiteralPath (Join-Path $root "python\embeddebug\app\main.py") -PathType Leaf) {
    Pass "Python/PyQt entrypoint exists"
} else {
    Fail "Python/PyQt entrypoint missing"
}

if ($RunTests) {
    if ($uv) {
        Invoke-CheckedCommand "uv run test-embeddebug-py" $uv @("run", "test-embeddebug-py") $root
        Invoke-CheckedCommand "uv run test-embeddebug-tools" $uv @("run", "test-embeddebug-tools") $root
    } else {
        Fail "Cannot run tests because uv is unavailable"
    }
} else {
    Skip "Test check not requested; use -RunTests"
}

if ($RunPackageDryRun) {
    if ($uv) {
        Invoke-CheckedCommand "uv run package-embeddebug --dry-run" $uv @("run", "package-embeddebug", "--dry-run") $root
    } else {
        Fail "Cannot run package dry-run because uv is unavailable"
    }
} else {
    Skip "Package dry-run not requested; use -RunPackageDryRun"
}

if ($RunLaunch) {
    $bat = Join-Path $root "EmbedDebug.bat"
    if (Test-Path -LiteralPath $bat -PathType Leaf) {
        Invoke-CheckedCommand "EmbedDebug.bat Python/PyQt smoke" $bat @("--smoke") $root
    } else {
        Fail "EmbedDebug.bat not found"
    }
} else {
    Skip "Launch check not requested; use -RunLaunch"
}

Write-Host "-------------------------------------------------"
Write-Host ("Doctor summary: {0} failure(s), {1} warning(s)" -f $script:Failures, $script:Warnings)
Write-Host "-------------------------------------------------"

if ($script:Failures -gt 0) {
    exit 1
}
exit 0

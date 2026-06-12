param(
    [switch]$RunBuild,
    [switch]$RunLaunch
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

function Read-LocalEnv([string]$EnvFile) {
    $result = @{}
    if (-not (Test-Path -LiteralPath $EnvFile -PathType Leaf)) {
        return $result
    }

    foreach ($line in Get-Content -LiteralPath $EnvFile) {
        if ($line -match '^\s*set\s+"([^=]+)=(.*)"\s*$') {
            $result[$matches[1]] = $matches[2]
        }
    }
    return $result
}

function Resolve-CommandSource([string]$Name) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($cmd) { return $cmd.Source }
    return $null
}

function Test-Leaf([string]$Path) {
    return -not [string]::IsNullOrWhiteSpace($Path) -and (Test-Path -LiteralPath $Path -PathType Leaf)
}

function Test-ContainerPath([string]$Path) {
    return -not [string]::IsNullOrWhiteSpace($Path) -and (Test-Path -LiteralPath $Path -PathType Container)
}

function Check-EnvPath([hashtable]$EnvMap, [string]$Name, [string]$ExpectedLeaf) {
    if (-not $EnvMap.ContainsKey($Name) -or [string]::IsNullOrWhiteSpace($EnvMap[$Name])) {
        Warn "$Name is not set in local_env.bat"
        return
    }

    $base = $EnvMap[$Name]
    $target = if ([string]::IsNullOrWhiteSpace($ExpectedLeaf)) { $base } else { Join-Path $base $ExpectedLeaf }
    if (Test-Leaf $target -or Test-ContainerPath $target) {
        Pass "$Name=$base"
    } else {
        Fail "$Name points to missing path: $base"
    }
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

function Check-BuildDirs([string]$Root) {
    $bad = Get-ChildItem -LiteralPath $Root -Force -Directory |
        Where-Object { $_.Name -match '^(build2|build-debug|build-release|cmake-build)' }

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
$buildDir = Join-Path $root "build"
$exePath = Join-Path $buildDir "EmbedDebug.exe"
$envFile = Join-Path $root "local_env.bat"
$envMap = Read-LocalEnv $envFile

if ($envMap.ContainsKey("QT_PREFIX")) {
    $qtBin = Join-Path $envMap["QT_PREFIX"] "bin"
    if (Test-Path -LiteralPath $qtBin -PathType Container) {
        $env:PATH = "$qtBin;$env:PATH"
    }
    $qtToolBin = Join-Path $envMap["QT_PREFIX"] "share\qt6\bin"
    if (Test-Path -LiteralPath $qtToolBin -PathType Container) {
        $env:PATH = "$qtToolBin;$env:PATH"
    }
}
if ($envMap.ContainsKey("MINGW_BIN") -and (Test-Path -LiteralPath $envMap["MINGW_BIN"] -PathType Container)) {
    $env:PATH = "$($envMap["MINGW_BIN"]);$env:PATH"
}

Write-Host "-------------------------------------------------"
Write-Host "EmbedDebug Doctor"
Write-Host "Root : $root"
Write-Host "-------------------------------------------------"

if (Test-Path -LiteralPath (Join-Path $root "CLAUDE.md") -PathType Leaf) {
    Pass "Repo root detected"
} else {
    Fail "CLAUDE.md not found; run doctor from repository checkout"
}

Check-BuildDirs $root

if (Test-Path -LiteralPath $buildDir -PathType Container) {
    Pass "build directory exists"
} else {
    Warn "build directory does not exist yet"
}

if (Test-Leaf $exePath) {
    Pass "build/EmbedDebug.exe exists"
} else {
    Warn "build/EmbedDebug.exe is missing"
}

if (Test-Leaf $envFile) {
    Pass "local_env.bat exists"
} else {
    Warn "local_env.bat is missing; run tools\\bootstrap_env.bat if needed"
}

Check-EnvPath $envMap "QT_PREFIX" "bin\windeployqt.exe"
Check-EnvPath $envMap "MINGW_BIN" "g++.exe"

if ($envMap.ContainsKey("CMAKE_BIN") -and (Test-Leaf $envMap["CMAKE_BIN"])) {
    Pass "CMAKE_BIN=$($envMap["CMAKE_BIN"])"
} else {
    Warn "CMAKE_BIN is missing or invalid in local_env.bat"
}

if ($envMap.ContainsKey("NINJA_BIN") -and (Test-Leaf $envMap["NINJA_BIN"])) {
    Pass "NINJA_BIN=$($envMap["NINJA_BIN"])"
} else {
    Warn "NINJA_BIN is missing or invalid in local_env.bat"
}

Check-Command "cmake.exe" $true
Check-Command "ninja.exe" $true
Check-Command "go.exe" $false

$mocPath = $null
if ($envMap.ContainsKey("QT_PREFIX")) {
    $candidateMoc = Join-Path $envMap["QT_PREFIX"] "share\qt6\bin\moc.exe"
    if (Test-Leaf $candidateMoc) {
        $mocPath = $candidateMoc
    } else {
        $candidateMoc = Join-Path $envMap["QT_PREFIX"] "bin\moc.exe"
        if (Test-Leaf $candidateMoc) {
            $mocPath = $candidateMoc
        }
    }
}
if ($mocPath) {
    & $mocPath -h > $null 2>&1
    if ($LASTEXITCODE -eq 0) {
        Pass "moc.exe runnable: $mocPath"
    } else {
        Fail "moc.exe exists but cannot run: $mocPath"
    }
} else {
    Fail "moc.exe not found under QT_PREFIX"
}

$running = Get-Process EmbedDebug -ErrorAction SilentlyContinue | Select-Object -First 1
if ($running) {
    Warn "EmbedDebug is currently running with PID $($running.Id); linking may fail if a rebuild is needed"
} else {
    Pass "EmbedDebug process is not running"
}

if ($RunBuild) {
    $cmake = Resolve-CommandSource "cmake.exe"
    if ($cmake) {
        Invoke-CheckedCommand "cmake build" $cmake @("--build", ".\build", "--config", "Release", "--parallel", "4") $root
    } else {
        Fail "Cannot run build because cmake.exe is unavailable"
    }
} else {
    Skip "Build check not requested; use -RunBuild"
}

if ($RunLaunch) {
    $bat = Join-Path $root "EmbedDebug.bat"
    if (Test-Leaf $bat) {
        Invoke-CheckedCommand "EmbedDebug.bat launch" $bat @() $root
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

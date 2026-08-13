param(
    [ValidateSet("onedir", "onefile")]
    [string]$Mode = "onedir",
    [switch]$Ble,
    [string]$OutputRoot = "dist\release",
    [string]$SourceRevision = ""
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$entryPoint = Join-Path $projectRoot "src\serialforge\__main__.py"
$versionSource = Join-Path $projectRoot "src\serialforge\__init__.py"
$metadataScript = Join-Path $projectRoot "scripts\provenance.py"
$noticeSource = Join-Path $projectRoot "packaging\NOTICE.txt"
$licenseInventorySource = Join-Path $projectRoot "docs\third_party\THIRD_PARTY_NOTICES.md"
$uv = Get-Command uv -ErrorAction SilentlyContinue
$pythonPath = Join-Path $projectRoot ".venv\Scripts\python.exe"

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Executable,
        [Parameter(Mandatory = $true)]
        [object[]]$Arguments,
        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Description failed with exit code $LASTEXITCODE."
    }
}

function Assert-SafeGeneratedChild {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Target,
        [Parameter(Mandatory = $true)]
        [string]$Boundary,
        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    $targetFull = [System.IO.Path]::GetFullPath($Target)
    $boundaryFull = [System.IO.Path]::GetFullPath($Boundary).TrimEnd("\")
    $boundaryPrefix = "$boundaryFull\"
    if (
        $targetFull.Equals($boundaryFull, [System.StringComparison]::OrdinalIgnoreCase) -or
        -not $targetFull.StartsWith($boundaryPrefix, [System.StringComparison]::OrdinalIgnoreCase)
    ) {
        throw "$Label is not a safe generated child path: $targetFull"
    }
}

if (-not (Test-Path -LiteralPath $entryPoint)) {
    throw "Entry point not found: $entryPoint"
}
foreach ($requiredPath in @($versionSource, $metadataScript, $noticeSource, $licenseInventorySource)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required packaging input not found: $requiredPath"
    }
}

if ([System.IO.Path]::IsPathRooted($OutputRoot)) {
    $outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
}
else {
    $outputRootPath = [System.IO.Path]::GetFullPath((Join-Path $projectRoot $OutputRoot))
}

Push-Location $projectRoot
try {
    $versionArguments = @(
        "run", "--locked", "--extra", "dev",
        "python", $metadataScript, "version", "--source", $versionSource
    )
    if ($uv) {
        $version = ((& $uv.Source @versionArguments) -join [Environment]::NewLine).Trim()
        if ($LASTEXITCODE -ne 0) {
            throw "Reading the project version failed with exit code $LASTEXITCODE."
        }
    }
    elseif (Test-Path -LiteralPath $pythonPath) {
        $version = ((& $pythonPath $metadataScript version --source $versionSource) -join [Environment]::NewLine).Trim()
        if ($LASTEXITCODE -ne 0) {
            throw "Reading the project version failed with exit code $LASTEXITCODE."
        }
    }
    else {
        throw "需要 uv 或已同步的 .venv；请先安装 uv 并执行 uv sync --locked --extra dev。"
    }
}
finally {
    Pop-Location
}

if ($version -notmatch "^\d+\.\d+\.\d+$") {
    throw "Project version is not a supported x.y.z value: '$version'"
}

$variant = if ($Ble) { "ble" } else { "core" }
$releaseRoot = Join-Path $outputRootPath "$version\$variant\$Mode"
$appRoot = Join-Path $releaseRoot "app"
$buildRoot = Join-Path $projectRoot "build\pyinstaller\provenance\$version\$variant\$Mode"
$workRoot = Join-Path $buildRoot "work"
$specRoot = Join-Path $buildRoot "spec"
$versionFile = Join-Path $buildRoot "version.txt"
$archiveListing = Join-Path $releaseRoot "PYINSTALLER_ARCHIVE.txt"
$dependencyTree = Join-Path $releaseRoot "DEPENDENCY_TREE.txt"
$manifestPath = Join-Path $releaseRoot "PROVENANCE.json"
$revision = if ($SourceRevision) { $SourceRevision } elseif ($env:GITHUB_SHA) { $env:GITHUB_SHA } else { "local-unpinned" }

Assert-SafeGeneratedChild -Target $releaseRoot -Boundary $outputRootPath -Label "release output"
Assert-SafeGeneratedChild -Target $buildRoot -Boundary (Join-Path $projectRoot "build") -Label "PyInstaller build"

if (Test-Path -LiteralPath $releaseRoot) {
    Remove-Item -LiteralPath $releaseRoot -Recurse -Force
}
if (Test-Path -LiteralPath $buildRoot) {
    Remove-Item -LiteralPath $buildRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $releaseRoot -Force | Out-Null
New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null

Push-Location $projectRoot
try {
    if ($uv) {
        $syncArguments = @("sync", "--locked", "--extra", "dev")
        if ($Ble) {
            $syncArguments += @("--extra", "ble")
        }
        Invoke-Checked -Executable $uv.Source -Arguments $syncArguments -Description "Locked dependency sync"
    }
    elseif ($Ble) {
        throw "BLE 打包需要 uv 以同步可选依赖；请安装 uv 后重试。"
    }

    $metadataArguments = @("run", "--locked", "--extra", "dev")
    if ($Ble) {
        $metadataArguments += @("--extra", "ble")
    }
    $metadataArguments += @(
        "python", $metadataScript, "version-file",
        "--source", $versionSource,
        "--output", $versionFile
    )

    $pyInstallerArguments = @(
        "--noconfirm"
        "--clean"
        "--$Mode"
        "--windowed"
        "--name"
        "SerialForge"
        "--paths"
        (Join-Path $projectRoot "src")
        "--version-file"
        $versionFile
        "--hidden-import"
        "serial"
        "--hidden-import"
        "serial.tools.list_ports"
        "--collect-submodules"
        "serial"
        "--distpath"
        $appRoot
        "--workpath"
        $workRoot
        "--specpath"
        $specRoot
        $entryPoint
    )
    if ($Ble) {
        $pyInstallerArguments += @(
            "--collect-submodules", "bleak.backends.winrt",
            "--hidden-import", "bleak.backends._manufacturers",
            "--collect-submodules", "winrt"
        )
    }

    if ($uv) {
        Invoke-Checked -Executable $uv.Source -Arguments $metadataArguments -Description "Version-resource generation"
        $buildArguments = @("run", "--locked", "--extra", "dev")
        if ($Ble) {
            $buildArguments += @("--extra", "ble")
        }
        $buildArguments += @("python", "-m", "PyInstaller")
        $buildArguments += $pyInstallerArguments
        Invoke-Checked -Executable $uv.Source -Arguments $buildArguments -Description "PyInstaller build"
    }
    elseif (Test-Path -LiteralPath $pythonPath) {
        Invoke-Checked -Executable $pythonPath -Arguments @(
            $metadataScript, "version-file",
            "--source", $versionSource,
            "--output", $versionFile
        ) -Description "Version-resource generation"
        Invoke-Checked -Executable $pythonPath -Arguments (
            @("-m", "PyInstaller") + $pyInstallerArguments
        ) -Description "PyInstaller build"
    }
    else {
        throw "需要 uv 或已同步的 .venv；请先安装 uv 并执行 uv sync --locked --extra dev。"
    }

    if ($Mode -eq "onedir") {
        $artifact = Join-Path $appRoot "SerialForge\SerialForge.exe"
    }
    else {
        $artifact = Join-Path $appRoot "SerialForge.exe"
    }
    if (-not (Test-Path -LiteralPath $artifact)) {
        throw "Expected packaging artifact was not created: $artifact"
    }

    Copy-Item -LiteralPath $noticeSource -Destination (Join-Path $releaseRoot "NOTICE.txt") -Force
    Copy-Item -LiteralPath $licenseInventorySource -Destination (Join-Path $releaseRoot "THIRD_PARTY_NOTICES.md") -Force

    if ($uv) {
        $treeArguments = @("tree", "--locked", "--all-groups")
        & $uv.Source @treeArguments *> $dependencyTree
        if ($LASTEXITCODE -ne 0) {
            throw "Dependency tree capture failed with exit code $LASTEXITCODE."
        }
    }
    else {
        & $pythonPath -m pip freeze *> $dependencyTree
        if ($LASTEXITCODE -ne 0) {
            throw "Fallback dependency inventory capture failed with exit code $LASTEXITCODE."
        }
    }

    $archiveViewer = Join-Path $projectRoot ".venv\Scripts\pyi-archive_viewer.exe"
    if (Test-Path -LiteralPath $archiveViewer) {
        & $archiveViewer -r -b $artifact *> $archiveListing
        if ($LASTEXITCODE -ne 0) {
            throw "PyInstaller archive inspection failed with exit code $LASTEXITCODE."
        }
    }
    elseif ($uv) {
        $archiveArguments = @("run", "--locked", "--extra", "dev")
        if ($Ble) {
            $archiveArguments += @("--extra", "ble")
        }
        $archiveArguments += @("pyi-archive_viewer", "-r", "-b", $artifact)
        & $uv.Source @archiveArguments *> $archiveListing
        if ($LASTEXITCODE -ne 0) {
            throw "PyInstaller archive inspection failed with exit code $LASTEXITCODE."
        }
    }
    else {
        throw "PyInstaller archive viewer was not found in the synchronized .venv."
    }

    $versionInfo = (Get-Item -LiteralPath $artifact).VersionInfo
    $signature = Get-AuthenticodeSignature -LiteralPath $artifact
    $manifestArguments = @(
        "manifest",
        "--root", $releaseRoot,
        "--artifact", $artifact,
        "--version", $version,
        "--version-source", $versionSource,
        "--variant", $variant,
        "--mode", $Mode,
        "--source-revision", $revision,
        "--lockfile", (Join-Path $projectRoot "uv.lock"),
        "--archive-listing", $archiveListing,
        "--pe-file-version", ([string]$versionInfo.FileVersion).Trim(),
        "--pe-product-version", ([string]$versionInfo.ProductVersion).Trim(),
        "--pe-product-name", ([string]$versionInfo.ProductName).Trim(),
        "--signature-status", ([string]$signature.Status),
        "--license-inventory", "THIRD_PARTY_NOTICES.md",
        "--output", $manifestPath
    )

    if ($uv) {
        $manifestRunArguments = @("run", "--locked", "--extra", "dev")
        if ($Ble) {
            $manifestRunArguments += @("--extra", "ble")
        }
        $manifestRunArguments += @("python", $metadataScript)
        $manifestRunArguments += $manifestArguments
        Invoke-Checked -Executable $uv.Source -Arguments $manifestRunArguments -Description "Provenance manifest generation"
        Invoke-Checked -Executable $uv.Source -Arguments @(
            "run", "--locked", "--extra", "dev", "python", $metadataScript,
            "verify", "--manifest", $manifestPath
        ) -Description "Provenance manifest verification"
    }
    else {
        Invoke-Checked -Executable $pythonPath -Arguments (
            @($metadataScript) + $manifestArguments
        ) -Description "Provenance manifest generation"
        Invoke-Checked -Executable $pythonPath -Arguments @(
            $metadataScript, "verify", "--manifest", $manifestPath
        ) -Description "Provenance manifest verification"
    }

    Write-Host "Created $variant $Mode artifact: $artifact"
    Write-Host "Provenance manifest: $manifestPath"
}
finally {
    Pop-Location
}

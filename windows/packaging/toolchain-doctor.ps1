<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Read-only preflight for the optional Windows EXE build.
Module: Delivery / packaging toolchain boundary

This script does not install packages, mutate PATH, clean dist/build output,
or create a frozen artifact. It reports pending prerequisites explicitly.
#>

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$windowsRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\")).Path
$venvPython = Join-Path $windowsRoot ".venv\Scripts\python.exe"
$runtimeLock = Join-Path $windowsRoot "requirements.lock"
$buildLock = Join-Path $PSScriptRoot "requirements-build.lock"
$entryPoint = Join-Path $windowsRoot "run.py"
$buildScript = Join-Path $PSScriptRoot "build.ps1"
$templates = Join-Path $windowsRoot "token_tracker\templates"
$static = Join-Path $windowsRoot "token_tracker\static"
$pyinstallerModule = Join-Path $windowsRoot ".venv\Lib\site-packages\PyInstaller\__init__.py"
$pyinstallerLauncher = Join-Path $windowsRoot ".venv\Scripts\pyinstaller.exe"
$pending = [System.Collections.Generic.List[string]]::new()

function Write-Check([string]$Name, [bool]$Ready, [string]$PassDetail, [string]$PendingDetail) {
    if ($Ready) {
        Write-Host "[PASS] $Name`: $PassDetail"
    } else {
        Write-Host "[PENDING] $Name`: $PendingDetail"
        $pending.Add($Name)
    }
}

$pythonReady = Test-Path -LiteralPath $venvPython -PathType Leaf
Write-Check "Python build runtime" $pythonReady "windows/.venv Python is available" "windows/.venv/Scripts/python.exe is required"

$runtimeLockReady = Test-Path -LiteralPath $runtimeLock -PathType Leaf
Write-Check "Runtime lock" $runtimeLockReady "requirements.lock is present" "windows/requirements.lock is required"

$buildLockReady = Test-Path -LiteralPath $buildLock -PathType Leaf
if ($buildLockReady) {
    $buildLockText = Get-Content -Raw -LiteralPath $buildLock
    $buildLockPinned = $buildLockText -match "(?m)^pyinstaller==[^\r\n]+$"
} else {
    $buildLockText = ""
    $buildLockPinned = $false
}
Write-Check "Build lock" ($buildLockReady -and $buildLockPinned) "PyInstaller is version-pinned" "requirements-build.lock must pin PyInstaller"

Write-Check "EXE entry point" (Test-Path -LiteralPath $entryPoint -PathType Leaf) "run.py is present" "windows/run.py is required"
Write-Check "Build script" (Test-Path -LiteralPath $buildScript -PathType Leaf) "packaging/build.ps1 is present" "packaging/build.ps1 is required"
Write-Check "Frozen templates" (Test-Path -LiteralPath $templates -PathType Container) "template directory is present" "token_tracker/templates is required"
Write-Check "Frozen static assets" (Test-Path -LiteralPath $static -PathType Container) "static directory is present" "token_tracker/static is required"

$pyinstallerReady = $false
$pyinstallerVersion = ""
$pyinstallerReady = (Test-Path -LiteralPath $pyinstallerModule -PathType Leaf) -and (Test-Path -LiteralPath $pyinstallerLauncher -PathType Leaf)
if ($pyinstallerReady) {
    $versionPattern = "__version__\s*=\s*[^0-9]*([0-9][0-9A-Za-z\.-]*)"
    $versionLine = Get-Content -LiteralPath $pyinstallerModule | Where-Object { $_ -match $versionPattern } | Select-Object -First 1
    if ($versionLine -match $versionPattern) { $pyinstallerVersion = $Matches[1] }
}
Write-Check "PyInstaller" $pyinstallerReady "PyInstaller $pyinstallerVersion files are present" "PyInstaller module and launcher are required in windows/.venv"

if ($pending.Count -gt 0) {
    Write-Host "EXE packaging pending: $($pending -join ', ')"
    exit 3
}

Write-Host "EXE packaging ready: run packaging/build.ps1 in the approved build environment"
exit 0

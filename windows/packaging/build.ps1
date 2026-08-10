# Author: AI Token Tracker Engineering Team
# Maintainer: Project Owner
# Purpose: Reproducible optional PyInstaller build for the personal Windows EXE.
# Module: Delivery / packaging boundary

[CmdletBinding()]
param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$windowsRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\")).Path
$venvPython = Join-Path $windowsRoot ".venv\Scripts\python.exe"
$runtimeRequirements = Join-Path $windowsRoot "requirements.lock"
$buildRequirements = Join-Path $PSScriptRoot "requirements-build.lock"
$distRoot = Join-Path $windowsRoot "dist"
$buildRoot = Join-Path $windowsRoot "build"

if (-not (Test-Path -LiteralPath $venvPython)) {
    throw "windows/.venv is missing. Run windows/start.bat or create the project virtual environment first."
}
if (-not (Test-Path -LiteralPath $runtimeRequirements)) {
    throw "windows/requirements.lock is missing. Complete the runtime dependency lock first."
}
if (-not (Test-Path -LiteralPath $buildRequirements)) {
    throw "packaging/requirements-build.lock is missing. Complete the build dependency lock first."
}

if ($Clean) {
    $normalizedRoot = [System.IO.Path]::GetFullPath($windowsRoot).TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
    foreach ($target in @($distRoot, $buildRoot)) {
        if (Test-Path -LiteralPath $target) {
            $normalizedTarget = [System.IO.Path]::GetFullPath($target)
            if (-not $normalizedTarget.StartsWith($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
                throw "Refusing to clean a path outside the workspace: $normalizedTarget"
            }
            Remove-Item -LiteralPath $target -Recurse -Force
        }
    }
}

& $venvPython -m pip install --disable-pip-version-check -r $runtimeRequirements -r $buildRequirements
if ($LASTEXITCODE -ne 0) { throw "Dependency installation failed." }

$entryPoint = Join-Path $windowsRoot "run.py"
$templates = Join-Path $windowsRoot "token_tracker\templates"
$static = Join-Path $windowsRoot "token_tracker\static"
& $venvPython -m PyInstaller --noconfirm --clean --onedir `
    --name "AI-Token-Tracker" `
    --distpath $distRoot `
    --workpath $buildRoot `
    --specpath $buildRoot `
    --add-data "$templates;token_tracker/templates" `
    --add-data "$static;token_tracker/static" `
    $entryPoint
if ($LASTEXITCODE -ne 0) { throw "PyInstaller build failed." }

Write-Host "EXE generated: $distRoot\AI-Token-Tracker\AI-Token-Tracker.exe"

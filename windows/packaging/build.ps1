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
    throw "找不到 windows/.venv。请先运行 windows/start.bat 或创建 Python 虚拟环境。"
}
if (-not (Test-Path -LiteralPath $runtimeRequirements)) {
    throw "找不到 windows/requirements.lock。请先完成依赖锁定。"
}
if (-not (Test-Path -LiteralPath $buildRequirements)) {
    throw "找不到 packaging/requirements-build.lock。请先完成构建工具锁定。"
}

if ($Clean) {
    $normalizedRoot = [System.IO.Path]::GetFullPath($windowsRoot).TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
    foreach ($target in @($distRoot, $buildRoot)) {
        if (Test-Path -LiteralPath $target) {
            $normalizedTarget = [System.IO.Path]::GetFullPath($target)
            if (-not $normalizedTarget.StartsWith($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
                throw "拒绝清理工作区外路径：$normalizedTarget"
            }
            Remove-Item -LiteralPath $target -Recurse -Force
        }
    }
}

& $venvPython -m pip install --disable-pip-version-check -r $runtimeRequirements -r $buildRequirements
if ($LASTEXITCODE -ne 0) { throw "依赖安装失败。" }

$entryPoint = Join-Path $windowsRoot "run.py"
$templates = Join-Path $windowsRoot "token_tracker\templates"
$static = Join-Path $windowsRoot "token_tracker\static"
& $venvPython -m PyInstaller --noconfirm --clean --onedir `
    --name "AI-Token-Tracker" `
    --distpath $distRoot `
    --workpath $buildRoot `
    --add-data "$templates;token_tracker/templates" `
    --add-data "$static;token_tracker/static" `
    $entryPoint
if ($LASTEXITCODE -ne 0) { throw "PyInstaller 构建失败。" }

Write-Host "EXE 已生成：$distRoot\AI-Token-Tracker\AI-Token-Tracker.exe"

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
$distRoot = Join-Path $windowsRoot "dist"
$buildRoot = Join-Path $windowsRoot "build"

if (-not (Test-Path -LiteralPath $venvPython)) {
    throw "找不到 windows/.venv。请先运行 windows/start.bat 或创建 Python 虚拟环境。"
}

if ($Clean) {
    foreach ($target in @($distRoot, $buildRoot)) {
        if (Test-Path -LiteralPath $target) {
            Remove-Item -LiteralPath $target -Recurse -Force
        }
    }
}

& $venvPython -m pip install -r (Join-Path $windowsRoot "requirements.txt") pyinstaller
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

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$uv = Get-Command uv -ErrorAction SilentlyContinue
$pythonPath = Join-Path $projectRoot ".venv\Scripts\python.exe"

if (-not $uv -and -not (Test-Path -LiteralPath $pythonPath)) {
    throw "需要 uv 或已同步的 .venv；请先安装 uv 并执行 uv sync --locked --extra dev。"
}

Push-Location $projectRoot
try {
    if ($uv) {
        & $uv.Source sync --locked --extra dev
        if ($LASTEXITCODE -ne 0) {
            throw "Locked dependency sync failed."
        }
        & $uv.Source run --locked --extra dev python -m compileall -q src scripts
        & $uv.Source run --locked --extra dev python scripts\check_source_limits.py src
        & $uv.Source run --locked --extra dev python scripts\check_theme_tokens.py
    }
    else {
        & $pythonPath -m compileall -q src
        & $pythonPath scripts\check_source_limits.py src
        & $pythonPath scripts\check_theme_tokens.py
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Python compilation or source line limit check failed."
    }

    if ($uv) {
        & $uv.Source run --locked --extra dev ruff check src scripts
    }
    else {
        $ruffPath = Join-Path $projectRoot ".venv\Scripts\ruff.exe"
        & $ruffPath check src scripts
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Ruff check failed."
    }
}
finally {
    Pop-Location
}

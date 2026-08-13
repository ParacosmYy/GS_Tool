$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot

Push-Location $projectRoot
try {
    $uv = Get-Command uv -ErrorAction SilentlyContinue
    if ($uv) {
        & $uv.Source run --locked --extra dev python -m serialforge
    }
    else {
        $pythonPath = Join-Path $projectRoot ".venv\Scripts\python.exe"
        if (-not (Test-Path -LiteralPath $pythonPath)) {
            throw "需要 uv 或已同步的 .venv；请先安装 uv 并执行 uv sync --locked --extra dev。"
        }
        & $pythonPath -m serialforge
    }
    if ($LASTEXITCODE -ne 0) {
        throw "SerialForge exited with code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

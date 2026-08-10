<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Start the Windows production WSGI process only after HTTPS preflight.
Module: Deployment / production process boundary

This wrapper does not create certificates, open firewall ports, or manage a
Windows service. It is suitable as the executable action for Task Scheduler,
NSSM, or another approved process supervisor after the host is configured.
#>

[CmdletBinding()]
param(
    [string]$BindAddress = "127.0.0.1",
    [int]$BindPort = 5000
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$deploymentRoot = Split-Path -Parent $PSScriptRoot
$pythonPath = Join-Path $deploymentRoot ".venv\Scripts\python.exe"
if (-not (Test-Path -LiteralPath $pythonPath -PathType Leaf)) {
    throw "Windows Python environment not found: $pythonPath"
}

Push-Location $deploymentRoot
try {
    & $pythonPath -m token_tracker preflight --production
    if ($LASTEXITCODE -ne 0) {
        throw "Production preflight failed with exit code $LASTEXITCODE"
    }

    & $pythonPath -m token_tracker serve `
        --host $BindAddress `
        --port $BindPort `
        --production
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}

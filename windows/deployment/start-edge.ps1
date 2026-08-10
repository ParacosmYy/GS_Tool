<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Validate and foreground-start the project-local Caddy HTTPS edge.
Entry: start-edge deployment command.
Module: Deployment / edge process boundary

This script does not start Waitress, create certificates, modify firewall
rules, or change ACLs. The operator must provision Caddy, prepare the domain,
and create a restricted log directory before running it.
#>

[CmdletBinding()]
param(
    [string]$ConfigPath = "$PSScriptRoot\Caddyfile",
    [string]$LogsDirectory = "$PSScriptRoot\logs",
    [string]$CaddyPath = "$PSScriptRoot\..\.cache\caddy\2.11.4\caddy.exe"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$preflight = Join-Path $PSScriptRoot "preflight-edge.ps1"
if (-not (Test-Path -LiteralPath $preflight -PathType Leaf)) {
    throw "Edge preflight script not found: $preflight"
}
if (-not (Test-Path -LiteralPath $CaddyPath -PathType Leaf)) {
    throw "Caddy is missing: $CaddyPath. Run deployment\provision-caddy.ps1 first."
}

& $preflight -ConfigPath $ConfigPath -LogsDirectory $LogsDirectory -CaddyPath $CaddyPath
if ($LASTEXITCODE -ne 0) {
    throw "Edge preflight failed with exit code $LASTEXITCODE"
}

$resolvedConfig = (Resolve-Path -LiteralPath $ConfigPath).Path
Push-Location (Split-Path -Parent $resolvedConfig)
try {
    & $CaddyPath run --config $resolvedConfig --adapter caddyfile
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}

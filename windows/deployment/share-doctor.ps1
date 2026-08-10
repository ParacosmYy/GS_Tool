<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Run one read-only preflight for LAN or HTTPS sharing handoff.
Module: Deployment / share readiness composition boundary

This script does not start the application, install Caddy, request a
certificate, change firewall rules, modify ACLs, create a database, or change
the configured environment permanently. It composes existing read-only
checks so the deployment owner has one predictable handoff command.
#>

[CmdletBinding()]
param(
    [ValidateSet("LanPreview", "Production")]
    [string]$Mode = "LanPreview",

    [string]$BindAddress,

    [ValidateRange(1, 65535)]
    [int]$BindPort = 5000,

    [string]$Caddyfile,

    [string]$LogsDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$pythonPath = Join-Path $projectRoot ".venv\Scripts\python.exe"
$edgePreflight = Join-Path $PSScriptRoot "preflight-edge.ps1"
$isProduction = $Mode -eq "Production"
$effectiveAddress = if ($BindAddress) {
    $BindAddress.Trim()
} elseif ($isProduction) {
    "127.0.0.1"
} else {
    "0.0.0.0"
}

function Invoke-TrackerCheck([string[]]$Arguments, [string]$Label) {
    Write-Host "[share-doctor] $Label"
    & $pythonPath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Label 失败，exit code=$LASTEXITCODE"
    }
}

if (-not (Test-Path -LiteralPath $pythonPath -PathType Leaf)) {
    throw "Windows Python environment not found: $pythonPath"
}
if (-not (Test-Path -LiteralPath $edgePreflight -PathType Leaf)) {
    throw "Edge preflight script not found: $edgePreflight"
}
if ($isProduction -and $effectiveAddress -notin @("127.0.0.1", "::1", "localhost")) {
    throw "Production share must bind loopback; Caddy owns the external HTTPS address"
}
if ($isProduction -and (-not $Caddyfile -or -not $LogsDirectory)) {
    throw "Production share requires -Caddyfile and -LogsDirectory for the edge preflight"
}

$hadRuntimeMode = Test-Path -LiteralPath "Env:TOKEN_TRACKER_RUNTIME_MODE"
$originalRuntimeMode = $env:TOKEN_TRACKER_RUNTIME_MODE
try {
    $env:TOKEN_TRACKER_RUNTIME_MODE = if ($isProduction) { "production" } else { "lan" }
    $preflightArguments = @(
        "-m", "token_tracker", "preflight",
        "--host", $effectiveAddress
    )
    if ($isProduction) {
        $preflightArguments += "--production"
    }
    Push-Location $projectRoot
    try {
        Invoke-TrackerCheck $preflightArguments "Application configuration preflight"
    }
    finally {
        Pop-Location
    }

    if ($isProduction) {
        & $edgePreflight -ConfigPath $Caddyfile -LogsDirectory $LogsDirectory
        Write-Host "[share-doctor] Production handoff passed: Waitress=$effectiveAddress`:$BindPort; edge=Caddy"
    }
    else {
        Write-Host "[share-doctor] LAN preview handoff passed: bind=$effectiveAddress`:$BindPort"
        Write-Host "[share-doctor] Continue with deployment\start-lan-preview.bat and type SHARE at the explicit confirmation."
    }
}
finally {
    if ($hadRuntimeMode) {
        $env:TOKEN_TRACKER_RUNTIME_MODE = $originalRuntimeMode
    }
    else {
        Remove-Item Env:TOKEN_TRACKER_RUNTIME_MODE -ErrorAction SilentlyContinue
    }
}

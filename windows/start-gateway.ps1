<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Provide a safe, short Windows entry point for the local usage Gateway.
Module: Windows runtime / external client integration boundary

This wrapper never accepts provider keys or ingest tokens as parameters. The
Python CLI loads those values from the ignored .env/process environment.
Environment names are TOKEN_TRACKER_GATEWAY_PROVIDER_KEY,
TOKEN_TRACKER_GATEWAY_INGEST_TOKEN, and optional TOKEN_TRACKER_GATEWAY_ACCESS_TOKEN.
When -IngestUrl is omitted, the wrapper discovers the first ready local center
service in the root launcher port window.
#>

[CmdletBinding()]
param(
    [string]$UpstreamUrl = "https://api.kimi.com/coding/v1",
    [string]$IngestUrl,
    [string]$BindHost = "127.0.0.1",
    [int]$BindPort = 8787,
    [switch]$AllowHttp,
    [switch]$MemoryOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$windowsRoot = $PSScriptRoot
$pythonPath = Join-Path $windowsRoot ".venv\Scripts\python.exe"
if (-not (Test-Path -LiteralPath $pythonPath -PathType Leaf)) {
    throw "Windows Python environment not found: $pythonPath"
}
if ($BindPort -lt 1 -or $BindPort -gt 65535) {
    throw "Gateway port must be between 1 and 65535"
}
if ([string]::IsNullOrWhiteSpace($BindHost)) {
    throw "Gateway bind host is required"
}
if ([string]::IsNullOrWhiteSpace($IngestUrl)) {
    $resolver = Join-Path $windowsRoot "deployment\resolve-center-url.ps1"
    if (-not (Test-Path -LiteralPath $resolver -PathType Leaf)) {
        throw "Center service resolver not found: $resolver"
    }
    $IngestUrl = [string](& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $resolver)
    if ([string]::IsNullOrWhiteSpace($IngestUrl)) {
        throw "Unable to discover a ready center service"
    }
    Write-Host "Automatically discovered center ingest endpoint: $IngestUrl"
}
if ($IngestUrl.StartsWith("http://", [System.StringComparison]::OrdinalIgnoreCase) -and -not $AllowHttp) {
    throw "HTTP ingest requires explicit -AllowHttp; production/shared use HTTPS"
}

$argumentList = @(
    "-m", "token_tracker", "gateway",
    "--upstream-url", $UpstreamUrl,
    "--ingest-url", $IngestUrl,
    "--host", $BindHost,
    "--port", $BindPort.ToString()
)
if ($AllowHttp) {
    $argumentList += "--allow-http"
}
if ($MemoryOnly) {
    $argumentList += "--memory-only"
}

Push-Location $windowsRoot
try {
    & $pythonPath @argumentList
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}

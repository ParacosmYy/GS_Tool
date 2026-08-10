<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Discover the healthy local center service without reading secrets or
         changing process, firewall, or database state.
Module: Windows deployment / local service discovery

The root launcher may move the current source service away from port 5000 when
an older process is already listening. This read-only probe keeps the Gateway
from silently reporting usage to that stale or unrelated local instance.
#>

[CmdletBinding()]
param(
    [string]$HostName = "127.0.0.1",
    [int]$StartPort = 5000,
    [int]$EndPort = 5020,
    [switch]$AsJson
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($HostName)) {
    throw "Center service host is required"
}
if ($StartPort -lt 1 -or $StartPort -gt 65535 -or $EndPort -lt 1 -or $EndPort -gt 65535) {
    throw "Center service ports must be between 1 and 65535"
}
if ($EndPort -lt $StartPort) {
    throw "EndPort cannot be earlier than StartPort"
}

function Test-TcpPort {
    param(
        [string]$TargetHost,
        [int]$Port
    )

    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        $connectTask = $client.ConnectAsync($TargetHost, $Port)
        if (-not $connectTask.Wait(150)) {
            return $false
        }
        return $client.Connected
    }
    catch {
        return $false
    }
    finally {
        $client.Dispose()
    }
}

function Test-ReadyResponse {
    param([string]$Url)

    try {
        $response = Invoke-WebRequest -Uri $Url -Method Get -UseBasicParsing -MaximumRedirection 0 -TimeoutSec 1
        if ($response.StatusCode -ne 200) {
            return $false
        }
        $payload = $response.Content | ConvertFrom-Json
        return ([string]$payload.status -eq "ready")
    }
    catch {
        return $false
    }
}

$candidate = $null
foreach ($port in $StartPort..$EndPort) {
    if (-not (Test-TcpPort -TargetHost $HostName -Port $port)) {
        continue
    }
    $readyUrl = "http://${HostName}:$port/api/v1/ready"
    if (Test-ReadyResponse -Url $readyUrl) {
        $candidate = [ordered]@{
            host = $HostName
            port = $port
            ready_url = $readyUrl
            ingest_url = "http://${HostName}:$port/api/v1/ingest/usage"
        }
        break
    }
}

if ($null -eq $candidate) {
    throw "No ready AI Token Tracker center found in ${HostName}:$StartPort-$EndPort"
}

if ($AsJson) {
    $candidate | ConvertTo-Json -Compress
}
else {
    $candidate.ingest_url
}

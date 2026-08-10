<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Download and verify the project-local Caddy edge binary.
Module: Deployment / edge toolchain provisioning boundary

This script writes only to the ignored project cache. It does not install a
Windows service, modify PATH, change firewall rules, change ACLs, request a
certificate, or start Caddy.
#>

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$version = "2.11.4"
$archiveName = "caddy_2.11.4_windows_amd64.zip"
$archiveUrl = "https://github.com/caddyserver/caddy/releases/download/v2.11.4/$archiveName"
$expectedSha512 = "CD5CCFD86A4B40732CF715890D0DCA5BF3F63ADEFEC5A7914DE85ADF240C60CE7E5D2791631B88EF9758E46B23BB1730E020B9C5D696889740B284FFD4788E35"
$projectRoot = Split-Path -Parent $PSScriptRoot
$cacheRoot = Join-Path $projectRoot ".cache\caddy\$version"
$archivePath = Join-Path $cacheRoot $archiveName
$caddyPath = Join-Path $cacheRoot "caddy.exe"

New-Item -ItemType Directory -Path $cacheRoot -Force | Out-Null

if (-not (Test-Path -LiteralPath $archivePath -PathType Leaf) -or (Get-Item -LiteralPath $archivePath).Length -eq 0) {
    if (Test-Path -LiteralPath $archivePath -PathType Leaf) {
        Remove-Item -LiteralPath $archivePath -Force
    }
    $curl = Get-Command curl.exe -CommandType Application -ErrorAction SilentlyContinue
    if (-not $curl) {
        throw "Windows curl.exe is required to download the pinned Caddy archive."
    }
    Write-Host "Downloading Caddy $version to the project cache..."
    & $curl.Source --fail --location --silent --show-error --retry 3 --connect-timeout 10 --max-time 300 --output $archivePath $archiveUrl
    if ($LASTEXITCODE -ne 0) {
        throw "Caddy archive download failed with exit code $LASTEXITCODE."
    }
}

$actualSha512 = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA512).Hash.ToUpperInvariant()
if ($actualSha512 -ne $expectedSha512) {
    Remove-Item -LiteralPath $archivePath -Force
    throw "Caddy archive SHA-512 mismatch. Expected $expectedSha512, got $actualSha512."
}

Expand-Archive -LiteralPath $archivePath -DestinationPath $cacheRoot -Force
if (-not (Test-Path -LiteralPath $caddyPath -PathType Leaf)) {
    throw "Verified Caddy archive did not contain caddy.exe: $caddyPath"
}

Write-Host "Caddy $version ready: $caddyPath"
Write-Host "Archive SHA-512: $actualSha512"
Write-Host "Next step: run deployment\start-edge.ps1 after configuring the domain and log ACL."

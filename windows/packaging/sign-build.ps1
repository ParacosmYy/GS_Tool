<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Sign a Windows onedir build before package manifest generation.
Module: Delivery / Authenticode signing boundary

This script intentionally requires an explicitly selected certificate and
timestamp service. It never creates certificates, exports private keys, or
falls back to a self-signed certificate. Run package.ps1 only after signing.
#>

[CmdletBinding()]
param(
    [string]$BuildDirectory = (Join-Path $PSScriptRoot "..\dist\AI-Token-Tracker"),
    [Parameter(Mandatory = $true)]
    [string]$CertificateThumbprint,
    [Parameter(Mandatory = $true)]
    [uri]$TimestampUrl,
    [string]$SigntoolPath = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$buildRoot = (Resolve-Path -LiteralPath $BuildDirectory).Path
$normalizedThumbprint = ($CertificateThumbprint -replace "\s", "").ToUpperInvariant()
if ($normalizedThumbprint -notmatch "^[0-9A-F]{40}$") {
    throw "CertificateThumbprint must be a 40-character SHA-1 thumbprint."
}
if ($TimestampUrl.Scheme -ne "https") {
    throw "TimestampUrl must use HTTPS; an unencrypted timestamp service is rejected."
}

$certificate = Get-ChildItem Cert:\CurrentUser\My |
    Where-Object { $_.Thumbprint -eq $normalizedThumbprint } |
    Select-Object -First 1
if ($null -eq $certificate) {
    throw "The selected certificate is not present in Cert:\CurrentUser\My."
}
if (-not $certificate.HasPrivateKey) {
    throw "The selected certificate has no private key and cannot sign."
}
if ($certificate.NotAfter -lt (Get-Date)) {
    throw "The selected certificate is expired."
}
$codeSigningOid = "1.3.6.1.5.5.7.3.3"
$eku = $certificate.Extensions |
    Where-Object { $_.Oid.Value -eq "2.5.29.37" } |
    Select-Object -First 1
if ($null -eq $eku -or $eku.EnhancedKeyUsages.ObjectId.Value -notcontains $codeSigningOid) {
    throw "The selected certificate does not advertise the Code Signing EKU."
}

if ([string]::IsNullOrWhiteSpace($SigntoolPath)) {
    $command = Get-Command signtool.exe -ErrorAction SilentlyContinue
    if ($null -eq $command) {
        throw "signtool.exe is missing. Install an approved Windows SDK in the build environment."
    }
    $SigntoolPath = $command.Source
} else {
    $SigntoolPath = (Resolve-Path -LiteralPath $SigntoolPath).Path
}

$targets = @(Get-ChildItem -LiteralPath $buildRoot -Recurse -File |
    Where-Object { $_.Extension.ToLowerInvariant() -in @(".exe", ".dll", ".pyd") })
if ($targets.Count -eq 0) {
    throw "No Windows PE targets were found under $buildRoot."
}

foreach ($target in $targets) {
    & $SigntoolPath sign `
        /sha1 $normalizedThumbprint `
        /fd SHA256 `
        /tr $TimestampUrl.AbsoluteUri `
        /td SHA256 `
        $target.FullName
    $signExitCode = 0
    if (Test-Path variable:LASTEXITCODE) {
        $signExitCode = [int]$LASTEXITCODE
    }
    if ($signExitCode -ne 0) {
        throw "signtool failed for $($target.FullName) with exit code $signExitCode."
    }
}

Write-Host "Signed $($targets.Count) PE files with SHA256 and HTTPS timestamping."
Write-Host "Certificate thumbprint: $normalizedThumbprint"
Write-Host "Next step: run packaging\package.ps1 so RELEASE-MANIFEST.json captures the signed EXE hash."

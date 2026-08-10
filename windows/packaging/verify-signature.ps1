<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Verify Authenticode signatures for every packaged Windows PE file.
Module: Delivery / package trust boundary

This command is read-only. It treats anything other than a trusted Valid
signature as failure and never turns an unsigned or self-signed build into a
release claim.
#>

[CmdletBinding()]
param(
    [string]$PackageDirectory = $PSScriptRoot,
    [string]$ExpectedThumbprint = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$packageRoot = (Resolve-Path -LiteralPath $PackageDirectory).Path
$targets = @(Get-ChildItem -LiteralPath $packageRoot -Recurse -File |
    Where-Object { $_.Extension.ToLowerInvariant() -in @(".exe", ".dll", ".pyd") })
if ($targets.Count -eq 0) {
    throw "No Windows PE targets were found under $packageRoot."
}

$normalizedThumbprint = ($ExpectedThumbprint -replace "\s", "").ToUpperInvariant()
if ($normalizedThumbprint -and $normalizedThumbprint -notmatch "^[0-9A-F]{40}$") {
    throw "ExpectedThumbprint must be a 40-character SHA-1 thumbprint."
}

$failures = [System.Collections.Generic.List[string]]::new()
foreach ($target in $targets) {
    $signature = Get-AuthenticodeSignature -LiteralPath $target.FullName
    if ([string]$signature.Status -ne "Valid") {
        $failures.Add("$($target.FullName): status=$($signature.Status)")
        continue
    }
    if ($normalizedThumbprint) {
        $actualThumbprint = [string]$signature.SignerCertificate.Thumbprint
        if ($actualThumbprint.ToUpperInvariant() -ne $normalizedThumbprint) {
            $failures.Add("$($target.FullName): signer thumbprint mismatch")
        }
    }
}

if ($failures.Count -gt 0) {
    $detail = $failures -join "`n"
    throw "Authenticode verification failed for $($failures.Count) of $($targets.Count) PE files:`n$detail"
}

Write-Host "Authenticode verification passed: files=$($targets.Count)"
if ($normalizedThumbprint) {
    Write-Host "Signer thumbprint: $normalizedThumbprint"
}

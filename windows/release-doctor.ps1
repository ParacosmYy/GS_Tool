<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Compose read-only source, toolchain, and deployment preflight checks.
Module: Release governance / delivery composition boundary

This script intentionally does not install software, start the application,
change firewall/ACL state, create databases, or request certificates. Exit
code 0 means all requested checks pass; 3 means one or more external gates
remain pending; 2 means at least one check failed.
#>

[CmdletBinding()]
param(
    [ValidateSet("Local", "LanPreview", "Production")]
    [string]$Mode = "Local",
    [string]$Caddyfile = "",
    [string]$LogsDirectory = ""
)

$ErrorActionPreference = "Continue"
$windowsRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$python = Join-Path $windowsRoot ".venv\Scripts\python.exe"
$results = [System.Collections.Generic.List[object]]::new()

function Add-Result {
    param(
        [string]$Name,
        [ValidateSet("pass", "pending", "fail")]
        [string]$Status,
        [string]$Detail
    )

    $results.Add([pscustomobject]@{
            name = $Name
            status = $Status
            detail = $Detail
        })
    $label = $Status.ToUpperInvariant().PadRight(7)
    Write-Host "[$label] $Name - $Detail"
}

function Invoke-CommandCheck {
    param(
        [string]$Name,
        [scriptblock]$Command,
        [ValidateSet("pass", "pending")]
        [string]$NonZeroStatus = "pending"
    )

    & $Command
    $exitCode = if ($null -eq $LASTEXITCODE) { 0 } else { $LASTEXITCODE }
    if ($exitCode -eq 0) {
        Add-Result -Name $Name -Status "pass" -Detail "命令通过"
        return
    }
    $status = if ($NonZeroStatus -eq "pass") { "fail" } else { $NonZeroStatus }
    Add-Result -Name $Name -Status $status -Detail "exit=$exitCode"
}

Write-Host "AI Token Tracker release doctor: mode=$Mode"

if (-not (Test-Path -LiteralPath $python -PathType Leaf)) {
    Add-Result -Name "source-runtime" -Status "fail" -Detail "windows/.venv Python 不存在"
} else {
    $auditRaw = & $python -m token_tracker audit --json 2>&1
    $auditExit = if ($null -eq $LASTEXITCODE) { 0 } else { $LASTEXITCODE }
    try {
        $audit = ($auditRaw -join "`n") | ConvertFrom-Json
        $summary = $audit.summary
        if ($auditExit -ne 0 -or [int]$summary.fail -gt 0) {
            Add-Result -Name "source-audit" -Status "fail" -Detail "pass=$($summary.pass) pending=$($summary.pending) fail=$($summary.fail)"
        } elseif ([int]$summary.pending -gt 0) {
            Add-Result -Name "source-audit" -Status "pending" -Detail "pass=$($summary.pass) pending=$($summary.pending)"
        } else {
            Add-Result -Name "source-audit" -Status "pass" -Detail "所有源码审计通过"
        }
    } catch {
        Add-Result -Name "source-audit" -Status "fail" -Detail "审计 JSON 无法解析"
    }
}

Invoke-CommandCheck -Name "local-preflight" -Command {
    & $python -m token_tracker preflight --host 127.0.0.1
}

Invoke-CommandCheck -Name "exe-toolchain" -Command {
    & (Join-Path $windowsRoot "packaging\toolchain-doctor.ps1")
}

Invoke-CommandCheck -Name "android-toolchain" -Command {
    & (Join-Path (Split-Path -Parent $windowsRoot) "android\toolchain-doctor.ps1")
}

if ($Mode -eq "LanPreview") {
    Invoke-CommandCheck -Name "lan-deployment" -Command {
        & (Join-Path $windowsRoot "deployment\share-doctor.ps1") -Mode LanPreview
    }
}

if ($Mode -eq "Production") {
    if ([string]::IsNullOrWhiteSpace($Caddyfile) -or [string]::IsNullOrWhiteSpace($LogsDirectory)) {
        Add-Result -Name "production-deployment" -Status "fail" -Detail "Production 必须同时提供 -Caddyfile 和 -LogsDirectory"
    } else {
        Invoke-CommandCheck -Name "production-deployment" -Command {
            & (Join-Path $windowsRoot "deployment\share-doctor.ps1") `
                -Mode Production `
                -Caddyfile $Caddyfile `
                -LogsDirectory $LogsDirectory
        }
    }
}

$failed = @($results | Where-Object { $_.status -eq "fail" }).Count
$pending = @($results | Where-Object { $_.status -eq "pending" }).Count
$passed = @($results | Where-Object { $_.status -eq "pass" }).Count
Write-Host "release-doctor summary: pass=$passed pending=$pending fail=$failed"

if ($failed -gt 0) {
    exit 2
}
if ($pending -gt 0) {
    exit 3
}
exit 0

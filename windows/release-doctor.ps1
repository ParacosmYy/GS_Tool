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
    [string]$LogsDirectory = "",
    [switch]$CheckBackups,
    [string]$BackupDirectory = "",
    [int]$MinBackupCount = -1,
    [int]$MaxBackupAgeDays = -1,
    [int]$MaxBackupSizeMiB = -1,
    [switch]$VerifyBackups
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
        Add-Result -Name $Name -Status "pass" -Detail "command passed"
        return
    }
    $status = if ($NonZeroStatus -eq "pass") { "fail" } else { $NonZeroStatus }
    Add-Result -Name $Name -Status $status -Detail "exit=$exitCode"
}

function Invoke-BackupInventoryCheck {
    if (-not $CheckBackups) {
        return
    }

    $invalidPolicy = @(
        @("MinBackupCount", $MinBackupCount),
        @("MaxBackupAgeDays", $MaxBackupAgeDays),
        @("MaxBackupSizeMiB", $MaxBackupSizeMiB)
    ) | Where-Object { $_[1] -lt -1 }
    if ($invalidPolicy.Count -gt 0) {
        $names = ($invalidPolicy | ForEach-Object { $_[0] }) -join ", "
        Add-Result -Name "backup-inventory" -Status "fail" -Detail "policy values must be -1 or non-negative: $names"
        return
    }

    $backupArguments = @("-m", "token_tracker", "backup-inventory")
    if (-not [string]::IsNullOrWhiteSpace($BackupDirectory)) {
        $backupArguments += @("--output-dir", $BackupDirectory)
    }
    if ($MinBackupCount -ge 0) {
        $backupArguments += @("--min-count", $MinBackupCount.ToString())
    }
    if ($MaxBackupAgeDays -ge 0) {
        $backupArguments += @("--max-age-days", $MaxBackupAgeDays.ToString())
    }
    if ($MaxBackupSizeMiB -ge 0) {
        $backupArguments += @("--max-size-mib", $MaxBackupSizeMiB.ToString())
    }
    if ($VerifyBackups) {
        $backupArguments += "--verify"
    }

    Invoke-CommandCheck -Name "backup-inventory" -Command {
        & $python @backupArguments
    } -NonZeroStatus "pass"
}

Write-Host "AI Token Tracker release doctor: mode=$Mode"

if (-not (Test-Path -LiteralPath $python -PathType Leaf)) {
    Add-Result -Name "source-runtime" -Status "fail" -Detail "windows/.venv Python is missing"
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
            Add-Result -Name "source-audit" -Status "pass" -Detail "source audit passed"
        }
    } catch {
        Add-Result -Name "source-audit" -Status "fail" -Detail "audit JSON could not be parsed"
    }
}

Invoke-CommandCheck -Name "local-preflight" -Command {
    & $python -m token_tracker preflight --host 127.0.0.1
}

Invoke-BackupInventoryCheck

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
        Add-Result -Name "production-deployment" -Status "fail" -Detail "Production requires both -Caddyfile and -LogsDirectory"
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

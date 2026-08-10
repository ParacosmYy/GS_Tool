<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Verify EXE package upgrade and rollback while preserving user data.
Module: Delivery / release transition boundary

The command launches only the two explicitly supplied package directories. It
uses a fresh project-cache LocalAppData boundary, creates one synthetic release
record through the public API, and never deletes user data or package files.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PreviousPackageDirectory,
    [Parameter(Mandatory = $true)]
    [string]$CurrentPackageDirectory,
    [string]$PreviousAsset = "embedded-rust-engineer-bg-v12.png",
    [string]$CurrentAsset = "embedded-rust-engineer-bg-v13.png",
    [int]$Port = 5020,
    [string]$RunDirectory = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$windowsRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$previousRoot = (Resolve-Path -LiteralPath $PreviousPackageDirectory).Path
$currentRoot = (Resolve-Path -LiteralPath $CurrentPackageDirectory).Path
$timestamp = Get-Date -Format "yyyyMMddHHmmss"
if ([string]::IsNullOrWhiteSpace($RunDirectory)) {
    $RunDirectory = Join-Path $windowsRoot ".cache\exe-upgrade-rollback-$timestamp"
}
$runRoot = [IO.Path]::GetFullPath($RunDirectory)
$localAppData = Join-Path $runRoot "localappdata"
$baseUri = "http://127.0.0.1:$Port"
$activeProcess = $null
$savedEnvironment = @{}

function Assert-Package {
    param([string]$PackageRoot)

    $verifier = Join-Path $PackageRoot "VERIFY-PACKAGE.ps1"
    if (-not (Test-Path -LiteralPath $verifier -PathType Leaf)) {
        throw "Package verifier is missing: $verifier"
    }
    & $verifier -PackageDirectory $PackageRoot -ExpectedVersion "0.1.0"
    $verifyExitCode = 0
    if (Test-Path variable:LASTEXITCODE) {
        $verifyExitCode = [int]$LASTEXITCODE
    }
    if ($verifyExitCode -ne 0) {
        throw "Package verification failed: $PackageRoot"
    }
    $exe = Join-Path $PackageRoot "AI-Token-Tracker.exe"
    if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
        throw "Package executable is missing: $exe"
    }
}

function Assert-Asset {
    param(
        [string]$PackageRoot,
        [string]$AssetName
    )

    $asset = Join-Path $PackageRoot ("_internal\token_tracker\static\assets\$AssetName")
    if (-not (Test-Path -LiteralPath $asset -PathType Leaf)) {
        throw "Expected packaged scene asset is missing: $asset"
    }
    return Get-Item -LiteralPath $asset
}

function Assert-PortFree {
    $listeners = @(Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue)
    if ($listeners.Count -gt 0) {
        throw "Refusing release verification: port $Port is already in use."
    }
}

function Wait-PortFree {
    foreach ($attempt in 1..20) {
        $listeners = @(Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue)
        if ($listeners.Count -eq 0) {
            return
        }
        Start-Sleep -Milliseconds 250
    }
    throw "Port $Port was not released after stopping the owned EXE."
}

function Wait-Ready {
    param([Diagnostics.Process]$Process)

    foreach ($attempt in 1..40) {
        if ($Process.HasExited) {
            throw "EXE exited before readiness: pid=$($Process.Id)"
        }
        try {
            $response = Invoke-WebRequest -UseBasicParsing -Uri "$baseUri/api/v1/ready" -TimeoutSec 2
            if ($response.StatusCode -eq 200) {
                return
            }
        } catch {
            # The server may still be binding its loopback socket.
        }
        Start-Sleep -Milliseconds 250
    }
    throw "EXE did not become ready on $baseUri."
}

function Start-Package {
    param([string]$PackageRoot)

    Assert-PortFree
    $env:LOCALAPPDATA = $localAppData
    $env:TOKEN_TRACKER_HOST = "127.0.0.1"
    $env:TOKEN_TRACKER_PORT = $Port.ToString()
    $env:TOKEN_TRACKER_NO_BROWSER = "1"
    Remove-Item Env:TOKEN_TRACKER_DB -ErrorAction SilentlyContinue
    $exe = Join-Path $PackageRoot "AI-Token-Tracker.exe"
    $script:activeProcess = Start-Process `
        -FilePath $exe `
        -WorkingDirectory $PackageRoot `
        -PassThru `
        -WindowStyle Hidden
    Wait-Ready -Process $script:activeProcess
}

function Stop-Package {
    if ($null -ne $script:activeProcess) {
        if (-not $script:activeProcess.HasExited) {
            Stop-Process -Id $script:activeProcess.Id -Force
        }
        $script:activeProcess = $null
        Wait-PortFree
    }
}

function Register-ReleaseAccount {
    $session = New-Object Microsoft.PowerShell.Commands.WebRequestSession
    $page = Invoke-WebRequest -UseBasicParsing -Uri "$baseUri/register" -WebSession $session -TimeoutSec 5
    $match = [regex]::Match($page.Content, 'name="csrf_token"\s+value="([^"]+)"')
    if (-not $match.Success) {
        throw "Registration page did not expose a CSRF token."
    }
    $username = "release_" + ([Guid]::NewGuid().ToString("N").Substring(0, 12))
    $password = "Release-" + ([Guid]::NewGuid().ToString("N"))
    $form = @{
        csrf_token = $match.Groups[1].Value
        username = $username
        password = $password
    }
    Invoke-WebRequest -UseBasicParsing -Method Post -Uri "$baseUri/register" -WebSession $session -Body $form -TimeoutSec 5 | Out-Null
    return @{
        username = $username
        password = $password
    }
}

function Get-AccessToken {
    param([hashtable]$Account)

    $body = @{ username = $Account.username; password = $Account.password } | ConvertTo-Json -Compress
    $response = Invoke-RestMethod `
        -Method Post `
        -Uri "$baseUri/api/v1/auth/login" `
        -Body $body `
        -ContentType "application/json" `
        -TimeoutSec 5
    if ([string]::IsNullOrWhiteSpace([string]$response.access_token)) {
        throw "Token exchange did not return an access token."
    }
    return [string]$response.access_token
}

function Write-ReleaseRecord {
    param([string]$AccessToken)

    $headers = @{
        Authorization = "Bearer $AccessToken"
        "Idempotency-Key" = [Guid]::NewGuid().ToString()
    }
    $body = @{
        model = "kimi-code"
        input_tokens = 222
        output_tokens = 333
        timestamp = (Get-Date).ToString("yyyy-MM-ddTHH:mm")
        note = "isolated EXE upgrade rollback evidence"
    } | ConvertTo-Json -Compress
    $response = Invoke-RestMethod `
        -Method Post `
        -Uri "$baseUri/api/v1/records" `
        -Headers $headers `
        -Body $body `
        -ContentType "application/json" `
        -TimeoutSec 5
    if ($null -eq $response.record) {
        throw "Record write did not return a record projection."
    }
    return $response.record
}

function Read-ReleaseRecord {
    param([string]$AccessToken, [int]$ExpectedId)

    $headers = @{ Authorization = "Bearer $AccessToken" }
    $response = Invoke-RestMethod `
        -Method Get `
        -Uri "$baseUri/api/v1/records?period=all&limit=50&offset=0" `
        -Headers $headers `
        -TimeoutSec 5
    $record = @($response.records | Where-Object { [int]$_.id -eq $ExpectedId })
    if ($record.Count -ne 1) {
        throw "Expected record id=$ExpectedId was not returned after transition."
    }
    if ([int]$record[0].input_tokens -ne 222 -or [int]$record[0].output_tokens -ne 333) {
        throw "Record id=$ExpectedId changed during package transition."
    }
    return $record[0]
}

function Assert-ServedAsset {
    param([string]$AssetName)

    $response = Invoke-WebRequest -UseBasicParsing -Uri "$baseUri/static/assets/$AssetName" -TimeoutSec 5
    if ($response.StatusCode -ne 200) {
        throw "Packaged asset did not return 200: $AssetName"
    }
    return [int]$response.RawContentLength
}

function Save-Environment {
    foreach ($name in @("LOCALAPPDATA", "TOKEN_TRACKER_HOST", "TOKEN_TRACKER_PORT", "TOKEN_TRACKER_NO_BROWSER", "TOKEN_TRACKER_DB")) {
        $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, "Process")
    }
}

function Restore-Environment {
    foreach ($name in $savedEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], "Process")
    }
}

try {
    New-Item -ItemType Directory -Path $localAppData -Force | Out-Null
    Assert-Package -PackageRoot $previousRoot
    Assert-Package -PackageRoot $currentRoot
    $previousAssetInfo = Assert-Asset -PackageRoot $previousRoot -AssetName $PreviousAsset
    $currentAssetInfo = Assert-Asset -PackageRoot $currentRoot -AssetName $CurrentAsset
    Save-Environment

    Start-Package -PackageRoot $previousRoot
    $account = Register-ReleaseAccount
    $previousToken = Get-AccessToken -Account $account
    $record = Write-ReleaseRecord -AccessToken $previousToken
    $recordId = [int]$record.id
    $previousAssetBytes = Assert-ServedAsset -AssetName $PreviousAsset
    Stop-Package

    Start-Package -PackageRoot $currentRoot
    $currentToken = Get-AccessToken -Account $account
    $currentRecord = Read-ReleaseRecord -AccessToken $currentToken -ExpectedId $recordId
    $currentAssetBytes = Assert-ServedAsset -AssetName $CurrentAsset
    Stop-Package

    Start-Package -PackageRoot $previousRoot
    $rollbackToken = Get-AccessToken -Account $account
    $rollbackRecord = Read-ReleaseRecord -AccessToken $rollbackToken -ExpectedId $recordId
    $rollbackAssetBytes = Assert-ServedAsset -AssetName $PreviousAsset
    Stop-Package

    Write-Host "EXE upgrade/rollback verification passed."
    Write-Host "record_id=$recordId input_tokens=$($rollbackRecord.input_tokens) output_tokens=$($rollbackRecord.output_tokens)"
    Write-Host "previous_asset_bytes=$previousAssetBytes current_asset_bytes=$currentAssetBytes rollback_asset_bytes=$rollbackAssetBytes"
    Write-Host "package_previous_asset_bytes=$($previousAssetInfo.Length) package_current_asset_bytes=$($currentAssetInfo.Length)"
    Write-Host "isolated_data_directory=$localAppData\AITokenTracker"
} finally {
    Stop-Package
    Restore-Environment
}

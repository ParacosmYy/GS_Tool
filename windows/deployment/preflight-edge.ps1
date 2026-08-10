<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Validate the Caddy edge boundary before an approved HTTPS launch.
Module: Deployment / edge preflight boundary

This script is read-only. It does not install Caddy, request certificates,
change firewall rules, change ACLs, or start a server.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ConfigPath,

    [Parameter(Mandatory = $true)]
    [string]$LogsDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-ExistingFile([string]$PathValue, [string]$Label) {
    $resolved = Resolve-Path -LiteralPath $PathValue -ErrorAction SilentlyContinue
    if (-not $resolved -or -not (Test-Path -LiteralPath $resolved.Path -PathType Leaf)) {
        throw "$Label 不存在或不是文件：$PathValue"
    }
    return $resolved.Path
}

function Resolve-ExistingDirectory([string]$PathValue, [string]$Label) {
    $resolved = Resolve-Path -LiteralPath $PathValue -ErrorAction SilentlyContinue
    if (-not $resolved -or -not (Test-Path -LiteralPath $resolved.Path -PathType Container)) {
        throw "$Label 不存在或不是目录：$PathValue"
    }
    return $resolved.Path
}

function Assert-NoBroadWriteAcl([string]$PathValue) {
    $broadIdentities = @(
        "S-1-1-0",             # Everyone
        "S-1-5-32-545",        # BUILTIN\Users
        "Everyone",
        "BUILTIN\Users"
    )
    $writeRights = [System.Security.AccessControl.FileSystemRights]::WriteData -bor `
        [System.Security.AccessControl.FileSystemRights]::AppendData -bor `
        [System.Security.AccessControl.FileSystemRights]::CreateFiles -bor `
        [System.Security.AccessControl.FileSystemRights]::CreateDirectories -bor `
        [System.Security.AccessControl.FileSystemRights]::WriteAttributes -bor `
        [System.Security.AccessControl.FileSystemRights]::WriteExtendedAttributes -bor `
        [System.Security.AccessControl.FileSystemRights]::Delete -bor `
        [System.Security.AccessControl.FileSystemRights]::DeleteSubdirectoriesAndFiles -bor `
        [System.Security.AccessControl.FileSystemRights]::ChangePermissions -bor `
        [System.Security.AccessControl.FileSystemRights]::TakeOwnership -bor `
        [System.Security.AccessControl.FileSystemRights]::FullControl
    $acl = Get-Acl -LiteralPath $PathValue
    foreach ($entry in $acl.Access) {
        $identity = $entry.IdentityReference.Value
        $rights = $entry.FileSystemRights.ToString()
        $isBroad = $broadIdentities -contains $identity
        $canWrite = (($entry.FileSystemRights -band $writeRights) -ne 0)
        if ($entry.AccessControlType -eq "Allow" -and $isBroad -and $canWrite) {
            throw "日志路径存在宽泛写权限：identity=$identity rights=$rights path=$PathValue"
        }
    }
}

$caddy = Get-Command caddy -CommandType Application -ErrorAction SilentlyContinue
if (-not $caddy) {
    throw "找不到 caddy。请先由部署负责人安装并校验 Caddy，再运行边缘预检。"
}

$config = Resolve-ExistingFile $ConfigPath "Caddy 配置"
$logs = Resolve-ExistingDirectory $LogsDirectory "Caddy 日志目录"
Assert-NoBroadWriteAcl $logs
Get-ChildItem -LiteralPath $logs -File -Force | ForEach-Object {
    Assert-NoBroadWriteAcl $_.FullName
}

& $caddy.Source validate --config $config --adapter caddyfile
if ($LASTEXITCODE -ne 0) {
    throw "Caddyfile 校验失败，拒绝进入 HTTPS 启动步骤。"
}

Write-Host "Edge preflight passed: config=$config logs=$logs"
Write-Host "该检查未申请证书、未修改 ACL/防火墙、未启动 Caddy。"

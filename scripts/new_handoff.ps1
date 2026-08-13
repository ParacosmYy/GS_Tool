[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $HandoffId,
    [Parameter(Mandatory = $true)] [string] $Delivery,
    [string] $Owner = "architect",
    [ValidateSet("in-progress", "accepted-with-limits", "completed", "blocked")]
    [string] $Status = "in-progress"
)

$ErrorActionPreference = "Stop"

if ($HandoffId -notmatch '^[a-z0-9][a-z0-9._-]*$') {
    throw "HandoffId must match ^[a-z0-9][a-z0-9._-]*$"
}
if ([string]::IsNullOrWhiteSpace($Delivery) -or [string]::IsNullOrWhiteSpace($Owner)) {
    throw "Delivery and Owner must be non-empty."
}

$root = (Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$handoffDirectory = Join-Path $root ("docs\handoffs\" + $HandoffId)
$handoffPath = Join-Path $handoffDirectory "handoff.md"
$templatePath = Join-Path $root "docs\handoffs\HANDOFF_TEMPLATE.md"
$indexPath = Join-Path $root "docs\handoffs\index.json"

if (Test-Path -LiteralPath $handoffPath) {
    throw "Handoff already exists: docs/handoffs/$HandoffId/handoff.md"
}
if (-not (Test-Path -LiteralPath $templatePath)) {
    throw "Handoff template is missing: docs/handoffs/HANDOFF_TEMPLATE.md"
}
if (-not (Test-Path -LiteralPath $indexPath)) {
    throw "Handoff index is missing: docs/handoffs/index.json"
}

New-Item -ItemType Directory -Path $handoffDirectory -Force | Out-Null
$createdAt = (Get-Date).ToString("o")
$content = Get-Content -Raw -LiteralPath $templatePath
$content = $content.Replace("<HANDOFF_ID>", $HandoffId)
$content = $content.Replace("<DELIVERY>", $Delivery)
$content = $content.Replace("<STATUS>", $Status)
$content = $content.Replace("<OWNER>", $Owner)
$content = $content.Replace("<CREATED_AT>", $createdAt)
Set-Content -LiteralPath $handoffPath -Value $content -Encoding utf8

$index = Get-Content -Raw -LiteralPath $indexPath | ConvertFrom-Json
$entries = @($index.entries)
if (@($entries | Where-Object { [string]$_.id -eq $HandoffId }).Count -gt 0) {
    throw "Handoff ID is already present in the index: $HandoffId"
}
$relativePath = "docs/handoffs/$HandoffId/handoff.md"
$entries += [pscustomobject]@{
    id = $HandoffId
    path = $relativePath
    delivery = $Delivery
    status = $Status
    owner = $Owner
    created_at = $createdAt
    acceptance_ids = @()
}
$index.entries = $entries
$index.latest_id = $HandoffId
$index.latest_path = $relativePath
Set-Content -LiteralPath $indexPath -Value ($index | ConvertTo-Json -Depth 10) -Encoding utf8

Write-Host "Created $relativePath"
Write-Host "Update docs/agent-team/delivery-register.json and the handoff evidence before running scripts/check.ps1."

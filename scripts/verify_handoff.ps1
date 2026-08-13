$ErrorActionPreference = "Stop"

$root = (Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
Set-Location -LiteralPath $root

function Read-JsonFile {
    param([Parameter(Mandatory = $true)] [string] $RelativePath)

    $path = Join-Path $root ($RelativePath -replace '/', '\')
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required JSON file is missing: $RelativePath"
    }
    try {
        return Get-Content -Raw -LiteralPath $path | ConvertFrom-Json
    }
    catch {
        throw "Invalid JSON in ${RelativePath}: $($_.Exception.Message)"
    }
}

function Normalize-RelativePath {
    param([Parameter(Mandatory = $true)] [string] $Value)

    $normalized = $Value.Replace('\', '/')
    if ([string]::IsNullOrWhiteSpace($normalized) -or [System.IO.Path]::IsPathRooted($normalized.Replace('/', '\'))) {
        throw "Handoff paths must be non-empty relative paths: $Value"
    }
    if ($normalized -match '(^|/)\.\.?(/|$)') {
        throw "Handoff paths may not contain traversal segments: $Value"
    }
    return $normalized.TrimStart('/')
}

function Assert-ExactList {
    param(
        [Parameter(Mandatory = $true)] [object[]] $Actual,
        [Parameter(Mandatory = $true)] [string[]] $Expected,
        [Parameter(Mandatory = $true)] [string] $Label
    )

    if ($Actual.Count -ne $Expected.Count) {
        throw "$Label count is invalid. Expected $($Expected.Count), got $($Actual.Count)."
    }
    for ($index = 0; $index -lt $Expected.Count; $index++) {
        if ([string]$Actual[$index] -ne $Expected[$index]) {
            throw "$Label is invalid at position $index. Expected '$($Expected[$index])', got '$($Actual[$index])'."
        }
    }
}

$policy = Read-JsonFile "docs/agent-team/workflow-policy.json"
$index = Read-JsonFile "docs/handoffs/index.json"
$templatePath = Join-Path $root "docs\handoffs\HANDOFF_TEMPLATE.md"
if (-not (Test-Path -LiteralPath $templatePath)) {
    throw "Handoff template is missing: docs/handoffs/HANDOFF_TEMPLATE.md"
}

if ($policy.schema_version -ne "1.0" -or $policy.project -ne "QuillForge" -or $policy.owner -ne "architect" -or $policy.primary_agent -ne "architect") {
    throw "Workflow policy must be schema 1.0 and Architect-owned."
}

$expectedRoles = @("architect", "project-manager", "product", "developer-1", "developer-2", "qa")
Assert-ExactList @($policy.required_roles) $expectedRoles "Required team roles"

if (
    $policy.child_routing.default.route -ne "luna_max" -or
    $policy.child_routing.default.model -ne "gpt-5.6-luna" -or
    $policy.child_routing.default.reasoning_effort -ne "max" -or
    $policy.child_routing.default.service_tier -ne "Fast"
) {
    throw "The default child-agent route must be luna_max / gpt-5.6-luna / max / Fast."
}
Assert-ExactList @($policy.child_routing.allowed_routes) @("luna_max", "terra_max", "sol_medium") "Allowed child-agent routes"
if (
    @($policy.child_routing.escalations).Count -ne 2 -or
    $policy.child_routing.escalations[0].route -ne "terra_max" -or
    $policy.child_routing.escalations[0].model -ne "gpt-5.6-terra" -or
    $policy.child_routing.escalations[0].reasoning_effort -ne "max" -or
    $policy.child_routing.escalations[0].service_tier -ne "Fast" -or
    $policy.child_routing.escalations[1].route -ne "sol_medium" -or
    $policy.child_routing.escalations[1].model -ne "gpt-5.6-sol" -or
    $policy.child_routing.escalations[1].reasoning_effort -ne "medium" -or
    $policy.child_routing.escalations[1].service_tier -ne "Fast"
) {
    throw "Child-agent escalation routes do not match the project policy."
}

if (
    $policy.checkout.current_local_only -ne $true -or
    $policy.checkout.worktree_forbidden -ne $true -or
    $policy.checkout.single_shared_checkout_writer -ne $true -or
    $policy.checkout.parent_integrates -ne $true -or
    $policy.checkout.child_agents_read_only_by_default -ne $true
) {
    throw "Checkout and writer-safety constraints are missing or weakened."
}
if (
    $policy.verification.unit_tests_by_default -ne $false -or
    $policy.verification.test_assets_by_default -ne $false -or
    $policy.verification.parent_owns_final_verification -ne $true
) {
    throw "Default verification policy must remain non-test-asset-generating and parent-owned."
}
if (
    $policy.launch_policy.software_start_allowed -ne $false -or
    $policy.launch_policy.qt_window_allowed -ne $false -or
    $policy.launch_policy.requires_explicit_user_reversal -ne $true -or
    $policy.launch_policy.static_build_allowed -ne $true -or
    $policy.launch_policy.runtime_visual_review -ne "user-owned-and-unrun" -or
    $policy.launch_policy.runtime_startup_review -ne "user-owned-and-unrun"
) {
    throw "The no-launch policy is missing, weakened, or inconsistent."
}

$requiredHandoffSections = @($policy.handoff.required_sections)
if (
    $policy.handoff.required -ne $true -or
    $policy.handoff.file_name -ne "handoff.md" -or
    $policy.handoff.root -ne "docs/handoffs" -or
    $policy.handoff.template -ne "docs/handoffs/HANDOFF_TEMPLATE.md" -or
    $policy.handoff.index -ne "docs/handoffs/index.json" -or
    $policy.handoff.must_update_delivery_register -ne $true -or
    $requiredHandoffSections.Count -lt 1
) {
    throw "The handoff policy is incomplete or weakened."
}

if ($index.schema_version -ne "1.0" -or $index.project -ne "QuillForge" -or $index.owner -ne "architect") {
    throw "Handoff index must be schema 1.0 and Architect-owned."
}
$handoffRoot = Join-Path $root "docs\handoffs"
if (-not (Test-Path -LiteralPath $handoffRoot)) {
    throw "Handoff root is missing: docs/handoffs"
}

$entries = @($index.entries)
if ($entries.Count -lt 1) {
    throw "Handoff index must contain at least one entry."
}
$allowedHandoffStatuses = @("planned", "in-progress", "blocked", "accepted-with-limits", "completed")
$seenIds = @{}
$seenPaths = @{}
foreach ($entry in $entries) {
    if ([string]::IsNullOrWhiteSpace([string]$entry.id)) {
        throw "Handoff index contains an entry with no ID."
    }
    if ($allowedHandoffStatuses -notcontains [string]$entry.status) {
        throw "Handoff index contains an unsupported status for $($entry.id): $($entry.status)"
    }
    if ($seenIds.ContainsKey([string]$entry.id)) {
        throw "Handoff index contains a duplicate ID: $($entry.id)"
    }
    $seenIds[[string]$entry.id] = $true

    $relativePath = Normalize-RelativePath ([string]$entry.path)
    if (-not $relativePath.StartsWith("docs/handoffs/", [System.StringComparison]::OrdinalIgnoreCase) -or -not $relativePath.EndsWith("/handoff.md", [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Handoff paths must be docs/handoffs/<id>/handoff.md: $relativePath"
    }
    if ($seenPaths.ContainsKey($relativePath)) {
        throw "Handoff index contains a duplicate path: $relativePath"
    }
    $seenPaths[$relativePath] = $true

    $expectedId = Split-Path -Leaf (Split-Path -Parent $relativePath)
    if ([string]$entry.id -ne $expectedId) {
        throw "Handoff ID and directory disagree: $($entry.id) / $expectedId"
    }
    $absolutePath = Join-Path $root ($relativePath -replace '/', '\')
    if (-not (Test-Path -LiteralPath $absolutePath)) {
        throw "Indexed handoff file is missing: $relativePath"
    }
    $content = Get-Content -Raw -LiteralPath $absolutePath
    $titlePattern = '(?m)^# Handoff:\s*' + [regex]::Escape([string]$entry.id) + '\s*$'
    if ($content -notmatch $titlePattern) {
        throw "Handoff title is missing or does not match its index ID: $relativePath"
    }
    $statusMatches = [regex]::Matches($content, '(?m)^\| Status \| `([^`]+)` \|\r?$')
    if ($statusMatches.Count -ne 1) {
        throw "Handoff must contain exactly one well-formed status row: $relativePath"
    }
    $handoffStatus = $statusMatches[0].Groups[1].Value
    if (-not [string]::Equals([string]$entry.status, $handoffStatus, [System.StringComparison]::Ordinal)) {
        throw "Handoff status disagrees with its index entry: $($entry.id) index=$($entry.status) handoff=$handoffStatus"
    }
    foreach ($section in $requiredHandoffSections) {
        $sectionPattern = '(?m)^' + [regex]::Escape([string]$section) + '\s*$'
        if ($content -notmatch $sectionPattern) {
            throw "Handoff section is missing from ${relativePath}: $section"
        }
    }
}

$latestPath = Normalize-RelativePath ([string]$index.latest_path)
if ([string]$index.latest_id -ne $entries[0].id -and -not $seenIds.ContainsKey([string]$index.latest_id)) {
    throw "Handoff index latest_id is not present: $($index.latest_id)"
}
if (-not $seenPaths.ContainsKey($latestPath)) {
    throw "Handoff index latest_path is not present in entries: $latestPath"
}
$latestEntry = $entries | Where-Object { [string]$_.id -eq [string]$index.latest_id } | Select-Object -First 1
if ($null -eq $latestEntry -or (Normalize-RelativePath ([string]$latestEntry.path)) -ne $latestPath) {
    throw "Handoff latest_id and latest_path disagree."
}

$actualHandoffFiles = @(Get-ChildItem -LiteralPath $handoffRoot -Recurse -File -Filter "handoff.md")
foreach ($file in $actualHandoffFiles) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    if (-not $seenPaths.ContainsKey($relative)) {
        throw "Handoff file is not indexed: $relative"
    }
}

Write-Host "QuillForge handoff checks passed."

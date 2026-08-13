$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $root

if (-not (Test-Path -LiteralPath "uv.lock")) {
    throw "uv.lock is missing. Run 'uv sync' before running checks."
}

$projectVersionMatch = [regex]::Match(
    (Get-Content -Raw -LiteralPath (Join-Path $root "pyproject.toml")),
    '(?m)^version\s*=\s*"([^"]+)"'
)
$packageVersionMatch = [regex]::Match(
    (Get-Content -Raw -LiteralPath (Join-Path $root "src\quillforge\__init__.py")),
    '(?m)^__version__\s*=\s*"([^"]+)"'
)
if (-not $projectVersionMatch.Success -or -not $packageVersionMatch.Success) {
    throw "Canonical QuillForge version metadata is missing."
}
if ($projectVersionMatch.Groups[1].Value -ne $packageVersionMatch.Groups[1].Value) {
    throw "pyproject.toml and quillforge.__version__ disagree."
}
if (-not (Test-Path -LiteralPath (Join-Path $root "packaging\version_info.txt"))) {
    throw "Windows version resource is missing: packaging/version_info.txt"
}
if (-not (Test-Path -LiteralPath (Join-Path $root "docs\third_party\NOTICE.md"))) {
    throw "Third-party notice inventory is missing: docs/third_party/NOTICE.md"
}
& uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice docs/third_party/NOTICE.md
if ($LASTEXITCODE -ne 0) {
    throw "Third-party notice inventory is incomplete."
}
foreach ($requiredReleaseFile in @(
    "docs\adr\0012-bounded-recovery-handoff.md",
    "docs\adr\0011-release-distribution-decisions.md",
    "docs\RELEASE_HANDOFF.md",
    "docs\support\ISSUES.md",
    "docs\support\HANDOFF.md",
    "scripts\verify_clean_machine.ps1",
    "scripts\verify_interactive_startup.ps1",
    "scripts\verify_release_handoff.ps1"
)) {
    if (-not (Test-Path -LiteralPath (Join-Path $root $requiredReleaseFile))) {
        throw "Release handoff contract file is missing: $requiredReleaseFile"
    }
}

foreach ($requiredWorkflowFile in @(
    "AGENTS.md",
    "docs\agent-team\workflow-policy.json",
    "docs\agent-team\TEAM.md",
    "docs\handoffs\README.md",
    "docs\handoffs\HANDOFF_TEMPLATE.md",
    "docs\handoffs\index.json",
    "scripts\verify_handoff.ps1",
    "scripts\new_handoff.ps1"
)) {
    if (-not (Test-Path -LiteralPath (Join-Path $root $requiredWorkflowFile))) {
        throw "Project workflow contract file is missing: $requiredWorkflowFile"
    }
}

& (Join-Path $root "scripts\verify_handoff.ps1")

$acceptancePath = Join-Path $root "docs\agent-team\acceptance.json"
if (-not (Test-Path -LiteralPath $acceptancePath)) {
    throw "docs\agent-team\acceptance.json is missing."
}

$deliveryPath = Join-Path $root "docs\agent-team\delivery-register.json"
if (-not (Test-Path -LiteralPath $deliveryPath)) {
    throw "docs/agent-team/delivery-register.json is missing."
}

$acceptance = Get-Content -Raw -LiteralPath $acceptancePath | ConvertFrom-Json
if ($acceptance.schema_version -ne "1.0") {
    throw "Unsupported acceptance schema version: $($acceptance.schema_version)"
}
if ($acceptance.owner -ne "architect") {
    throw "The Architect must own the acceptance contract."
}
if (
    $acceptance.policy.workflow_policy -ne "docs/agent-team/workflow-policy.json" -or
    $acceptance.policy.handoff_index -ne "docs/handoffs/index.json" -or
    $acceptance.policy.handoff_required -ne $true -or
    $acceptance.policy.software_start_allowed -ne $false
) {
    throw "Acceptance policy must point to the enforced workflow/handoff contract and keep software start disabled."
}
if ($acceptance.gates.Count -lt 1) {
    throw "At least one acceptance gate is required."
}
$requiredScenarioIds = @("S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "S10", "S11", "S12", "S13", "S14", "S15", "S16", "S17", "S18", "S19", "S20", "S21", "S22", "S23", "S24", "S25", "S26", "S27", "S28", "S29", "S30", "S31", "S32")
$actualScenarioIds = @($acceptance.scenarios | ForEach-Object { $_.id })
foreach ($scenarioId in $requiredScenarioIds) {
    if ($actualScenarioIds -notcontains $scenarioId) {
        throw "Required acceptance scenario is missing: $scenarioId"
    }
}
$requiredSubdeliveryCriteriaIds = @(
    "D6-AC01", "D6-AC02", "D6-AC03", "D6-AC04", "D6-AC05", "D6-AC06", "D6-AC07", "D6-AC08",
    "D73-AC01", "D73-AC02", "D73-AC03", "D73-AC04", "D73-AC05",
    "D74-AC01", "D74-AC02", "D74-AC03", "D74-AC04", "D741-AC01", "D742-AC01", "D751-AC01",
    "D84-AC01", "D8-AC01", "D8-AC02", "D8-AC03", "D8-AC04", "D8-AC05", "D8-AC06", "D9-AC01", "D9-AC02", "D9-AC03", "D9-AC04", "D9-AC05", "D10-AC01"
)
$actualSubdeliveryCriteriaIds = @($acceptance.subdelivery_criteria | ForEach-Object { $_.id })
foreach ($criteriaId in $requiredSubdeliveryCriteriaIds) {
    if ($actualSubdeliveryCriteriaIds -notcontains $criteriaId) {
        throw "Required subdelivery criterion is missing: $criteriaId"
    }
}

$criterionStatuses = @("defined", "in-progress", "accepted-with-limits", "completed")
$scenarioStatuses = @(
    "baseline_only", "code_verified", "smoke_verified", "environment_pending",
    "in-progress", "accepted-with-limits", "completed"
)
$evidenceBackedStatuses = @("accepted-with-limits", "completed", "code_verified", "smoke_verified")
$unrunStatuses = @("defined", "environment_pending", "baseline_only")

function Assert-AcceptanceEvidenceContract {
    param(
        [Parameter(Mandatory = $true)] [object] $Item,
        [Parameter(Mandatory = $true)] [string] $Kind,
        [Parameter(Mandatory = $true)] [string[]] $AllowedStatuses
    )

    if ([string]::IsNullOrWhiteSpace($Item.id)) {
        throw "Acceptance $Kind has a missing ID."
    }
    if ($AllowedStatuses -notcontains $Item.status) {
        throw "Unsupported acceptance $Kind status for $($Item.id): $($Item.status)"
    }
    $requiredEvidence = @($Item.required_evidence)
    if ($requiredEvidence.Count -eq 0) {
        throw "Acceptance $Kind has no required evidence: $($Item.id)"
    }
    foreach ($requiredItem in $requiredEvidence) {
        if ([string]::IsNullOrWhiteSpace([string]$requiredItem)) {
            throw "Acceptance $Kind has blank required evidence: $($Item.id)"
        }
    }

    $evidence = if ($null -eq $Item.evidence) { @() } else { @($Item.evidence) }
    if ($evidenceBackedStatuses -contains $Item.status -and $evidence.Count -eq 0) {
        throw "Evidence-backed acceptance $Kind has no evidence: $($Item.id)"
    }
    foreach ($evidenceItem in $evidence) {
        if ($evidenceItem -isnot [string] -or [string]::IsNullOrWhiteSpace($evidenceItem)) {
            throw "Acceptance $Kind has blank or non-string evidence: $($Item.id)"
        }
        $firstToken = ($evidenceItem -split "\s+", 2)[0].Replace('/', '\').TrimEnd(':', ',', ';')
        if (
            $firstToken.StartsWith('.\') -or
            $firstToken.StartsWith('docs\') -or
            $firstToken.StartsWith('src\') -or
            $firstToken.StartsWith('scripts\') -or
            $firstToken.StartsWith('packaging\') -or
            $firstToken.StartsWith('assets\') -or
            $firstToken.StartsWith('dist\')
        ) {
            $evidencePath = Join-Path $root $firstToken
            if (-not (Test-Path -LiteralPath $evidencePath)) {
                throw "Acceptance $Kind evidence path is missing for $($Item.id): $firstToken"
            }
        }
    }
    if ($unrunStatuses -contains $Item.status -and @($Item.limits).Count -eq 0) {
        throw "Unrun acceptance $Kind must declare limits or an unrun reason: $($Item.id)"
    }
}

foreach ($criterion in @($acceptance.subdelivery_criteria)) {
    Assert-AcceptanceEvidenceContract $criterion "subdelivery criterion" $criterionStatuses
}
foreach ($scenario in @($acceptance.scenarios)) {
    Assert-AcceptanceEvidenceContract $scenario "scenario" $scenarioStatuses
}

if ($acceptance.thresholds.startup_smoke_seconds -lt 1) {
    throw "Startup smoke threshold must be positive."
}
$editorOperations = $acceptance.thresholds.editor_operations
if ($null -eq $editorOperations) {
    throw "Editor operation thresholds are missing from the acceptance contract."
}
if (
    $editorOperations.replace_all_max_matches -lt 1 -or
    $editorOperations.replace_all_slice_budget_ms -lt 1 -or
    $editorOperations.replace_all_items_per_slice -lt 1 -or
    $editorOperations.recovery_capture_slice_budget_ms -lt 1 -or
    $editorOperations.recovery_capture_chunks_per_slice -lt 1 -or
    $editorOperations.recovery_capture_characters_per_chunk -lt 1 -or
    $editorOperations.recovery_capture_queue_chunks -lt 1 -or
    $editorOperations.recovery_capture_queue_bytes -lt 1 -or
    $editorOperations.cancel_must_restore_text -ne $true
) {
    throw "Editor operation thresholds must be positive and cancellation must restore text."
}
$pluginCatalog = $acceptance.thresholds.plugin_catalog
if (
    $null -eq $pluginCatalog -or
    $pluginCatalog.max_manifest_files -lt 1 -or
    $pluginCatalog.max_directory_entries -lt $pluginCatalog.max_manifest_files -or
    $pluginCatalog.max_manifest_bytes -lt 1 -or
    $pluginCatalog.external_execution -ne $false -or
    $pluginCatalog.default_trust_state -ne "untrusted"
) {
    throw "Plugin catalog thresholds must remain bounded and non-executable by default."
}
$pluginManifest = $acceptance.thresholds.plugin_manifest
if (
    $null -eq $pluginManifest -or
    $pluginManifest.max_field_length -lt 1 -or
    [string]::IsNullOrWhiteSpace($pluginManifest.required_api_version) -or
    [string]::IsNullOrWhiteSpace($pluginManifest.plugin_id_pattern) -or
    $pluginManifest.permissions_must_be_unique -ne $true
) {
    throw "Plugin manifest policy thresholds are missing or weakened."
}
$pluginApproval = $acceptance.thresholds.plugin_approval
if (
    $null -eq $pluginApproval -or
    $pluginApproval.schema_version -ne 1 -or
    $pluginApproval.max_records -lt 1 -or
    $pluginApproval.max_bytes -lt 1 -or
    $pluginApproval.default_state -ne "not-approved" -or
    $pluginApproval.approval_does_not_enable_runtime -ne $true
) {
    throw "Plugin approval policy must be bounded, deny-by-default, and non-executable."
}
$pluginRuntime = $acceptance.thresholds.plugin_runtime
if (
    $null -eq $pluginRuntime -or
    $pluginRuntime.external_activation -ne $false -or
    $pluginRuntime.control_scope -ne "explicitly-registered-in-process-only" -or
    $pluginRuntime.lifecycle_thread -ne "owning-ui-thread" -or
    $pluginRuntime.status_projection -ne "immutable-application-contract"
) {
    throw "Plugin runtime control must remain in-process, UI-thread-owned, and contract-based."
}
$pluginEnablement = $acceptance.thresholds.plugin_enablement
if (
    $null -eq $pluginEnablement -or
    $pluginEnablement.schema_version -ne 1 -or
    $pluginEnablement.max_records -lt 1 -or
    $pluginEnablement.max_bytes -lt 1 -or
    $pluginEnablement.missing_policy_default -ne "trusted-enabled" -or
    $pluginEnablement.corrupt_policy_default -ne "disabled" -or
    $pluginEnablement.mutation_overwrite_corrupt -ne $false -or
    $pluginEnablement.external_activation -ne $false
) {
    throw "Plugin enablement policy must be bounded, default-safe, fail-closed, and non-executable."
}
$pluginHost = $acceptance.thresholds.plugin_host
if (
    $null -eq $pluginHost -or
    $pluginHost.protocol_name -ne "quillforge.plugin-host" -or
    $pluginHost.protocol_version -ne 1 -or
    $pluginHost.max_frame_bytes -lt 1 -or
    $pluginHost.timeout_ms -lt 1 -or
    $pluginHost.external_execution -ne $false -or
    $pluginHost.capabilities.Count -ne 1 -or
    $pluginHost.capabilities[0] -ne "probe" -or
    $pluginHost.execution_capability_must_remain_false -ne $true -or
    $pluginHost.shell -ne $false -or
    $pluginHost.stdio_only -ne $true -or
    $null -eq $pluginHost.containment -or
    $pluginHost.containment.windows -ne "job-object" -or
    $pluginHost.containment.non_windows -ne "explicit-unsupported-diagnostic-fallback" -or
    $pluginHost.containment.max_active_processes -ne 2 -or
    $pluginHost.containment.process_memory_bytes -ne 268435456 -or
    $pluginHost.containment.kill_on_close -ne $true -or
    $pluginHost.containment.attach_failure -ne "fail-closed" -or
    $pluginHost.containment.security_sandbox -ne $false
) {
    throw "Plugin host policy must remain versioned, bounded, contained, stdio-only, and non-executable."
}

$pluginExecutionGate = $acceptance.thresholds.plugin_execution_gate
if (
    $null -eq $pluginExecutionGate -or
    $pluginExecutionGate.external_execution_enabled -ne $false -or
    $pluginExecutionGate.deny_by_default -ne $true -or
    $pluginExecutionGate.required_approval -ne "approved" -or
    $pluginExecutionGate.required_signature -ne "valid" -or
    $pluginExecutionGate.required_code_identity -ne "matched" -or
    $pluginExecutionGate.required_containment -ne "attached" -or
    $pluginExecutionGate.executor_available -ne $false
) {
    throw "Plugin execution gate must remain deny-by-default and non-executable."
}
$workspaceSearch = $acceptance.thresholds.workspace_search
if (
    $null -eq $workspaceSearch -or
    $workspaceSearch.max_files -lt 1 -or
    $workspaceSearch.max_total_bytes -lt $workspaceSearch.max_file_bytes -or
    $workspaceSearch.max_file_bytes -lt 1 -or
    $workspaceSearch.max_directory_entries -lt 1 -or
    $workspaceSearch.max_results -lt 1 -or
    $workspaceSearch.max_depth -lt 0 -or
    $workspaceSearch.max_line_bytes -lt 1 -or
    $workspaceSearch.max_preview_chars -lt 1 -or
    $workspaceSearch.max_issue_records -lt 1 -or
    $workspaceSearch.recursive -ne $true -or
    $workspaceSearch.literal_only -ne $true -or
    $workspaceSearch.follow_symlinks -ne $false -or
    $workspaceSearch.cancellation -ne "cooperative" -or
    $workspaceSearch.diagnostics_visible -ne $true -or
    $workspaceSearch.diagnostics_relative_paths -ne $true -or
    $workspaceSearch.diagnostics_truncation_explicit -ne $true -or
    $workspaceSearch.excluded_directory_names.Count -lt 1
) {
    throw "Workspace search policy must remain bounded, literal, recursive, cancellable, and symlink-safe."
}
$taskRunner = $acceptance.thresholds.task_runner
if (
    $null -eq $taskRunner -or
    $taskRunner.lifecycle_observation -ne "pending-count" -or
    $taskRunner.close_guard -ne $true -or
    $taskRunner.ui_thread_wait_for_done -ne $false -or
    $taskRunner.force_termination -ne $false -or
    $taskRunner.queued_completion_must_drain -ne $true
) {
    throw "TaskRunner lifecycle policy must guard close without blocking or force termination."
}
$ui = $acceptance.thresholds.ui
if (
    $null -eq $ui -or
    $ui.theme_id -ne "quillforge-ink-violet" -or
    $ui.base_spacing_px -ne 4 -or
    $ui.min_window_width -ne 980 -or
    $ui.min_window_height -ne 640 -or
    $ui.command_bar_actions.Count -lt 7 -or
    $ui.icon_asset -ne "assets/quillforge.ico" -or
    $ui.runtime_visual_review -ne "intentionally-unrun-on-user-request"
) {
    throw "UI iteration thresholds must remain centralized, bounded, branded, and explicit about unrun visual review."
}
$session = $acceptance.thresholds.session
if (
    $null -eq $session -or
    $session.schema_version -ne 1 -or
    $session.max_documents -lt 1 -or
    $session.max_path_chars -lt 1 -or
    $session.max_bytes -lt 1 -or
    $session.storage -ne "user-local" -or
    $session.atomic_replace -ne $true -or
    $session.persisted_content -ne $false -or
    $session.persisted_caret -ne $true -or
    $session.persisted_dirty_content -ne $false -or
    $session.recovery_precedes_restore -ne $true -or
    $session.restore_serially -ne $true -or
    $session.latest_wins_single_flight -ne $true
) {
    throw "Session continuity policy must remain bounded, path-only, atomic, recovery-first, and single-flight."
}
$delivery = Get-Content -Raw -LiteralPath $deliveryPath | ConvertFrom-Json
if ($delivery.schema_version -ne "1.0" -or $delivery.owner -ne "architect") {
    throw "The delivery register must use schema 1.0 and be Architect-owned."
}
$deliveryIds = @($delivery.deliveries | ForEach-Object { $_.id })
if ($deliveryIds.Count -eq 0 -or @($deliveryIds | Select-Object -Unique).Count -ne $deliveryIds.Count) {
    throw "The delivery register must contain unique delivery IDs."
}
if ($deliveryIds -notcontains $delivery.current_delivery) {
    throw "The current delivery is missing from the delivery register: $($delivery.current_delivery)"
}
$allowedDeliveryStatuses = @($delivery.status_values)
foreach ($deliveryItem in $delivery.deliveries) {
    if ($allowedDeliveryStatuses -notcontains $deliveryItem.status) {
        throw "Unsupported delivery status for $($deliveryItem.id): $($deliveryItem.status)"
    }
}
if (-not (Test-Path -LiteralPath (Join-Path $root "assets\quillforge.ico"))) {
    throw "The Windows application icon is missing: assets\quillforge.ico"
}

$boundaryRules = @(
    @{ Layer = "domain"; Path = "src\quillforge\domain"; Pattern = "^\s*(from|import)\s+.*(PyQt6|PySide|application|infrastructure|presentation|plugins)" },
    @{ Layer = "application"; Path = "src\quillforge\application"; Pattern = "^\s*(from|import)\s+.*(PyQt6|PySide|infrastructure|presentation)" },
    @{ Layer = "plugins"; Path = "src\quillforge\plugins"; Pattern = "^\s*(from|import)\s+.*(PyQt6|PySide|presentation)" },
    @{ Layer = "infrastructure"; Path = "src\quillforge\infrastructure"; Pattern = "^\s*(from|import)\s+.*(PyQt6|PySide|presentation)" }
)
$rgCommand = Get-Command rg -CommandType Application -ErrorAction SilentlyContinue
if ($null -eq $rgCommand) {
    throw "ripgrep (rg) is required for architecture boundary checks."
}
foreach ($rule in $boundaryRules) {
    $violations = @(& $rgCommand.Source --files-with-matches --glob "*.py" $rule.Pattern $rule.Path 2>$null)
    $rgExitCode = $LASTEXITCODE
    if ($rgExitCode -gt 1) {
        throw "Architecture boundary scan failed for $($rule.Layer) with rg exit code $rgExitCode."
    }
    if ($rgExitCode -eq 0 -and $violations.Count -gt 0) {
        throw "Layer boundary violation in $($rule.Layer): $($violations -join ', ')"
    }
    $LASTEXITCODE = 0
}

& uv run python scripts/audit_presentation_contracts.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

uv lock --check
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

uv run ruff format --check src scripts
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

uv run ruff check src scripts
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

uv run python -m compileall -q src scripts
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "QuillForge checks passed."

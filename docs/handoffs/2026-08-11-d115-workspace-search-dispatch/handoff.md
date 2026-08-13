# Handoff: 2026-08-11-d115-workspace-search-dispatch

| Field | Value |
|---|---|
| ID | 2026-08-11-d115-workspace-search-dispatch |
| Delivery / slice | D115 / ARCH-89 workspace-search dispatch callback boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T02:45:00+08:00 |

## User outcome

Workspace search now binds TaskRunner callbacks through the existing typed
Qt-free WorkspaceSearchCoordinator. MainWindow continues to own query
construction, cooperative cancellation, service/runner admission, surface,
containment, locale, notification, and close policy.

## Scope and boundaries

### In scope

- Add typed workspace-search operation/success/failure/dispatcher contracts.
- Add coordinator submit(...) with generation binding.
- Route MainWindow workspace search through the existing lifecycle owner.
- Preserve valid/zero-match/invalid/failure/stale/invalidated/cancel and
  synchronous dispatcher-exception behavior.
- Record public-source applicability, parent review, independent review,
  simplification, static, package, and release-limit evidence.

### Out of scope

- No WorkspaceSearchService/provider, query model, policy, filesystem search,
  cancellation tracker, surface, locale, notification, persistence, TaskRunner,
  or close-policy implementation changed.
- No new async framework, retry, cache, event bus, runner adapter, or
  test-only asset was introduced.
- No QApplication launch, native rendering, filesystem workload,
  screenshot, clean-machine, cross-machine, signing, installer, updater,
  legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Harvey the 4th / Luna max | Read-only D115 boundary consultation; NO_CONCLUSION after two bounded waits |
| Independent review | Hilbert the 4th / Luna max | Read-only D115 review; NO_CONCLUSION after two bounded waits |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- src/quillforge/presentation/workspace_search_coordinator.py — typed
  dispatcher contract and submit(...) binding.
- src/quillforge/presentation/main_window.py — workspace search dispatch now
  uses the coordinator boundary.
- docs/adr/0145-workspace-search-dispatch-callback-boundary.md — decision,
  invariants, alternatives, applicability, review, simplification, and limits.
- docs/agent-team/reviews/D115-arch-89-workspace-search-parent-review.md —
  parent review.
- docs/agent-team/reviews/D115-arch-89-workspace-search-independent-review.md
  — independent review record.
- docs/specs/enterprise-architecture-migration.md, docs/ARCHITECTURE.md,
  docs/ROADMAP.md, tasks/plan.md, tasks/todo.md.

## Decisions and constraints

- The existing WorkspaceSearchCoordinator remains the sole search callback
  lifecycle owner; no parallel dispatcher abstraction was added.
- MainWindow retains WorkspaceSearchService, query/cancellation, TaskRunner,
  surface, containment, locale, notification, persistence, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 presentation code. Embedded C/C++ assurance and vendor
  manufacturer requirements are N/A.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D115-WORKSPACE-SEARCH-DISPATCH-PROBE=PASS | PASS | Production-class probe covered valid/zero-match warning, invalid, failure, stale, invalidated/cancel, and dispatcher exception paths. |
| D115-SOURCE-DEPENDENCY-PROBE=PASS | PASS | Typed submit is present, MainWindow search has no direct runner callback closure, and coordinator remains Qt-free/service-free. |
| uv run python -m compileall -q src scripts | PASS | Static compilation only; no QApplication launch. |
| uv run ruff check src scripts | PASS | All checks passed. |
| uv run ruff format --check src scripts | PASS | All files are formatted. |
| uv run python scripts/audit_presentation_contracts.py | PASS | Presentation contracts passed after D115 traceability synchronization; no QApplication launch. |
| D115-PACKAGE-IDENTITY-PROBE=PASS | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| D115-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no QuillForge process was present afterward. |
| D115-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, release manifest, and current dossier bind to D115 identity. |
| D115-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is D115-bound and records the expected no-go decision. |
| D115-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the verifier non-zero as required by the evidence boundary. |
| scripts\verify_handoff.ps1 | PASS | Handoff schema and traceability checks passed. |
| scripts\check.ps1 | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Native TaskRunner timing, actual filesystem search, QApplication startup,
  dialog rendering, accessibility, DPI, fonts, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and lifecycle ordering, not native
  scheduling or filesystem behavior.
- Harvey architecture and Hilbert independent review both returned
  NO_CONCLUSION; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: S149, D115-AC01.
- Evidence: ADR-0145, D115 source/dispatch probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: dist/QuillForge.exe and root QuillForge.exe
- SHA-256: 5380764FBCB1A26AE28116D140BD5561B980B55E2BFA8EDF78F04C299DEB5CEE
- Size: 38498664 bytes
- Source revision: tree-sha256:9f893481065d13587e043f3fc4d1972fd947b33e35628e86b38230e8174a3f91
- Manifest: dist/QuillForge.release.json

## Disposition

accepted-with-limits: workspace search callback binding is consolidated behind
the existing Qt-free coordinator while native search timing, runtime, and
enterprise release gates remain open.

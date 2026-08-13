# Handoff: 2026-08-10-d65-document-tab-forwarding

| Field | Value |
|---|---|
| ID | `2026-08-10-d65-document-tab-forwarding` |
| Delivery / slice | `D65 / ARCH-50 document-tab forwarding simplification` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:30:00+08:00` |

## User outcome

MainWindow now calls the cohesive DocumentTabSurface lookup contract directly.
The four redundant query wrappers are gone, reducing coordinator indirection
without changing document, session, recovery, close, or operation behavior.

## Scope and boundaries

### In scope

- Direct active-tab, editor, path, and containment calls through
  DocumentTabSurface.
- Retained path exclusion and session-snapshot active-tab semantics.
- Static, package, handoff, and release evidence.

### Out of scope

- No DocumentTabSurface API change, session restore policy change, save/open/
  close/recovery/Replace All/Find behavior change, service, TaskRunner,
  notification, runtime, screenshot, clean-machine, signing, installer,
  updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Laplace the 2nd / Luna max | Cross-module boundary consultation; no conclusion after two bounded waits |
| Independent review | Bacon the 3rd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — direct surface calls and
  forwarding-wrapper removal.
- `docs/adr/0090-document-tab-forwarding-simplification.md` — decision.
- `docs/agent-team/reviews/D65-document-tab-forwarding-parent-review.md` and
  `D65-document-tab-forwarding-independent-review.md` — review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability.

## Decisions and constraints

- DocumentTabSurface remains the tab-registry and path-identity owner;
  SessionRestoreTracker remains the restore subset/order owner.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++ assurance
  and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D65 tab-forwarding simplification probe | `PASS` | Wrapper removal, surface contracts, path exclusion, and snapshot active tab. |
| Targeted compileall / Ruff / format | `PASS` | Changed MainWindow source. |
| Full compileall / Ruff / format | `PASS` | Source and documentation synchronization completed. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing external gates and report bindings remain open. |

## Unrun checks and reason

- Native callback ordering, runtime startup, screenshots, accessibility, DPI,
  fonts, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static direct-call evidence does not prove native callback interleaving.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D65-AC01`, `S94`.
- Evidence: ADR-0090, source probe, parent/independent reviews, static checks,
  package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded coordinator/UI slice or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `B247E10E0F810C9A9EF1DC368B774DBA47A5E053EB3FA2458D2C1FE32E6EE463` / `38,434,918` bytes.
- Source revision: `tree-sha256:4872d2476460b665eb12e3c7340a221fc59fb19fadb067eb2d952b288eef7555`.

## Disposition

`accepted-with-limits`: tab lookup forwarding simplification is integrated and
the D65 package identity is recorded; runtime and release gates remain open.

# Handoff: 2026-08-10-d59-status-phase-forwarding

| Field | Value |
|---|---|
| ID | `2026-08-10-d59-status-phase-forwarding` |
| Delivery / slice | `D59 / ARCH-48 status-phase forwarding simplification` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T18:00:00+08:00` |

## User outcome

Status phase projection has one clear coordinator entry point. Redundant
forwarding was removed while dirty-document attention, workspace completion,
current-tab changes, working precedence, and ready-state projection remain
unchanged.

## Scope and boundaries

### In scope

- Remove `_sync_active_document_phase()`.
- Route its existing callers directly to `_sync_status_surface()`.
- Preserve status precedence and all application policy.
- Record static, package, handoff, and release evidence.

### Out of scope

- No StatusSurface API, QSS, notification, editor, workspace, session,
  document, TaskRunner, operation, locale, theme, runtime event, or public
  behavior redesign.
- No new coordinator, test asset, runtime launch, screenshot, clean-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Planck the 2nd / Luna max | Status boundary consultation; no conclusion after two bounded waits |
| Independent review | Epicurus the 2nd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — removes the alias and routes
  existing callers to the unified status projection method.
- `docs/adr/0084-status-phase-forwarding-simplification.md` — decision.
- `docs/agent-team/reviews/D59-status-phase-forwarding-parent-review.md` and
  `D59-status-phase-forwarding-independent-review.md` — review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability.

## Decisions and constraints

- `_sync_status_surface()` remains the only status-phase policy implementation;
  MainWindow keeps work/dirty/ready precedence and StatusSurface projection.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D59 status-phase forwarding probe | `PASS` | Alias absent; direct callers and unified policy remain. |
| Targeted compileall / Ruff / format | `PASS` | Changed source slice. |
| Full compileall / Ruff / format | `PASS` | Run after final documentation/package sync. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff indexed and synchronized. |
| `scripts\check.ps1` | `PASS` | Acceptance/register synchronized. |
| `scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing external gates and stale report bindings remain. |

## Unrun checks and reason

- Native Qt events, status rendering, runtime startup, screenshots,
  accessibility, DPI, fonts, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static call-site evidence cannot prove native event timing or visual phase
  rendering.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D59-AC01`, `S88`.
- Evidence: ADR-0084, source probe, parent/independent reviews, static checks,
  package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract extraction
  or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `DCA94EEC3595BA04EB9BD0E765A73D1054CBE6114125AE93D7C08C797B71B71C` / `38,434,757` bytes; root/dist identity matches.
- Source revision: `tree-sha256:d5bd2460d43734f94370afccace76037d7993f4655032443e73a36dffd6e79b9`.

## Disposition

`accepted-with-limits`: the forwarding alias is removed and static behavior
checks pass; package identity and full traceability are synchronized after the
final package, while runtime and release gates remain open.

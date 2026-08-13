# Handoff: 2026-08-10-d102-close-guard

| Field | Value |
|---|---|
| ID | `2026-08-10-d102-close-guard` |
| Delivery / slice | `D102 / ARCH-76 close-readiness coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Close readiness now has one explicit Qt-free decision boundary. QuillForge
still blocks in-progress operations, cancels workspace search cooperatively,
protects dirty tabs and background work, requests the final session save,
rechecks queued work, and stops timers only when close is safe. The visible
error dialog and Qt event acceptance remain unchanged.

## Scope and boundaries

### In scope

- `CloseGuardCoordinator` and immutable `CloseGuardDecision` contract.
- Direct MainWindow wiring and replacement of the former close-event policy
  block.
- Exact gate precedence and ordered session-save/pending-work/timer behavior.
- Source, behavior-probe, static, package, handoff, and release evidence.

### Out of scope

- No QCloseEvent, QMessageBox, QTimer, TaskRunner, session schema/store,
  recovery, workspace-search, plugin, document, notification, locale, or close
  behavior contract changed.
- No worker wait/termination, retry, cache, mutable shutdown state, or generic
  application shutdown framework added.
- No Qt launch, screenshot, native close interaction, accessibility/DPI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | James the 3rd / Luna max | Read-only D102 boundary consultation; `NO_CONCLUSION` after bounded windows |
| Independent review | Nietzsche the 3rd / Luna max | Read-only D102 ordering/risk review; `NO_CONCLUSION` after bounded windows |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/close_guard_coordinator.py` — Qt-free ordered
  close-readiness decision.
- `src/quillforge/presentation/main_window.py` — direct coordinator wiring,
  typed block projection, and concrete timer callback.
- `docs/adr/0127-close-guard-coordinator-boundary.md` — architecture decision
  and invariants.
- D102 parent/independent review records, handoff/index, acceptance/delivery
  register, architecture/roadmap/spec/task/release records.

## Decisions and constraints

- `CloseGuardCoordinator` owns only precedence and ordered callbacks;
  MainWindow owns Qt event acceptance, message projection, session/service,
  TaskRunner, timer, and concrete operation policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A` for this slice.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D102-CLOSE-GUARD-SOURCE-PROBE=PASS` | `PASS` | Qt-free source and MainWindow integration. |
| `D102-CLOSE-GUARD-ORDER-PROBE=PASS` | `PASS` | Busy/search/dirty/background/pending/allow sequences. |
| `python -m compileall -q src/quillforge` | `PASS` | No launch or QApplication instantiation. |
| `uv run ruff check src/quillforge` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge` | `PASS` | 119 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| `D102-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | SHA `B919221A030D23D404A86A881DA52203B4C74A8884AE95A577258F7247584AD6`; 38,488,600 bytes; source `tree-sha256:bccb699177aefa0a2bafb144d134a3914dd8133bdfdfa6e490a7a3640ee899b0`. |
| `scripts\verify_release_handoff.ps1` wrapped expected NO-GO | `PASS` | Dossier reports `no-go`, the three expected mechanical report-binding failures, and 10 open gates. |
| `D102-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Dossier artifact SHA/size matches the D102 manifest and root/dist candidate. |

## Unrun checks and reason

- Native Qt close-event delivery, QMessageBox rendering, worker interleavings,
  accessibility, DPI, clean-machine, cross-machine, hardware, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes do not prove native Qt event delivery, modal rendering, or
  close-time worker interleavings.
- Both delegated D102 roles returned `NO_CONCLUSION`; no child PASS is claimed.
  Parent source review and simplification assessment are the recorded
  acceptance evidence.
- The portable candidate remains unsigned and release remains `NO-GO`; report
  binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D102-AC01`, `S131`.
- Evidence: ADR-0127, D102 source/order probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, `D102-RELEASE-DOSSIER-PROBE=PASS`, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: run synchronized handoff/repository/traceability/release checks and
  continue the next highest-value MainWindow/application or visual slice.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `B919221A030D23D404A86A881DA52203B4C74A8884AE95A577258F7247584AD6` /
  `38,488,600` bytes.
- Source revision: `tree-sha256:bccb699177aefa0a2bafb144d134a3914dd8133bdfdfa6e490a7a3640ee899b0`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: close-readiness classification is isolated behind a
Qt-free typed boundary, while native runtime and enterprise release gates
remain open.

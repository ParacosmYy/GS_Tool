# Handoff: 2026-08-10-d108-presentation-contract-audit

| Field | Value |
|---|---|
| ID | `2026-08-10-d108-presentation-contract-audit` |
| Delivery / slice | `D108 / ARCH-80 cross-module contract/error/observability audit` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:35:00+08:00` |

## User outcome

QuillForge now has a deterministic source gate for three existing enterprise
contracts: Qt-free presentation coordinators stay independent of Qt shell and
concrete worker owners; MainWindow notifications carry explicit semantic
levels; and TaskRunner pending work remains visible through MainWindow's
WORKING status projection.

## Scope and boundaries

### In scope

- Add the stdlib AST audit in `scripts/audit_presentation_contracts.py`.
- Run it from `scripts/check.ps1` with non-zero failure propagation.
- Cover coordinator imports, direct `self.notify(..., level=...)`, and
  TaskRunner/MainWindow pending-work wiring.
- Record a positive audit and in-memory negative-case AST probe.
- Record public-source applicability, review status, simplification, package,
  handoff, and release-limit evidence.

### Out of scope

- No runtime behavior, Qt event loop, notification text/levels, TaskRunner
  scheduling, coordinator state, close policy, or public API changed.
- No dynamic instrumentation, metrics backend, log schema, GUI launch,
  screenshot, worker timing, filesystem durability, startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Laplace the 3rd / Luna max | Read-only D108 boundary consultation; `NO_CONCLUSION` after bounded wait |
| Independent review | Mendel the 3rd / Luna max | Read-only D108 audit review; `NO_CONCLUSION` after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- `scripts/audit_presentation_contracts.py` — stdlib AST/static contract gate.
- `scripts/check.ps1` — invokes the audit before existing deterministic checks.
- `docs/adr/0135-presentation-contract-observability-audit.md` — decision,
  invariants, alternatives, applicability, review, simplification, and limits.
- `docs/agent-team/reviews/D108-arch-80-presentation-contract-parent-review.md` —
  parent review.
- `docs/agent-team/reviews/D108-arch-80-presentation-contract-independent-review.md` —
  independent `NO_CONCLUSION` record.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- The gate remains source-only and deliberately avoids runtime confidence
  claims from names or static strings.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline AST, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 desktop tooling. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python scripts/audit_presentation_contracts.py` | `PASS` | Existing coordinator, notification, and TaskRunner/MainWindow contracts passed. |
| `D108-CONTRACT-AUDIT-NEGATIVE-PROBE=PASS` | `PASS` | In-memory AST substitutions proved forbidden imports and missing levels are detected while explicit levels pass. No test asset was written. |
| `uv run python -m compileall -q src/quillforge scripts` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge scripts` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge scripts` | `PASS` | 122 files already formatted. |
| `scripts\check.ps1` | `PASS` | The new gate and existing notice/workflow/acceptance/boundary/lock/lint/compile checks passed. |
| `D108-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | Root/dist SHA `6060D1560BB329A6A109FA5053ADF52C27947B351905D43FDA8ABE3F3CD9AE87`; 38,494,546 bytes; source `tree-sha256:17d335d05ce87e0fc17883391ba6fcf5333ee8ca47905d1df4e5714bc01cbc48`. |
| `D108-NO-LAUNCH-PROBE=PASS` | `PASS` | Package completed and no QuillForge process was running afterward. |
| `D108-JSON-TRACEABILITY-PROBE=PASS` | `PASS` | Acceptance, delivery register, handoff index, and release manifest point to D108/current identity. |
| `D108-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Current dossier is `no-go`, bound to the D108 artifact, with 10 open gates and the three known mechanical report-binding failures. |
| `D108-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | `verify_release_handoff.ps1` remains intentionally non-zero because authorized runtime/report refresh and release gates are still open. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff status/index contract passed. |

## Unrun checks and reason

- Native Qt event ordering, worker timing, runtime error completeness,
  QApplication startup, screen-reader output, visual/DPI/font behavior,
  performance, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source contracts do not prove runtime event ordering or business-level
  error completeness.
- Laplace and Mendel review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S139`, `D108-AC01`.
- Evidence: ADR-0135, D108 audit/negative probes, parent/independent review
  records, compile/lint/format/check results, package identity, handoff/index/
  register checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract slice and
  complete authorized runtime/release gates when authority and environment
  permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `6060D1560BB329A6A109FA5053ADF52C27947B351905D43FDA8ABE3F3CD9AE87`
- Size: `38,494,546` bytes
- Source revision: `tree-sha256:17d335d05ce87e0fc17883391ba6fcf5333ee8ca47905d1df4e5714bc01cbc48`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: three cross-module presentation contracts now have a
deterministic static gate, while runtime correctness and enterprise release
gates remain open.

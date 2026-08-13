# Handoff: 2026-08-10-d8-5-ledger-provenance-semantics

| Field | Value |
|---|---|
| ID | `2026-08-10-d8-5-ledger-provenance-semantics` |
| Delivery / slice | `D8 / D8.5 live artifact-provenance ledger semantics` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T04:05:00+08:00` |

## User outcome

The live delivery and acceptance ledgers now distinguish historical package
captures from the current candidate. The old hash/size/source records remain
unchanged as historical evidence, and the D6.2 binary capture no longer points
at the manifest path.

## Scope and boundaries

### In scope

- Re-label stale D6/D7/D8/D9/D10 binary and source-snapshot identities as
  historical captures.
- Keep the current D9/UI-11 candidate identity explicit.
- Correct the D6.2 38 MB artifact path from the manifest path to
  `dist/QuillForge.exe`.
- Record the provenance decision, review, and verification evidence.

### Out of scope

- Production Python/UI/runtime source changes.
- Rewriting historical reports, performance captures, or packaged evidence.
- Package rebuild, executable launch, QApplication/Qt startup, screenshots,
  tests, signing, installer, updater, legal, clean-machine, pressure,
  hard-power, or cross-machine operations.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent | Scope, integration, final ledger review, verification, handoff |
| Project Manager | Franklin / Luna | Dependency and stale-identity inventory |
| Product | Gibbs / Luna | User and release conclusion impact |
| Developer 1 | Ampere / Luna | Ledger/current-identity boundary review |
| Developer 2 | Hegel / Luna | Cross-record provenance consistency review |
| QA | Kepler / Luna | Authorized non-destructive verification boundary |
| Independent reviewer | Aristotle / Luna | Read-only post-change semantic review |

## Changed files and modules

- `docs/agent-team/delivery-register.json` — historical labels, D6.2 path
  correction, and current D9 aggregate identity.
- `docs/agent-team/acceptance.json` — historical labels for old package
  captures while preserving statuses, limits, and required-evidence wording.
- `docs/agent-team/reviews/D8.5-current-artifact-provenance-rebind-parent-review.md`
  — architecture, role, simplification, applicability, and evidence record.
- `docs/handoffs/index.json` — this handoff index entry.

No production source file or package artifact changed in this slice.

## Decisions and constraints

- Canonical current identity is projected from the current manifest, release
  dossier, human handoff, and current UI-11/D9 evidence.
- Historical hashes, sizes, source snapshots, statuses, limits, outcomes, and
  reports are preserved; only stale time semantics were corrected.
- The parent Architect remained the only writer in the current local checkout.
- The permanent no-launch boundary remains active.
- Embedded C/C++ assurance is not applicable to this Python/PyQt6/PowerShell
  documentation-only slice.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Live JSON parse | PASS | Delivery register, acceptance ledger, handoff index, manifest, and dossier parse successfully. |
| Read-only current manifest/dossier/handoff identity comparison | PASS | Root/dist both match `8AF11E…115E6` and `38,365,225` bytes; dossier remains no-go. |
| `scripts/verify_handoff.ps1` | PASS | Handoff index and Markdown status/required sections are synchronized. |
| `scripts/check.ps1` | PASS | Static repository, policy, notice, JSON, and handoff checks passed. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Parent-owned; stale-runtime failures and external gates remain open. |
| Package rebuild | NOT RUN | No production source or artifact changed. |

## Unrun checks and reason

Qt/QApplication/EXE startup, visual interaction, runtime report refresh,
performance measurement, tests, clean-machine, cross-machine, signing,
installer, update, legal, permission/disk-pressure, and hard-power checks were
not run because the project policy and current authorization exclude them.

## Known risks and limits

- The ledgers still use free-form evidence strings; a future machine-readable
  current-artifact field or semantic validator may be useful, but is outside
  this documentation-only slice.
- Historical runtime and performance reports remain bound to their original
  package identities and cannot prove the current candidate.
- QSS/UI behavior and release readiness are unaffected by this rebind and
  remain subject to their existing no-launch and external-gate limits.

## Acceptance and evidence IDs

- Existing acceptance: `D8-AC06`, `S32`.
- Review: `docs/agent-team/reviews/D8.5-current-artifact-provenance-rebind-parent-review.md`.
- Current identity: root/dist `8AF11EE8D661FDFF2A564DE6F48D7056D3A41887A53B969AA5CAF99AE5B115E6`
  / `38,365,225` bytes; source tree `87abe606819cd85810d11b4c1849ad5a6940a590a381b7131fb117990ac97866`.

## Next owner and next action

- Owner: Release Engineering / authorized QA.
- Action: if release work resumes, regenerate only the authorized packaged,
  interactive, clean-machine, permission-pressure, and cross-machine evidence
  against the current candidate; do not reuse historical captures.

## Artifact information

- Artifact path: `QuillForge.exe`, `dist/QuillForge.exe`.
- Current candidate: SHA-256 `8AF11EE8D661FDFF2A564DE6F48D7056D3A41887A53B969AA5CAF99AE5B115E6`,
  `38,365,225` bytes for both root and dist copies.
- Source snapshot: `tree-sha256:87abe606819cd85810d11b4c1849ad5a6940a590a381b7131fb117990ac97866`.
- Packaging note: unchanged in this docs-only slice; unsigned portable/manual
  release posture remains unchanged.

## Disposition

`accepted-with-limits`: the live ledger provenance wording is corrected and
historical evidence is retained. The enterprise release decision remains
`no-go` until authorized runtime and external release gates are completed.

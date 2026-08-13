# Handoff: 2026-08-09-d8-5-handoff-freshness

| Field | Value |
|---|---|
| ID | `2026-08-09-d8-5-handoff-freshness` |
| Delivery / slice | `D8 / D8.5 artifact-bound handoff freshness` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T20:45:00+08:00` |

## User outcome

After a package changes, the release dossier now identifies the current
artifact instead of leaving the previous artifact's dossier as if it were
current. Stale startup/performance evidence is listed as named mechanical
failures, the decision stays `no-go`, and the verifier still returns non-zero.

## Scope and boundaries

### In scope

- Current-artifact dossier generation before verifier failure.
- Machine-readable `mechanical_failures` traceability.
- Release acceptance, roadmap, ADR, review, and handoff synchronization.

### Out of scope

- Refreshing startup/performance reports or launching QuillForge.
- Signing, installer, update, clean-machine, legal, support, or cross-machine
  release approval.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Scope, implementation, integration, and final release disposition |
| Project Manager | Architect-led bounded record | Release-gate sequencing and support-risk record |
| Product | Architect-led bounded record | Confirmed stale evidence must remain no-go and visible |
| Developer 1 | Architect-led bounded record | Audited verifier identity and failure path |
| Developer 2 | Architect-led bounded record | Integrated the release-dossier output change |
| QA | Architect-led bounded record | Verified expected non-zero stale-evidence behavior and static gates |

No child PASS or independent review is claimed; the parent owns the final
integration and verification evidence in this checkout.

## Changed files and modules

- `scripts/verify_release_handoff.ps1` — writes the dossier before returning
  non-zero and records `mechanical_failures`.
- `docs/adr/0030-release-handoff-freshness.md` — records the fail-closed
  freshness decision.
- `docs/agent-team/acceptance.json` — adds D8-AC06 and S32.
- `scripts/check.ps1` — requires the new acceptance IDs.
- `docs/agent-team/delivery-register.json` and `docs/ROADMAP.md` — record D8.5.
- `docs/agent-team/reviews/D8.5-parent-review.md` — records the parent audit.
- `docs/RELEASE_HANDOFF.md` and `docs/release/handoff-2026-08-09.json` — bind
  the current no-go dossier and its failures.
- `docs/handoffs/index.json` — indexes this handoff as latest.

## Decisions and constraints

- A verifier failure must not leave a stale dossier as the only apparent
  current release record.
- The script writes before throwing; automation still receives a non-zero exit
  and cannot treat inconsistency as a pass.
- A mechanically consistent dossier remains `no-go` while release gates are
  open.
- Shared checkout writer: Architect, limited to the files listed above.
- Runtime launch policy: software and Qt windows remain forbidden; static
  build/package verification is allowed.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `scripts/verify_release_handoff.ps1` | `EXPECTED NON-ZERO` | Wrote the current dossier, then reported `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`. |
| `docs/release/handoff-2026-08-09.json` | `PASS` | Current root/dist artifact identity and named mechanical failures are recorded. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff/index structure passes after ledger update. |
| `scripts/check.ps1` | `PASS` | Acceptance, architecture, lock, formatting, lint, and compile checks. |
| `scripts/package.ps1` | `PASS` | Fresh PyInstaller one-file build; root and dist hashes match. |

## Unrun checks and reason

- Startup, packaged diagnostics, and performance refresh — intentionally unrun
  because the current project instruction prohibits starting QuillForge.
- Clean-machine, legal, signing, installer, update, support, permission/
  disk-pressure, hard-power, and cross-machine gates — require external
  authorized evidence.
- Unit tests and test-only assets — not created or run under project policy.

## Known risks and limits

- The current dossier is correctly `no-go`; its runtime reports are bound to a
  previous artifact until a permitted refresh is performed.
- The verifier now exposes inconsistency but does not make stale evidence
  useful for runtime claims.
- This is release-governance hardening, not a signing, sandbox, or clean-machine
  guarantee.

## Acceptance and evidence IDs

- Acceptance: `S32`, `D8-AC06` (supplements `S13` / `D8-AC05`).
- Evidence: `scripts/verify_release_handoff.ps1`,
  `docs/adr/0030-release-handoff-freshness.md`,
  `docs/release/handoff-2026-08-09.json`,
  `docs/agent-team/reviews/D8.5-parent-review.md`.

## Next owner and next action

- Owner: Release Engineering / QA / Product.
- Action: after explicit permission to launch, refresh artifact-bound startup
  and performance evidence, rerun the verifier, and reassess the remaining
  enterprise release gates.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `C46B2AF4C76E6F0A28C6BD43B6C953AA7EB640582F18B6B52EB856F8829BC29A` / `38,326,107` bytes for both root and dist copies.
- Packaging note: portable one-file candidate; the verifier dossier is
  artifact-bound and intentionally no-go until runtime evidence is refreshed.

## Disposition

`accepted-with-limits`: the current release record is no longer silently stale,
while external/runtime release evidence remains open.

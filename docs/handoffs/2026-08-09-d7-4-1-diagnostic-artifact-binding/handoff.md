# Handoff: 2026-08-09-d7-4-1-diagnostic-artifact-binding

| Field | Value |
|---|---|
| ID | `2026-08-09-d7-4-1-diagnostic-artifact-binding` |
| Delivery / slice | `D7 / D7.4.1 packaged diagnostic artifact binding` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:38:13+08:00` |

## User outcome

The Qt-free workspace-search diagnostic now records whether it ran from source
Python or a frozen EXE; frozen reports carry the producing EXE's resolved path,
size, and SHA-256 so future evidence can be artifact-bound.

## Scope and boundaries

### In scope

- `app.py` diagnostic execution/artifact identity payload.
- Source-mode no-write probe and static/package verification.
- D7.4.1 follow-up review and evidence synchronization.

### Out of scope

- Running the packaged diagnostic, launching QuillForge, interactive GUI review,
  cross-machine evidence, or performance claims.
- Changing bounded search traversal, search features, trust policy, unit tests,
  mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Implement artifact binding and final verification |
| Project Manager | Parent role record | Evidence freshness and D7 scope |
| Product | Parent role record | Artifact-traceable diagnostic outcome |
| Developer 1 | Parent role record | Qt-free diagnostic payload |
| Developer 2 | Parent role record | Packaging and release evidence synchronization |
| QA | Parfit / Luna, read-only | Identified unbound report and residual traversal limit |

## Changed files and modules

- `src/quillforge/app.py` — add source/packaged execution and artifact identity.
- `docs/adr/0021-bounded-workspace-search.md` — record artifact binding.
- `docs/agent-team/reviews/D7.4.1-artifact-binding-parent-review.md` — record
  the parent fix audit.
- `docs/handoffs/2026-08-09-d7-4-1-diagnostic-artifact-binding/handoff.md` —
  record the follow-up slice.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  and `docs/handoffs/index.json` — synchronize evidence.

## Decisions and constraints

- Frozen reports hash the exact producing `sys.executable`; source reports use
  `artifact: null` and cannot be mistaken for packaged evidence.
- Hash failures are explicit report metadata, not a fabricated identity.
- Shared-checkout writer: Architect only; the Luna reviewer was read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `Parfit / Luna read-only audit` | `FINDING` | Identified unbound historical report; no files changed. |
| Source-mode artifact identity probe | `PASS` | Non-frozen source path explicitly returns `artifact: null`; no write or launch. |
| `scripts/verify_handoff.ps1` | `PASS` | Indexed status parity and required handoff structure pass. |
| `scripts/check.ps1` | `PASS` | Static, formatting, compile, and acceptance checks passed; 62 files were already formatted. |
| `scripts/package.ps1` | `PASS` | Root/dist identity is `BF532EDD63773EF5E902D4A95571B30D6EA3443D52F31294825EF7CBCA7C9EEF`, 38,327,952 bytes. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Wrote the current dossier; three historical runtime reports remain artifact-stale. |

## Unrun checks and reason

- Frozen `--diagnose-workspace-search` execution, startup, packaged GUI, and
  visual review — prohibited by the no-launch project instruction.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Cross-machine/hash reproducibility and directory-order determinism — remain
  open follow-up evidence/engineering concerns.

## Known risks and limits

- Existing historical search reports remain unbound until a permitted packaged
  diagnostic regenerates them.
- Directory traversal still samples at most the configured bound from the
  filesystem iterator; enumeration-order determinism is a separate limitation.
- D7.4.1 remains overall `in-progress` until packaged interactive and
  independent simplification/runtime evidence is available.

## Acceptance and evidence IDs

- Acceptance: `D741-AC01`, `S23`
- Evidence: `src/quillforge/app.py`,
  `docs/agent-team/reviews/D7.4.1-artifact-binding-parent-review.md`,
  `docs/adr/0021-bounded-workspace-search.md`, the rebuilt package identity,
  and the current release dossier.

## Next owner and next action

- Owner: Architect
- Action: preserve explicit artifact binding and address the remaining
  bounded-directory determinism concern only as a separately scoped change.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `BF532EDD63773EF5E902D4A95571B30D6EA3443D52F31294825EF7CBCA7C9EEF` / `38,327,952`
- Source snapshot: `tree-sha256:2dbfc18e7b3b04a0ad097909a53bf16f40695c63dfb690ec7bc2d14b36b488dd`
- Packaging note: application source changed; prior runtime reports remain
  historical until explicitly refreshed.

## Disposition

`accepted-with-limits`: future packaged search diagnostics are artifact-bound;
launch, interactive, cross-machine, and traversal-determinism limits remain.

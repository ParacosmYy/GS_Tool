# Handoff: 2026-08-09-d7-4-cancellation-callback-contract

| Field | Value |
|---|---|
| ID | `2026-08-09-d7-4-cancellation-callback-contract` |
| Delivery / slice | `D7 / D7.4 cancellation callback input contract` |
| Status | `in-progress` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T23:10:00+08:00` |

## User outcome

Find in Files now rejects malformed falsey cancellation inputs instead of
silently substituting the default callback. Normal `None` default behavior and
callable cooperative cancellation remain unchanged.

## Scope and boundaries

### In scope

- Strict runtime validation of the optional cancellation callback.
- Independent source review and a non-destructive source probe.
- D7.4 evidence and artifact synchronization after the source edit.

### Out of scope

- Search traversal policy, result ordering, directory enumeration, or new search
  features.
- Launching QuillForge, packaged search, interactive GUI, or cross-machine
  measurement.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate the smallest contract fix and final gates |
| Project Manager | Parent role record | Track D7.4 evidence and remaining release limits |
| Product | Parent role record | Preserve cooperative cancellation semantics |
| Developer 1 | Parent role record | Audit service/provider callback ownership |
| Developer 2 | Parent role record | Apply the input validation fix |
| QA | Aquinas / Luna | Independent post-fix source audit |

## Changed files and modules

- `src/quillforge/application/workspace_search.py` — distinguish `None` from
  falsey non-callable cancellation values and provide `_never_cancelled()`.
- `docs/agent-team/reviews/D7.4-cancellation-callback-parent-review.md` —
  record the parent fix and independent result.
- `docs/adr/0021-bounded-workspace-search.md` — record the callback contract.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, and this handoff — synchronize evidence.

## Decisions and constraints

- `None` means no cancellation requested; every other input must be callable.
- The callback remains provider-owned at execution time; the service does not
  invoke or mutate it.
- No search behavior, policy bounds, or stale-result semantics changed.
- Shared-checkout writer: Architect only. The Luna reviewer was read-only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Falsey non-callable cancellation source probe | `PASS` | `cancel_requested=0` raises `TypeError` |
| Aquinas / Luna post-fix source audit | `PASS` | No files changed; no launch or tests |
| `scripts/check.ps1` | `PASS` | Formatting, compile, JSON, acceptance, and project checks passed |
| `scripts/verify_handoff.ps1` | `PASS` | Index, status, required sections, and paths validated |
| `scripts/package.ps1` | `PASS` | Root/dist copies rebuilt and synchronized after the source fix |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Current dossier refreshed; exactly three stale runtime artifact failures remain |

## Unrun checks and reason

- QuillForge.exe, QApplication, packaged search, interactive cancellation,
  visual review, cross-machine, permission-pressure, and disk-pressure checks —
  prohibited or unavailable in this checkout-only boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under project policy.

## Known risks and limits

- Cancellation remains cooperative and provider-specific; a blocking provider
  can keep close rejected until its worker boundary returns.
- D7.4 remains literal and line-local; packaged interactive, cross-machine,
  directory-order, and broad performance evidence remain open.
- This fix establishes input-shape validation, not crash-proof shutdown or
  immediate worker interruption.

## Acceptance and evidence IDs

- Acceptance: `D74-AC03`, `S22`
- Evidence: `src/quillforge/application/workspace_search.py`,
  `docs/agent-team/reviews/D7.4-cancellation-callback-parent-review.md`,
  Aquinas/Luna source audit, and the falsey cancellation source probe.

## Next owner and next action

- Owner: Architect
- Action: rebuild the package, bind the current artifact identity, and retain
  D7.4 as `in-progress` until packaged/runtime and cross-machine gates resolve.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `8B52D5209B6A080972D85F150595A3F85FF0CE54AA3EA7FA000E87E679A9AB7B` /
  `38,328,441` bytes; root and dist copies match
- Source snapshot: `tree-sha256:59ff0e8850061812defb57ac63cb3fff1d866ee2a1c604421577c3183150c1bb`
- Packaging note: source fix is packaged; runtime reports remain bound to older
  artifacts and the release decision remains no-go.

## Disposition

`in-progress`: the cancellation callback contract fix and independent source
review passed, but D7.4's broader runtime, packaged, and environment gates
remain open.

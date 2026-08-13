# Handoff: 2026-08-10-d7-4-bounded-read-budget

| Field | Value |
|---|---|
| ID | `2026-08-10-d7-4-bounded-read-budget` |
| Delivery / slice | `D7 / D7.4 scan-time bounded workspace-search read budget` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-10T02:30:24+08:00` |

## User outcome

Workspace search now enforces its per-file and remaining-total byte budgets
during the actual read, even if a file changes after the initial size check.
The existing literal search, UTF-8/UTF-16 decoding, cancellation, result
limits, and UI/application boundaries remain unchanged.

## Scope and boundaries

### In scope

- Add a streaming raw-reader budget at the filesystem adapter boundary.
- Preserve exact EOF, growth detection, cancellation, max-results, and UTF text
  semantics.
- Record architecture review, independent review findings, simplification,
  public-source applicability, and static/source verification.

### Out of scope

- Regex, indexing, remote roots, cross-file replacement, plugin search, or a
  general large-file support claim.
- Handle-based no-follow/complete TOCTOU security sandboxing.
- Application/Qt launch, packaged search report regeneration, or new test assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent | Scope, architecture, integration, review, verification, and handoff |
| Project Manager | Parent role record | Dependencies, status, and open D7/D8 gates |
| Product | Parent role record | Confirm bounded-search user outcome and limits |
| Developer 1 | Parent writer | `workspace_search_provider.py` bounded reader |
| Developer 2 | Parent role record | ADR, acceptance, ledger, and package synchronization |
| QA | Fermat / Luna-max read-only; parent probes | Independent review findings and non-destructive source checks |

## Changed files and modules

- `src/quillforge/infrastructure/workspace_search_provider.py` — enforce the
  scan-time per-file/total byte budget with a streaming bounded raw reader.
- `docs/adr/0021-bounded-workspace-search.md` — record the scan-time budget
  decision and its limits.
- `docs/agent-team/reviews/D7.4-bounded-read-budget-parent-review.md` — review,
  applicability, simplification, evidence, and risks.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  and `docs/ROADMAP.md` — record the source-boundary evidence while leaving
  D74-AC04 open.
- `docs/handoffs/index.json` — current handoff ledger entry.

## Decisions and constraints

- The filesystem adapter, not the application service or dialog, owns byte-read
  enforcement; DTOs and presentation/application seams remain unchanged.
- `limit == 0` is valid only as a zero-byte budget; the reader probes for growth
  but never consumes body bytes.
- A boundary probe byte is not counted as scanned content.
- Shared checkout writer: parent Architect only; no worktree or second writer.
- Runtime launch policy: prohibited by the active project no-launch boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge` | PASS | Static compilation |
| `uv run ruff check src/quillforge/infrastructure/workspace_search_provider.py` | PASS | Static lint |
| Bounded reader source probe | PASS | Cap, exact EOF, growth, close forwarding, and zero-budget behavior |
| UTF-8/UTF-16 provider source probe | PASS | Existing search semantics preserved |
| `scripts/verify_handoff.ps1` | PASS | Handoff index and Markdown status agree |
| `scripts/check.ps1` | PASS | Formatting, static, JSON, acceptance, and project checks pass |
| `scripts/package.ps1` | PASS | Root/dist Windows x64 candidate rebuilt |
| Package identity | PASS | Root/dist SHA-256 `D6C08C7B1C3098643A7921C85B9791E87A2DD41C5A745AD3E0E964CE6F918488`; both 38,365,221 bytes; source revision `tree-sha256:e4e9406ae3c75f83eb6245cf60803e7f787ecb914bbbef3195035bf457007acd` |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Current dossier matches the candidate; three stale runtime-report failures and ten external release gates remain |

## Unrun checks and reason

- Packaged/source interactive search, report regeneration, QApplication, visual,
  clean-machine, cross-machine, pressure, and hard-power checks — prohibited or
  unavailable under current instructions.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under project policy.

## Known risks and limits

- A bounded read budget is not a complete handle-based no-follow or security
  sandbox boundary; existing symlink/reparse and TOCTOU limits remain explicit.
- D74-AC04 remains `in-progress` because current packaged report freshness,
  interactive search, and cross-machine evidence require authorized runtime.
- D7.3 permission/disk-pressure evidence and D8 release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D74-AC01`, `D74-AC02`, `D74-AC03`, `D74-AC04`, `S22`
- Evidence: `docs/agent-team/reviews/D7.4-bounded-read-budget-parent-review.md`,
  `src/quillforge/infrastructure/workspace_search_provider.py`, bounded-reader
  source probe, UTF-8/UTF-16 provider source probe, and final package identity.

## Next owner and next action

- Owner: Architect.
- Action: run handoff/project/package gates, then continue with D7/D8 only where
  the remaining evidence can be produced without violating the no-launch rule.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `D6C08C7B1C3098643A7921C85B9791E87A2DD41C5A745AD3E0E964CE6F918488` / `38,365,221` bytes; root/dist match.
- Packaging note: rebuilt portable one-file candidate; unsigned/manual-update/no-installer posture unchanged.

## Disposition

`accepted-with-limits`: the source-level search work bound is implemented and
reviewed. D74-AC04 remains open for packaged/report/runtime evidence; no broad
performance or security claim is made.

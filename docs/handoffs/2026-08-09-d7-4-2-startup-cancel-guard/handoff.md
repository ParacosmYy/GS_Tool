# Handoff: 2026-08-09-d7-4-2-startup-cancel-guard

| Field | Value |
|---|---|
| ID | `2026-08-09-d7-4-2-startup-cancel-guard` |
| Delivery / slice | `D7 / D7.4.2 startup workspace-restore cancellation` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:05:00+08:00` |

## User outcome

Cancelling a workspace open that was started during session restoration no
longer leaves QuillForge waiting forever on an unreleased startup barrier. The
cancelled worker and its stale queued completion remain contained by the
existing TaskRunner and generation guards.

## Scope and boundaries

### In scope

- Release the session-restore workspace barrier on the explicit cancellation
  path.
- Preserve stale completion invalidation and the existing normal workspace
  cancellation behavior.
- Synchronize the D7.4.2 ADR, review, acceptance evidence, delivery register,
  and handoff index.

### Out of scope

- A new cancellation primitive, forced worker termination, or synchronous
  shutdown wait.
- Redesigning the bounded workspace directory-sampling policy.
- External plugin execution, release signing, installer/update work, or visual
  UI acceptance.
- Starting QuillForge or instantiating a Qt application under the current
  no-launch project instruction.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Call-chain audit, smallest fix, integration, and final verification |
| Project Manager | Parent role record | Bounded D7.4.2 follow-up scope and remaining evidence gates |
| Product | Parent role record | Confirmed cancellation must not strand startup restoration |
| Developer 1 | Parent role record | Audited operation/generation and TaskRunner ownership boundaries |
| Developer 2 | Parent role record | Applied the presentation lifecycle fix in `MainWindow` |
| QA | Gauss / Luna, read-only | Identified the stale completion/barrier defect and recorded unrun limits |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — release the startup session
  barrier after cancelling its workspace operation.
- `docs/adr/0022-task-runner-lifecycle-guard.md` — record the startup-barrier
  cancellation decision and scope.
- `docs/agent-team/reviews/D7.4.2-startup-cancellation-parent-review.md` —
  record the parent audit and review limits.
- `docs/agent-team/acceptance.json` — add startup-barrier release evidence to
  D742-AC01.
- `docs/agent-team/delivery-register.json` and `docs/ROADMAP.md` — synchronize
  D7.4.2 evidence and limitation wording.
- `docs/handoffs/index.json` — index this material slice.

## Decisions and constraints

- The existing `_finish_session_workspace_restore()` helper is reused rather
  than duplicating session-restore queue logic.
- The barrier is released only when the cancelled workspace operation owns it;
  ordinary workspace cancellation remains unchanged.
- Generation invalidation remains authoritative for the stale worker callback;
  cancellation is not treated as worker completion.
- Shared-checkout writer: Architect only; no child writer was used.
- Runtime launch policy: launch is forbidden by the current project
  instruction; static checks and packaging are allowed.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D7.4.2 parent source audit` | `PASS` | The cancel path now snapshots and releases the startup barrier while invalidating the operation generation. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff index, path, headings, and evidence coverage passed before packaging. |
| `scripts/check.ps1` | `PASS` | Formatting, compile, JSON/acceptance policy, and project checks passed before packaging. |
| `scripts/package.ps1` | `PASS` | Root/dist copies rebuilt and synchronized after the source fix. |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, interactive cancel, and visual
  review — intentionally unrun because the project instruction prohibits
  launching the software; these remain user-owned follow-up evidence.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Clean-machine, cross-machine, hard-power, and permission-pressure evidence —
  not authorized or available in this local static-only slice.
- Independent after-source/simplification child PASS — not claimed; the
  available read-only Luna review supplied the finding but not an independent
  completion gate.

## Known risks and limits

- The independent source review is now recorded in the follow-up handoff;
  the current delivery register is `accepted-with-limits`. Runtime and
  broader environment gates remain open.
- A stale workspace worker may continue briefly after cancellation; the
  guarantee is containment and stale-result rejection, not immediate stop.
- Directory entries are bounded by a sampled prefix of filesystem enumeration
  before sorting; this is not a full-directory deterministic selection
  guarantee when the bound is exceeded.
- Current packaged search/performance/startup evidence is bound to older
  artifacts and must not be reused as evidence for the post-fix package.

## Acceptance and evidence IDs

- Acceptance: `D742-AC01`, `S24`
- Evidence: `src/quillforge/presentation/main_window.py`,
  `docs/adr/0022-task-runner-lifecycle-guard.md`,
  `docs/agent-team/reviews/D7.4.2-startup-cancellation-parent-review.md`,
  `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and the post-fix package
  identity.

## Next owner and next action

- Owner: Architect
- Action: run the required handoff/static/package gates, bind the resulting
  artifact identity, then continue with the D8/D9 source-only gaps while
  retaining the no-launch release blockers.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `F55591416171ADBBEC0EA54443C060040619EB920B7D2B3553E4E5D38BC93447` / `38,326,950` bytes; root/dist copies match
- Packaging note: rebuilt after the source fix; startup/performance evidence
  remains unrun or bound to older artifacts.

## Disposition

`accepted-with-limits`: the startup barrier defect is fixed in source, the
required static/package gates pass, and the follow-up Luna source review
returned PASS. Project-owned runtime/interactive evidence and broader
environment gates remain open.

# Handoff: 2026-08-09-d7-4-2-independent-luna-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d7-4-2-independent-luna-review` |
| Delivery / slice | `D7 / D7.4.2 TaskRunner lifecycle guard` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T22:15:00+08:00` |

## User outcome

An independent Luna source audit confirms that TaskRunner retains work through
one queued completion, startup workspace-restore cancellation releases its
barrier, stale completions remain generation-guarded, and the close guard is
non-blocking. The delivery remains bounded by provider-specific cooperative
cancellation and unrun runtime evidence.

## Scope and boundaries

### In scope

- Independent post-fix read-only review of the D7.4.2 lifecycle contract.
- Verification that the existing source and evidence state match the stated
  accepted-with-limits disposition.
- Recording the reviewer, exact source evidence, and unresolved boundary.

### Out of scope

- New cancellation primitives, forced termination, or synchronous shutdown
  waits.
- Starting QuillForge, instantiating a Qt application, or interactive visual
  and accessibility acceptance.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integration, disposition, and final static gates |
| Project Manager | Parent role record | Kept the follow-up bounded to the D7.4.2 source contract |
| Product | Parent role record | Confirmed close must not strand startup restoration |
| Developer 1 | Parent role record | Confirmed TaskRunner and generation ownership boundaries |
| Developer 2 | Parent role record | Confirmed the previously applied startup-barrier fix |
| QA | Banach / Luna | Independent read-only source audit |

## Changed files and modules

- `docs/agent-team/reviews/D7.4.2-independent-luna-follow-up.md` — record the
  independent Luna disposition and source references.
- `docs/handoffs/2026-08-09-d7-4-2-independent-luna-review/handoff.md` —
  record this documentation-only handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `docs/adr/0022-task-runner-lifecycle-guard.md`,
  and `docs/ROADMAP.md` — synchronize the new evidence without changing the
  delivery or acceptance status.

## Decisions and constraints

- The independent result is recorded as source-only `PASS`, while the
  delivery remains `accepted-with-limits` and the acceptance/register remain
  `in-progress`.
- No source or packaging code changed, so the current packaged identity is
  referenced for context but no new package is claimed.
- The current no-launch policy, single shared-checkout writer rule, and
  no-test-asset policy remain in force.

## Evidence and decisions

- Banach returned `PASS` for the source-only audit and recommended
  accepted-with-limits.
- `task_runner.py:21-88` retains submitted tasks through one queued completion
  and cleanup.
- `main_window.py:682-698` releases the startup restore barrier on the
  applicable cancellation path; `main_window.py:733` rejects stale completion
  ownership.
- `main_window.py:2211-2259` keeps close non-blocking and does not force
  terminate or synchronously wait for the thread pool.
- The highest-risk remaining behavior is explicit: a blocking provider can
  keep close rejected until its cooperative worker boundary returns.
- The acceptance and delivery register now record D7.4.2 as
  `accepted-with-limits`; this handoff does not establish runtime startup,
  interactive cancellation, or cross-machine behavior.

## Known risks and limits

- Cancellation remains cooperative and provider-specific; a blocking provider
  can keep close rejected until its worker boundary returns.
- Runtime startup, interactive cancellation, screen-reader/DPI/visual review,
  clean-machine, cross-machine, permission-pressure, and hard-power evidence
  remain open.
- This source audit does not establish crash-proof shutdown, forced
  termination safety, or OS-termination durability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Banach / Luna read-only source audit | `PASS` | No files changed; no runtime launch or tests |
| `scripts/verify_handoff.ps1` | `PASS` | Index, path, title, status, and required sections validated |
| `scripts/check.ps1` | `PASS` | Formatting, compilation, JSON, acceptance, and project checks passed |
| `scripts/package.ps1` | `NOT RUN` | No source or packaging files changed |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, interactive cancellation,
  screen-reader/DPI/visual review — prohibited by the current project
  instruction.
- Clean-machine, cross-machine, permission-pressure, and hard-power evidence —
  not authorized or available in this local static-only slice.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.

## Artifact information

No source or packaging artifact changed in this documentation-only follow-up.
The current release artifact is
`8B52D5209B6A080972D85F150595A3F85FF0CE54AA3EA7FA000E87E679A9AB7B`,
38,328,441 bytes, with source snapshot
`tree-sha256:59ff0e8850061812defb57ac63cb3fff1d866ee2a1c604421577c3183150c1bb`;
this review does not claim fresh runtime evidence for it.

## Next owner and next action

- Owner: Architect
- Action: retain the explicit D7.4.2 limits and continue the remaining
  D6/D7/D8/D9 external and runtime evidence gaps.

## Acceptance and evidence IDs

- Acceptance: `D742-AC01`, `S24`
- Evidence: `docs/agent-team/reviews/D7.4.2-independent-luna-follow-up.md`,
  `src/quillforge/presentation/task_runner.py`,
  `src/quillforge/presentation/main_window.py`, and the synchronized handoff,
  acceptance, register, ADR, and roadmap records.

## Disposition

`accepted-with-limits`: the independent source audit returned PASS. Runtime,
interactive cancellation, and broader environment evidence remain open, and
the provider-specific cooperative cancellation boundary remains explicit.

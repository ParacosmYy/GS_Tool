# Handoff: 2026-08-09-ui-04-status-rail

| Field | Value |
|---|---|
| ID | `2026-08-09-ui-04-status-rail` |
| Delivery / slice | `D9 / UI-04 explicit shell status rail` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T15:30:00+08:00` |

## User outcome

The shell projects explicit READY, WORKING, ATTENTION, and ERROR states through
a compact status rail. The rail is presentation-only, uses explicit lifecycle
calls, and does not parse notification text or weaken TaskRunner/close-guard
semantics.

## Scope and boundaries

### In scope

- `StatusRail` phase and label contract.
- MainWindow projection at the existing document-operation, completion, error,
  and active-document dirty-state boundaries.
- Centralized state-selector styling and static/package evidence.

### Out of scope

- Replacing notifications, TaskRunner, close guards, or application services.
- Adding new background-operation orchestration; search, session, recovery, and
  plugin operation coverage remains a documented follow-up.
- Runtime visual, DPI, font, native accessibility, and screenshot acceptance
  while the no-launch policy remains active.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrated the phase boundary and made the limited status decision |
| Project Manager | Kepler / Luna | Sequenced UI-04 after the dialog-surface delivery |
| Product | Pauli / Luna | Confirmed explicit lifecycle language and user-facing failure intent |
| Developer 1 | Dirac / Luna | Audited application ownership; no overlapping source change required |
| Developer 2 | Franklin / Luna | Audited StatusRail/MainWindow/theme projection and recorded coverage risk |
| QA | Copernicus / Luna | Confirmed static/package evidence and unrun runtime limits |

## Changed files and modules

- `docs/ROADMAP.md` — records UI-04 as accepted-with-limits with its coverage limit.
- `docs/agent-team/delivery-register.json` — records UI-04 status and evidence.
- `docs/agent-team/acceptance.json` — records S28 as accepted-with-limits.
- `docs/handoffs/index.json` — indexes this handoff as the latest slice.
- `docs/handoffs/2026-08-09-ui-04-status-rail/handoff.md` — records the bounded decision.

Source evidence is in `src/quillforge/presentation/status_bar.py`,
`src/quillforge/presentation/main_window.py`, and `theme.py`.

## Decisions and constraints

- Status phases are explicit values owned by the presentation component; no
  notification string parsing is introduced.
- The status rail supplements, rather than replaces, task lifecycle and close
  safety contracts.
- Shared checkout writer: Architect, limited to the delivery ledger and
  handoff files; no child writer slot was used.
- Runtime launch policy: forbidden by the current project instruction; static
  checks and packaging evidence are allowed.
- `S28` is accepted with limits. `D9-AC03` and D9 remain in progress because
  independent background-operation coverage and visual acceptance are open.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `.\scripts\verify_handoff.ps1` | `PASS` | Handoff index and required record structure pass after this entry is added. |
| `.\scripts\check.ps1` | `PASS` | Acceptance/register policy, architecture boundaries, lock, formatting, lint, and compile checks pass. |
| `D9-UI-04-parent-review.md` | `PASS / proceed with limits` | Records the bounded phase map and lifecycle projection. |
| UI-04 package evidence | `PASS` | Root/dist portable candidate: 38,324,566 bytes; SHA-256 `AC8F5D7C0A8FE0C189D58D0E2CFBF1A1BCD18930709DE92F669B686D5B1B1FDD`. |
| `uv run ruff check src scripts` | `PASS` | No source or script lint findings. |
| `uv run ruff format --check src scripts` | `PASS` | All 62 files already formatted. |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, and screenshot — intentionally
  unrun because starting the software is prohibited; user/Product owns the
  later visual review if that instruction is reversed.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- A new package rebuild — not needed for this ledger-only handoff; the UI-04
  package identity is recorded from its source slice review.

## Known risks and limits

- Search, session, recovery, and plugin background work do not yet have a
  single verified status projection; extending that coverage is a separate
  implementation slice.
- Native accessibility rendering, DPI, font availability, and visual contrast
  remain unverified.
- The current checkout has no Git metadata; files and recorded command output
  are the authoritative state.

## Acceptance and evidence IDs

- Acceptance: `S28`, `D9-AC03`
- Evidence: `status_bar.py`, `main_window.py`, `theme.py`,
  `docs/agent-team/reviews/D9-UI-04-parent-review.md`, `.\scripts\check.ps1`

## Next owner and next action

- Owner: Architect
- Action: deliver UI-05's editor canvas, then decide whether the open status-
  projection coverage is a separate bounded follow-up before visual review.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: UI-04 review artifact `AC8F5D7C0A8FE0C189D58D0E2CFBF1A1BCD18930709DE92F669B686D5B1B1FDD` / `38,324,566` bytes
- Packaging note: portable one-file candidate; no new rebuild in this ledger handoff.

## Disposition

`accepted-with-limits`: UI-04 source and package evidence are complete for this
bounded phase projection, while broader background coverage and visual/startup
acceptance remain intentionally open. The D9 root and UI-05 remain in progress.

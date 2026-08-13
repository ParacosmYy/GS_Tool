# Handoff: 2026-08-09-ui-02-command-rail

| Field | Value |
|---|---|
| ID | `2026-08-09-ui-02-command-rail` |
| Delivery / slice | `D9 / UI-02 modern command rail` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T15:10:00+08:00` |

## User outcome

QuillForge has a compact top command rail for New, Open, Save, Find, Replace,
Command palette, and Workspace. It delegates to the existing MainWindow
intents, keeps menu shortcuts and application ownership unchanged, and carries
the ink/violet shell identity into the packaged candidate.

## Scope and boundaries

### In scope

- Presentation-only command rail projection in `MainWindow`.
- Fixed, non-floating toolbar behavior with standard native icons where
  available and stable text labels/tooltips for core actions.
- Centralized command-rail styling, workspace icon mapping, and packaged icon
  registration evidence.

### Out of scope

- New application commands, service ownership, plugin APIs, keyboard shortcut
  semantics, or session-persisted toolbar layout.
- Dialog surfaces, status-rail coverage, editor-canvas styling beyond the
  already separate UI-03/UI-04/UI-05 slices.
- Runtime startup, screenshot, DPI, font, native-dialog, or accessibility
  visual acceptance while the no-launch policy remains active.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrated the source/package evidence and made the bounded status decision |
| Project Manager | Kepler / Luna | Selected UI-02 as the lowest-coupling evidence-closure slice |
| Product | Pauli / Luna | Confirmed the fast-access command-rail user outcome and limits |
| Developer 1 | Dirac / Luna | Audited application/infrastructure alternatives; no overlapping change required |
| Developer 2 | Franklin / Luna | Confirmed the toolbar call sites, theme selectors, and package evidence |
| QA | Copernicus / Luna | Confirmed static/package evidence and recorded unrun visual/startup checks |

## Changed files and modules

- `docs/ROADMAP.md` — records UI-02 as accepted-with-limits while D9 remains in progress.
- `docs/agent-team/delivery-register.json` — records UI-02 status, evidence, and D9 as current delivery.
- `docs/agent-team/acceptance.json` — records S26 as accepted-with-limits with its handoff evidence.
- `docs/handoffs/index.json` — indexes this slice as the latest handoff.
- `docs/handoffs/2026-08-09-ui-02-command-rail/handoff.md` — records the delivery boundary.

Source evidence for the slice is held in `src/quillforge/presentation/main_window.py`,
`src/quillforge/presentation/theme.py`, `src/quillforge/presentation/icons.py`,
`src/quillforge/presentation/workspace_panel.py`, and
`packaging/quillforge.spec`.

## Decisions and constraints

- Toolbar actions call existing MainWindow intents; no command or domain logic
  is duplicated in the toolbar.
- The toolbar is non-movable and non-floating, so session continuity does not
  acquire a new layout payload.
- Shared checkout writer: Architect, limited to the delivery ledger and
  handoff files; no child writer slot was used.
- Runtime launch policy: forbidden by the current project instruction; static
  checks and packaging evidence are allowed.
- `S26` is accepted with limits. `D9-AC01` and the D9 root remain in progress
  because the broader visual sprint and user-owned runtime review are open.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `.\scripts\verify_handoff.ps1` | `PASS` | Handoff index and required record structure pass after this entry is added. |
| `.\scripts\check.ps1` | `PASS` | Acceptance/register policy, architecture boundaries, lock, formatting, lint, and compile checks pass. |
| `D9-UI-02-parent-review.md` | `PASS / proceed with limits` | Records Ruff, static check, package completion, and root/dist identity. |
| UI-02 package evidence | `PASS` | Root/dist portable candidate: 38,320,593 bytes; SHA-256 `AFB7D1A1B07060375D1A5783F3820287FA26541957BD7F3FF0DF8798D100AD80`. |
| `uv run ruff check src scripts` | `PASS` | No source or script lint findings. |
| `uv run ruff format --check src scripts` | `PASS` | All 62 files already formatted. |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, and screenshot — intentionally
  unrun because starting the software is prohibited; user/Product owns the
  later visual review if that instruction is reversed.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- A new package rebuild — not needed for this ledger-only handoff; the UI-02
  package identity is recorded from its source slice review.

## Known risks and limits

- Native icon glyph appearance, DPI scaling, font availability, accessibility
  contrast, and native rendering remain unverified.
- The command rail does not establish release readiness, clean-machine support,
  signing, installer, update, or cross-machine visual claims.
- The current checkout has no Git metadata; files and recorded command output
  are the authoritative state.

## Acceptance and evidence IDs

- Acceptance: `S26`, `D9-AC01`
- Evidence: `src/quillforge/presentation/main_window.py`,
  `src/quillforge/presentation/theme.py`, `src/quillforge/presentation/icons.py`,
  `packaging/quillforge.spec`,
  `docs/agent-team/reviews/D9-UI-02-parent-review.md`, `.\scripts\check.ps1`

## Next owner and next action

- Owner: Architect
- Action: deliver UI-03 dialog surfaces with the same static-only boundary,
  then UI-04/UI-05; request user-permitted runtime visual review before any
  D9 slice is called fully completed.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: UI-02 review artifact `AFB7D1A1B07060375D1A5783F3820287FA26541957BD7F3FF0DF8798D100AD80` / `38,320,593` bytes
- Packaging note: portable one-file candidate; no new rebuild in this ledger handoff.

## Disposition

`accepted-with-limits`: UI-02 source and package evidence are complete for this
bounded iteration, while visual/startup acceptance remains intentionally open.
The D9 root and UI-03/UI-04/UI-05 remain in progress.

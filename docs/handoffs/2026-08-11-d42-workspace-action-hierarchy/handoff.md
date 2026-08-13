# Handoff: 2026-08-11-d42-workspace-action-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d42-workspace-action-hierarchy` |
| Delivery / slice | `D42 / ARCH-32 / UI-28 Workspace action hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T01:30:00+08:00` |

## User outcome

The workspace panel now presents Open Workspace as the primary action while
Back and Cancel read as quiet secondary actions. Hover, keyboard focus,
pressed, and disabled states are explicitly distinguishable through the
selected theme, addressing the previous flat/undifferentiated button rhythm.

## Scope and boundaries

### In scope

- Centralized QSS state selectors for existing `workspaceBack` and
  `workspaceCancel` controls.
- Preservation of the existing primary action, signals, labels, locale,
  loading state, and application policy.
- Static source, architecture, handoff, package, and release evidence.

### Out of scope

- New widgets, theme tokens, animation engines, signal wiring, workspace
  services, loading guards, locale policy, or application behavior.
- Qt startup, native rendering, screenshots, accessibility runtime, clean
  machine, cross-machine, signing, installer, update, and release approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | Visual hierarchy outcome and acceptance |
| Developer 1 | Parent architect | Centralized QSS implementation |
| Developer 2 | Parent architect | Contract/docs/package synchronization |
| QA | Parent architect | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — explicit Back/Cancel QSS states.
- `docs/adr/0067-workspace-action-hierarchy.md` — architecture decision,
  applicability, simplification, and limits.
- `docs/agent-team/reviews/D42-workspace-action-hierarchy-parent-review.md` —
  parent architecture/review record.
- `docs/agent-team/reviews/D42-workspace-action-hierarchy-independent-review.md`
  — explicit no-conclusion independent-review record.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` —
  traceability and package evidence.

## Decisions and constraints

- Use existing object names and existing theme tokens; do not add a second
  styling system or a new state owner.
- Preserve every signal, object name, locale key, loading guard, and policy
  boundary.
- Bacon the 2nd / Luna max architecture window returned no conclusion after
  two bounded waits; no architecture PASS is claimed. Gibbs the 2nd / Luna
  max independent review window also returned no conclusion; no child PASS is
  claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| UI-28 theme-hierarchy source probe | PASS | Existing object names plus Back/Cancel hover, focus, pressed, disabled, warning/gold, and no-new-theme-data checks. |
| `uv run python -m compileall -q src/quillforge/presentation/theme.py` | PASS | Authorized static compilation only. |
| `uv run ruff check src/quillforge/presentation/theme.py` | PASS | No diagnostics. |
| `uv run ruff format --check src/quillforge/presentation/theme.py` | PASS | File already formatted. |
| `scripts\verify_handoff.ps1` | PASS | Indexed handoff and Markdown status/required headings agree. |
| `scripts\check.ps1` | PASS | Repository static and traceability checks pass. |
| `scripts\package.ps1` | PASS | Root/dist portable candidates share the recorded D42 identity. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime-report consistency and external release gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, native QSS rendering, focus traversal, screenshots,
  screen-reader output, font metrics, DPI, and cross-machine appearance —
  blocked by the active no-launch policy and lack of authorized runtime
  acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- QSS selector specificity and platform style metrics may render differently
  in a native Qt session; the source keeps selectors local and explicit but
  cannot prove runtime pixels.
- The warning/gold Cancel hover state uses existing theme endpoints; source
  evidence covers token selection, while installed-font and platform contrast
  remain runtime limits.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Acceptance and evidence IDs

- Acceptance: `D42-AC01`, `S71`.
- Evidence: `docs/adr/0067-workspace-action-hierarchy.md`,
  `docs/agent-team/reviews/D42-workspace-action-hierarchy-parent-review.md`,
  `docs/agent-team/reviews/D42-workspace-action-hierarchy-independent-review.md`,
  `UI28-theme-hierarchy-probe=PASS`, static checks, and
  `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: Architect.
- Action: continue with D7/D8 runtime, clean-machine, and release-owner gates
  when separately authorized.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identity matches.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the D42 action-hierarchy QSS slice is integrated with
explicit architecture, review status, simplification, and static verification
evidence. Native rendering and release-owner gates remain conditions for later
work.

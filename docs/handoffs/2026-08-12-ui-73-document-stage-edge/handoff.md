# Handoff: 2026-08-12-ui-73-document-stage-edge

| Field | Value |
|---|---|
| ID | 2026-08-12-ui-73-document-stage-edge |
| Delivery / slice | UI-73 / ARCH-128 document-stage edge |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T11:00:00+08:00 |

## User outcome

The document tab rail now flows into the editor canvas without a duplicated
intermediate pane outline. The tab rail and editor canvas continue to provide
the navigation and work-area boundaries that users need to scan the shell.

## Scope and boundaries

### In scope

- The centralized `QTabWidget#documentTabs::pane` QSS selector.
- Static selector/state coverage and theme/accent projection.
- Package identity and traceability records.

### Out of scope

- No `DocumentTabSurface`, `EditorWidget`, `FindSurface`, signal, keyboard,
  locale, font, motion, settings, or application-policy change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | James the 5th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Godel the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — scoped document-pane QSS only.
- `tasks/plan.md` and `tasks/todo.md` — bounded UI-73 scope and status.
- `docs/adr/0190-document-stage-edge.md`
- `docs/agent-team/reviews/UI-73-document-stage-edge-parent-review.md`
- `docs/agent-team/reviews/UI-73-document-stage-edge-independent-review.md`

## Decisions and constraints

- `theme.py` remains the single visual source; no second stylesheet or asset
  was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `UI73-DOCUMENT-STAGE-PROBE=PASS`
- `UI73-QSS-PROJECTION-PROBE=PASS:12 theme/accent projections`
- `UI73-COMPILEALL=PASS`
- `UI73-RUFF=PASS`
- `UI73-FORMAT=PASS`
- `UI73-PACKAGE-BUILD=PASS`
- `UI73-PACKAGE-IDENTITY-PROBE=PASS`
- `UI73-NO-PROCESS-PROBE=PASS`
- `UI73-CHECK=PASS`
- `UI73-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native QSS rendering, runtime startup, font/DPI, accessibility,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS evidence cannot prove native style-engine specificity or final
  output on every Windows font/DPI combination.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S194`, `UI-73-AC01`.
- Evidence: ADR-0190, parent/independent review records, UI73 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `709FFAB43528F82819A4E2565EE02CC3FF5B7A72E701EC2AED3671DFCEECC9F6`
- Size: `38536315` bytes
- Source revision: `tree-sha256:f5de41605177c15c4964b43628036883df419843a60ceca127d13aeabaaef662`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: the redundant document-stage outline is removed in the
centralized stylesheet; native rendering, runtime, release, and external
evidence gates remain open.


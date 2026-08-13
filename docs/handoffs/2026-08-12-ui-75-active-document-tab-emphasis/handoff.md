# Handoff: 2026-08-12-ui-75-active-document-tab-emphasis

| Field | Value |
|---|---|
| ID | 2026-08-12-ui-75-active-document-tab-emphasis |
| Delivery / slice | UI-75 / ARCH-133 active document tab emphasis |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T16:00:00+08:00 |

## User outcome

The active document tab now regains a visible token-driven left edge in the
specific document-tab rail. The selected tab is easier to scan without adding
another widget or changing tab geometry, focus, close affordance, or behavior.

## Scope and boundaries

### In scope

- Existing `QTabBar#documentTabBar::tab:selected` and selected-hover QSS.
- Theme/accent projection and selected-state preservation source coverage.
- Package identity and traceability records.

### Out of scope

- No tab widget, signal, keyboard, document state, icon, locale, font, motion,
  close lifecycle, layout metric, or application-policy change.
- No QApplication/EXE launch, native rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Kuhn the 5th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Pascal the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — selected-tab accent-left QSS only.
- `tasks/plan.md` and `tasks/todo.md` — bounded UI-75 scope and status.
- `docs/adr/0195-active-document-tab-emphasis.md`
- `docs/agent-team/reviews/UI-75-active-document-tab-emphasis-parent-review.md`
- `docs/agent-team/reviews/UI-75-active-document-tab-emphasis-independent-review.md`

## Decisions and constraints

- `theme.py` remains the single visual source; no second stylesheet or asset
  was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `UI75-TAB-HIGHLIGHT-PROBE=PASS:12 theme/accent projections`
- `UI75-STATE-PRESERVATION-PROBE=PASS`
- `UI75-PRESENTATION-AUDIT=PASS`
- `UI75-COMPILEALL=PASS`
- `UI75-RUFF=PASS`
- `UI75-FORMAT=PASS`
- `UI75-PACKAGE-BUILD=PASS`
- `UI75-PACKAGE-IDENTITY-PROBE=PASS`
- `UI75-CHECK=PASS`
- `UI75-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native QSS rendering, tab metrics, font/DPI, accessibility,
  runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static stylesheet projection cannot prove native QSS specificity and tab
  geometry under every Windows style/DPI combination.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S199`, `UI-75-AC01`.
- Evidence: ADR-0195, parent/independent review records, UI75 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the stylesheet change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `0479DD015FE188C66759C9B7B3BDC828C0BEAF6BD80F3B87501065ECA8E57250`
- Size: `38543443` bytes
- Source revision: `tree-sha256:0d06b3c925489a64dc62091eea8c1b463662cd21c315bd954a993305c2f1bb5d`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: selected document-tab emphasis is restored through
centralized token QSS; native rendering, runtime, release, and external
evidence gates remain open.

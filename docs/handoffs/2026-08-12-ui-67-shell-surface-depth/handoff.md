# Handoff: 2026-08-12-ui-67-shell-surface-depth

| Field | Value |
|---|---|
| ID | 2026-08-12-ui-67-shell-surface-depth |
| Delivery / slice | UI-67 / ARCH-110 shell surface depth and primary-work-area hierarchy |
| Status | `accepted-with-limits` |
| Checkout | Current local checkout only |
| Created | 2026-08-12T01:30:00+08:00 |
| Owner | architect |
| Source change | `src/quillforge/presentation/theme.py` |
| ADR | `docs/adr/0172-shell-surface-depth.md` |
| Artifact | `dist/QuillForge.exe` and `QuillForge.exe` |
| Artifact SHA-256 | `DC0B1B8DF12B843E0B1755246C01BD52F5EEEB25B222B99D19197606EE326E8F` |
| Artifact bytes | `38515462` |
| Source revision | `tree-sha256:26bad544d962f2f2ba9e16e25f939ed841bd3218d0dfa0fefbb139cf394d6949` |

## User outcome

The centralized stylesheet now gives the main canvas, editor stage, command
rail, status rail, document tab rail/item, workspace dock, and workspace panel
an explicit surface ladder. Existing semantic hover, pressed, checked,
selected, focus, disabled, warning, primary, quiet, and context states remain
unchanged. No behavior or ownership moved.

## Scope and boundaries

### In scope

- Existing surface-token values and semantic QSS selectors in
  `src/quillforge/presentation/theme.py`.
- Static token/contrast/source checks and a rebuilt portable package.

### Out of scope

- No command callback, document/tab/Find, workspace signal, locale, font,
  motion, or application policy change.
- No new widget, token family, animation, async path, worker, or test-only
  asset.
- No QApplication launch, native rendering capture, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Locke the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Erdos the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — existing QSS surface projection only.
- `docs/adr/0172-shell-surface-depth.md`
- `docs/agent-team/reviews/UI-67-shell-surface-parent-review.md`
- `docs/agent-team/reviews/UI-67-shell-surface-independent-review.md`

## Decisions and constraints

- The centralized theme stylesheet remains the only owner of shell colors and
  state selectors; layout and behavior owners remain unchanged.
- Existing contrast-derived foreground helpers remain the single source for
  accent and warning text.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; static, inline, package, and handoff
  evidence are the authorized boundary.

## Review record

- Locke the 4th / Luna max architecture window: `NO_CONCLUSION`.
- Erdos the 4th / Luna max independent review window: `NO_CONCLUSION`.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Public applicability: Python 3.12/PyQt6 presentation QSS; embedded C/C++
  and manufacturer requirements are not applicable. CloudWeGo remains an
  engineering reference only.

## Verification commands and results

- `UI67-COMPILEALL=PASS`
- `UI67-RUFF=PASS`
- `UI67-FORMAT=PASS`
- `UI67-TOKEN-CONTRAST-PROBE=PASS`
- `UI67-SHELL-SELECTOR-PROBE=PASS`
- `UI67-PACKAGE-BUILD=PASS`
- `UI67-CHECK=PASS`
- `UI67-VERIFY-HANDOFF=PASS`
- package root/dist identity and no-process probe recorded after handoff sync

## Unrun checks and reason

No QApplication or executable was started. Native QSS rendering, visual
screenshots, keyboard traversal, screen-reader output, DPI/font fallback,
clean-machine behavior, cross-machine behavior, signing, installer, and
release-owner gates remain open.

## Known risks and limits

- Static QSS and contrast evidence cannot prove native geometry or visual
  quality on every Windows style, DPI, and installed-font configuration.
- Locke architecture and Erdos independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S176`, `UI-67-AC01`.
- Evidence: ADR-0172, shell selector and contrast probes, parent and
  independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO,
  and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded visual-quality or MainWindow/application
  contract slice and complete authorized runtime/release gates when authority
  and environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `DC0B1B8DF12B843E0B1755246C01BD52F5EEEB25B222B99D19197606EE326E8F`
- Size: `38515462` bytes
- Source revision: `tree-sha256:26bad544d962f2f2ba9e16e25f939ed841bd3218d0dfa0fefbb139cf394d6949`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: shell surface hierarchy is centralized at the
existing presentation token boundary while native/runtime/release evidence
remains open.

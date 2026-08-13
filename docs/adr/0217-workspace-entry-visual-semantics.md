# ADR-0217: Make workspace entry kinds visually scannable

- **Status:** accepted-with-limits; D168 / UI-80 / ARCH-155 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace tree already used authored vector icons, but file, directory, and
inaccessible entries were projected with the same normal foreground/accent
pair. The shape difference was not reinforced by localized guidance, so quick
visual scanning was weaker than intended. The existing first-click file,
double-click folder, and keyboard activation behavior is already the correct
interaction contract and must not move.

## Decision

Keep the existing `WorkspacePanel` presentation boundary and add two small
projections:

1. Files use the existing `QPalette.Link` semantic color for their document
   outline on dark canvases. On light canvases they use the primary text role,
   because the named pressed-row surface can make the link color too weak.
2. Folders retain a primary-text outline with the existing link-colored detail
   fill; inaccessible entries retain the existing warning icon and disabled
   palette rendering.
3. File, folder, and unavailable-item tooltips are localized in English and
   Chinese. A provider diagnostic (`WorkspaceEntry.error`) always takes
   precedence and remains unchanged.

The existing icon refresh route reprojects visible entries after a theme or
accent change. Locale refresh updates only semantic hints and does not rewrite
diagnostic tooltips.

## Invariants

1. Only `src/quillforge/presentation/workspace_panel.py` and
   `src/quillforge/presentation/i18n.py` change for the feature.
2. `WorkspaceEntry`, `_PATH_ROLE`, `_KIND_ROLE`, `file_requested`,
   `directory_requested`, click/double-click/Enter routing, and all service,
   containment, busy, duplicate-tab, and asynchronous opening policy remain
   unchanged.
3. No new theme token, global QSS selector, custom delegate, or domain/service
   dependency is introduced.
4. Existing inaccessible diagnostics are not replaced by a generic localized
   hint.
5. The visual distinction is a projection only; it does not change whether an
   entry is actionable.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation change. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Dalton the 5th / Luna max — `NO_CONCLUSION` after bounded
  waits; the window was closed without claiming a child PASS.
- Independent review window: Socrates the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no Git
  baseline for complete-diff proof.
- Parent review: `PASS` for the presentation-only boundary, palette-role
  reuse, light-canvas contrast fallback, tooltip precedence, locale refresh,
  icon refresh, and unchanged activation signals.
- Simplification assessment: `PASS`; the existing icon provider and refresh
  route are reused, with one small kind-to-hint mapping and no new theme or
  policy layer.

## Verification target and limits

- Authorized evidence: AST/source-shape probes, localized semantic-hint
  probes, tooltip-precedence and signal-preservation probes, 3-theme ×
  4-accent contrast projection, compileall, Ruff, format, PyInstaller package
  build, package identity, project static checks, handoff checks, and expected
  release NO-GO evidence.
- Not proven: native Qt icon rendering, tooltip timing, accessibility tree
  output, DPI/font metrics, clean-machine behavior, cross-machine behavior,
  signing, installer/update, legal, support-owner, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

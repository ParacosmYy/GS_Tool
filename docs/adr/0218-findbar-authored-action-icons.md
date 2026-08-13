# ADR-0218: Add authored action icons to FindBar

- **Status:** accepted-with-limits; D169 / UI-81 / ARCH-156 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Find/Replace bar had localized text and keyboard behavior, but its action
buttons were text-only while the command rail and workspace already used the
application-authored icon provider. Previous/next, replace, cancel, and close
therefore took longer to scan and looked inconsistent with the rest of the
shell.

## Decision

Keep FindBar as the sole action projection owner and add compact authored icons
to its existing buttons:

- previous: `ARROW_UP`;
- next: new `ARROW_DOWN`;
- replace and replace-all: `REPLACE`;
- cancel and close: `CLOSE`.

`FindBar.refresh_icons()` resolves the current `ButtonText`, `Link`, and
disabled palette roles and is called from the existing `set_locale()` path.
That path already runs after a saved theme/accent/locale projection, so the
icons are retinted without adding an application callback or a second refresh
boundary. On light canvases the secondary icon detail falls back to the button
text color because the existing pressed surface can weaken the accent-alt
detail; dark canvases retain the accent detail.

## Invariants

1. Only `find_bar.py`, `icon_contract.py`, and `icons.py` change for the
   feature.
2. Existing FindBar signals, shortcuts, query/replacement/case state,
   visibility, operation-active/cancel behavior, and primary-action
   `objectName` projection remain unchanged.
3. `ARROW_DOWN` is a pure semantic icon contract and does not import Qt into
   application/domain code or change existing icon keys.
4. Disabled icons are supplied through the existing `QIcon.Mode.Disabled`
   provider path.
5. No theme token, global QSS selector, service, coordinator, or editor policy
   changes.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation change. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Heisenberg the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Carson the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no Git
  baseline for complete-diff proof.
- Parent review: `PASS` for icon ownership, palette refresh, disabled mode,
  action/state preservation, and the pure icon-contract extension.
- Simplification assessment: `PASS`; the existing `themed_icon()` provider
  and locale refresh seam are reused, with no new coordinator or QSS layer.

## Verification target and limits

- Authorized evidence: AST/source-shape probes, icon mapping and signal
  preservation probes, locale-refresh probe, 3-theme × 4-accent icon contrast
  projection, compileall, Ruff, format, PyInstaller package build, package
  identity, project static checks, handoff checks, and expected release NO-GO
  evidence.
- Not proven: native Qt icon painting, text/icon metrics, screen-reader
  output, DPI/font metrics, clean-machine behavior, cross-machine behavior,
  signing, installer/update, legal, support-owner, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

# ADR-0143: Command-rail visual role hierarchy

- **Status:** accepted-with-limits; UI-56 / ARCH-87 bounded visual slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The top command rail gave creation, save, search, replacement, command-palette,
and workspace actions almost the same visual weight. That made the most
important action harder to scan and gave quiet utilities too much competition
for attention.

## Decision

Extend the existing presentation-only ToolbarActionSpec with a closed
ToolbarActionRole literal: standard, primary, quiet, or context. CommandSurface
projects the role to each created QToolButton through the commandRole dynamic
property. MainWindow assigns the roles at the composition site: Save is
primary, Replace and Command Palette are quiet, and Workspace is context;
existing actions remain standard.

Add only commandBar-scoped QSS for the non-standard roles. Primary actions use
contrast-safe accent endpoints, quiet actions reduce visual weight, and the
workspace action receives a context rail. Hover, focus, pressed, and disabled
states remain explicit and use canonical theme tokens.

## Invariants

1. ToolbarActionSpec role metadata is presentation-only; callbacks, actions,
   shortcuts, menus, locale retranslation, icons, and command registry
   behavior remain unchanged.
2. The role contract is closed and typed; no arbitrary theme string or
   business command policy enters CommandRegistry.
3. QSS is scoped to QToolBar#commandBar and uses canonical ThemeColors tokens;
   no global QToolButton styling is added.
4. Tooltips/action text remain the accessible name source; no action is hidden,
   disabled, reordered, or made destructive by this slice.
5. MainWindow retains document/workspace, TaskRunner, persistence, and close
   policy; CommandSurface retains menu/toolbar projection ownership.

## Alternatives considered

- **Style toolbar buttons by translated text:** rejected; labels vary by
  locale and would make visual roles unstable.
- **Add colors directly in MainWindow or CommandRegistry:** rejected; it would
  mix policy with presentation and bypass the canonical theme token resolver.
- **Create a separate toolbar hierarchy widget:** rejected; role metadata and
  scoped QSS are the smallest complete change.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance workflow and simplifier are N/A for source scope; no embedded source
was changed. Public CloudWeGo material remains an engineering reference only.
No private ByteDance standard, certification, MISRA, ISO 26262, ASIL, ASPICE,
or compliance claim is made.

## Review and simplification

- Architect: Wegener the 4th / Luna max; bounded read-only consultation timed
  out and was closed. Status is NO_CONCLUSION; no child architecture PASS is
  claimed.
- Independent review: Pauli the 4th / Luna max; status is recorded in the
  UI-56 independent-review record after the bounded read-only window. No child
  PASS is claimed unless the record explicitly reports one.
- Parent source review: PASS for closed role typing, composition-site
  ownership, QSS scope/state coverage, token contrast, API compatibility,
  accessibility preservation, and business-policy isolation.
- Simplification assessment: PASS. One closed role field, one projection
  property, and three scoped role blocks are the smallest complete change; no
  visual subsystem, command abstraction, or runtime state is needed.

## Verification target and limits

- Authorized evidence includes the UI-56 command-role source probe, token
  contrast probe, compileall, Ruff, format, presentation-contract audit,
  package identity, handoff/register/index synchronization, repository checks,
  no-process evidence, and expected release NO-GO evidence.
- Not proven: native Qt stylesheet specificity/painting, actual human visual
  perception on every DPI/font combination, screen-reader output, QApplication
  startup, clean-machine/cross-machine behavior, signing, installer, updater,
  legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.

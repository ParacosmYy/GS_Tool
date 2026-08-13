# ADR-0129: Workspace resource-manager hierarchy

- **Status:** accepted-with-limits; UI-52 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The workspace panel already distinguished files and folders through semantic
signals and authored icons, but its initial blank tree gave no clear empty
state. The path, navigation controls, and native tree also shared a dense
native-widget baseline, which made the shell feel old and obscured the
file-opening affordance.

## Decision

Keep `WorkspacePanel` as the sole owner of the workspace widget projection.
Add a localized `workspaceEmpty` label and a small internal
`_sync_content_state()` projection so the panel explicitly distinguishes an
unselected workspace, an in-flight initial load, an empty directory, and a
directory with rows. Add semantic accessible names for the path and tree,
retain the native tree's file/folder icons and keyboard route, and keep all
visual treatment in the centralized `theme.py` stylesheet.

Use the existing surface ladder and accent tokens: the path is a readable
context card, the empty state is a quiet dashed card with a restrained
secondary accent, and the tree remains the interactive surface. Remove
shouty workspace microcopy while preserving the file-first-click,
folder-double-click, and Enter/Return interaction guidance.

## Invariants

1. `folder_requested`, `directory_requested`, `file_requested`,
   `back_requested`, and `cancel_requested` retain their names, payloads, and
   owners.
2. Single-click files, double-click folders, and Enter/Return all continue to
   use `_emit_item_intent()`; no application or service policy enters the
   widget.
3. Loading continues to disable the tree/open controls and expose the cancel
   control; error projection remains status-label based and does not clear the
   last good directory.
4. `WorkspaceSurface` and `WorkspaceNavigationCoordinator` contracts,
   directory paging, containment, session restore, and async ownership are
   unchanged.
5. QSS remains centralized in `theme.py`; no widget-local stylesheet, custom
   delegate, custom painting, new dependency, or new state owner is added.

## Alternatives considered

- **Leave the tree blank before workspace selection:** rejected; a blank
  surface hides the next action and looks like a missing feature.
- **Replace the native tree with a custom file browser:** rejected; that would
  expand a presentation refinement into a new model, delegate, accessibility,
  and activation contract.
- **Put styling in `WorkspacePanel.setStyleSheet()`:** rejected; it would
  split theme ownership and make accent/contrast auditing harder.
- **Add application-level empty/loading state:** rejected; the existing
  `WorkspacePanel` already owns the visible projection and no policy moves.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power-control, motor-control,
and manufacturer requirements are not applicable. The mandatory embedded
assurance gate is recorded as N/A for this change. The Qt Company `QTreeView`
and `QTreeWidget` Qt 6 documentation is an engineering reference for the
existing `QTreeWidget` API (`QTreeWidget` inherits `QTreeView`, and the view
documents header-hidden and uniform-row presentation properties):
<https://doc.qt.io/qt-6/qtreeview.html> and
<https://doc.qt.io/qt-6/qtreewidget.html>. It is not treated as a private
vendor requirement or compliance evidence. Public CloudWeGo material remains
an engineering reference only; no private ByteDance standard, certification,
or compliance claim is made.

## Review and simplification

- Architect: Darwin the 3rd / Luna max; two bounded read-only waits returned
  `NO_CONCLUSION`, then the agent was closed. No child architecture PASS is
  claimed.
- Independent review: Pasteur the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No independent PASS is
  claimed.
- Parent source review: PASS for signal/lifecycle preservation, explicit
  state coverage, centralized QSS, localization, accessibility names, and
  presentation-only ownership.
- Simplification assessment: PASS. The change reuses `WorkspacePanel`, the
  native tree, existing feedback states, existing icons, and existing theme
  tokens. One small private state-sync helper is retained because it prevents
  four mutually exclusive visibility branches from drifting. No further safe
  reduction was identified.

## Verification target and limits

- UI-52 source, i18n, and all-theme/all-accent contrast probes must pass.
- Compileall, Ruff, format, package identity, handoff, repository,
  traceability, and expected release NO-GO evidence are required.
- Native Qt layout/selector specificity, fonts, DPI, screen-reader output,
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open under the
  no-launch/authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run.

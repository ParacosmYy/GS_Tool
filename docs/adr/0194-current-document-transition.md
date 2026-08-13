# ADR-0194: current-document transition projection boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D147 / ARCH-132

## Context

`MainWindow._on_current_tab_changed` performed a stable sequence of Find
invalidation, Find-session reset, active-tab lookup, context notification,
status refresh, and debounced session-save request. The handler contained no
Qt-specific policy beyond callbacks to existing surfaces, so its ordering can
be made explicit without moving tab, notification, or session ownership.

## Decision

Add the Qt-free generic `CurrentDocumentTransitionCoordinator[TabT]` with a
frozen/slotted `CurrentDocumentTransitionPorts[TabT]` contract. It owns only
the established transition sequence:

1. Invalidate the current Find match.
2. Reset the Find session.
3. Resolve the active tab.
4. If a tab exists, remove the dirty-marker prefix from its title and emit the
   existing info-context notification.
5. Synchronize status and request the existing debounced session save.

`MainWindow` retains the concrete tab surface, FindSurface, StatusSurface,
notification localization/policy, session timer/coordinator, tab identity,
Qt signal connection, and application policy. The empty-tab behavior remains
an intentional no-op only for context notification; status and session-save
projection still occur.

## Alternatives rejected

- Leaving the ordered sequence inline would keep a lifecycle policy hidden in
  the monolithic window handler.
- Moving tab identity, locale, notification policy, or session persistence
  into the coordinator would make a framework-neutral boundary application
  aware.
- Combining current-tab transition with editor mutation would violate the
  separate selection-versus-mutation ownership established by D146.

## Review and evidence

Linnaeus the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Ptolemy the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the coordinator contains only the existing ordered callback
sequence and one semantic `notify_info` port.

Authorized non-destructive evidence:

- `D147-TRANSITION-PROBE=PASS`
- `D147-SOURCE-WIRING-PROBE=PASS`
- `D147-PRESENTATION-AUDIT=PASS`
- `D147-COMPILEALL=PASS`
- `D147-RUFF=PASS`
- `D147-FORMAT=PASS`
- `D147-PACKAGE-BUILD=PASS`
- `D147-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `4B942B56320B727BF0022B71EAD55E7D95B278467E200D1AA3BBF5B2F3698660`
- bytes: `38542225`
- source revision: `tree-sha256:1cbff80f9605e40c6ea5667f8a97355627d1d41c22cd82dd4cc7f3dbe78badff`

Public-source applicability is Python 3.12/PyQt6 centralized presentation
contracts; embedded C/C++, MCU, RTOS, and manufacturer requirements do not
apply. Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

Static and inline probes do not prove native Qt signal timing, focus behavior,
font/DPI metrics, accessibility, runtime startup, clean-machine or
cross-machine behavior, signing, installer/update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.

# ADR-0236: Responsive Settings scroll boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D187 / UI-98 / ARCH-174

## Decision

Keep `SettingsDialog` as the owner of the existing form controls, preview,
locale projection, and snapshot assembly, but place its existing content
surfaces in one named `QScrollArea#settingsScroll`. The appearance group,
`SettingsPreviewSurface`, editor group, and guidance notes are owned by the
named `QWidget#settingsContent` inside that viewport. The existing
`QDialogButtonBox#dialogActions` remains in the outer dialog layout, so Save
and Cancel stay reachable while the content scrolls at short heights or with
larger selected fonts.

The scroll area is resizable, has no native frame, hides horizontal overflow,
and receives only scoped transparent QSS. It reuses the existing centralized
token-driven scrollbar rules. No new settings contract, signal, persistence
path, locale catalog, theme application timing, motion policy, or application
ownership is introduced.

## Non-goals and evidence

This is Python/PyQt6 presentation work. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable. Public engineering guidance and
public CloudWeGo material remain references only; this ADR does not claim a
private ByteDance standard, certification, or release readiness.

Recorded evidence includes `D187-SETTINGS-SCROLL-PROBE=PASS`,
`D187-COMPILE-RUFF-FORMAT=PASS`, `D187-PACKAGE-BUILD=PASS`, and the current
artifact-bound no-go handoff. Architect and independent review windows both
returned `NO_CONCLUSION` after bounded waits; parent review and simplification
assessment are the only claimed conclusions. Native Qt scroll metrics,
keyboard traversal, DPI, accessibility, screenshots, GUI/EXE runtime,
clean-machine, cross-machine, and release-owner evidence remain open.

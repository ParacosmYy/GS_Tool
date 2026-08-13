# ADR-0344: Workspace open-action affordance

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D308 / UI-127 / ARCH-278

## Context

The workspace dock already routed file clicks, folder navigation, and the
native document picker through separate callbacks. The two top actions were
visually distinct, but their purpose was not consistently exposed through
localized hints or assistive-technology descriptions. That made the file
opening path easy to miss and could make the shell feel folder-only.

## Decision

Keep the existing `folder_requested`, `file_picker_requested`,
`directory_requested`, and `file_requested` signals unchanged. Assign the
four workspace actions semantic `workspaceRole` properties and let one small
`_set_action_accessibility()` helper project the translated label, tooltip,
and accessible description at the existing `set_locale()` boundary. Add
distinct folder/document left-edge tokens in the centralized QSS so the two
opening paths remain visually scannable without relying on text or color
alone.

The change remains presentation-only. It does not inject Locale into
application coordinators, alter the file picker, change workspace containment,
change asynchronous document opening, or add persistence/state.

## Evidence

- `D308-QSS-MATRIX=PASS themes=3 accents=4 workspace_roles=folderPicker,documentPicker minimum_edge_contrast=3.57`
- `D308-COMPILEALL=PASS`
- `D308-RUFF=PASS`
- `D308-FORMAT=PASS`
- `D308-AUDIT=PASS`
- Source startup and README regular-file-open diagnostics passed with no
  window shown and no event loop entered.
- The rebuilt root/dist candidate has SHA-256
  `BEE2B6E4E5709FFFABE9F99A441853F8AA675531BB8A411EE8023C95D24CC647` and
  38,599,052 bytes.

## Review and applicability

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded Luna/max waits. Parent review and
behavior-preserving simplification assessment are `PASS`.

Qt QWidget accessibility APIs, dynamic properties, QSS, and WCAG 2.2 are
public engineering references for this Python 3.12/PyQt6 desktop UI. Embedded
vendor-source applicability is N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or
firmware code changed. No manufacturer, MISRA, ISO 26262, ASPICE,
certification, or private corporate-standard claim is made.

## Limits

Native EXE/Qt launch, screen-reader output, pixel rendering, focus traversal,
DPI, clean-machine behavior, real DLL loading, signing, installer, updater,
and release-owner gates remain unrun under the active non-destructive launch
policy. Unit tests, mocks, fixtures, and test harnesses were not created or
run.

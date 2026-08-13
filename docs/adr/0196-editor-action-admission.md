# ADR-0196: editor-action admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D148 / ARCH-134

## Context

`MainWindow._run_editor_action` combined the active-tab lookup, the busy
guard, editor-action invocation, and focus restoration. The operation is
editor-local and synchronous, but its admission order is an application
contract: an empty shell must not read busy state, a busy shell must not
mutate or focus an editor, and an admitted action must restore focus exactly
once.

## Decision

Add the Qt-free generic `EditorActionAdmissionCoordinator[EditorT, TabT]`
with the frozen/slotted `EditorActionAdmissionPorts[EditorT, TabT]` contract.
The coordinator owns only this sequence:

`active_tab -> busy guard -> editor_of -> action(editor) -> focus_editor`.

`MainWindow` retains the concrete tab surface, busy-state policy, editor
adapter, Qt focus call, QAction/shortcut/menu composition, and all document
and editor behavior. `_run_editor_action` remains the stable application
entry point and delegates one action to the coordinator.

## Alternatives rejected

- A command registry or shortcut abstraction would broaden a local admission
  extraction into command composition.
- Moving editor methods or focus calls into the coordinator would leak Qt and
  concrete widget ownership across the boundary.
- Combining this slice with undo/document dirty policy would make behavior
  changes harder to isolate and verify.

## Review and evidence

Fermat the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Newton the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the four named ports are the smallest complete boundary and
the coordinator contains no policy beyond the existing guard/order contract.

Authorized non-destructive evidence:

- `D148-ADMISSION-PROBE=PASS`
- `D148-SOURCE-WIRING-PROBE=PASS`
- `D148-PRESENTATION-AUDIT=PASS`
- `D148-COMPILEALL=PASS`
- `D148-RUFF=PASS`
- `D148-FORMAT=PASS`
- `D148-PACKAGE-BUILD=PASS`
- `D148-PACKAGE-IDENTITY-PROBE=PASS`
- `D148-CHECK=PASS`
- `D148-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `0B038932A80A89DA83D74498D45858445358ED0A00989D58C0081BE4D8DD3DD3`
- bytes: `38543502`
- source revision: `tree-sha256:57410fda037f821ca3f97a83a36fbf411d54994a0e8a241bf2719b653ed5965b`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

Inline/static probes do not prove native Qt event timing, editor rendering,
focus behavior under every platform style, font/DPI metrics, accessibility,
runtime startup, clean-machine or cross-machine behavior, signing, installer,
update, legal clearance, support ownership, or release readiness. Those gates
remain open under the active no-launch/no-release authorization boundary.

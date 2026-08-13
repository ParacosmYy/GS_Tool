# D68 parent review — message-dialog visual hierarchy

| Field | Value |
|---|---|
| Delivery | `D68 / UI-41` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

MessageSurface now owns equivalent instance-based QMessageBox composition for
unsaved confirmation, About, and errors. RecoveryPromptSurface adds one
dialog identity and reuses existing primary/warning/quiet action roles. Theme
QSS adds message-box surface, edge, label, and button hierarchy.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. Save/Discard/Cancel order,
  default Save, localized title/text, icons, modal `exec`, and return mapping
  remain explicit; recovery clicked-button mapping is unchanged.
- **Readability/simplicity:** PASS. One private MessageSurface factory names
  the shared QMessageBox setup; no new dialog class or policy object exists.
- **Architecture:** PASS with limits. MessageSurface and RecoveryPromptSurface
  remain presentation owners; theme remains the sole QSS token owner;
  MainWindow is untouched.
- **Security:** PASS by scope. No input, filesystem, plugin, process, or
  persistence boundary changes.
- **Performance:** PASS by scope. One modal QMessageBox is still created per
  user interaction; no new loop, dependency, or retained object is added.

## Simplification assessment

No further safe simplification was identified. Keeping the three equivalent
message-box constructions behind one local factory removes repetition without
creating a cross-surface abstraction; recovery stays separate because its
button identities and decision mapping are distinct.

## Authorized non-destructive validation

- `D68-message-dialog-hierarchy-probe=PASS`.
- Targeted compileall, Ruff, and format checks — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native QMessageBox rendering, keyboard traversal, accessibility, callback
timing, runtime startup, clean-machine, cross-machine, and release-owner
evidence remain open. Embedded C/C++ and vendor-public-source requirements are
N/A.

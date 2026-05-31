# UI/UX Review - Iteration 048

**Reviewer**: UI/UX Product Experience
**Scope**: DTR/RTS buttons, dark_terminal.qss new selectors, DataExporter EDL range export
**Date**: 2026-06-01

---

## Issues Found

### P1 - Missing `signalState` property initialization on construction

**File**: `src/serial/SerialConfigPanel.cpp`, lines 126-144

During construction, both DTR and RTS buttons are created with `setChecked(true)` but `refreshDtrStyle()` / `refreshRtsStyle()` are never called. The `signalState` dynamic property is only set when:

1. The user toggles the button (toggled signal handler, lines 149/155)
2. `restoreConfig()` is called (lines 390/398)

This means on first display, neither button has a `signalState` property. The QSS selectors `[signalState="high"]` and `[signalState="low"]` both fail to match, so the buttons fall through to the base `QPushButton#dtrBtn, QPushButton#rtsBtn` rule which defines `border`, `border-radius`, `padding`, `font-size`, `font-weight`, `min-width` but **no `background-color`** or `color` at all.

The generic `QPushButton` rule (lines 158-179 in dark_terminal.qss) provides `background-color: #313244` and `color: #cdd6f4`, which will be used as fallback. So the buttons appear as default gray instead of the intended green HIGH state. The visual mismatch only resolves after the first toggle or a config restore.

**Fix**: Add `refreshDtrStyle()` and `refreshRtsStyle()` calls at the end of `setupSignalAndConnectControls()`, right after the signal connections are established. This sets the initial `signalState="high"` property before the first paint.

```cpp
// After connect(m_rtsBtn, ...) on line 157:
refreshDtrStyle();
refreshRtsStyle();
```

### P2 - QSS base rule for DTR/RTS missing background-color safety net

**File**: `resources/themes/dark_terminal.qss`, lines 560-567

The base selector `QPushButton#dtrBtn, QPushButton#rtsBtn` defines layout properties but no `background-color` or `color`. While the fix above resolves the missing property issue, adding a default background-color to this base rule would provide a safety net if the property fails to propagate for any reason (e.g., timing edge during polish).

Same observation applies to `modern_dark.qss` (lines 552-558) and `light.qss` (lines 552-558).

**Recommendation**: Add `background-color` and `color` to the base rule, matching the LOW state as the visual default:

```css
QPushButton#dtrBtn, QPushButton#rtsBtn {
    background-color: #45475a;   /* fallback matching LOW state */
    color: #a6adc8;
    border: 1px solid #45475a;
    /* ... existing properties ... */
}
```

### P3 - DTR/RTS disabled state overrides HIGH background on initial connection

**File**: `src/serial/SerialConfigPanel.cpp`, line 221-222

When `setConnected(true)` is called, `m_dtrBtn->setEnabled(connected)` enables the buttons. However, the `signalState` property is not refreshed during `setConnected()`. If the initial construction bug (P1) is fixed, this is not an issue. But if `setConnected()` is called without a prior toggle or restore, the property may still be unset.

The transition from disabled (gray) to enabled (should be green HIGH) relies entirely on the `signalState` property being correct at that point.

**Fix**: Add `refreshDtrStyle()` and `refreshRtsStyle()` calls in `setConnected()` after enabling the buttons, to guarantee the correct visual state on connection:

```cpp
m_dtrBtn->setEnabled(connected);
m_rtsBtn->setEnabled(connected);
if (connected) {
    refreshDtrStyle();
    refreshRtsStyle();
}
```

---

## Observations (No Action Required)

### DTR/RTS QSS structure -- well designed

The three-state approach (HIGH green, LOW gray, disabled gray) across all three theme files is consistent and follows the semantic color conventions correctly. Each theme uses its own palette-appropriate colors rather than copy-pasting the same hex values.

### All three theme files synchronized

The `dtrBtn`/`rtsBtn` selectors with `signalState` property-based styling are present in all three theme files:
- `dark_terminal.qss` (lines 559-601)
- `modern_dark.qss` (lines 551-593)
- `light.qss` (lines 551-593)

Each uses palette-appropriate colors for HIGH (green variants), LOW (neutral variants), and disabled states.

### No hardcoded colors in C++ code

`SerialConfigPanel.cpp` uses only QSS property-driven styling (`setProperty` + `unpolish`/`polish`). Zero inline `setStyleSheet()` calls, zero hardcoded hex colors. This fully complies with CLAUDE.md section 6.8 rule 1.

### DataExporter.cpp -- no UI concerns

The EDL range export (`exportRange`, `readEdlRange`) is a pure data utility with no visual output. No QSS, no colors, no UI elements. The implementation is clean and follows the project's data layer conventions.

### Button spacing consistent with existing UI

The DTR/RTS buttons use `padding: 4px 12px`, `font-size: 12px`, `font-weight: bold`, `min-width: 80px`, `border-radius: 4px` -- consistent with other secondary buttons in the config panel (e.g., `refreshBtn` at lines 536-556 uses `padding: 4px 12px`, `font-size: 12px`). The buttons are in a horizontal layout with `addStretch()` on the right, which aligns them to the left within the `signalGroup` QGroupBox, consistent with the `portGroup` layout pattern.

---

## Summary

| Severity | Count | Description |
|----------|-------|-------------|
| P1 | 1 | Missing `signalState` property initialization causes wrong initial button color |
| P2 | 1 | QSS base rule lacks background-color safety net (all 3 themes) |
| P3 | 1 | `setConnected()` should refresh signal button styles on enable |

The core issue is that `refreshDtrStyle()` and `refreshRtsStyle()` are wired to user interaction and config restore only, never to construction or connection state transitions. A three-line fix in `setupSignalAndConnectControls()` resolves the primary visual bug.

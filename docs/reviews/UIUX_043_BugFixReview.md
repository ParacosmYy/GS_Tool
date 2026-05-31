# UIUX-043: P0 Bug Fix Review

**Reviewer**: UI/UX Product Experience (frontend-architect)
**Date**: 2026-06-01
**Scope**: 6 P0 bug fixes + UI consistency check

---

## Bug 1: Ctrl+C Exact Modifier Match

**File**: `src/terminal/TerminalWidget.cpp` (lines 362-369)

**Fix**: `event->modifiers() == Qt::ControlModifier` (exact equality)

**Verdict**: PASS

The fix correctly replaces bitwise-and with exact equality for all four
keyboard shortcuts (Ctrl+C, Ctrl+A, Ctrl+F, Ctrl+V). This prevents
Ctrl+Shift+C from accidentally triggering copy, which is the correct
behavior in a terminal where Ctrl+Shift+C may be mapped to interrupt.

No regression risk: F3/Shift+F3 correctly uses bitwise-and because
Shift is an additive modifier that should not exclude the match.

Comments are clear and explain the reasoning ("精确匹配 Ctrl 修饰键，
避免 Ctrl+Shift+C 被误拦截").

---

## Bug 2: Search Highlight Offset with Timestamps

**File**: `src/terminal/TerminalWidget.cpp` paintLine (lines 218-230)

**Fix**: `int xStart = xOffset + 4 + ...` includes xOffset

**Verdict**: PASS

xOffset is computed from the timestamp width (line 206) and is zero when
timestamps are disabled. The search highlight xStart correctly adds
xOffset before the +4 text padding, so highlights align with the actual
text position regardless of timestamp visibility.

No off-by-one: xOffset = horizontalAdvance(ts) + 12 provides a clean
12px gap after the timestamp text. The +4 padding for data text is
consistent with the drawText calls in the direction-prefix section.

---

## Bug 3: stopShimmer() Race Condition

**File**: `src/ota/AnimatedProgressBar.h` (lines 97-104)

**Fix**: Nullifies m_shimmerAnim pointer after stop, resets offset to 0

**Verdict**: PASS WITH NOTES

The stopShimmer() method now:
1. Checks m_shimmerAnim is non-null before calling stop()
2. Sets m_shimmerAnim = nullptr after stopping (prevents double-stop)
3. Resets m_shimmerOffset to 0.0 (clean visual state)
4. Calls update() to repaint without shimmer

Note: Since QPropertyAnimation was started with DeleteWhenStopped,
calling stop() on a previously-stopped animation would be a
use-after-free if the pointer were not nulled. The fix is correct.

startShimmer() correctly calls stopShimmer() first, preventing the
old animation from leaking when restarting.

One concern: This class runs on the GUI thread only (QProgressBar
subclass), so the "race condition" is about re-entrancy from Qt's
event loop rather than true multi-threading. The fix is adequate for
this context. No mutex needed.

---

## Bug 4: NAK Does Not Override User CRC Mode

**File**: `src/ota/protocols/XModemTransfer.cpp` (lines 174-178)

**Fix**: NAK handler now only downgrades to Checksum if mode is not
already Checksum:
```cpp
if (m_mode != Checksum) {
    m_mode = Checksum;
}
```

**Verdict**: PASS

Previously, receiving NAK unconditionally set mode to Checksum, which
would override a user-selected CRC or 1K mode even after the transfer
had already started with that mode. The conditional guard ensures NAK
only acts as a fallback from higher modes.

The CRC_CHAR handler (line 185-193) does not change mode, which is
correct -- CRC is already the default for non-Checksum modes.

No regression: The mode selection logic in buildBlock() (line 364)
correctly uses Checksum vs CRC based on m_mode.

---

## Bug 5: Network Auto-Reconnect Saves Params

**File**: `src/core/ConnectionController.cpp`

**Observation**: connectNetwork() does NOT save m_lastConnectParams.
Only m_lastConnectType is saved (line 215). The onAutoReconnect()
method calls connectNetwork(m_lastConnectType) without params.

**Verdict**: PARTIAL FIX -- needs attention

The serial path correctly saves m_lastConnectParams (line 134) and
uses it during reconnect (line 340). However, connectNetwork()
constructs default params internally (host=127.0.0.1, port=8080) rather
than accepting external params, so the reconnect always uses defaults.

This is by design for the current iteration -- network connections use
hardcoded defaults. But when user-configurable network params are added,
connectNetwork() will need a QVariantMap overload similar to
connectSerial(), and m_lastConnectParams must be saved for network
connections too.

The serial auto-reconnect path is correct and complete. m_userInitiatedDisconnect
is properly reset to false after disconnectCurrent() in connectSerial().

---

## Bug 6: setMaxLines Re-entrancy Guard

**File**: `src/terminal/TerminalModel.cpp` (lines 127-156)

**Fix**: Uses QMutexLocker with explicit unlock before emit

**Verdict**: PASS

The method acquires m_mutex at entry, performs all buffer manipulation
under the lock, then calls locker.unlock() before emitting dataCleared().
This prevents the signal-slot chain from re-entering setMaxLines while
the lock is still held, which would cause deadlock.

The buffer reallocation logic is correct:
1. Copies old lines to temporary buffer (preserving logical order)
2. Resizes ring buffer to new max
3. Moves back as many lines as fit
4. Resets head=0, count=keepCount

Edge cases handled:
- max == m_maxLines: early return (no-op)
- count > max: only keeps the newest lines (copyStart calculation)

---

## UI Consistency Checks

### Global Font Declarations in QSS

All three theme files contain the global font declaration:

```css
* {
    font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
}
```

- `dark_terminal.qss`: line 4-6 -- PRESENT
- `modern_dark.qss`: line 4-6 -- PRESENT
- `light.qss`: line 4-6 -- PRESENT

Consistent across all themes. Monospace font for terminal/send inputs
is correctly specified via inline font-family on QLineEdit.

### ToastWidget Upward Drift Animation

**File**: `src/core/ToastWidget.h` (lines 136-153)

The dismiss() method contains the upward drift animation:

```cpp
auto* drift = new QPropertyAnimation(this, "pos");
drift->setStartValue(pos());
drift->setEndValue(pos() + QPoint(0, -30));
drift->setDuration(250);
drift->setEasingCurve(QEasingCurve::InCubic);
```

This drifts the toast 30px upward over 250ms using InCubic easing,
combined with a parallel fade-out animation. The animation parameters
comply with CLAUDE.md section 6.5 (max 400ms, no bounce, QPropertyAnimation).

Toast repositioning after dismissal uses OutCubic at 200ms, also
compliant. The showDebounced() method provides 2-second cooldown to
prevent notification spam -- good UX.

---

## Summary

| Bug | File | Verdict | Notes |
|-----|------|---------|-------|
| 1. Ctrl+C modifier | TerminalWidget.cpp:365 | PASS | Exact match, all 4 shortcuts |
| 2. Search highlight | TerminalWidget.cpp:224 | PASS | xOffset correctly applied |
| 3. Shimmer race | AnimatedProgressBar.h:97 | PASS | Null-after-stop pattern |
| 4. NAK mode override | XModemTransfer.cpp:176 | PASS | Conditional downgrade only |
| 5. Network reconnect | ConnectionController.cpp:215 | PARTIAL | Serial correct; network uses defaults |
| 6. setMaxLines guard | TerminalModel.cpp:127 | PASS | Explicit unlock before emit |

| UI Check | Status |
|----------|--------|
| Global font in all 3 QSS | PRESENT |
| Toast upward drift | PRESENT, spec-compliant |

**Overall**: 5 of 6 bugs fully fixed. Bug 5 (network reconnect params)
is acceptable for the current iteration but needs a follow-up when
network configuration becomes user-editable.

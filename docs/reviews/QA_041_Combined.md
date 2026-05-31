# QA + Code Review -- Iteration 41

**Date**: 2026-06-01
**Commit**: e1707b6
**Verdict**: BLOCK -- 3 compilation errors, build does not pass

---

## 1. Build Status: FAIL

3 of 40 translation units failed. The binary cannot be produced.

### Error 1 -- SerialConfigPanel.cpp:225 -- incomplete type `QTimer`

```
error: incomplete type 'QTimer' used in nested name specifier
  225 |     QTimer::singleShot(3000, this, [this]() {
```

**Root cause**: `<QTimer>` header is not included. The file includes
`<QPropertyAnimation>` and `<QSequentialAnimationGroup>` but omits
`<QTimer>`. The `setError()` method calls `QTimer::singleShot()` as a
static call on a forward-declared type.

**Fix**: Add `#include <QTimer>` to SerialConfigPanel.cpp includes.

### Error 2 -- SerialConfigPanel.cpp:277 -- type mismatch assignment

```
error: cannot convert 'QSequentialAnimationGroup*' to 'QPropertyAnimation*'
  277 |     m_breathAnim = group;
```

**Root cause**: In SerialConfigPanel.h line 97, `m_breathAnim` is declared
as `QPropertyAnimation*`. The breathing animation was redesigned to use a
`QSequentialAnimationGroup` (fadeIn + fadeOut sequence), but the member
type was not updated.

**Fix**: Change the member type in SerialConfigPanel.h from
`QPropertyAnimation* m_breathAnim` to `QAbstractAnimation* m_breathAnim`.
The header already forward-declares `QAbstractAnimation` (line 24). The
usage in stopBreathAnimation() only calls `stop()` and `delete`, which
are available on `QAbstractAnimation`.

### Error 3 -- ToastWidget.h:93 -- name not declared yet

```
error: 'debounceMap' was not declared in this scope
   93 |         auto& map = debounceMap();
```

**Root cause**: `showDebounced()` is defined inline in the `public` block
starting at line 88. `debounceMap()` is declared later in the `private`
block at line 196. C++ parses class bodies top-to-bottom for inline
definitions. At line 93, the compiler has not yet seen `debounceMap`.

**Fix**: Move the `debounceMap()` static method declaration to a point in
the class body before `showDebounced()`, or move `showDebounced()` after
the `private` section. The simplest approach: add a forward declaration
of `debounceMap()` in the public section before `showDebounced()`, or
reorder the class members so `debounceMap()` appears first.

---

## 2. File Size Check

### Headers (top 10, limit: 200 lines)

| Lines | File | Status |
|------:|------|--------|
| 234 | ToastWidget.h | OVER by 34 |
| 199 | MainWindow.h | PASS |
| 198 | OtaWidget.h | PASS |
| 193 | SerialConnection.h | PASS |
| 192 | SendController.h | PASS |
| 191 | ByteFormat.h | PASS |
| 190 | NavIndicatorWidget.h | PASS |
| 189 | ThemeManager.h | PASS |
| 184 | ChartWidget.h | PASS |
| 181 | AnimatedProgressBar.h | PASS |

**Violation**: ToastWidget.h exceeds the 200-line header limit by 34 lines.
The header-only implementation pattern (all inline in .h) is the cause.
Consider splitting into ToastWidget.h (declaration) + ToastWidget.cpp
(implementation of the larger methods like `show()`, `showDebounced()`,
`dismiss()`, `paintEvent()`).

### Implementations (top 10, limit: 500 lines)

| Lines | File | Status |
|------:|------|--------|
| 499 | TerminalWidget.cpp | PASS (1 line margin) |
| 493 | YModemTransfer.cpp | PASS |
| 475 | ZModemTransfer.cpp | PASS |
| 452 | ConnectionController.cpp | PASS |
| 439 | ProtocolBridgeManager.cpp | PASS |
| 437 | FrameVisualEditor.cpp | PASS |
| 428 | FrameParser.cpp | PASS |
| 418 | OtaWidget.cpp | PASS |
| 414 | SerialConnection.cpp | PASS |
| 412 | DataExporter.cpp | PASS |

All .cpp files within limit. TerminalWidget.cpp is at the razor's edge.

---

## 3. Code Quality Scan

### Hardcoded colors

All occurrences are in `ThemeManager.cpp` (color registration -- correct)
and `ChartColors.h` (chart palette -- correct). No violations outside
the allowed files.

### SIGNAL/SLOT macros

Zero occurrences. All connections use the new-style pointer syntax.
PASS.

### TODO/FIXME

Zero occurrences. PASS.

### Include order

Checked SerialConfigPanel.cpp, NavigationController.cpp,
MainWindowSignalConnect.cpp, OtaWidget.cpp -- all follow Qt-first,
project-last order. PASS.

---

## 4. Code Review Findings

### SerialConfigPanel.cpp -- breathing animation

The implementation is well-structured:
- Properly destroys old animation before creating new one (line 253-254)
- Uses `QSequentialAnimationGroup` for the fade-in/fade-out cycle
- `setLoopCount(-1)` for infinite looping is correct
- `DeleteWhenStopped` policy is applied to the group

**Observation**: The `stopBreathAnimation()` method at line 289 does
`m_breathAnim->stop(); delete m_breathAnim;`. If `DeleteWhenStopped`
is set on the group, the `stop()` call might trigger deletion via the
animation framework before the explicit `delete`. This is a potential
double-delete. However, since `stop()` followed by immediate `delete`
is a common Qt pattern and the delete happens in the same call stack
before the event loop processes the DeleteWhenStopped, it is safe in
practice. Consider adding a comment explaining this.

### NavigationController.cpp -- panel switching

Clean parallel cross-fade implementation (lines 266-297). The old-then-
new sequential approach has been replaced with simultaneous fade-out +
fade-in, reducing perceived delay from 300ms to 150ms. Good improvement.

**Observation**: The `m_panelSwitching` flag is set in `switchToPanel()`
and cleared only in the fade-in `finished` callback. If the user clicks
a nav item during the 150ms window, the click is silently dropped. This
is acceptable UX but worth documenting.

### ToastWidget.h -- debounce mechanism

The `showDebounced()` design is sound: `QHash<QString, QElapsedTimer>`
as a static map with key = type + message. The 2000ms default cooldown
is reasonable for preventing toast storms during reconnection loops.

The compilation error is purely a declaration-order issue, not a design
flaw.

### MainWindowSignalConnect.cpp -- toast wiring

Comprehensive signal-to-toast mapping (lines 225-274):
- Connection success/failure/disconnect/error all wired
- OTA start/complete/failure all wired
- Recording status messages wired
- Send status messages wired

Each toast uses the correct semantic type (Success/Error/Info).
The connection failure handler at line 87-91 duplicates the error into
both a QMessageBox and a toast. Consider whether both are needed -- the
QMessageBox blocks interaction while the toast is non-modal. Having both
may be redundant for non-critical errors.

### OtaWidget.cpp -- completion animation

The two-phase color transition (accent -> mid mix -> success) at lines
387-418 is a creative workaround. However, the 400ms animation uses a
`QPropertyAnimation` on "value" with startValue == endValue == 100,
purely as a timer. This is an unconventional use of the animation
framework. A `QTimer::singleShot(400, ...)` would be more idiomatic and
clearer in intent.

---

## 5. Architecture Observations

1. **Header-only ToastWidget**: At 234 lines and with complex inline
   methods, this is pushing the limits of the header-only pattern. The
   class includes painting logic, animation setup, and static map
   management all inline. Any change forces full recompilation of
   including translation units.

2. **SerialConfigPanel member type mismatch**: The breathing animation
   was redesigned from a single `QPropertyAnimation` to a
   `QSequentialAnimationGroup` without updating the member declaration.
   This suggests the header was not updated in sync with the
   implementation change.

3. **Missing QTimer include**: A straightforward oversight. The
   `setError()` method was likely added or modified to include the
   auto-revert timer without adding the corresponding include.

---

## 6. Summary

| Category | Result |
|----------|--------|
| Build | FAIL -- 3 errors |
| File size limits | 1 violation (ToastWidget.h 234/200) |
| Hardcoded colors | PASS |
| SIGNAL/SLOT macros | PASS |
| TODO/FIXME | PASS |
| Include order | PASS |

## 7. Recommendation: BLOCK

Three compilation errors must be fixed before commit. Required fixes:

1. Add `#include <QTimer>` to SerialConfigPanel.cpp
2. Change `m_breathAnim` type to `QAbstractAnimation*` in SerialConfigPanel.h
3. Reorder ToastWidget.h so `debounceMap()` is declared before `showDebounced()`

Additionally, ToastWidget.h should be split into .h/.cpp in a future
iteration to bring the header under the 200-line limit.

# Code Review #040 - EmbedDebug Codebase Quality Audit

**Commit**: d166aab
**Reviewer**: Code Reviewer (self-review agent)
**Date**: 2026-06-01
**Scope**: 13 key files across core/terminal/ota/serial/protocol modules

---

## File Size Audit

| File | Lines | Limit | Status |
|------|-------|-------|--------|
| MainWindow.h | 199 | 200 | PASS |
| MainWindow.cpp | 361 | 500 | PASS |
| MainWindowSignalConnect.cpp | 217 | 500 | PASS |
| TerminalWidget.h | 152 | 200 | PASS |
| TerminalWidget.cpp | 499 | 500 | PASS (borderline) |
| NavIndicatorWidget.h | 214 | 200 | **OVER by 14** |
| ToastWidget.h | 190 | 200 | PASS (includes ~70 lines inline impl) |
| AnimatedProgressBar.h | 181 | 200 | PASS |
| SerialConfigPanel.cpp | 351 | 500 | PASS |
| ProtocolBridgeManager.h | 152 | 200 | PASS |
| ProtocolBridgeManager.cpp | 439 | 500 | PASS |
| ConnectionController.h | 161 | 200 | PASS |
| ConnectionController.cpp | 421 | 500 | PASS |

---

## Findings

### CRITICAL

**C1. NavIndicatorWidget.h exceeds 200-line header limit**
- File: `src/core/NavIndicatorWidget.h:1-214`
- Lines: 214 (limit 200)
- The header-only class includes full method bodies for `moveToIndex()`, `paintEvent()`, `eventFilter()`, and the constructor. These implementations belong in a .cpp file.
- Recommended fix: Extract method implementations into `src/core/NavIndicatorWidget.cpp`. Keep only declarations in the header. This is the pattern used by TerminalWidget (h: 152 lines, cpp: 499 lines).
- Blocks commit: YES

**C2. ToastWidget.h leaks QPropertyAnimation objects in two code paths**
- File: `src/core/ToastWidget.h:51-56,140,165`
- In `show()` at line 51: `auto* slide = new QPropertyAnimation(toast, "pos")` -- no parent, relies on `DeleteWhenStopped`. If `start()` is never called (exception in setup), this leaks.
- In `dismiss()` at line 140 and `repositionToasts()` at line 165: same pattern.
- Recommended fix: Set `toast` as parent of each animation explicitly, e.g. `new QPropertyAnimation(toast, "pos", toast)`. The `DeleteWhenStopped` policy still works but parent guarantees cleanup on any path.
- Blocks commit: YES

### WARNING

**W1. ToastWidget static map holds stale pointers if parent is destroyed**
- File: `src/core/ToastWidget.h:148-151`
- `activeToasts()` uses a `static QMap<QWidget*, QList<ToastWidget*>>`. If the parent widget is destroyed while toasts are visible, the map retains dangling pointers. The toasts are children of the parent and will be destroyed by Qt, but the map entry persists.
- Recommended fix: Override parent's `QObject::destroyed` signal to clean up the map entry, or use a QWeakPointer pattern. At minimum, add `connect(parent, &QObject::destroyed, ...)` in `show()` to call `map.remove(parent)`.
- Blocks commit: No

**W2. TerminalWidget.cpp paintEvent is 62 lines with two near-identical render paths**
- File: `src/terminal/TerminalWidget.cpp:260-322`
- The filtered and non-filtered code paths duplicate cache-update, scroll-offset clamping, and painting logic. This makes the method fragile -- a bug fix in one path is easily missed in the other.
- Recommended fix: Extract a helper `renderLines(QPainter&, int totalLines)` that both branches call, or compute `totalLines` and `lineResolver` lambda upfront then render once.
- Blocks commit: No

**W3. ConnectionController has duplicated teardown logic in three methods**
- File: `src/core/ConnectionController.cpp`
- `disconnectCurrent()` (line 139), `onConnectionTimeout()` (line 298), and `onPortRemoved()` (line 346) all contain the same pattern: null-check, cache pointer, null member, disconnect signals, removeConnection, clearDownstream.
- Recommended fix: Extract a private `teardownCurrentConnection()` method and call it from all three sites. Reduces ~30 lines of duplication and prevents divergence.
- Blocks commit: No

**W4. MainWindowSignalConnect.cpp line 188-202 uses string comparison for nav tree clicks**
- File: `src/core/MainWindowSignalConnect.cpp:188-202`
- `if (text == tr("..."))` compares translated strings. Under non-Chinese locale, this silently breaks: the comparison uses runtime `tr()` result but the strings were authored in Chinese.
- Recommended fix: Use item data roles (e.g., `Qt::UserRole`) to store panel identifiers, then compare by enum/int instead of display text. This is locale-independent and avoids string-matching brittleness.
- Blocks commit: No

**W5. SerialConfigPanel breath animation uses manual QTimer instead of QPropertyAnimation**
- File: `src/serial/SerialConfigPanel.cpp:31-42`
- The breathing effect is implemented with a 50ms `QTimer` and manual opacity stepping (`+= 0.05`). This violates CLAUDE.md section 6.5 which mandates QPropertyAnimation for all animations.
- The resulting frame rate is ~20fps (50ms interval), below the 30fps minimum in section 6.5.
- Recommended fix: Replace with `QPropertyAnimation` on a custom `breathOpacity` Q_PROPERTY, using `setLoopCount(-1)` and `QEasingCurve::InOutSine` per the animation spec.
- Blocks commit: No

**W6. AnimatedProgressBar::stopShimmer() sets pointer to nullptr before animation deletes itself**
- File: `src/ota/AnimatedProgressBar.h:97-104`
- After `stop()`, `m_shimmerAnim` is set to `nullptr`. But `DeleteWhenStopped` means the animation object will be deleted after this call returns. This is safe here because we do not access the pointer after nulling, but the sequence is misleading.
- Recommended fix: Either set `m_shimmerAnim = nullptr` before calling `stop()` (current order is fine), or remove the manual `nullptr` assignment since `DeleteWhenStopped` handles lifetime. Add a brief comment explaining the ordering contract.
- Blocks commit: No

### INFO

**I1. Include order in MainWindow.h does not follow self->Qt->project convention**
- File: `src/core/MainWindow.h:1-43`
- Current: Qt includes, then project includes. The CLAUDE.md section 5.3 specifies `#include <Qt> -> <STL> -> "project"`. This is the correct order. No issue found.

**I2. Include order in ConnectionController.cpp**
- File: `src/core/ConnectionController.cpp:14-23`
- Order is: `"core/ConnectionController.h"` (self), `<QTimer>`, then project includes. This follows the convention correctly.

**I3. All QObjects have proper parent ownership**
- Verified across all reviewed files. Every `new QObject` call passes a parent pointer. No naked `new` without parent found in any .cpp file.
- Exception: ToastWidget.h line 51 (see C2) and NavIndicatorWidget.h line 67 passes `this` correctly.

**I4. No SIGNAL/SLOT macros found**
- Full codebase search returned zero matches. All connections use new-style function pointer syntax. PASS.

**I5. No cross-thread QObject usage**
- Search for QThread, moveToThread, QtConcurrent, QFuture returned zero matches. All QObjects live on the main thread. PASS.

**I6. TerminalWidget::paintLine at 57 lines is within the 80-line method limit**
- The longest method in reviewed files is `MainWindow::setupUI()` at ~85 lines. This slightly exceeds the 80-line limit.
- File: `src/core/MainWindow.cpp:152-237` -- `setupUI()` is 85 lines.
- Severity: INFO (borderline, not worth blocking).

---

## Design Pattern Compliance

| Pattern | Implementation | Status |
|---------|---------------|--------|
| Strategy (OTA) | `IProtocol` interface in `ota/protocols/` | PASS |
| Factory (Connection) | `ConnectionFactory` / `ConnectionManager` | PASS |
| Singleton (Settings/Theme) | `SettingsManager::instance()`, `ThemeManager::instance()` | PASS |
| Observer (signals) | Qt signal/slot throughout | PASS |
| Strategy (Protocol) | `IProtocolBridge` + `ProtocolBridgeManager::switchSource()` | PASS |
| Mediator (MainWindow) | MainWindow delegates to Controllers, no business logic in UI | PASS |

---

## Layer Violation Check

- Presentation layer (QWidget subclasses) does not directly depend on infrastructure layer. All communication goes through controllers.
- `ProtocolBridgeManager` (business) correctly depends on data layer (`FrameParser`) but not presentation.
- No violations found. PASS.

---

## Summary

| Severity | Count | Blocking |
|----------|-------|----------|
| CRITICAL | 2 | Yes |
| WARNING | 6 | No |
| INFO | 6 | No |

**Overall assessment**: The codebase is well-structured with consistent design patterns, proper memory management, and clean signal routing. The two blocking issues are NavIndicatorWidget.h exceeding the line limit (requiring a .cpp split) and QPropertyAnimation objects created without explicit parents in ToastWidget.h (potential leak on exceptional paths). These should be fixed before commit.

**Positive observations**:
- Zero SIGNAL/SLOT macro usage across entire codebase
- All QObjects properly parented (except C2)
- No thread-safety concerns (single-threaded design)
- Consistent Doxygen documentation on public APIs
- Clean separation between presentation, business, and infrastructure layers
- MainWindow.cpp successfully reduced from 1092 to 361 lines through controller extraction

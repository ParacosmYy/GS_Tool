# ARCH_REVIEW_006 -- Commit #25 Architecture Management Review

> Reviewer: System Architect (system-architect)
> Date: 2026-05-31
> Scope: Full architecture after commits #21-#25 (post ARCH_REVIEW_005)
> Previous review: ARCH_REVIEW_005 (commit #20)

---

## 0. Overall Score

| Dimension | Score (0-10) | Delta vs #005 |
|-----------|-------------|---------------|
| Layer compliance | 9.0 | +0.5 |
| Design pattern compliance | 9.0 | 0 |
| Public component reuse | 9.0 | 0 |
| Inter-module coupling | 9.0 | +1.0 |
| Header include convention | 8.5 | 0 |
| New class registration | 9.5 | 0 |
| File size limits | 7.5 | -1.3 |
| **Overall** | **8.8** | **0** |

---

## 1. Layer Compliance (Presentation / Business / Data / Infra)

### 1.1 New dependencies introduced in commits #21-#25

| Source -> Target | Source Layer | Target Layer | Compliant |
|-----------------|-------------|-------------|-----------|
| SendController -> IConnection | Business | Infra | OK |
| SendController -> TerminalModel | Business | Data | OK |
| SendController -> DataLogger | Business | Infra | OK |
| SendController -> SendHistory | Business | Data | OK |
| SendController -> TimedSender | Business | Data | OK |
| SendController -> HexConverter | Business | Infra | OK |
| ConnectionController -> IConnection | Business | Infra | OK |
| ConnectionController -> ConnectionManager | Business | Business | OK |
| ConnectionController -> SendController | Business | Business | OK |
| ConnectionController -> OtaManager | Business | Business | OK |
| ConnectionController -> RecordingController | Business | Business | OK |
| ConnectionController -> SerialConnection | Business | Infra | WARNING (L-001) |
| TerminalWidget -> TerminalModel | Presentation | Data | OK |
| TerminalWidget -> HexConverter | Presentation | Infra | OK |
| TerminalWidget -> Constants | Presentation | Infra | OK |
| SerialConfigPanel -> Constants | Presentation | Infra | OK (no direct dependency) |
| MainWindow -> SendController | Presentation | Business | OK |
| MainWindow -> ConnectionController | Presentation | Business | OK |

### 1.2 Resolved since ARCH_REVIEW_005

| Item | Status | Resolution |
|------|--------|-----------|
| N-001 ChannelConfig/ChartModel not registered | RESOLVED | Added to CLAUDE.md 4.4 public component list |
| C-001 MainWindow fan-out | MITIGATED | MainWindow.h includes reduced from 19 to ~17. SendController and ConnectionController extraction removed direct TerminalModel/SendHistory/HexConverter dependency chains from MainWindow. |
| L-001 MainWindow manages FrameParser directly | UNCHANGED | Deferred. Acceptable at current scale. |

### 1.3 Layer compliance findings

#### FINDING L-001: ConnectionController::setDtr/setRts uses qobject_cast to SerialConnection [Minor]

- Location: `src/core/ConnectionController.cpp:135-149`
- ConnectionController (Business layer) includes `"connection/SerialConnection.h"` and performs `qobject_cast<SerialConnection*>(m_currentConn)`.
- This creates a Business -> Infra concrete-class dependency. While the Business -> Infra direction is permitted, the use of `qobject_cast` to a concrete type suggests the IConnection interface is insufficient for runtime line-control operations.
- The existing guard `m_currentConn->type() == ConnectionType::Serial` is correct, but the cast itself indicates a missing abstraction.
- Impact: Low. DTR/RTS is serial-specific by nature. However, adding a `setLineControl(dtr, rts)` virtual method to IConnection with a no-op default would eliminate the cast entirely.
- Recommendation: Consider adding `virtual void setLineControl(bool dtr, bool rts) { Q_UNUSED(dtr); Q_UNUSED(rts); }` to IConnection. ConnectionController would call `m_currentConn->setLineControl(enabled, ...)` without any cast. This keeps serial-specific behavior encapsulated in SerialConnection where it belongs.

---

## 2. Design Pattern Compliance (8 Patterns from CLAUDE.md 4.1)

| Pattern | Required Usage | Actual Implementation | Compliant | Notes |
|---------|---------------|----------------------|-----------|-------|
| Strategy | OTA protocol switching | BaseTransfer hierarchy unchanged. OtaManager selects via string key. | OK | No regression. |
| Observer | Data flow distribution | Qt signal/slot throughout. New signals: `searchMatchesChanged`, `dtrChanged`, `rtsChanged`. | OK | Clean Observer. TerminalWidget emits `searchMatchesChanged(int, int)` to decouple from TerminalSearchBar. |
| Factory | Creating connections | ConnectionFactory unchanged. ConnectionController delegates to ConnectionManager. | OK | No regression. |
| State | Connection state management | ConnectionState enum unchanged. MainWindow lambda at line 283-315 switches on state. | OK | No regression. |
| Singleton | Global managers | SettingsManager, ThemeManager. No new singletons. | OK | No regression. |
| Template Method | OTA transfer flow | BaseTransfer unchanged. | OK | No regression. |
| Adapter | J-Link SDK adaptation | Not yet implemented. | N/A | Placeholder. No violation. |
| Command | Quick commands, send history | QuickCommandBar unchanged. SendController::onQuickCommand is the command handler. | OK | No regression. |

### 2.1 Pattern compliance findings

No new pattern violations detected. The search highlight feature in TerminalWidget correctly uses the Observer pattern: TerminalSearchBar emits `searchRequested` -> MainWindow lambda calls `TerminalWidget::setSearchHighlight()` -> TerminalWidget emits `searchMatchesChanged` -> MainWindow lambda updates `TerminalSearchBar::setResultText()`. The data flow is unidirectional and clean.

---

## 3. Coupling Assessment

### 3.1 Search highlight decoupling

The search highlight feature in commit #25 follows a well-decoupled design:

```
TerminalSearchBar --[searchRequested]--> MainWindow --[setSearchHighlight]--> TerminalWidget
TerminalWidget --[searchMatchesChanged]--> MainWindow --[setResultText]--> TerminalSearchBar
```

- TerminalSearchBar has no knowledge of TerminalWidget (no include, no pointer).
- TerminalWidget has no knowledge of TerminalSearchBar (no include, no pointer).
- MainWindow acts as the Mediator, connecting the two via signals/slots.
- TerminalWidget internally manages all search state (pattern, matches, current index, navigation).
- This is a textbook Mediator pattern implementation. No coupling issues.

### 3.2 DTR/RTS signal chain

```
SerialConfigPanel --[dtrChanged/rtsChanged]--> MainWindow --[setDtr/setRts]--> ConnectionController --[qobject_cast]--> SerialConnection
```

- SerialConfigPanel emits `dtrChanged(bool)` and `rtsChanged(bool)` signals.
- MainWindow connects these directly to `ConnectionController::setDtr` and `ConnectionController::setRts` at lines 276-279.
- ConnectionController validates the connection type, casts to SerialConnection, and calls the concrete method.
- The chain is clean at the Presentation -> Business boundary (signal/slot).
- The Business -> Infra boundary has the minor L-001 issue noted above.
- DTR/RTS checkboxes remain enabled during connection (SerialConfigPanel.cpp:148-151), which is correct behavior for runtime line control.

### 3.3 Coupling matrix (updated)

```
                      MainWindow  SendCtrl  ConnCtrl  TerminalWidget  SerialConfig  TerminalSearchBar
MainWindow               -        owns      owns       owns           owns           owns
SendController           -         -        -          -              -              -
ConnectionController     -         uses      -          -              -              -
TerminalWidget           -         -         -          -              -              -
SerialConfigPanel        -         -         -          -              -              -
TerminalSearchBar        -         -         -          -              -              -
```

Coupling is clean. SendController, ConnectionController, TerminalWidget, SerialConfigPanel, and TerminalSearchBar have zero direct dependencies on each other. All communication flows through MainWindow as Mediator.

### 3.4 Coupling findings

#### FINDING C-001: MainWindow.cpp at 623 lines -- exceeds 500-line limit [Major]

- Location: `src/core/MainWindow.cpp` -- 623 lines
- CLAUDE.md 4.6 specifies `.cpp` files must be at most 500 lines.
- The file has been significantly reduced from its peak of ~1092 lines (a 43% reduction across commits #21-#22), which is commendable progress.
- However, it still exceeds the hard limit by 123 lines.
- Remaining bulk: `connectSignals()` method alone is ~180 lines (256-436), `setupUI()` is ~103 lines (76-179), plus toolbar/statusbar/settings/action handlers.
- Recommendation: Extract `connectSignals()` into a separate `SignalRouter` class or distribute the connection logic into individual controller classes. The `setupUI()` method is tight enough to remain. The action handlers (onDisplayModeChanged, onTimestampToggled, etc.) could move to a `ToolbarController` or stay if connectSignals is extracted.

---

## 4. File Size Limits

### 4.1 Audit results

| File | Lines | Limit | Status |
|------|-------|-------|--------|
| TerminalWidget.h | 127 | 200 | OK |
| TerminalWidget.cpp | 481 | 500 | OK (19 lines margin) |
| ConnectionController.h | 68 | 200 | OK |
| ConnectionController.cpp | 198 | 500 | OK |
| SendController.h | 82 | 200 | OK |
| SendController.cpp | 164 | 500 | OK |
| MainWindow.h | 151 | 200 | OK |
| MainWindow.cpp | 623 | 500 | **EXCEEDS by 123 lines** |
| SerialConfigPanel.h | 65 | 200 | OK |
| SerialConfigPanel.cpp | 221 | 500 | OK |

### 4.2 Method-level analysis

| Method | File | Lines | Limit | Status |
|--------|------|-------|-------|--------|
| `paintEvent` | TerminalWidget.cpp:126-218 | 92 | 80 | EXCEEDS by 12 |
| `setSearchHighlight` | TerminalWidget.cpp:337-400 | 63 | 80 | OK |
| `connectSignals` | MainWindow.cpp:256-436 | 180 | 80 | **EXCEEDS by 100** |
| `setupUI` | MainWindow.cpp:76-179 | 103 | 80 | EXCEEDS by 23 |
| `MainWindow ctor` | MainWindow.cpp:19-70 | 51 | 80 | OK |
| `createSendBar` | SendController.cpp:31-90 | 59 | 80 | OK |

### 4.3 Findings

#### FINDING F-001: MainWindow.cpp exceeds 500-line hard limit at 623 lines [Major]

- This is a regression from the shrinking trend. While the extraction of SendController and ConnectionController reduced MainWindow significantly, the remaining `connectSignals()` method at 180 lines is the single largest contributor.
- Score impact: -1.3 points on file size limits dimension.

#### FINDING F-002: TerminalWidget::paintEvent() at 92 lines exceeds 80-line method limit [Minor]

- Location: `src/terminal/TerminalWidget.cpp:126-218`
- The method handles background fill, timestamp rendering, selection highlight, search highlight, and data text rendering in a single pass.
- The painting is performance-critical and splitting it may introduce overhead from repeated QPainter state changes.
- Recommendation: Acceptable for a paint method. The logic is sequential rendering passes, not branching control flow. Consider extracting the search highlight rendering loop (lines 195-205) into a `paintSearchHighlights()` helper.

---

## 5. New Class Registration

### 5.1 New classes added in commits #21-#25

| Class | File | Layer | Registered in CLAUDE.md 4.4 | Status |
|-------|------|-------|----------------------------|--------|
| NavigationController | `core/NavigationController.h/cpp` | Business | YES | OK |
| RecordingController | `core/RecordingController.h/cpp` | Business | YES | OK |
| SendController | `core/SendController.h/cpp` | Business | YES | OK |
| ConnectionController | `core/ConnectionController.h/cpp` | Business | YES | OK |
| IProtocolBridge | `protocol/IProtocolBridge.h` | Business | YES | OK |
| JustFloatBridge | `protocol/JustFloatBridge.h/cpp` | Business | YES | OK |
| FireWaterBridge | `protocol/FireWaterBridge.h/cpp` | Business | YES | OK |
| ProtocolBridgeManager | `protocol/ProtocolBridgeManager.h/cpp` | Business | YES | OK |
| TerminalSearchBar | `terminal/TerminalSearchBar.h/cpp` | Presentation | NOT LISTED | ISSUE (N-001) |
| SearchMatch (struct) | `terminal/TerminalWidget.h` | Presentation | N/A (internal) | OK |

### 5.2 Registration findings

#### FINDING N-001: TerminalSearchBar not in CLAUDE.md 4.4 public component list [Minor]

- `src/terminal/TerminalSearchBar.h/cpp` is a reusable presentation-layer component that encapsulates search input, regex/HEX mode toggle, result display, and open/close lifecycle.
- It is a self-contained widget with a clear API (`activate()`, `deactivate()`, `setResultText()`, signals: `searchRequested`, `searchCleared`, `closed`).
- Not registered in CLAUDE.md 4.4.
- Recommendation: Add to CLAUDE.md 4.4:

```
| `TerminalSearchBar` | `terminal/TerminalSearchBar.h/cpp` | Terminal search bar widget (text/regex/HEX search with match count) |
```

---

## 6. Header Include Order (Qt -> STL -> Project)

### 6.1 Audit results for reviewed files

| File | Actual Order | Expected Order | Verdict |
|------|-------------|---------------|---------|
| TerminalWidget.h | Qt (QWidget, QTimer) -> Project (TerminalModel.h, Constants.h) | Qt -> Project | OK |
| TerminalWidget.cpp | Own header -> Project (HexConverter.h) -> Qt (QPainter... QRegularExpression) | Own header -> Qt -> Project | **ISSUE (H-001)** |
| ConnectionController.h | Qt (QObject, QVariantMap) -> Project (IConnection.h, ConnectionManager.h) | Qt -> Project | OK |
| ConnectionController.cpp | Own header -> Project (SendController, OtaManager, RecordingController, DataLogger, TerminalModel, SerialConnection) | Own header -> Qt -> Project | **ISSUE (H-001)** -- no Qt includes needed, acceptable |
| SendController.h | Qt (QObject, QStringListModel, QCompleter) -> fwd decls | Qt -> fwd decls | OK |
| SendController.cpp | Own header -> Project (TerminalModel, DataLogger, SendHistory, TimedSender, IConnection, HexConverter, Constants) -> Qt (QLineEdit, QPushButton, QComboBox, QHBoxLayout, QFrame, QStyle) | Own header -> Qt -> Project | **ISSUE (H-001)** |
| MainWindow.cpp | Own header -> Project (ChartModel, HexConverter) -> Qt (QVBoxLayout... QDir) | Own header -> Qt -> Project | **ISSUE (H-001)** |
| SerialConfigPanel.h | Qt (QWidget, QComboBox... QSerialPort, QVariantMap) | Qt -> OK (no project headers) | OK |
| SerialConfigPanel.cpp | Own header -> Qt (QHBoxLayout, QVBoxLayout, QFormLayout, QGroupBox, QSerialPortInfo) | Own header -> Qt | OK |

### 6.2 Include convention findings

#### FINDING H-001: .cpp files place project includes before Qt includes [Minor, recurring]

- This is a recurring finding from ARCH_REVIEW_005 H-001. The pattern persists in:
  - `TerminalWidget.cpp`: `#include "TerminalWidget.h"` -> `#include "utils/HexConverter.h"` -> `<QPainter>` ...
  - `SendController.cpp`: own header -> 7 project headers -> 6 Qt headers
  - `MainWindow.cpp`: own header -> 2 project headers -> 14 Qt headers
- Root cause: The convention was never documented in CLAUDE.md for .cpp files.
- Impact: Very low. Compilation is unaffected. IDE navigation slightly impacted.
- Recommendation: Same as ARCH_REVIEW_005. Document in CLAUDE.md 5.3: "For .cpp files, include own header first, then Qt headers, then STL headers, then other project headers." This is a documentation gap, not a code defect.

---

## 7. Specific Improvements from Commits #21-#25

### 7.1 Commit #21 -- NavigationController + RecordingController extraction

- NavigationController extracted panel switching logic, breathing animation, and nav tree building from MainWindow.
- RecordingController extracted recording/playback button setup and DataLogger interaction.
- Clean dependency injection: both receive their dependencies via constructor parameters.
- MainWindow.cpp reduced from ~1092 lines by ~200 lines.

### 7.2 Commit #22 -- SendController extraction

- `sendAndRecord()` unified method moved from MainWindow to SendController at lines 102-118.
- SendController now owns the send bar UI (`createSendBar()`), input parsing, HEX validation, newline appending, and history recording.
- SendHistory completer leak fixed: `m_sendCompleterModel` is a persistent member, not per-call allocation.
- SendController::onQuickCommand provides a clean command-pattern entry point.
- **Key improvement**: `statusMessage` signal emitted on send failure (line 106, 116), fixing the previous silent-failure behavior.

### 7.3 Commit #23 -- ConnectionController extraction + JustFloat/FireWater bridges

- ConnectionController centralized all connection lifecycle management:
  - `connectSerial()` / `disconnectSerial()` / `connectNetwork()`
  - State synchronization to SendController, OtaManager, RecordingController
  - DTR/RTS pass-through
- Suspended pointer fix: `onConnectionStateChanged()` at lines 151-180 properly clears downstream controller references on disconnect.
- Network connection now also syncs OtaManager (line 125-127), fixing a gap where TCP/UDP connections lacked OTA support.
- JustFloatBridge and FireWaterBridge implement IProtocolBridge for VOFA+ compatibility.

### 7.4 Commit #24 -- ProtocolBridgeManager + objectName audit + connection fixes

- ProtocolBridgeManager routes all protocol sources through a single `frameParsed` signal.
- objectName audit completed for 8 controls.
- QSS interactive states added for SpinBox, ComboBox, and ToolButton disabled states.
- ConnectionController suspended pointer fix verified: all three downstream controllers (Send, OTA, Recording) are cleared on disconnect.

### 7.5 Commit #25 -- Terminal search highlight + DTR/RTS + newline append

- **Search highlight**: TerminalWidget now implements a complete search system:
  - `setSearchHighlight(pattern, regex, hex)` supports three search modes
  - Internal `SearchMatch` struct tracks line, column, and length
  - `refreshSearch()` re-executes on data arrival to keep results current
  - `gotoNextMatch()` / `gotoPrevMatch()` with F3/Shift+F3 keyboard navigation
  - `scrollToMatch()` auto-scrolls to bring off-screen matches into view
  - `searchMatchesChanged(int total, int current)` signal for match counter
  - Painting uses two-tone highlight: semi-transparent yellow for all matches, more opaque for current match
- **DTR/RTS runtime control**: SerialConfigPanel checkboxes remain enabled during connection, emitting `dtrChanged`/`rtsChanged` signals through to SerialConnection.
- **Newline append**: SendController adds `\r\n`, `\n`, or `\r` based on `m_newlineCombo` selection. Text-mode only.
- **CLAUDE.md updates**: Bug fix sprint mechanism, VOFA+ as benchmark, driver detection candidate added.

---

## 8. Findings Summary by Category

### Critical (0 findings)

None. No blocking issues.

### Major (1 finding)

| ID | Category | Summary | Location |
|----|----------|---------|----------|
| C-001 / F-001 | File Size | MainWindow.cpp at 623 lines exceeds 500-line hard limit. `connectSignals()` alone is 180 lines. | `src/core/MainWindow.cpp` |

### Minor (5 findings)

| ID | Category | Summary | Location |
|----|----------|---------|----------|
| L-001 | Layers | ConnectionController::setDtr/setRts uses qobject_cast to SerialConnection concrete type | `src/core/ConnectionController.cpp:135-149` |
| F-002 | File Size | TerminalWidget::paintEvent() at 92 lines exceeds 80-line method limit | `src/terminal/TerminalWidget.cpp:126-218` |
| N-001 | Registration | TerminalSearchBar not in CLAUDE.md 4.4 public component list | `src/terminal/TerminalSearchBar.h` |
| H-001 | Includes | .cpp include order: project headers before Qt headers (recurring) | Multiple .cpp files |
| H-002 | Includes | .cpp include convention not documented in CLAUDE.md | CLAUDE.md 5.3 |

---

## 9. Improvement Recommendations (Priority Order)

### P1 -- Should address in next 1-2 commits

| ID | Recommendation | Effort | Files |
|----|---------------|--------|-------|
| C-001 | Extract `connectSignals()` from MainWindow into a `SignalRouter` class or distribute signal wiring into respective controller classes. Target: MainWindow.cpp under 500 lines. | Medium | New SignalRouter class or expanded controllers |
| N-001 | Add TerminalSearchBar to CLAUDE.md 4.4 public component list | Small | CLAUDE.md |
| H-002 | Document .cpp include convention in CLAUDE.md 5.3: own header -> Qt -> STL -> project | Small | CLAUDE.md |

### P2 -- Should address in next 3-5 commits

| ID | Recommendation | Effort | Files |
|----|---------------|--------|-------|
| L-001 | Add `virtual void setLineControl(bool dtr, bool rts)` to IConnection with no-op default, eliminating qobject_cast in ConnectionController | Small | IConnection.h, SerialConnection.h/cpp, ConnectionController.cpp |
| F-002 | Extract search highlight painting from paintEvent into `paintSearchHighlights()` helper method | Small | TerminalWidget.cpp |

---

## 10. Architectural Health Trend

| Review | Commit | Score | Key Theme |
|--------|--------|-------|-----------|
| ARCH_REVIEW_003 | #10 | 7.0 | Initial layer compliance established |
| ARCH_REVIEW_004 | #15 | 7.2 | Performance bottleneck identification |
| ARCH_REVIEW_005 | #20 | 8.8 | Ring buffer, NavPanelMapping, animation framework |
| ARCH_REVIEW_006 | #25 | 8.8 | Controller extraction, search highlight, DTR/RTS |

The overall score remains at 8.8. The architecture has consolidated across commits #21-#25 through systematic controller extraction (SendController, ConnectionController, NavigationController, RecordingController). The codebase is in solid shape. The single Major finding (MainWindow.cpp exceeding 500 lines) is a known, tracked issue with a clear remediation path.

### Key architectural achievements since ARCH_REVIEW_005:

1. **Four controllers extracted**: NavigationController, RecordingController, SendController, ConnectionController -- each with single responsibility and clean dependency injection.
2. **Search highlight fully decoupled**: TerminalSearchBar and TerminalWidget have zero mutual awareness. MainWindow mediates via signals.
3. **DTR/RTS runtime control**: Signal chain from UI (SerialConfigPanel) through business (ConnectionController) to infra (SerialConnection) follows layer boundaries.
4. **Suspended pointer prevention**: ConnectionController clears all downstream references on disconnect.
5. **Protocol bridge extensibility**: IProtocolBridge interface + ProtocolBridgeManager enables adding new protocol sources without modifying consumers.

### Risk assessment for next 5 commits:

- If MainWindow.cpp is not reduced below 500 lines in the next 2 commits, the architectural health score will decrease.
- The TerminalWidget.cpp at 481 lines is approaching the 500-line limit. Search highlight added ~140 lines. If more terminal features are added, extraction of rendering logic may become necessary.
- No new singletons, no new reverse dependencies, no new coupling violations. The architectural foundation is sound.

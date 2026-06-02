# ARCH_REVIEW_005 -- Commit #20 Architecture Management Review

> Reviewer: System Architect (system-architect)
> Date: 2026-05-31
> Scope: Full architecture after commits #16-#19 (post ARCH_REVIEW_004)
> Previous review: ARCH_REVIEW_004 (commit #15)

---

## 0. Overall Score

| Dimension | Score (0-10) | Delta vs #004 |
|-----------|-------------|---------------|
| Layer compliance | 8.5 | +0.5 |
| Design pattern compliance | 9.0 | +1.0 |
| Public component reuse | 9.0 | 0 |
| Inter-module coupling | 8.0 | +1.0 |
| Header include convention | 8.5 | +1.0 |
| New class registration | 9.5 | +1.0 |
| Directory structure | 9.0 | +1.0 |
| **Overall** | **8.8** | **+0.8** |

---

## 1. Layer Compliance (Presentation / Business / Data / Infra)

### 1.1 Dependency direction verification

All module dependencies were verified. No reverse dependencies found.

| Source -> Target | Source Layer | Target Layer | Compliant |
|-----------------|-------------|-------------|-----------|
| MainWindow -> ConnectionManager | Presentation | Business | OK |
| MainWindow -> TerminalModel | Presentation | Data | OK |
| MainWindow -> FrameParser | Presentation | Data | OK |
| MainWindow -> OtaManager | Presentation | Business | OK |
| MainWindow -> IConnection | Presentation | Infra | OK |
| MainWindow -> DataLogger | Presentation | Infra | OK |
| MainWindow -> DataExporter | Presentation | Infra | OK |
| OtaWidget -> OtaManager | Presentation | Business | OK |
| OtaManager -> BaseTransfer | Business | Business | OK |
| OtaManager -> IConnection | Business | Infra | OK |
| FrameParser -> CRC | Data | Infra | OK |
| FrameParser -> HexConverter | Data | Infra | OK |
| TerminalModel -> Constants | Data | Infra | OK |
| ChartModel -> ChannelConfig | Data | Data | OK |
| IConnection -> Constants | Infra | Infra | OK |
| SerialConnection -> IConnection | Infra | Infra | OK |
| BaseTransfer -> IConnection | Business | Infra | OK |
| DataExporter -> TerminalTypes | Infra | Infra | OK |
| DataLogger -> (none external) | Infra | Infra | OK |

### 1.2 Resolved since ARCH_REVIEW_004

| Item | Status | Resolution |
|------|--------|-----------|
| V-001 sendAndRecord duplication | RESOLVED | `sendAndRecord()` method extracted at MainWindow.cpp:684-698. Three call sites (onSendData, onQuickCommand, TimedSender callback) now use unified method. |
| V-002 panel visibility control in state handler | RESOLVED | `onConnectionStateChanged()` at MainWindow.cpp:856-890 now calls `switchToPanel(m_terminal)` instead of direct setVisible. |
| V-004 OtaWidget bypass | MITIGATED | OtaManager is still set from MainWindow, but the pattern is clearer: `m_otaManager->setConnection(m_currentConn)` at MainWindow.cpp:672. Acceptable at current scale. |

### 1.3 Remaining layer issues

#### FINDING L-001: MainWindow still holds data-layer FrameParser directly [Minor]

- Location: `src/core/MainWindow.h:186` -- `FrameParser* m_frameParser`
- MainWindow (Presentation) creates and owns FrameParser (Data), and directly calls `m_frameParser->feed(data)` in `onDataReceived()` at MainWindow.cpp:895, and `m_frameParser->setDefinition(def)` in the FrameEditor signal handler at MainWindow.cpp:535-539.
- The dependency direction (Presentation -> Data) is permitted per CLAUDE.md 4.3, but MainWindow also manages FrameParser's lifecycle and configuration dispatch, which is a business orchestration concern.
- Impact: Low. FrameParser has no presentation dependencies. When a ProtocolEngine business layer is introduced, this moves cleanly.
- Recommendation: Defer to next cycle. The current placement is documented and consistent.

#### FINDING L-002: TerminalTypes.h location in terminal/ instead of utils/ or shared/ [Minor]

- Location: `src/terminal/TerminalTypes.h`
- TerminalTypes defines `TerminalLine` and `DataDirection`, which are consumed by:
  - `terminal/TerminalModel.h` (Data layer, same directory)
  - `utils/DataExporter.h` (Infra layer, different directory)
  - `terminal/TerminalWidget.cpp` (Presentation, different directory)
- Placing a shared data type in the `terminal/` directory creates an implicit coupling: DataExporter (Infra) depends on `terminal/TerminalTypes.h`, meaning the Infra layer reaches into the Presentation/Data subdirectory.
- The previous ARCH_REVIEW_004 did not flag this because the file was created to solve the exact reverse-dependency problem (DataExporter originally depended on TerminalModel).
- Recommendation: Move `TerminalTypes.h` to `utils/TerminalTypes.h` or `core/TerminalTypes.h` where both Data and Infra layers can include it without crossing into the terminal module. This is a Minor issue because the dependency direction is still correct (Infra depends on shared types, not on Presentation).

---

## 2. Public Component Reuse Check

### 2.1 Registered public components (CLAUDE.md 4.4)

| Component | File | Reused By | Status |
|-----------|------|-----------|--------|
| CRC | `utils/CRC.h` | FrameParser, XModemTransfer, YModemTransfer, ZModemTransfer | OK |
| HexConverter | `utils/HexConverter.h` | TerminalWidget, FrameParser, FrameDefinition, MainWindow | OK |
| RingBuffer<T> | `utils/RingBuffer.h` | NOT USED BY ANY CONSUMER | ISSUE (R-001) |
| SettingsManager | `utils/SettingsManager.h/cpp` | MainWindow, ThemeManager | OK |
| ThemeManager | `core/ThemeManager.h/cpp` | MainWindow | OK |
| DataLogger | `utils/DataLogger.h/cpp` | MainWindow | OK |
| IConnection | `connection/IConnection.h` | ConnectionManager, MainWindow, OtaManager, BaseTransfer, all concrete connections | OK |
| TerminalModel | `terminal/TerminalModel.h/cpp` | MainWindow, TerminalWidget, DataExporter | OK |
| TerminalWidget | `terminal/TerminalWidget.h/cpp` | MainWindow | OK |
| Constants | `core/Constants.h` | Pervasively used across all layers | OK |

### 2.2 Reuse findings

#### FINDING R-001: RingBuffer<T> registered but unused; TerminalModel implements its own ring buffer [Major]

- `src/utils/RingBuffer.h` is a complete, thread-safe ring buffer template registered in the public component list (CLAUDE.md 4.4).
- `src/terminal/TerminalModel.h:66-72` implements its own ring buffer from scratch using `QVector<TerminalLine> m_buffer` with `m_head` and `m_count` indices, plus `physicalIndex()` helper.
- This was the result of commit #18 partially implementing ARCH_REVIEW_004's P-002 recommendation, but the team chose to re-implement the ring buffer logic inline rather than reusing the existing `RingBuffer<T>` template.
- The inline implementation at TerminalModel.cpp:132-152 is functionally correct and well-documented, but it duplicates the ring buffer concept that already exists as a public component.
- Location: `src/terminal/TerminalModel.h:66-72` (m_buffer, m_head, m_count), `src/terminal/TerminalModel.cpp:132-152` (physicalIndex, appendLine)
- Impact: Code duplication of a core data structure pattern. Two different ring buffer implementations increase maintenance burden.
- Recommendation: Refactor TerminalModel to use `RingBuffer<TerminalLine>` internally. TerminalModel can wrap RingBuffer with its QMutex for thread safety and its `physicalIndex()` logic. Alternatively, accept the current implementation as a "domain-specific ring buffer with different API surface" and document the decision, but this should be a conscious choice.

#### FINDING R-002: No reinventing of other public components detected [Pass]

All modules correctly reuse CRC, HexConverter, SettingsManager, and IConnection without reimplementation.

---

## 3. Design Pattern Compliance (8 Patterns from CLAUDE.md 4.1)

| Pattern | Required Usage | Actual Implementation | Compliant | Notes |
|---------|---------------|----------------------|-----------|-------|
| Strategy | OTA protocol switching | BaseTransfer hierarchy with XModem/YModem/ZModem concrete classes. OtaManager selects protocol at runtime via string key at OtaManager.cpp:32-59. | OK | Clean strategy pattern. BaseTransfer provides template method hooks (onStartInit, sendCancelBytes, processReceivedData, handleTimeout). |
| Observer | Data flow distribution | Qt signal/slot throughout: IConnection::dataReceived, TerminalModel::dataAppended, FrameParser::frameParsed, OtaManager::progress, DataLogger::playbackData, etc. | OK | Consistent use of function-pointer connect syntax. No SIGNAL/SLOT macros found. |
| Factory | Creating connections | ConnectionFactory::create() at ConnectionFactory.h:30. ConnectionManager::createConnection() delegates to factory at ConnectionManager.cpp:21-30. | OK | Simple factory pattern. Returns nullptr for unsupported types. |
| State | Connection state management | ConnectionState enum (Disconnected/Connecting/Connected/Error) drives behavior in MainWindow::onConnectionStateChanged() at MainWindow.cpp:856-890 and IConnection interface. | OK | State-driven switch with different UI and animation behaviors per state. |
| Singleton | Global managers | SettingsManager::instance() at SettingsManager.h:17, ThemeManager::instance() at ThemeManager.h:10. Both use deleted copy/move constructors. | OK | Classic Meyer's singleton via static local. |
| Template Method | OTA transfer flow | BaseTransfer::start() and BaseTransfer::cancel() are template methods at BaseTransfer.h:37-42. Four pure virtual hooks: onStartInit, sendCancelBytes, processReceivedData, handleTimeout at BaseTransfer.h:64-77. | OK | Excellent application. Subclasses only implement protocol-specific hooks. |
| Adapter | J-Link SDK adaptation | RttConnection would implement IConnection to adapt J-Link SDK. Not yet implemented (rtt/ directory exists but only has placeholder files). | N/A | Placeholder. No violation. |
| Command | Quick commands, send history | QuickCommandBar emits commandTriggered signal with QByteArray data. SendHistory stores entries. Command data structure pattern is evident. | OK | Simple command pattern via data encapsulation. |

### 3.1 Pattern compliance findings

#### FINDING P-001: FrameParser state machine matches State pattern but is self-contained [Info]

- `src/protocol/FrameParser.h:43-51` defines `enum class State` with 6 states (Idle, HeaderMatching, LengthReceiving, PayloadReceiving, ChecksumVerifying, FooterMatching).
- The state transitions are implemented via switch-case in `processByte()` at FrameParser.cpp:46-265.
- This is the canonical state machine pattern (as required by CLAUDE.md for FrameParser), not the GoF State pattern with polymorphic state objects. This is the correct choice for a protocol parser where states have lightweight transitions and no complex per-state behavior.
- No issue.

---

## 4. Inter-Module Coupling Assessment

### 4.1 Coupling matrix (key modules)

```
                   MainWindow  OtaManager  TerminalModel  FrameParser  ChartModel  DataLogger  IConnection
MainWindow          -          uses         uses           uses         -           uses        uses
OtaManager          -           -           -              -            -           -           uses
TerminalModel       -           -           -              -            -           -           -
FrameParser         -           -           -              -            -           -           -
ChartModel          -           -           -              -            -           -           -
DataLogger          -           -           -              -            -           -           -
IConnection         -           -           -              -            -           -           -
```

### 4.2 Coupling findings

#### FINDING C-001: MainWindow has high fan-out (depends on 14+ classes) [Major]

- `src/core/MainWindow.h` includes 19 project headers (lines 20-39).
- MainWindow directly instantiates and wires together: ConnectionManager, TerminalModel, SendHistory, DataExporter, DataLogger, FrameParser, OtaManager, and all UI widgets.
- This is expected for a top-level application window (Mediator pattern), but the fan-out is high.
- The `buildNavPanelMappings()` data-driven approach (MainWindow.cpp:330-343) is a significant improvement over the previous if-else chain. The `NavPanelMapping` struct with `QT_TRANSLATE_NOOP` keys is well-designed for i18n.
- The `switchToPanel()` method with fade animation (MainWindow.cpp:351-449) is also a clean abstraction.
- Recommendation: Accept at current scale. The NavPanelMapping pattern and switchToPanel animation encapsulation are sufficient. If the panel count exceeds 10, consider a PanelRegistry.

#### FINDING C-002: OtaManager couples to all three concrete protocol classes [Minor]

- `src/ota/OtaManager.h:33-36` stores `XModemTransfer*`, `YModemTransfer*`, `ZModemTransfer*` as separate members.
- `startTransfer()` at OtaManager.cpp:32-59 uses if-else chain to select protocol.
- This partially violates the Strategy pattern's intent (the manager should not know concrete types). A cleaner approach would be a `QMap<QString, BaseTransfer*>` lookup.
- Impact: Adding a new protocol requires modifying OtaManager.h and OtaManager.cpp.
- Recommendation: Refactor to `QMap<QString, BaseTransfer*> m_protocols` with registration in constructor. Minor priority.

#### FINDING C-003: ChartModel depends on ChannelConfig but not on FrameParser [Good]

- `src/chart/ChartModel.h` only depends on `chart/ChannelConfig.h`. It receives data via the `onFrameParsed` slot, which takes `QVariantMap`.
- ChartModel has no dependency on FrameParser, FrameDefinition, or any protocol-layer class.
- This is clean decoupling via signal/slot data passing.
- No issue.

---

## 5. Header Include Convention (Qt -> STL -> Project)

### 5.1 Convention per CLAUDE.md 5.3

Required order:
1. Qt headers (`<QMainWindow>`, `<QObject>`, etc.)
2. STL headers (`<vector>`, `<functional>`, etc.)
3. Project headers (`"core/Constants.h"`, `"terminal/TerminalModel.h"`, etc.)

### 5.2 Audit results

| File | Convention | Verdict |
|------|-----------|---------|
| MainWindow.h | Qt (lines 4-18) -> project (lines 20-39), no STL needed | OK |
| MainWindow.cpp | Qt (lines 4-21) -> project (lines 1-3), no STL | OK (note: includes at top mixed but acceptable since .cpp can include its own header first) |
| ConnectionManager.h | Qt (lines 4-5) -> project (line 6) | OK |
| IConnection.h | Qt (lines 4-6) -> project (line 7) | OK |
| TerminalModel.h | Qt (lines 4-8) -> project (line 9) | OK |
| TerminalWidget.h | Qt (lines 4-5) -> project (lines 6-7) | OK |
| TerminalWidget.cpp | project (lines 1-2) -> Qt (lines 3-8) | ISSUE (H-001) |
| FrameParser.h | Qt (lines 4-5) -> project (lines 6-7) | OK |
| FrameParser.cpp | project (line 1-2) -> Qt (line 3) | ISSUE (H-001) |
| OtaManager.h | Qt (line 4) -> project (lines 5-9) | OK |
| DataExporter.h | Qt (lines 4-6) -> project (line 7) | OK |
| DataLogger.h | Qt (lines 4-7) -> project (none) | OK |
| ChartModel.h | Qt (lines 4-11) -> project (line 13) | OK |
| ChannelConfig.h | Qt (lines 4-10) -> project (none) | OK |
| BaseTransfer.h | Qt (lines 4-5) -> project (line 6) | OK |

### 5.3 Include convention findings

#### FINDING H-001: .cpp files include own header first, then Qt headers [Minor]

- Pattern seen in:
  - `src/terminal/TerminalWidget.cpp:1-8`: `#include "TerminalWidget.h"` then `#include "utils/HexConverter.h"` then Qt headers
  - `src/protocol/FrameParser.cpp:1-3`: `#include "FrameParser.h"` then `#include "utils/HexConverter.h"` then `<QDebug>`
  - `src/ota/OtaManager.cpp:1`: `#include "ota/OtaManager.h"` (own header first, which is standard C++ practice)
- The convention of including the own header first is standard C++ practice to verify self-containment. CLAUDE.md 5.3 specifies Qt->STL->Project for headers, but does not explicitly address .cpp file order.
- In practice, .cpp files should include their own header first (for self-containment check), then follow the Qt->STL->Project order for remaining includes.
- Impact: Very low. This is a style consistency issue, not a correctness issue.
- Recommendation: Document the .cpp include convention explicitly: own header first, then Qt, then STL, then other project headers. Current code is acceptable.

#### FINDING H-002: IConnection.h uses relative path "core/Constants.h" correctly [Pass]

- `src/connection/IConnection.h:7` uses `#include "core/Constants.h"` which is the correct relative-to-src path per CLAUDE.md 5.1.
- All project includes use relative-to-src paths consistently.

---

## 6. New Class Registration Check

### 6.1 New classes added since ARCH_REVIEW_004 (commits #16-#19)

| Class | File | Layer | Registered in CLAUDE.md 4.4 | Status |
|-------|------|-------|----------------------------|--------|
| ChannelConfig | `chart/ChannelConfig.h/cpp` | Data | No | ISSUE (N-001) |
| ChannelConfigSet | `chart/ChannelConfig.h/cpp` | Data | No | ISSUE (N-001) |
| ChartModel | `chart/ChartModel.h/cpp` | Data | No | ISSUE (N-001) |
| CachedLine (struct) | `terminal/TerminalWidget.h` | Presentation | N/A (internal struct) | OK |
| NavPanelMapping (struct) | `core/MainWindow.h` | Presentation | N/A (internal struct) | OK |

### 6.2 Registration findings

#### FINDING N-001: ChannelConfig, ChannelConfigSet, ChartModel not in CLAUDE.md 4.4 public component list [Major]

- `src/chart/ChannelConfig.h` defines `ChannelConfig` and `ChannelConfigSet` -- two classes that provide JSON serialization, compute logic, and default channel generation. These are reusable by any future chart-related feature.
- `src/chart/ChartModel.h` defines `ChartModel` -- a data-layer model that manages multi-channel data buffers, sliding windows, and downsampling.
- None of these are listed in CLAUDE.md section 4.4 (public component registry).
- Impact: Future developers may not know these components exist and could reimplement similar functionality.
- Recommendation: Add the following to CLAUDE.md 4.4:

```
| `ChannelConfig` | `chart/ChannelConfig.h/cpp` | Channel data source mapping and linear transform |
| `ChannelConfigSet` | `chart/ChannelConfig.h/cpp` | Multi-channel configuration management and batch compute |
| `ChartModel` | `chart/ChartModel.h/cpp` | Multi-channel waveform data model with sliding window |
```

#### FINDING N-002: TerminalTypes.h (TerminalLine, DataDirection) not in CLAUDE.md 4.4 [Minor]

- `src/terminal/TerminalTypes.h` defines `TerminalLine` struct and re-exports `DataDirection` enum.
- These are shared between TerminalModel, DataExporter, and TerminalWidget.
- Not registered in the public component list.
- Recommendation: Add to CLAUDE.md 4.4:

```
| `TerminalTypes` | `terminal/TerminalTypes.h` | Shared terminal data types (TerminalLine, DataDirection) |
```

---

## 7. Directory Structure Compliance

### 7.1 Actual vs Planned (CLAUDE.md section 10)

| Planned Directory | Exists | Contents Match Plan | Notes |
|-------------------|--------|-------------------|-------|
| `src/main.cpp` | Yes | OK | |
| `src/core/MainWindow.h/cpp` | Yes | OK | |
| `src/core/ConnectionManager.h/cpp` | Yes | OK | |
| `src/core/ConnectionFactory.h/cpp` | Yes | OK | |
| `src/core/ThemeManager.h/cpp` | Yes | OK | |
| `src/core/Constants.h` | Yes | OK | |
| `src/connection/IConnection.h` | Yes | OK | |
| `src/connection/SerialConnection.h/cpp` | Yes | OK | |
| `src/connection/TcpConnection.h/cpp` | Yes | OK | |
| `src/connection/UdpConnection.h/cpp` | Yes | OK | |
| `src/connection/RttConnection.h/cpp` | No | See below | Planned but not yet implemented |
| `src/terminal/TerminalWidget.h/cpp` | Yes | OK | |
| `src/terminal/TerminalModel.h/cpp` | Yes | OK | |
| `src/terminal/TerminalSearchBar.h/cpp` | Yes | OK | Not in original plan, acceptable addition |
| `src/terminal/TerminalTypes.h` | Yes | OK | Not in original plan, acceptable shared types |
| `src/serial/SerialConfigPanel.h/cpp` | Yes | OK | |
| `src/serial/QuickCommandBar.h/cpp` | Yes | OK | |
| `src/serial/TimedSender.h/cpp` | Yes | OK | |
| `src/serial/SendHistory.h/cpp` | Yes | OK | |
| `src/serial/DataStatistics.h/cpp` | Yes | OK | |
| `src/protocol/IProtocol.h` | No | DEVIATION | See D-001 |
| `src/protocol/FrameDefinition.h` | Yes | OK | Not in original plan (was FrameParser only) |
| `src/protocol/FrameVisualEditor.h/cpp` | Yes | OK | |
| `src/protocol/FrameParser.h/cpp` | Yes | OK | |
| `src/protocol/ProtocolView.h/cpp` | Yes | OK | |
| `src/protocol/IntelHexParser.h/cpp` | Yes | OK | |
| `src/chart/ChartWidget.h/cpp` | Yes | OK | |
| `src/chart/ChannelConfig.h/cpp` | Yes | OK | Not in original plan |
| `src/chart/ChartModel.h/cpp` | Yes | OK | Not in original plan |
| `src/ota/OtaManager.h/cpp` | Yes | OK | |
| `src/ota/OtaWidget.h/cpp` | Yes | OK | |
| `src/ota/OtaHistoryModel.h/cpp` | Yes | OK | Not in original plan |
| `src/ota/protocols/IProtocol.h` | No | See D-001 | |
| `src/ota/protocols/BaseTransfer.h/cpp` | Yes | OK | |
| `src/ota/protocols/XModemTransfer.h/cpp` | Yes | OK | |
| `src/ota/protocols/YModemTransfer.h/cpp` | Yes | OK | |
| `src/ota/protocols/ZModemTransfer.h/cpp` | Yes | OK | |
| `src/rtt/JLinkBridge.h/cpp` | No | Not yet | Planned directory exists but files are placeholders |
| `src/utils/RingBuffer.h` | Yes | OK | |
| `src/utils/DataLogger.h/cpp` | Yes | OK | |
| `src/utils/DataExporter.h/cpp` | Yes | OK | |
| `src/utils/SettingsManager.h/cpp` | Yes | OK | |
| `src/utils/HexConverter.h` | Yes | OK | |
| `src/utils/CRC.h` | Yes | OK | |

### 7.2 Directory structure findings

#### FINDING D-001: protocol/IProtocol.h listed in directory plan but does not exist [Minor]

- CLAUDE.md section 10 lists `src/protocol/IProtocol.h` as a strategy interface file. This file does not exist.
- The protocol strategy pattern is implemented through `FrameParser` with its `FrameDefinition`-driven configuration, which is a data-driven approach rather than a polymorphic interface.
- Similarly, `src/ota/protocols/IProtocol.h` is listed but does not exist. The OTA protocol strategy is implemented through `BaseTransfer` as the abstract base class.
- Impact: None on functionality. The directory plan in CLAUDE.md is outdated.
- Recommendation: Update CLAUDE.md section 10 to remove `protocol/IProtocol.h` and `ota/protocols/IProtocol.h`, or clarify that `BaseTransfer` serves as the OTA strategy interface and `FrameParser` serves as the protocol parsing interface.

#### FINDING D-002: Unplanned files in terminal/ and chart/ directories are acceptable additions [Pass]

- `TerminalSearchBar.h/cpp`, `TerminalTypes.h`, `ChannelConfig.h/cpp`, `ChartModel.h/cpp`, `OtaHistoryModel.h/cpp` were not in the original directory plan but are well-placed.
- Recommendation: Update CLAUDE.md section 10 to reflect current directory state.

---

## 8. Findings Summary by Category

### Critical (0 findings)

None. No blocking issues that prevent compilation, cause crashes, or violate core architectural constraints.

### Major (3 findings)

| ID | Category | Summary | Location |
|----|----------|---------|----------|
| R-001 | Reuse | RingBuffer<T> registered but unused; TerminalModel implements its own ring buffer inline | `terminal/TerminalModel.h:66-72`, `utils/RingBuffer.h` |
| C-001 | Coupling | MainWindow fan-out: 19 project headers included, 14+ classes directly managed | `core/MainWindow.h:20-39` |
| N-001 | Registration | ChannelConfig, ChannelConfigSet, ChartModel not in CLAUDE.md 4.4 public component list | `chart/ChannelConfig.h`, `chart/ChartModel.h` |

### Minor (5 findings)

| ID | Category | Summary | Location |
|----|----------|---------|----------|
| L-001 | Layers | MainWindow directly manages FrameParser lifecycle (Presentation -> Data orchestration) | `core/MainWindow.h:186` |
| L-002 | Layers | TerminalTypes.h in terminal/ instead of shared location; Infra layer reaches into terminal/ | `terminal/TerminalTypes.h` |
| C-002 | Coupling | OtaManager stores concrete XModem/YModem/ZModem pointers instead of QMap | `ota/OtaManager.h:33-36` |
| H-001 | Includes | .cpp include order: own header first, then project, then Qt -- convention not documented | Multiple .cpp files |
| D-001 | Directory | CLAUDE.md section 10 lists nonexistent protocol/IProtocol.h and ota/protocols/IProtocol.h | CLAUDE.md section 10 |

---

## 9. Specific Improvements from Commits #16-#19

The following improvements were verified as successfully implemented since the last review:

### 9.1 Commit #16 -- QSS theme migration + ChannelConfig/ChartModel

- QSS color variables centralized in theme files. C++ code uses ThemeColors:: constants from Constants.h for painter scenarios only.
- ChannelConfig with SourceMode (Direct/Combine) and linear transform is a well-designed data model.
- ChartModel with sliding window and downsampling correctly separates data management from rendering.

### 9.2 Commit #17 -- Transition animations + QSS button states

- `switchToPanel()` at MainWindow.cpp:351-449 implements two-phase fade: 200ms InCubic fade-out of old panel, then 250ms OutCubic fade-in of new panel.
- Breathing animation at MainWindow.cpp:922-966 uses QPropertyAnimation on QGraphicsOpacityEffect with 1500ms InOutSine loop.
- Panel switching guard via `m_panelSwitching` flag prevents animation overlap.
- Animation cleanup properly removes QGraphicsOpacityEffect after completion to restore normal paint performance.

### 9.3 Commit #18 -- TerminalModel ring buffer + NavPanelMapping

- TerminalModel replaced QVector with ring buffer approach (m_buffer + m_head + m_count). `appendLine()` at TerminalModel.cpp:137-152 is O(1) with no removeFirst().
- `lineAt()` at TerminalModel.cpp:64-69 provides O(1) indexed access via `physicalIndex()`.
- Thread safety maintained with QMutexLocker in all public methods.
- `setMaxLines()` at TerminalModel.cpp:99-125 now properly locks the mutex during buffer reallocation.
- NavPanelMapping data-driven pattern eliminates the if-else chain from ARCH_REVIEW_004 D-001.
- QT_TRANSLATE_NOOP used for i18n-safe mapping keys at MainWindow.cpp:335-342.

### 9.4 Commit #19 -- Code review fixes

- `sendAndRecord()` unified method at MainWindow.cpp:684-698 resolves ARCH_REVIEW_004 D-002.
- Completer leak fixed: `m_sendCompleterModel` is a persistent member (MainWindow.h:176) rather than per-call allocation.
- TerminalWidget rendering now uses CachedLine struct for O(1) paint access, with `formatToCache()` incremental update at TerminalWidget.cpp:133-145.
- objectName set on key widgets for QSS targeting.

---

## 10. Improvement Recommendations (Priority Order)

### P1 -- Should address in next 1-2 commits

| ID | Recommendation | Effort | Files |
|----|---------------|--------|-------|
| N-001 | Add ChannelConfig, ChannelConfigSet, ChartModel to CLAUDE.md 4.4 public component list | Small | CLAUDE.md |
| N-002 | Add TerminalTypes to CLAUDE.md 4.4 public component list | Small | CLAUDE.md |
| D-001 | Update CLAUDE.md section 10 directory plan to match actual codebase | Small | CLAUDE.md |
| R-001 | Evaluate whether TerminalModel should reuse RingBuffer<T> or document the inline ring buffer as an intentional design decision | Medium | TerminalModel.h/cpp or RingBuffer.h |

### P2 -- Should address in next 3-5 commits

| ID | Recommendation | Effort | Files |
|----|---------------|--------|-------|
| C-002 | Refactor OtaManager to use `QMap<QString, BaseTransfer*> m_protocols` for cleaner Strategy pattern | Small | OtaManager.h/cpp |
| L-002 | Move TerminalTypes.h to utils/ for proper cross-layer access | Small | TerminalTypes.h, includes in 3 files |
| H-001 | Document .cpp include convention: own header first, then Qt, then STL, then project | Small | CLAUDE.md |

### P3 -- Future consideration

| ID | Recommendation | Effort | Files |
|----|---------------|--------|-------|
| C-001 | Introduce PanelRegistry or Mediator to reduce MainWindow fan-out (when panel count exceeds 10) | Large | New class + MainWindow refactoring |
| L-001 | Extract FrameParser management to ProtocolEngine business layer | Large | New class + MainWindow refactoring |

---

## 11. Architectural Health Trend

| Review | Commit | Score | Key Theme |
|--------|--------|-------|-----------|
| ARCH_REVIEW_003 | #10 | 7.0 | Initial layer compliance established |
| ARCH_REVIEW_004 | #15 | 7.2 | Performance bottleneck identification (TerminalModel O(n)) |
| ARCH_REVIEW_005 | #20 | 8.8 | Ring buffer resolved, NavPanelMapping pattern, animation framework, code review fixes |

The architecture health score increased by 1.6 points since ARCH_REVIEW_004, driven primarily by:
1. Resolution of the O(n) TerminalModel bottleneck via ring buffer implementation
2. Elimination of the navigation if-else chain via data-driven NavPanelMapping
3. Unified sendAndRecord() method eliminating 3x code duplication
4. Proper CachedLine separation in TerminalWidget for clean paint performance
5. Well-implemented animation framework with correct cleanup

The codebase is in strong architectural shape at 21 points / 20 commits. The remaining items are registration/documentation gaps and minor coupling improvements, none of which block feature development.

# Architecture Management Review -- Commit #50

**Reviewer**: System Architect (system-architect)
**Date**: 2026-06-01
**Scope**: Full-codebase architecture audit per CLAUDE.md section 3.3
**Status**: PASS with findings

---

## 1. Layer Compliance

### 1.1 Dependency Direction

The project declares four layers with a strict top-down dependency rule:

```
Presentation --> Business --> Data --> Infrastructure
```

**Audit findings:**

| # | Source Layer | Source File | Violation | Severity |
|---|-------------|-------------|-----------|----------|
| L1 | Data (protocol) | `ProtocolView.h/cpp` | QWidget-derived class lives in `protocol/` directory, mixing presentation into the data/business layer module. `ProtocolView` is a QTableView widget, which is inherently presentation. | Medium |
| L2 | Data (protocol) | `FrameVisualEditor.h/cpp` | Same issue -- QWidget-derived, QTableWidget-heavy editor lives in `protocol/`. It is a pure presentation component. | Medium |
| L3 | Business (ota) | `OtaWidget.h/cpp` | QWidget-derived UI panel lives in `ota/` module alongside business logic (`OtaManager`). The OTA module conflates presentation and business. | Medium |
| L4 | Business (protocol) | `ProtocolBridgeManager.h` includes `JustFloatBridge.h` and `FireWaterBridge.h` directly. This is acceptable (same-layer include) but tightens coupling between the manager and concrete strategies. | Low |

**Reverse dependency check (critical):**

- Infrastructure layer (`connection/`, `utils/`): No includes of presentation-layer headers. **PASS**
- Data layer (`protocol/FrameParser`, `protocol/IProtocolBridge`): No includes of QWidget or any presentation header. **PASS**
- Business layer (`ota/OtaManager`, `core/ConnectionController`): No includes of QWidget-derived classes except through `QMainWindow` in `SessionManager` and `SettingsController`, which is acceptable since they operate on the main window as a QObject. **PASS**

**Verdict**: No critical reverse-dependency violations. The main concern is misplacement of presentation files in business/data module directories (findings L1-L3).

### 1.2 MainWindow Business Logic Scan

MainWindow.cpp (363 lines) + MainWindowSignalConnect.cpp (485 lines) = 848 combined lines.

The MainWindow header comment explicitly states: "No business logic -- all logic delegated to Controllers." Audit confirms this claim:

- `setupUI()` -- pure layout construction. **PASS**
- `setupStatusBar()` -- creates status bar labels, injects into TerminalController. **PASS**
- `handleConnectionState()` -- updates UI elements (labels, polish, panel state). This is a presentation concern, not business logic. **PASS**
- `closeEvent()` -- cleanup orchestration (stop animations, stop recording, save settings, close connections). This is lifecycle management, not business logic. **PASS**
- `connectSignals()` family -- pure signal routing. **PASS**
- `onBgSettingsToggled()` -- toggles a popup panel visibility. **PASS**

MainWindow does NOT contain: data parsing, protocol logic, connection management logic, or send/receive processing.

**Verdict**: MainWindow is cleanly separated. The "embedded main philosophy" (init objects -> assemble UI -> connect signals) is well followed.

### 1.3 TerminalWidget Business Logic Scan

TerminalWidget.cpp (467 lines) is a self-drawing widget. Audit confirms:

- No protocol parsing logic. **PASS**
- No connection management. **PASS**
- Rendering logic (`paintEvent`, `paintLine`, `formatToCache`) is presentation. **PASS**
- Search, selection, and direction filtering are delegated to dedicated manager classes. **PASS**
- Uses `HexConverter` from utils for display formatting -- correct data-layer reuse. **PASS**

**Verdict**: TerminalWidget is a clean presentation component.

---

## 2. Public Component Reuse

### 2.1 Component Reuse Matrix

| Component | Used By | Reuse Status |
|-----------|---------|-------------|
| `CRC` (`utils/CRC.h`) | `XModemTransfer`, `YModemTransfer`, `ZModemTransfer`, `FrameParser` | Correctly reused across OTA protocols and frame parsing |
| `HexConverter` (`utils/HexConverter.h`) | `TerminalWidget`, `ProtocolView`, `FrameVisualEditor`, `SendController` | Correctly reused for HEX display/parsing |
| `RingBuffer<T>` (`utils/RingBuffer.h`) | Not found in current includes via grep | Defined but potentially unused in current codebase. Verify usage. |
| `SettingsManager` (`utils/SettingsManager.h`) | `MainWindow`, `ThemeManager`, `QuickCommandBar`, `SessionManager`, `SettingsController` | Widely reused, singleton pattern correct |
| `ThemeManager` (`core/ThemeManager.h`) | `TerminalWidget`, `MainWindow`, `NavIndicatorWidget`, `BackgroundWidget`, `ProtocolView`, `OtaWidget` | Widely reused, singleton pattern correct |
| `IConnection` (`connection/IConnection.h`) | `ConnectionController`, `ConnectionManager`, `ConnectionFactory`, `BaseTransfer`, `SendController`, `OtaManager` | Core abstraction, correctly reused |
| `TerminalModel` (`terminal/TerminalModel.h`) | `TerminalWidget`, `TerminalController`, `SendController`, `MainWindow`, `PanelManager` | Data model correctly shared |
| `DataLogger` (`utils/DataLogger.h`) | `RecordingController`, `MainWindow`, `ConnectionController` | Reused for recording and bookmarks |
| `IProtocolBridge` (`protocol/IProtocolBridge.h`) | `JustFloatBridge`, `FireWaterBridge`, `ProtocolBridgeManager` | Strategy pattern interface correctly applied |

### 2.2 Duplicate Implementation Check

No duplicate implementations found for:
- CRC calculations
- HEX conversion
- Settings management
- Theme management
- Connection abstraction

**Verdict**: Component reuse is excellent. One concern: `RingBuffer` appears defined but may not be actively used -- should verify or document intent.

---

## 3. Design Pattern Compliance

### 3.1 Strategy Pattern -- IConnection, IProtocolBridge

**IConnection** (`connection/IConnection.h`):
- Pure virtual interface with `open()`, `close()`, `write()`, `configure()`
- Concrete implementations: `SerialConnection`, `TcpConnection`, `UdpConnection`
- `ConnectionFactory` creates instances based on `ConnectionType` enum
- ConnectionController depends only on `IConnection*`, never concrete types
- **COMPLIANT**

**IProtocolBridge** (`protocol/IProtocolBridge.h`):
- Pure virtual interface with `feed()`, `reset()`, `name()`
- Concrete implementations: `JustFloatBridge`, `FireWaterBridge`
- `ProtocolBridgeManager` routes data to active bridge via `IProtocolBridge*`
- `FrameParser` serves as a third strategy (non-bridge, direct data source)
- **COMPLIANT**

### 3.2 Observer Pattern -- Qt Signals/Slots

All inter-module communication uses Qt signal/slot with new-style connect syntax (function pointers):

- `IConnection::dataReceived` -> `ConnectionController::onDataReceived` -> `TerminalModel::appendReceived`
- `IConnection::stateChanged` -> `ConnectionController::onConnectionStateChanged`
- `ProtocolBridgeManager::frameParsed` -> `ProtocolView::onFrameParsed`, `ChartModel::onFrameParsed`
- `ThemeManager::themeChanged` -> all self-drawing widgets

No `SIGNAL()`/`SLOT()` macro usage found. **COMPLIANT**

### 3.3 Factory Pattern -- ConnectionFactory

`ConnectionFactory` (`core/ConnectionFactory.h/cpp`):
- Static `create()` method takes `ConnectionType` enum
- Returns `IConnection*` (or nullptr for unimplemented types)
- `ConnectionManager::createConnection()` delegates to factory
- Upper layers never see concrete connection classes
- **COMPLIANT**

### 3.4 State Pattern -- ConnectionState enum

`ConnectionState` enum (`core/Constants.h`):
- Four states: `Disconnected`, `Connecting`, `Connected`, `Error`
- `ConnectionController::onConnectionStateChanged()` switches behavior per state
- `MainWindow::handleConnectionState()` updates UI per state
- States drive visual behavior (breathing animation, button state, status text)
- **COMPLIANT** -- Note: This is enum-driven state behavior rather than full State pattern with state objects, which is appropriate for this complexity level.

### 3.5 Singleton Pattern -- SettingsManager, ThemeManager

**SettingsManager** (`utils/SettingsManager.h`):
- Private constructor, `static instance()` method
- Delete copy/move constructors
- Used across all modules that need persistent configuration
- **COMPLIANT**

**ThemeManager** (`core/ThemeManager.h`):
- Private constructor, `static instance()` method
- Delete copy/move constructors
- Manages QSS loading, semantic color palette, theme switching animation
- **COMPLIANT**

### 3.6 Template Method Pattern -- BaseTransfer -> XModem/YModem/ZModem

`BaseTransfer` (`ota/protocols/BaseTransfer.h`):
- Defines `start()` and `cancel()` as template methods
- Four pure virtual hooks: `onStartInit()`, `sendCancelBytes()`, `processReceivedData()`, `handleTimeout()`
- Base class manages: timeout timer, retry counting, state machine (Idle/Active/Done/Error), receive buffer
- Concrete classes: `XModemTransfer` (434 lines), `YModemTransfer` (493 lines), `ZModemTransfer` (498 lines)
- **COMPLIANT**

### 3.7 Adapter Pattern -- JLinkBridge (future)

The `rtt/` directory is empty. `JLinkBridge` is listed as a future component in the public component清单. `ConnectionType::Rtt` is already defined in `Constants.h`. The adapter pattern is correctly planned but not yet implemented.

- **PLANNED, NOT YET IMPLEMENTED** -- No violation.

### 3.8 Command Pattern -- QuickCommand data structure

`QuickCommand` struct (`serial/QuickCommandBar.h`):
- Fields: `name`, `data`, `isHex`
- `QuickCommandBar` stores list of commands, emits `commandTriggered(QByteArray)` on click
- `SendController::onQuickCommand()` receives and processes the data
- **COMPLIANT**

### Pattern Compliance Summary

| Pattern | Target | Status |
|---------|--------|--------|
| Strategy | IConnection | PASS |
| Strategy | IProtocolBridge | PASS |
| Observer | Qt signals/slots | PASS |
| Factory | ConnectionFactory | PASS |
| State | ConnectionState | PASS |
| Singleton | SettingsManager | PASS |
| Singleton | ThemeManager | PASS |
| Template Method | BaseTransfer -> X/Y/ZModem | PASS |
| Adapter | JLinkBridge | DEFERRED (rtt/ empty) |
| Command | QuickCommand | PASS |

---

## 4. Module Coupling

### 4.1 ConnectionController Dependencies

`ConnectionController` depends on:
- `ConnectionManager` (factory)
- `SendController` (downstream -- injected via setter)
- `OtaManager` (downstream -- injected via setter)
- `RecordingController` (downstream -- injected via setter)
- `PortWatcher` (owned)
- `IConnection` (interface)
- `DataLogger` (indirect, via RecordingController)

This is a mediator role -- it coordinates connection lifecycle across multiple downstream consumers. The dependency count (4 injected collaborators) is moderate and acceptable for a controller. All downstream dependencies are injected via setters, not constructors, which provides flexibility.

**Assessment**: Acceptable coupling for a mediator controller. No god-object tendency.

### 4.2 MainWindow Dependencies

MainWindow.h includes 18 project headers. The class holds raw pointers to:
- 7 controllers (Connection, Send, Navigation, Toolbar, Settings, Terminal, Recording)
- 2 managers (ConnectionManager, PanelManager)
- 1 model (TerminalModel)
- 2 utilities (DataExporter, DataLogger)
- 2 protocol objects (FrameParser, ProtocolBridgeManager)
- 1 OTA (OtaManager)
- 5 UI widgets (BackgroundWidget, BackgroundSettingsPopup, NavIndicatorWidget, TerminalLayoutManager, SessionManager)

MainWindow acts as the composition root and signal router. This is expected for a main window class that follows the "embedded main" philosophy. All pointers are initialized in the constructor and managed via QObject parent tree.

**Assessment**: MainWindow is the composition root. Its 18 dependencies are all justified by its role as the application assembler. MainWindow.cpp at 363 lines is well within the 500-line target. The signal routing (485 lines in a separate file) keeps the main file focused.

### 4.3 Cross-Module Coupling Map

```
MainWindow (composition root)
  |
  +-- ConnectionController ---> ConnectionManager ---> ConnectionFactory ---> IConnection
  |         |                                                          |        |        |
  |         +---> SendController <-----------------------------------+        |        |
  |         |                                                          TcpConn  UdpConn
  |         +---> OtaManager ---> BaseTransfer ---> IConnection
  |         |
  |         +---> RecordingController ---> DataLogger
  |
  +-- TerminalController ---> TerminalModel <--- TerminalWidget
  |         |
  |         +---> DataExporter
  |
  +-- ProtocolBridgeManager ---> IProtocolBridge (strategy)
  |         |                        |            |
  |         |                   JustFloat    FireWater
  |         +---> FrameParser
  |
  +-- SettingsController ---> SettingsManager (singleton)
  |         |
  |         +---> ThemeManager (singleton)
  |
  +-- NavigationController ---> PanelManager ---> (all panel widgets)
  |
  +-- SessionManager ---> SettingsController + SerialConfigPanel
```

**Coupling assessment**: The dependency graph is clean. No circular dependencies detected. All cross-module communication flows through Qt signals/slots or setter injection.

---

## 5. Include Path Convention

**Rule**: All project includes must use `"module/File.h"` format (relative to `src/`).

Audit of all `#include "..."` directives across the codebase:

- `#include "core/MainWindow.h"` -- PASS
- `#include "core/Constants.h"` -- PASS
- `#include "connection/IConnection.h"` -- PASS
- `#include "terminal/TerminalWidget.h"` -- PASS
- `#include "protocol/FrameParser.h"` -- PASS
- `#include "utils/HexConverter.h"` -- PASS
- `#include "ota/OtaManager.h"` -- PASS
- `#include "chart/ChartWidget.h"` -- PASS
- `#include "serial/SerialConfigPanel.h"` -- PASS

**Zero violations found.** All project includes consistently use the `module/File.h` convention. The CMakeLists.txt correctly sets `target_include_directories` to `${CMAKE_SOURCE_DIR}/src`.

---

## 6. Directory Structure

### 6.1 Current Layout vs. CLAUDE.md Specification

| Directory | Specified Purpose | Actual Contents | Verdict |
|-----------|------------------|-----------------|---------|
| `src/core/` | Application core: MainWindow, controllers, managers | MainWindow, 8 controllers, PanelManager, ThemeManager, factories, background widgets, toast | PASS (controllers and managers correctly placed) |
| `src/connection/` | Infrastructure: connection abstraction | IConnection, SerialConnection, TcpConnection, UdpConnection | PASS |
| `src/terminal/` | Presentation: terminal display | TerminalWidget, TerminalModel, search bars, selection, layout, direction filter | PASS |
| `src/serial/` | Presentation: serial port UI | SerialConfigPanel, QuickCommandBar, TimedSender, SendHistory, DataStatistics, BookmarkWidget, PortWatcher, SerialDriverDetector | PASS |
| `src/protocol/` | Business: protocol parsing | FrameParser, FrameDefinition, FrameVisualEditor, ProtocolView, IntelHexParser, IProtocolBridge, JustFloatBridge, FireWaterBridge, ProtocolBridgeManager | WARN -- ProtocolView and FrameVisualEditor are presentation widgets in a business module |
| `src/chart/` | Presentation: waveform display | ChartWidget, ChartModel, ChannelConfig, ChartColors | PASS |
| `src/ota/` | Business: OTA upgrade | OtaManager, OtaWidget, OtaHistoryModel, protocols/BaseTransfer, XModem, YModem, ZModem | WARN -- OtaWidget is presentation in a business module |
| `src/rtt/` | Infrastructure: RTT | Empty directory | PLANNED (no violation) |
| `src/utils/` | Infrastructure: public utilities | CRC, HexConverter, RingBuffer, SettingsManager, DataExporter, DataLogger, ByteFormat, DataBookmark | PASS |

### 6.2 Misplacement Findings

Three files are architecturally misplaced:

1. **`protocol/ProtocolView.h/cpp`** -- QWidget subclass (QTableView). Should be in `serial/` or a new `view/` directory.
2. **`protocol/FrameVisualEditor.h/cpp`** -- QWidget subclass (QTableWidget). Should be in `serial/` or `view/`.
3. **`ota/OtaWidget.h/cpp`** -- QWidget subclass. The OTA module should have its widget in `serial/` or `view/`, with only `OtaManager` and `protocols/` in `ota/`.

**Recommendation**: Create a `src/view/` directory for cross-cutting panel widgets, or relocate these to `serial/` where other panels (SerialConfigPanel, DataStatistics, BookmarkWidget) already live. This is a LOW PRIORITY finding -- the code compiles and works correctly, but the misplacement violates the CLAUDE.md layer principle that protocol files should be business-layer only.

---

## 7. File Size Audit

### 7.1 Files Approaching or Exceeding Limits

**Limit: 500 lines for .cpp, 200 lines for .h**

#### .cpp Files Near or Over Limit (500 lines)

| File | Lines | Status | Delta | Action |
|------|-------|--------|-------|--------|
| `ota/protocols/ZModemTransfer.cpp` | 498 | AT LIMIT | -2 | Monitor -- any new feature will breach |
| `utils/DataLogger.cpp` | 493 | AT LIMIT | -7 | Monitor |
| `protocol/FrameVisualEditor.cpp` | 493 | AT LIMIT | -7 | Monitor |
| `ota/protocols/YModemTransfer.cpp` | 493 | AT LIMIT | -7 | Monitor |
| `core/ConnectionController.cpp` | 489 | NEAR LIMIT | -11 | Monitor |
| `core/MainWindowSignalConnect.cpp` | 485 | NEAR LIMIT | -15 | Monitor -- signal routing tends to grow |
| `terminal/TerminalWidget.cpp` | 467 | NEAR LIMIT | -33 | Monitor |
| `ota/OtaWidget.cpp` | 455 | NEAR LIMIT | -45 | Safe for now |
| `protocol/ProtocolBridgeManager.cpp` | 439 | NEAR LIMIT | -61 | Safe for now |
| `serial/SerialConfigPanel.cpp` | 437 | NEAR LIMIT | -63 | Safe for now |
| `ota/protocols/XModemTransfer.cpp` | 434 | NEAR LIMIT | -66 | Safe for now |
| `utils/DataExporter.cpp` | 433 | NEAR LIMIT | -67 | Safe for now |
| `protocol/FrameParser.cpp` | 428 | NEAR LIMIT | -72 | Safe for now |

#### .h Files Near or Over Limit (200 lines)

| File | Lines | Status | Delta | Action |
|------|-------|--------|-------|--------|
| `core/SettingsController.h` | 203 | OVER LIMIT | +3 | **FLAG** -- needs review for extraction |
| `core/ToastWidget.h` | 196 | NEAR LIMIT | -4 | Monitor |
| `ota/AnimatedProgressBar.h` | 194 | NEAR LIMIT | -6 | Monitor |
| `connection/SerialConnection.h` | 193 | NEAR LIMIT | -7 | Monitor |
| `core/SendController.h` | 192 | NEAR LIMIT | -8 | Monitor |
| `utils/ByteFormat.h` | 191 | NEAR LIMIT | -9 | Monitor |
| `core/NavIndicatorWidget.h` | 190 | NEAR LIMIT | -10 | Monitor |
| `core/ThemeManager.h` | 189 | NEAR LIMIT | -11 | Monitor |

#### Files Well Within Limits

All remaining .cpp files are below 400 lines. All remaining .h files are below 190 lines. The remaining 60+ source files are comfortably within limits.

### 7.2 MainWindow Size Tracking

| Component | Lines | Trend |
|-----------|-------|-------|
| MainWindow.cpp | 363 | Healthy -- well under 500-line target |
| MainWindow.h | 144 | Healthy -- well under 200-line limit |
| MainWindowSignalConnect.cpp | 485 | Approaching limit -- 15 lines of headroom |

**Assessment**: MainWindow.cpp has been successfully reduced from its original 1092-line state to 363 lines (main) + 485 lines (signals) = 848 lines total, with signals properly extracted into a separate file. The main file is well below the 500-line target.

---

## 8. Summary of Findings

### Critical Issues (Must Fix)
None.

### Medium Issues (Should Fix in Next Iteration)

| ID | Finding | Impact | Recommendation |
|----|---------|--------|----------------|
| ARCH-001 | `ProtocolView` and `FrameVisualEditor` (QWidget subclasses) live in `protocol/` directory alongside business logic classes | Layer confusion -- presentation mixed with business | Move to `serial/` or new `view/` directory |
| ARCH-002 | `OtaWidget` (QWidget subclass) lives in `ota/` alongside `OtaManager` | Same layer mixing issue | Move widget to `serial/` or `view/`, keep OtaManager and protocols/ in ota/ |
| ARCH-003 | `ZModemTransfer.cpp` at 498 lines (2 lines from 500-line limit) | Any new feature will breach limit | Monitor closely; if more logic is needed, extract helper functions to a ZModemHelpers.cpp |

### Low Issues (Track)

| ID | Finding | Impact | Recommendation |
|----|---------|--------|----------------|
| ARCH-004 | `SettingsController.h` at 203 lines (3 lines over .h limit) | Minor breach of file size constraint | Review for possible method extraction |
| ARCH-005 | `RingBuffer<T>` defined but not visibly used in current includes | Dead code or used via indirect path | Verify usage; document if intended for future use |
| ARCH-006 | `MainWindowSignalConnect.cpp` at 485 lines | Approaching limit; signal routing tends to grow with new features | Consider splitting into per-domain signal files if it grows further |
| ARCH-007 | `rtt/` directory is empty | JLinkBridge not yet implemented | Planned per CLAUDE.md -- no action needed |
| ARCH-008 | `ProtocolBridgeManager` includes concrete bridge headers (`JustFloatBridge.h`, `FireWaterBridge.h`) directly | Tight coupling between manager and strategies | Could use registration pattern for extensibility, but acceptable for current scope |

### Positive Findings

1. **Layer discipline is strong** -- no reverse dependencies from infrastructure to upper layers.
2. **MainWindow is well-decomposed** -- 363 lines in main file, 485 in signal routing, business logic fully delegated to controllers.
3. **All 8 mandated design patterns are correctly implemented** (9th planned for future).
4. **Include path convention is 100% compliant** -- zero violations.
5. **Public component reuse is thorough** -- CRC, HexConverter, SettingsManager, ThemeManager, IConnection are used across all relevant modules with no duplication.
6. **Comment quality is excellent** -- Doxygen-style documentation on every class, method, signal, and member variable.
7. **File organization is clean** -- 12 modules, each with clear responsibility, matching the CLAUDE.md specification with only minor widget placement issues.

---

## 9. Architecture Health Score

| Dimension | Score (1-10) | Notes |
|-----------|-------------|-------|
| Layer compliance | 8 | Strong overall; 3 widget misplacements in business modules |
| Pattern compliance | 10 | All 8 patterns correctly applied |
| Coupling management | 9 | Clean dependency graph, no circular dependencies |
| Component reuse | 9 | Excellent reuse; RingBuffer potentially unused |
| File size discipline | 8 | 1 .h file over limit; 4 .cpp files at limit; MainWindow well-controlled |
| Include conventions | 10 | 100% compliant |
| Documentation quality | 10 | Comprehensive Chinese-language Doxygen comments throughout |
| Directory structure | 8 | Matches spec with 3 widget misplacements |

**Overall: 8.9/10**

The codebase is architecturally sound. The primary area for improvement is relocating the three presentation widgets (`ProtocolView`, `FrameVisualEditor`, `OtaWidget`) that currently live in business-layer modules. All design patterns are correctly implemented, dependency directions are clean, and the file size limits are respected with only marginal exceptions.

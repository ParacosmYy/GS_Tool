# FEATURE-001: JustFloat / FireWater Protocol Support

> **Priority**: P0 (Candidate Pool)
> **Proposed at**: Iteration #21
> **Target delivery**: Iterations #22-#24 (3 commits)
> **Status**: Pending Review

---

## 1. Motivation: Why Users Need This NOW

### The Problem

Embedded developers debugging sensor data face a chicken-and-egg problem with the current EmbedDebug wave chart:

1. To see waveforms, they must first define a complete `FrameDefinition` in the Visual Editor -- header bytes, field offsets, field types, checksum algorithms.
2. This requires knowing the exact binary layout of their firmware's debug output, which may not exist yet.
3. Many developers just want to throw 4 floats at the serial port and instantly see 4 curves. Today they cannot do this without a full protocol specification.

### What VOFA+ Proved

VOFA+ is the dominant waveform tool in the Chinese embedded community precisely because it solved this friction:

- **JustFloat**: Firmware sends raw little-endian floats continuously. The host detects channel count automatically from the first few bytes. Zero configuration. Users add `printf("%f,%f,%f\n", temp, volt, current)` or `memcpy(buf, &values, sizeof(values))` and immediately see curves.
- **FireWater**: Firmware sends CSV lines terminated by `\n`. The host splits on commas. Channels = column count. Works with any `printf`-style debug output.

These two protocols cover 80% of the "I just want to see my data" use case. They are the equivalent of "Hello World" for embedded waveform debugging.

### Why Not Other P0 Candidates

| Candidate | Why Deprioritized |
|-----------|-------------------|
| CAN/CANFD | Requires USB-CAN hardware driver integration. Extremely high complexity. Does not leverage existing chart infrastructure in a meaningful way -- it is a completely new connection type. |
| Advanced Wave Engine | FFT, scatter plots, histograms are powerful but require deep Qt Charts replacement or custom QPainter rendering. Multiple iterations. Current chart engine serves basic needs adequately. |

JustFloat/FireWater delivers the highest user-value-per-line-of-code because it unlocks the existing full chart pipeline with only a new data ingestion layer.

---

## 2. Technical Approach

### 2.1 Core Design: Protocol Bridge Pattern

The feature introduces a **protocol bridge** that sits between the raw byte stream and the existing `ChartModel` pipeline. Instead of requiring a user-defined `FrameDefinition`, these protocols auto-detect channel structure from the data itself.

```
Before (current):
  IConnection::dataReceived -> FrameParser::feed -> frameParsed(QVariantMap) -> ChartModel

After (with JustFloat/FireWater):
  IConnection::dataReceived -> ProtocolBridge::feed -> frameParsed(QVariantMap) -> ChartModel
                                   |
                                   +-- JustFloatBridge (auto-detect from tail marker)
                                   +-- FireWaterBridge (split CSV lines)
                                   +-- FrameParser (existing, user-defined protocol)
```

The bridge emits the **same** `frameParsed(const QVariantMap& fields, const QByteArray& rawFrame)` signal. `ChartModel` and `ChartWidget` receive data without knowing or caring which protocol produced it. This is the Adapter pattern applied to the data source.

### 2.2 Classes and Interfaces

#### New Files

| File | Layer | Responsibility |
|------|-------|----------------|
| `src/protocol/IProtocolBridge.h` | Data (interface) | Abstract interface for protocol bridges. Defines `feed(data)`, `reset()`, `setChannelCount()`, and signal `frameParsed`. |
| `src/protocol/JustFloatBridge.h/.cpp` | Data | Parses little-endian float byte streams. Detects channel count from tail marker `0x00 0x00 0x80 0x7F` (NaN in IEEE 754). Auto-generates field names `CH1`, `CH2`, ... |
| `src/protocol/FireWaterBridge.h/.cpp` | Data | Parses CSV/text lines terminated by `\n`. Auto-detects channel count from the first complete line. Supports configurable delimiter (comma default). |
| `src/protocol/ProtocolBridgeManager.h/.cpp` | Business | Manages active protocol bridge. Switches between FrameParser / JustFloat / FireWater modes. Owns the bridge instance. Emits unified `frameParsed` signal. |

#### Affected Files

| File | Change |
|------|--------|
| `src/core/MainWindow.h/.cpp` | Replace direct `FrameParser` usage with `ProtocolBridgeManager`. Add protocol mode selector in toolbar or chart panel. Connect bridge manager's signal to `ChartModel` and `ProtocolView`. |
| `src/core/Constants.h` | Add `enum class ChartProtocolMode { FrameParser, JustFloat, FireWater }`. |
| `src/utils/SettingsManager.h/.cpp` | Persist selected protocol mode and per-protocol settings (JustFloat channel count override, FireWater delimiter). |
| `resources/themes/dark_terminal.qss` | Add styles for protocol mode selector ComboBox in chart toolbar. |
| `resources/themes/modern_dark.qss` | Same. |
| `resources/themes/light.qss` | Same. |

### 2.3 Interface Definitions

```cpp
// protocol/IProtocolBridge.h
// Abstract interface for all protocol data source bridges.
// Each bridge converts raw bytes into structured field maps.
class IProtocolBridge : public QObject {
    Q_OBJECT
public:
    explicit IProtocolBridge(QObject* parent = nullptr);
    virtual ~IProtocolBridge() = default;

    // Feed raw byte data from connection
    virtual void feed(const QByteArray& data) = 0;

    // Reset internal state
    virtual void reset() = 0;

    // Get human-readable protocol name
    virtual QString protocolName() const = 0;

    // Get auto-detected or configured channel count (0 = not yet detected)
    virtual int channelCount() const = 0;

    // Get auto-generated channel names
    virtual QStringList channelNames() const = 0;

signals:
    // Emits parsed fields, same shape as FrameParser::frameParsed
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);
};
```

```cpp
// protocol/JustFloatBridge.h
// Parses VOFA+ JustFloat protocol: continuous little-endian float32 stream
// with optional NaN tail marker for frame synchronization.
//
// Frame format (N channels):
//   [float32 CH1] [float32 CH2] ... [float32 CHN] [0x00 0x00 0x80 0x7F]
//
// The NaN marker (0x7F800000 in little-endian) acts as frame delimiter.
// Channel count is auto-detected from the distance between two consecutive NaN markers.
class JustFloatBridge : public IProtocolBridge {
    Q_OBJECT
public:
    explicit JustFloatBridge(QObject* parent = nullptr);

    void feed(const QByteArray& data) override;
    void reset() override;
    QString protocolName() const override;
    int channelCount() const override;
    QStringList channelNames() const override;

    // Override auto-detection with a fixed channel count
    void setFixedChannelCount(int count);

private:
    QByteArray m_buffer;
    int m_channelCount = 0;        // 0 = auto-detect
    bool m_fixedChannelCount = false;
    int m_frameCount = 0;

    bool parseFrame();
    static bool isNanMarker(const char* data);
    static float readFloatLE(const char* data);
};
```

```cpp
// protocol/FireWaterBridge.h
// Parses VOFA+ FireWater protocol: CSV text lines with configurable delimiter.
//
// Frame format:
//   value1,value2,...,valueN\n
//
// Channel count is auto-detected from the first complete line.
// Values are parsed as double (supports integer, float, hex, octal).
class FireWaterBridge : public IProtocolBridge {
    Q_OBJECT
public:
    explicit FireWaterBridge(QObject* parent = nullptr);

    void feed(const QByteArray& data) override;
    void reset() override;
    QString protocolName() const override;
    int channelCount() const override;
    QStringList channelNames() const override;

    // Set CSV delimiter (default: comma)
    void setDelimiter(const QString& delim);

private:
    QByteArray m_buffer;
    QString m_delimiter = ",";
    int m_channelCount = 0;
    int m_frameCount = 0;

    bool parseLine(const QByteArray& line);
};
```

```cpp
// protocol/ProtocolBridgeManager.h
// Manages the active protocol bridge. Provides a single unified signal
// that ChartModel and ProtocolView connect to, regardless of which
// protocol is active.
class ProtocolBridgeManager : public QObject {
    Q_OBJECT
public:
    explicit ProtocolBridgeManager(QObject* parent = nullptr);

    // Switch protocol mode
    void setMode(ChartProtocolMode mode);
    ChartProtocolMode mode() const;

    // Feed data to the active bridge
    void feed(const QByteArray& data);

    // Get active bridge (for configuration queries)
    IProtocolBridge* activeBridge() const;

    // Reset active bridge state
    void reset();

signals:
    // Unified signal -- same as IProtocolBridge::frameParsed
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // Protocol mode changed
    void modeChanged(ChartProtocolMode newMode);

    // Active bridge detected channel count
    void channelCountChanged(int count);

private:
    void switchBridge(ChartProtocolMode mode);

    ChartProtocolMode m_mode = ChartProtocolMode::FrameParser;
    IProtocolBridge* m_activeBridge = nullptr;

    // Owned bridge instances (created once, switched between)
    FrameParser* m_frameParser;          // Existing parser, wrapped as bridge
    JustFloatBridge* m_justfloatBridge;
    FireWaterBridge* m_firewaterBridge;
};
```

### 2.4 Signal Flow

```
IConnection::dataReceived
    |
    v
MainWindow::onDataReceived(data)
    |
    +---> TerminalWidget::appendData(data)          // Terminal always shows raw
    +---> DataLogger::logData(data, RX)              // Recording always logs raw
    +---> ProtocolBridgeManager::feed(data)          // NEW: unified protocol entry
              |
              v
          [active bridge].feed(data)
              |
              v
          IProtocolBridge::frameParsed(fields, rawFrame)
              |
              v
          ProtocolBridgeManager::frameParsed(fields, rawFrame)  // forwarded
              |
              +---> ChartModel::onFrameParsed(fields, rawFrame)
              +---> ProtocolView::onFrameParsed(fields, rawFrame)
```

### 2.5 Channel Auto-Detection and Chart Integration

When JustFloat or FireWater detects a new channel count (first frame parsed), the bridge emits `frameParsed` with auto-generated field names:

- JustFloat: `{"CH1": 23.5, "CH2": 1.23, "CH3": -0.5}` for 3 channels
- FireWater: `{"col0": 100, "col1": 200}` for 2 columns (or uses header row if present)

`ChartWidget` already calls `ChannelConfigSet::generateDefaults(fields)` to auto-create channels. The `onChannelsChanged` signal triggers series creation in the chart. This works without modification -- the field names from the bridge are treated identically to field names from a user-defined `FrameDefinition`.

### 2.6 FrameParser Bridge Wrapper

The existing `FrameParser` already emits `frameParsed(QVariantMap, QByteArray)`. To unify it under `IProtocolBridge`, we wrap it:

```cpp
// FrameParser already has the correct signal signature.
// ProtocolBridgeManager connects FrameParser::frameParsed directly
// to its own frameParsed signal when in FrameParser mode.
// No need to modify FrameParser itself.
```

This means `FrameParser` does NOT inherit from `IProtocolBridge`. Instead, `ProtocolBridgeManager` holds a `FrameParser*` as a special case and connects its signal directly. This avoids modifying the existing, tested `FrameParser` code.

---

## 3. Architecture Impact

### 3.1 New Components

| Component | Layer | Reuses |
|-----------|-------|--------|
| `IProtocolBridge` | Data | -- |
| `JustFloatBridge` | Data | `RingBuffer` (optional), CRC (not needed) |
| `FireWaterBridge` | Data | -- |
| `ProtocolBridgeManager` | Business | `FrameParser`, `SettingsManager` |

### 3.2 Modified Components

| Component | Change Scope |
|-----------|-------------|
| `MainWindow` | Replace `m_frameParser` with `m_protocolBridgeManager`. Add mode selector. ~50 lines changed. |
| `Constants.h` | Add `enum class ChartProtocolMode`. ~5 lines. |
| `SettingsManager` | Add protocol mode persistence. ~20 lines. |
| QSS theme files | Add selector ComboBox styles. ~15 lines per theme. |

### 3.3 Layer Compliance

- `IProtocolBridge`, `JustFloatBridge`, `FireWaterBridge` are in the **Data layer** -- they transform raw bytes into structured data, no UI.
- `ProtocolBridgeManager` is in the **Business layer** -- it orchestrates which bridge is active, persists configuration, and emits unified signals.
- No changes to the **Presentation layer** beyond adding a ComboBox selector.
- No changes to the **Infrastructure layer**.
- Dependency direction: Presentation -> Business -> Data -> Infrastructure. Correct.

### 3.4 Design Patterns Used

| Pattern | Application |
|---------|-------------|
| Strategy | `IProtocolBridge` is the strategy interface. `ProtocolBridgeManager` switches strategies at runtime. |
| Adapter | `ProtocolBridgeManager` adapts `FrameParser` (which does not implement `IProtocolBridge`) into the unified signal flow via direct signal connection. |
| Observer | Existing Qt signal/slot chain unchanged. New bridges emit the same `frameParsed` signal. |

---

## 4. Estimated Effort

### 4.1 Lines of Code Estimate

| File | Estimated LOC |
|------|--------------|
| `IProtocolBridge.h` | ~35 |
| `JustFloatBridge.h` | ~40 |
| `JustFloatBridge.cpp` | ~120 |
| `FireWaterBridge.h` | ~40 |
| `FireWaterBridge.cpp` | ~100 |
| `ProtocolBridgeManager.h` | ~50 |
| `ProtocolBridgeManager.cpp` | ~100 |
| `Constants.h` changes | ~8 |
| `MainWindow.h/.cpp` changes | ~60 |
| `SettingsManager` changes | ~25 |
| QSS theme files (3x) | ~45 |
| **Total** | **~620 LOC** |

### 4.2 Iteration Plan

| Iteration | Scope | Deliverable |
|-----------|-------|-------------|
| #22 | Core bridges: `IProtocolBridge`, `JustFloatBridge`, `FireWaterBridge` | Raw byte parsing with auto-detection. Unit-testable. No UI. |
| #23 | Integration: `ProtocolBridgeManager`, `MainWindow` wiring, `Constants`, `SettingsManager` persistence | Protocol switching works end-to-end. Chart displays data from JustFloat/FireWater sources. |
| #24 | Polish: Protocol mode selector UI in chart toolbar, QSS styles, i18n strings, edge case hardening (buffer overflow protection, malformed data recovery) | Feature complete. User can select protocol mode and see waves instantly. |

### 4.3 Risk Assessment

| Risk | Mitigation |
|------|-----------|
| JustFloat auto-detection may misdetect channel count on noisy data | Provide manual channel count override via `setFixedChannelCount()`. Fall back if detection disagrees with override. |
| FireWater delimiter ambiguity (comma in data values) | Default to comma. Allow user to configure delimiter. Document that values should not contain the delimiter character. |
| Large buffers accumulate if no NaN marker arrives in JustFloat | Impose maximum buffer size (4096 bytes). Discard oldest data when exceeded. Emit warning. |
| Performance with very high sample rates (>100kHz) | Existing `ChartModel` refresh batching already handles this. `JustFloatBridge` does minimal allocation -- it reads floats directly from the buffer without copying. |

---

## 5. Reference Benchmark

### VOFA+ (vofa.plus)

VOFA+ is the primary reference for this feature. It implements both protocols with the following characteristics:

- **JustFloat**: Auto-detects channel count from NaN tail marker. Supports up to 32 channels. Rendering handles 100kHz+ sample rates via WebGL.
- **FireWater**: Auto-detects from first CSV line. Supports header row for named channels.

### What EmbedDebug Will Do Differently

1. **Better protocol coexistence**: EmbedDebug allows switching between FrameParser (custom binary protocol) and JustFloat/FireWater without restarting the connection. VOFA+ requires restarting the session.
2. **Channel configuration reuse**: EmbedDebug's existing `ChannelConfig` system (scale, offset, combine operations) applies to JustFloat/FireWater channels too. VOFA+ has no per-channel math.
3. **Unified recording**: DataLogger captures raw bytes regardless of protocol mode. Playback works with any protocol. VOFA+ has no recording/playback.

### What EmbedDebug Will NOT Do (Scope Limitation)

- No WebGL rendering (Qt Charts with OpenGL acceleration is sufficient for typical embedded sample rates under 50kHz).
- No real-time FFT (that belongs to the Advanced Wave Engine P0 candidate).
- No drag-and-drop channel reordering (future enhancement).

---

## 6. Acceptance Criteria

1. User can select "JustFloat" mode from a dropdown in the chart toolbar.
2. User can select "FireWater" mode from the same dropdown.
3. In JustFloat mode, sending `[float1][float2][NaN]` from firmware produces 2 named curves in the chart within 2 frames of data.
4. In FireWater mode, sending `1.23,4.56\n` from firmware produces 2 named curves in the chart.
5. Switching between FrameParser / JustFloat / FireWater modes does not require reconnecting the serial port.
6. Channel count is auto-detected in both JustFloat and FireWater modes.
7. JustFloat supports manual channel count override.
8. FireWater supports configurable delimiter.
9. All three QSS themes include styles for the protocol selector.
10. All user-visible strings use `tr()` for i18n.
11. Zero compilation errors. EmbedDebug.bat launches successfully.
12. Existing FrameParser functionality is completely unaffected when FrameParser mode is selected.

---

## 7. Open Questions for Reviewer

1. **Protocol selector placement**: Should the mode selector be in the chart toolbar (next to Pause/Clear) or in the main toolbar? Chart toolbar keeps protocol concerns localized. Main toolbar is more discoverable. Proposal: chart toolbar, with a tooltip explaining the modes.

2. **FrameParser bridge interface**: Should `FrameParser` be refactored to inherit from `IProtocolBridge`, or should `ProtocolBridgeManager` wrap it via signal forwarding? Proposal: wrap via signal forwarding to avoid modifying existing, tested code. Reviewer to decide if the inconsistency is acceptable.

3. **Channel naming for FireWater**: When the first line looks like `temp,voltage,current\n` (non-numeric), should we treat it as a header row and use those names? VOFA+ does this. Proposal: yes, detect header row automatically.

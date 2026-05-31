# FEATURE REVIEW 001: JustFloat / FireWater Protocol Support

> **Reviewer**: New Feature Reviewer (#11), System Architect
> **Feature Proposal**: `docs/prd/FEATURE_001.md`
> **Review Date**: Iteration #21
> **Verdict**: **APPROVE**

---

## 1. Executive Summary

The proposal to add VOFA+-compatible JustFloat and FireWater protocol support is approved. It delivers the highest user-value-per-LOC of any P0 candidate by unlocking the existing ChartModel pipeline with only a new data ingestion layer. The architecture is clean, the Protocol Bridge pattern is a sound application of the Strategy pattern, and the integration surface with existing code is minimal and well-defined.

---

## 2. Evaluation by Dimension

### 2.A User Value: HIGH

**Real pain point, validated by market leader.**

VOFA+ dominates the Chinese embedded waveform debugging ecosystem precisely because it eliminated the configuration friction of defining a binary protocol before seeing data. The proposal correctly identifies the chicken-and-egg problem: developers cannot use EmbedDebug's chart until they have defined a complete `FrameDefinition`, but many debug sessions start with "I just want to see my 4 sensor values as curves."

JustFloat and FireWater are the "Hello World" of embedded waveform debugging. Any developer who has ever written `printf("%f,%f\n", a, b)` and wished they could see those values plotted has this need. This is not a niche feature -- it is table stakes for competing with VOFA+.

The competitive analysis is honest. The proposal acknowledges what VOFA+ does better (WebGL rendering, high sample rates) and correctly scopes out what EmbedDebug will not attempt. The differentiators (protocol coexistence without reconnection, ChannelConfig math, unified recording/playback) are real advantages.

**Assessment**: Solves a high-frequency pain point for the target user base. Approving this feature materially expands the tool's addressable use cases.

### 2.B Architecture Impact: LOW (Positive)

**Does not break existing layering. Adds exactly the right abstraction at the right layer.**

The proposal introduces a clean Strategy pattern at the data ingestion boundary. The key architectural insight is that `IProtocolBridge` emits the same `frameParsed(QVariantMap, QByteArray)` signal that `FrameParser` already emits. This means `ChartModel`, `ChartWidget`, and `ProtocolView` receive data without knowing or caring which protocol produced it. Downstream components are untouched.

New class count:

| Class | Layer | Purpose |
|-------|-------|---------|
| `IProtocolBridge` | Data | Strategy interface (abstract) |
| `JustFloatBridge` | Data | JustFloat byte stream parser |
| `FireWaterBridge` | Data | CSV line parser |
| `ProtocolBridgeManager` | Business | Runtime strategy switching + signal unification |

Four new classes. All in the correct layers. No cross-layer violations in the design.

The decision to wrap `FrameParser` via signal forwarding rather than forcing it to inherit from `IProtocolBridge` is the right call. `FrameParser` is already tested and working. It has additional concerns (state machine, header/length/checksum/footer matching, error signals) that do not map cleanly onto the simpler `IProtocolBridge` interface. Wrapping preserves backward compatibility and avoids modifying existing, stable code.

Layer compliance check:

- `IProtocolBridge`, `JustFloatBridge`, `FireWaterBridge` are pure Data layer -- they transform raw bytes into structured field maps. No UI dependency.
- `ProtocolBridgeManager` is Business layer -- it orchestrates bridge selection, persists configuration, emits unified signals.
- No Presentation layer changes beyond adding a ComboBox to the chart toolbar.
- No Infrastructure layer changes at all.
- Dependency direction: Presentation -> Business -> Data -> Infrastructure. Correct.

**Assessment**: The Protocol Bridge pattern strengthens the architecture by formalizing the data source abstraction that was previously implicit. No layering violations introduced.

### 2.C Development Cost: MODERATE-LOW

**~620 LOC across 3 iterations. Heavy reuse of existing infrastructure.**

The LOC estimate of ~620 lines is realistic for the scope. Breaking it down:

- Core bridge logic (IProtocolBridge + JustFloatBridge + FireWaterBridge): ~335 LOC. These are self-contained byte-parsing classes with no UI dependency. Unit-testable in isolation.
- Integration layer (ProtocolBridgeManager): ~150 LOC. Strategy switching, signal forwarding, mode persistence.
- UI integration (MainWindow wiring + Constants + SettingsManager + QSS): ~135 LOC. Minimal surface area.

Iteration plan assessment:

| Iteration | Scope | Risk |
|-----------|-------|------|
| #22 Core bridges | ~335 LOC, no UI, fully unit-testable | LOW -- pure byte parsing |
| #23 Integration | ~150 LOC, MainWindow wiring | MEDIUM -- touch existing signal connections |
| #24 Polish | ~135 LOC, UI selector + themes + edge cases | LOW -- additive UI work |

The incremental delivery plan is sound. JustFloat can be implemented and tested independently of FireWater. The `IProtocolBridge` interface is defined in iteration #22, and both concrete bridges can be tested against it before any UI work begins.

Reuse of existing infrastructure:

- `ChartModel::onFrameParsed()` -- unchanged. The bridges emit the same signal shape.
- `ChannelConfigSet::generateDefaults()` -- the PRD states it will be used for auto-channel creation from detected field names. However, `generateDefaults()` currently takes `QVector<FieldDef>`, not `QStringList`. The JustFloat/FireWater bridges produce `QVariantMap` with auto-generated names like `CH1`, `CH2` but no `FieldDef` structs. This is a minor integration gap that needs resolution during implementation (likely a new overload or a conversion helper). Not a blocker, but worth noting.
- `SettingsManager` -- reused for protocol mode persistence.
- No new external dependencies. Everything uses Qt Core and Qt Charts, which are already linked.

**Assessment**: 620 LOC is modest for the user value delivered. The three-iteration plan is achievable with low integration risk.

### 2.D Reuse Potential: HIGH

**The Protocol Bridge pattern is directly reusable for future protocol additions.**

The `IProtocolBridge` interface is a general-purpose data source abstraction. Once established, adding new protocols requires only a new concrete bridge class that implements `feed()`, `reset()`, and emits `frameParsed()`. The manager, signal chain, and UI infrastructure are reused without modification.

Immediate reuse candidates from the candidate pool:

| Future Protocol | Reuse Path |
|----------------|------------|
| Modbus RTU/ASCII (P2) | New `ModbusRtuBridge` implementing `IProtocolBridge`. Same signal chain. |
| MQTT gateway (P1) | New `MqttBridge` wrapping a Qt MQTT client into `IProtocolBridge`. |
| DBC database parsing (for CAN) | If CAN support is added later, CAN frames can flow through `IProtocolBridge` into the same chart pipeline. |

The pattern also establishes a precedent for "data source adapters" that the Advanced Wave Engine (P0) can leverage. When FFT or scatter plots are added, they will need the same `frameParsed` data -- the bridge pattern ensures they receive it from any protocol.

**Assessment**: The Protocol Bridge pattern has high strategic value beyond this single feature. It is the correct long-term abstraction for the data ingestion boundary.

---

## 3. Specific Technical Considerations

### 3.1 No External Dependencies Beyond Qt

Confirmed. JustFloat parsing requires only `QByteArray`, `memcpy` for float reading, and comparison for NaN marker detection. FireWater parsing requires `QByteArray`, `QString::split()`, and `QString::toDouble()`. Both are implementable with Qt Core alone. No third-party libraries needed.

### 3.2 Incremental Implementation is Feasible

The proposal correctly sequences JustFloat before FireWater. JustFloat is the simpler protocol (fixed-size binary frames with a single marker byte) and proves the `IProtocolBridge` interface design. FireWater (text-based CSV with variable-length lines) is a natural second step that reuses the same interface.

Each bridge is independently testable. The `ProtocolBridgeManager` can be wired with only `JustFloatBridge` in iteration #22, and `FireWaterBridge` can be added in the same iteration or deferred to #23 with zero architectural impact.

### 3.3 No Conflict with Existing FrameParser Workflow

The current data flow in `MainWindow::connectSignals()` (lines 343-354 of MainWindow.cpp):

```
FrameParser::frameParsed -> ProtocolView::onFrameParsed
FrameParser::frameParsed -> ChartModel::onFrameParsed
```

The proposal replaces `FrameParser` with `ProtocolBridgeManager` as the data source, but `FrameParser` remains as the "default strategy" wrapped inside the manager. When the user selects "FrameParser" mode, the existing signal connections are functionally identical -- the manager forwards `FrameParser::frameParsed` through its own `frameParsed` signal.

The only change to `MainWindow` is replacing direct `m_frameParser` usage with `m_protocolBridgeManager` and adding ~50 lines for the mode selector. The `FrameParser` class itself is completely untouched. The `FrameVisualEditor::definitionChanged` connection continues to work because `ProtocolBridgeManager` holds the `FrameParser` instance and delegates `setDefinition()` to it.

**One integration detail to verify during implementation**: The `FrameVisualEditor` currently connects to `MainWindow` to call `m_frameParser->setDefinition(def)` and `m_chartWidget->configureFromFrameDefinition(def)`. When `m_frameParser` is replaced by `m_protocolBridgeManager`, the manager must expose a `setFrameDefinition()` method that delegates to its internal `FrameParser`. The PRD does not explicitly call this out, but it is a minor addition (~5 lines).

### 3.4 ChannelConfigSet Integration Gap

As noted in Section 2.C, `ChannelConfigSet::generateDefaults()` takes `QVector<FieldDef>`, but JustFloat/FireWater bridges do not have `FieldDef` structs -- they auto-generate field names as strings. The implementation will need one of:

(a) A new overload `ChannelConfigSet::generateDefaults(const QStringList& fieldNames)` that creates Direct-mode configs with auto-assigned colors.
(b) Conversion of auto-detected names into temporary `FieldDef` structs with type `Float32`.

Option (a) is cleaner. This should be tracked as a sub-task in iteration #22 or #23.

### 3.5 Buffer Overflow Protection

The proposal mentions a 4096-byte maximum buffer for JustFloat. This is adequate. A typical JustFloat frame with 4 channels is 4*4 + 4 = 20 bytes. At 4096 bytes, the buffer can hold ~200 frames of 4-channel data before overflowing. The discard-oldest strategy with a warning emission is the correct approach.

---

## 4. Answers to Open Questions

**Q1: Protocol selector placement**

Agree with the proposal: chart toolbar, next to Pause/Clear. Protocol mode is a chart concern. Placing it in the main toolbar would pollute the main toolbar with chart-specific controls. Discoverability can be addressed with a tooltip and a status bar indicator.

**Q2: FrameParser bridge interface**

Agree with the proposal: wrap via signal forwarding, do not refactor FrameParser to inherit from IProtocolBridge. The inconsistency is acceptable because ProtocolBridgeManager encapsulates it. FrameParser has additional concerns (state machine, error signals) that make the IProtocolBridge interface a poor fit. Forcing inheritance would either bloat the interface with FrameParser-specific methods or require FrameParser to suppress functionality. Wrapping is the correct Adapter pattern application.

**Q3: FireWater header row detection**

Agree with the proposal: detect header row automatically. If the first line contains non-numeric tokens, treat them as column names. This is what VOFA+ does, and it is the expected behavior. Implementation note: use `QString::toDouble(ok)` with the `ok` output parameter to distinguish "1.23" (value) from "temperature" (header).

---

## 5. Risk Assessment

| Risk | Severity | Mitigation |
|------|----------|------------|
| JustFloat auto-detection misdetect on noisy startup data | LOW | `setFixedChannelCount()` override exists. Document that first 2-3 frames may be discarded during auto-detection. |
| FireWater delimiter in data values | LOW | Configurable delimiter + documentation. This is a fundamental limitation of CSV protocols, not a bug. |
| Performance at high sample rates (>100kHz) | LOW | ChartModel refresh batching already handles this. JustFloatBridge does zero-allocation float reads. |
| Integration regression in FrameParser mode | MEDIUM | Existing FrameParser signal chain must be verified to work identically through ProtocolBridgeManager. Add explicit test. |
| ChannelConfigSet interface mismatch | LOW | New overload needed. Minor, but should not be forgotten. |

No risk is HIGH severity. The highest risk (FrameParser regression) is mitigable by careful signal-forwarding implementation and a verification test.

---

## 6. Verdict: APPROVE

**Rationale**:

1. **User value is high and well-validated.** VOFA+ proved the market demand. JustFloat/FireWater are table stakes for embedded waveform tools.
2. **Architecture impact is positive.** The Protocol Bridge pattern formalizes a data source abstraction that was previously implicit. It strengthens the codebase.
3. **Development cost is modest.** ~620 LOC across 3 iterations, with heavy reuse of existing ChartModel and ChannelConfig infrastructure.
4. **Reuse potential is high.** The IProtocolBridge interface is directly applicable to Modbus, MQTT, and future protocol additions.
5. **No external dependencies.** Pure Qt Core implementation.
6. **Incremental delivery is natural.** JustFloat can ship first, FireWater second, with zero architectural rework between them.
7. **No conflict with FrameParser.** The wrapping approach preserves full backward compatibility.

**Conditions for implementation**:

1. Resolve the `ChannelConfigSet::generateDefaults()` overload gap (new `QStringList`-based overload) during iteration #22 or #23.
2. Ensure `ProtocolBridgeManager` exposes a `setFrameDefinition()` delegation method so `FrameVisualEditor` continues to work.
3. Verify FrameParser mode produces identical behavior to the current direct-connection approach.
4. Add the `ChartProtocolMode` enum to `Constants.h` as proposed.

**Recommended next step**: Begin iteration #22 with `IProtocolBridge` interface + `JustFloatBridge` implementation.

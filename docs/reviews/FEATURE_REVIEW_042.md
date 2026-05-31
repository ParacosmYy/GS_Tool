# FEATURE REVIEW 042: Iteration 42 New Feature Evaluation

> **Reviewer**: New Feature Reviewer (#11), System Architect
> **Review Date**: Iteration #42
> **Status**: No FEATURE_042_*.md proposals found. Review against candidate pool (CLAUDE.md 3.4.3).

---

## 0. Process Note

No `docs/prd/FEATURE_042_*.md` files exist. The feature designer (role #10) has
not submitted proposals for this cycle. Per section 3.4.2, this review evaluates
the top unimplemented candidates from the priority pool. JustFloat/FireWater was
approved in REVIEW_001 and delivered in commits #23-#24, so it is excluded.

**Candidates**: A) CAN/CANFD (P0), B) Advanced Wave Engine (P0), C) Data Recording/Playback Enhancement (P1)

---

## 1. Proposal A: CAN/CANFD Communication Support

**A. User Value: HIGH.** CAN is the backbone of automotive, industrial, and
robotics systems. Developers currently switch between EmbedDebug, CANoe, and
BUSMASTER. A modern Qt-based CAN monitor fills a real gap -- BUSMASTER has a
dated UI, CANoe is expensive. Target audience: STM32 CAN, ESP32 TWAI, MCP2515.

**B. Architecture Impact: VERY HIGH (concern).** Requires a new connection type
(`CanConnection` via `IConnection`), plus CanFrame model, DBC parser, CanFilterModel,
CanMessageView, CanTransmitPanel -- estimated 8-12 new classes. The DBC parser
alone rivals the entire OTA module in complexity. This is module-scale, not
feature-scale. `IConnection` strategy pattern supports it, but no CAN-aware
presentation views exist today.

**C. Development Cost: VERY HIGH.** ~2500-3500 LOC across 6-8 iterations.
Violates the 3-iteration delivery window (section 3.4.2).

**D. Reuse Potential: MODERATE.** `IConnection`, `RingBuffer`, `DataLogger`,
`SettingsManager` reused. But DBC parser, CanMessageView, signal encoding are
CAN-specific with no cross-module reuse.

**Decision: DEFER.** Strong long-term value but scope is too large. Recommend
phasing: (1) raw CAN TX/RX without DBC, (2) DBC support, (3) signal-level view.

---

## 2. Proposal B: Advanced Wave Engine (FFT/Histogram/Scatter)

**A. User Value: HIGH.** FFT, scatter plots, histograms, and multi-Y-axis would
elevate EmbedDebug from "basic line plot" to "engineering analysis tool." CLAUDE.md
explicitly targets surpassing VOFA+'s waveform experience. Motor control, audio,
vibration, and power electronics developers all need frequency-domain visualization.
Today they export to MATLAB/Python.

**B. Architecture Impact: HIGH.** Current `ChartWidget` uses Qt Charts, which has
limited FFT/spectrogram/scatter support. Two paths: (A) extend Qt Charts (lower
cost, may hit performance wall) or (B) custom QPainter engine like TerminalWidget
(higher cost, full control, aligns with project philosophy). FFT needs a library
dependency (KISS FFT) or hand-rolled radix-2. Estimated 6-8 new classes
(FftCalculator, HistogramModel, AdvancedChartWidget, ChartToolbar, CursorMeasurement).

**C. Development Cost: VERY HIGH.** ~2000-3000 LOC across 5-7 iterations.
The custom rendering engine is the dominant cost. Exceeds 3-iteration window.

**D. Reuse Potential: HIGH.** `FftCalculator` reusable for any frequency analysis.
Custom chart engine becomes the permanent rendering foundation. `CursorMeasurement`
reusable for oscilloscope and CAN timing views. `HistogramModel` reusable for
data quality dashboards.

**Decision: DEFER.** Excellent reuse but scope exceeds the delivery window.
Recommend phasing: (1) FFT + spectrum via Qt Charts, (2) custom QPainter engine,
(3) histogram/scatter/cursor.

---

## 3. Proposal C: Data Recording/Playback Enhancement

**A. User Value: MODERATE-HIGH.** Enhances existing `DataLogger` (commit #11,
minimally improved since) with: multi-stream timestamp alignment (serial + RTT
synchronized replay), variable-speed playback, bookmark markers, time-range export.
Multi-stream replay is critical for STM32 developers using serial and RTT
simultaneously -- currently they lose temporal correlation. Bookmarks and
time-range export benefit all DataLogger users.

**B. Architecture Impact: MODERATE.** Primarily enhances existing components
rather than adding new abstractions:

- `DataLogger`: add multi-stream index, bookmark storage, time-range query
- `RecordingController`: add speed control UI, bookmark button
- `DataExporter`: add time-range export with start/end markers
- `DataPlaybackWidget` (new): playback control bar with speed slider, bookmark list

New classes: 1-2. Multi-stream alignment requires a shared timeline index mapping
(stream_id, byte_offset) to wall-clock time, plus synchronized replay dispatching.
Stays within existing Data/Business layers. No layer violations. `RecordingController`
naturally owns playback speed and bookmark orchestration.

**C. Development Cost: MODERATE.** ~600-800 LOC across 2-3 iterations.
Lowest-cost candidate among remaining P0/P1 items. Fits the 3-iteration window.

**D. Reuse Potential: MODERATE.** Multi-stream timeline index reusable when CAN
or MQTT data sources are added. Bookmark model reusable for OTA history and error
event marking. DataPlaybackWidget reusable for any time-based replay.

**Decision: APPROVE.** Fits the 3-iteration window. Enhances existing
infrastructure rather than introducing new abstractions. Multi-stream replay is
a differentiated capability. DataLogger investment prevents it from becoming a
dead component. Bookmarks and time-range export benefit all users.

---

## 4. Summary

| Proposal | User Value | Arch Impact | Dev Cost | Reuse | Decision |
|----------|-----------|-------------|----------|-------|----------|
| A: CAN/CANFD | HIGH | VERY HIGH | VERY HIGH | MODERATE | **DEFER** |
| B: Adv. Wave Engine | HIGH | HIGH | VERY HIGH | HIGH | **DEFER** |
| C: Data Rec/Playback | MOD-HIGH | MODERATE | MODERATE | MODERATE | **APPROVE** |

**Approved**: C -- Data Recording/Playback Enhancement
**Deferred** (auto re-enter next cycle): A and B per section 3.4.2

---

## 5. Implementation Conditions

1. Multi-stream index must use wall-clock timestamps (ms precision) recorded at
   ingestion time. Do not reconstruct timing from byte offsets alone.
2. Bookmark persistence: use `SettingsManager` or companion JSON file alongside
   EDL recordings. Bookmarks are metadata, not inline data.
3. `DataPlaybackWidget` must follow section 6 UI standards: consistent button
   styles (6.6), expand/collapse animations for bookmark panel (6.5), all three
   QSS themes (6.9).
4. Time-range export must reuse `DataExporter` streaming architecture. Do not
   load entire recordings into memory.
5. Variable-speed playback: support 0.25x, 0.5x, 1x, 2x, 4x, 8x via QTimer
   interval scaling, not data dropping.
6. All new files must be registered in section 4.4 and the directory structure.

---

## 6. Recommendations for Next Cycle (Iteration 45)

1. **CAN/CANFD**: Propose Phase 1 only (raw CAN TX/RX, no DBC) -- could fit
   3 iterations with tight scope.
2. **Advanced Wave Engine**: Propose FFT spectrum display only, hosted in Qt
   Charts temporarily. Full QPainter rewrite as separate proposal.
3. **P2 palette cleanser**: After two high-complexity deferrals, consider
   proposing a low-cost P2 (virtual serial, theme editor) alongside a P0/P1
   to guarantee one approvable feature per cycle.

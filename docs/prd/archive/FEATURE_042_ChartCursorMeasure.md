# FEATURE-042A: Chart Cursor Measurement & Zoom/Pan

## Motivation

The current ChartWidget renders real-time line series but lacks the most fundamental
data inspection tools: users cannot read exact values at a given point, cannot zoom
into a region of interest, and cannot pan back to view historical data that has
scrolled out of the sliding window. Every reference product (VOFA+, Serial Studio,
PulseView) provides cursor measurement and interactive zoom. Without these, the chart
is a passive display rather than an analytical instrument. This is the single highest-
impact gap in the current feature set.

## User Story

As an embedded firmware developer debugging sensor data through JustFloat/FireWater
streams, I want to drag vertical cursors onto the waveform to read exact (X, Y)
values and the delta between two cursors, and I want to scroll-wheel zoom and
middle-click drag to pan, so that I can diagnose anomalies and verify signal
characteristics without exporting data to an external tool.

## Technical Approach

### Modules Affected
- `chart/ChartWidget.h/cpp` -- add cursor overlay, mouse event handlers, zoom/pan logic
- `chart/ChartModel.h/cpp` -- add data query API (valueAt, range query)
- New file: `chart/ChartCursorOverlay.h/cpp` -- lightweight QWidget overlay for cursor rendering
- QSS themes -- cursor color tokens (--cursor-color, --cursor-delta)

### Implementation Strategy
1. **ChartModel data query**: Add `valueAt(channel, xIndex)` and `dataRange(channel, xStart, xEnd)` methods that return data from the internal sliding window buffer.
2. **Cursor overlay**: A transparent QWidget stacked above QChartView. Renders two vertical dashed lines at cursor positions, value labels at each cursor intersection, and a delta readout between them. Uses QPainter for crisp rendering.
3. **Mouse interaction**: ChartWidget intercepts mouse events on the QChartView:
   - Left-click drag on empty area: pan (adjust axis range)
   - Scroll wheel: zoom Y axis (or X+Y with Ctrl)
   - Right-click: place cursor A / cursor B (context menu toggle)
   - Cursor drag: move cursor, real-time value readout update
4. **Zoom state**: When user zooms/pans, set `m_autoYRange = false` and `m_autoScroll = false`. "Reset View" button restores auto mode.
5. **Data retention for pan**: ChartModel already stores the sliding window. Increase default window to 2000 points to give reasonable pan range.

### Complexity Estimate
- Medium. ~400 lines new code across 2 new files + modifications to ChartWidget/ChartModel.
- No external dependencies beyond Qt Charts already in use.

## Architecture Impact

### New Classes
| Class | Layer | Responsibility |
|-------|-------|---------------|
| `ChartCursorOverlay` | Presentation | Transparent overlay widget, renders cursors and value labels |
| `ChartInteractionController` | Business | Manages cursor positions, zoom/pan state, coordinates ChartModel queries |

### Pattern Changes
- Introduces a lightweight interaction controller pattern: ChartWidget delegates mouse
  events to ChartInteractionController, which updates cursor/zoom state and requests
  data from ChartModel. Keeps ChartWidget focused on lifecycle and layout.
- No changes to existing design patterns or public component interfaces.

### Existing Component Reuse
- `ChartModel` -- extend with query methods (no breaking changes)
- `ThemeManager` -- cursor colors from semantic palette
- `ChartColors` -- existing theme-aware palette

## Development Cost

| Item | Estimate |
|------|----------|
| ChartModel query API | ~60 lines |
| ChartCursorOverlay | ~150 lines |
| ChartInteractionController | ~120 lines |
| ChartWidget mouse integration | ~80 lines |
| QSS theme additions | ~20 lines (3 themes) |
| **Total** | **~430 lines** |
| **Iterations** | **2 iterations** |

## Priority Justification

P0 from the candidate pool (Advanced Waveform Engine). This proposal extracts the
single most valuable sub-feature -- cursor measurement with zoom/pan -- as a
deliverable that can be completed in 2 iterations rather than the 5-6 that a full
FFT/scatter/multi-axis engine would require. It directly enables the primary use
case (debugging sensor values) and lays the interaction foundation for future
advanced chart features (FFT overlay, scatter mode, etc.).

JustFloat and FireWater protocols are already implemented; without cursor/zoom the
data they produce cannot be meaningfully inspected in-application.

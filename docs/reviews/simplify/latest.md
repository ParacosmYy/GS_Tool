# Simplify Scan Report

## 1. Metadata

| Field | Value |
|------|-------|
| Time | 2026-06-12 14:34:54 |
| Source files scanned | 82009 |
| Max rows per section | 5 |

## 2. Oversized Cpp Files

- src/core/analysis/WaveformPatternDetector.cpp - 572 lines
- src/protocol/reassembly/PacketReassembler.cpp - 546 lines
- src/widgets/chart/ScrollChartWidget.cpp - 545 lines
- src/utils/matrix20/SparseCholesky.cpp - 538 lines
- src/utils/signal19/SpectralSubtract.cpp - 510 lines

## 3. Oversized Header Files

- src/utils/tree18/PersistentTree2.h - 232 lines
- src/utils/hex_editor/MemoryHexEditor.h - 216 lines
- src/serial/health/SerialHealthMonitor.h - 201 lines
- src/utils/fft18/ZoomFFT.h - 201 lines

## 4. Frozen Directories Present

- src/core/animation2 - present, 2 files
- src/core/widgets2 - present, 2 files
- src/plugin/loader2 - present, 2 files
- src/core/fonts - present, 2 files
- src/core/icons - present, 3 files
- src/core/responsive - present, 2 files
- src/core/font - present, 2 files
- src/core/icon - present, 2 files
- src/core/shortcut - present, 2 files
- src/core/managers - present, 5 files

## 5. Duplicate Directory Families

- src/core/font vs src/core/fonts
- src/core/icon vs src/core/icons
- src/core/widgets vs src/core/widgets2
- src/plugin/loader vs src/plugin/loader2
- src/core/shortcut vs src/core/managers

## 6. Forbidden Parallel Build Directories

- none

## 7. Risk Keyword Counts

- TODO - 7 occurrence(s)
- FIXME - 0 occurrence(s)
- setStyleSheet - 9 occurrence(s)

## 8. Recommended Next Step

Pick one small cleanup target and route it through the technical debt workflow. Do not refactor directly from this scan without a focused Specs or PRD.

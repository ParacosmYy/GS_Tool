# PRD-135 B8 - Python/PyQt Waveform Preview Specs

## Scope

B8 adds the first visualization foundation for the Python/PyQt Serial Station:

- typed `ChannelBatch` built from measurement protocol events
- preallocated NumPy ring buffer for bounded hot-path storage
- PyQtGraph waveform preview widget
- protocol selector in the PyQt workbench so FireWater measurements can update the preview

## Non-Goals

- No FFT, histogram, XY, image channel, cursor, zoom, or dashboard parity.
- No hardware or virtual COM claim.
- No per-sample Qt signals.
- No unbounded Python list growth for chart samples.

## Architecture

- `core/measurements.py` owns NumPy batch and ring buffer logic.
- `controllers/` may orchestrate protocol selection and measurement callbacks, but must not import PyQt.
- `ui/` owns PyQtGraph widgets and visual updates.
- `protocols/` remains UI-free.

## Acceptance

- `uv run pytest tests/python/unit/test_measurement_ring_buffer.py`
- `uv run pytest tests/python/ui_smoke/test_serial_station_waveform_preview.py`
- `uv run test-embeddebug-py`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`
- `uv run package-embeddebug --version b8-smoke --clean`
- `uv run verify-package-embeddebug --package-dir dist/EmbedDebugPy-b8-smoke-windows-x64`

## E/U/D Target

- Engineering: `E4`, pure hot-path tests and pytest-qt smoke pass.
- User: `U3`, fake FireWater measurements are visible in the Python/PyQt workbench.
- Device: `D2`, substitute path only.

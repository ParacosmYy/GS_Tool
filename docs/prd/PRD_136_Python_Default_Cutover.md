# PRD-136 Python/PyQt Default Cutover

## Background

PRD-135 opened the Python/PyQt migration lane side by side with the C++/Qt baseline. The requested direction is now explicit: replace the previous C++ default startup mode and make Python/PyQt the default user-facing startup path.

## Decision

Switch the repository default startup and default uv tool commands to Python/PyQt:

- `EmbedDebug.bat` launches Python/PyQt through `uv run start-embeddebug`.
- `uv run start-embeddebug` launches the PyQt app.
- `uv run package-embeddebug` packages the Python/PyQt app with PyInstaller.
- `uv run verify-package-embeddebug` verifies the Python/PyQt package.

The existing C++ source tree is not deleted in this PRD. It remains as legacy source until a separate cleanup phase removes or archives it. The default user workflow, however, is Python.

## Non-Goals

- No deletion of `src/` or CMake files in this batch.
- No real serial hardware claim.
- No public binary release claim before license notices/SBOM.

## Acceptance

- `uv run start-embeddebug --smoke` returns 0.
- `cmd /c EmbedDebug.bat --smoke` returns 0.
- `uv run package-embeddebug --version cutover-smoke --clean` creates `dist/EmbedDebugPy-cutover-smoke-windows-x64`.
- `uv run verify-package-embeddebug --package-dir dist/EmbedDebugPy-cutover-smoke-windows-x64` returns 0.
- `uv run test-embeddebug-py` returns 0.
- `uv run test-embeddebug-tools` returns 0.

## E/U/D

- Engineering: `E4`, startup and packaging have automated tests and smoke evidence.
- User: `U3`, default startup path opens Python/PyQt with the current fake serial workflow.
- Device: `D2`, substitute transport only; no real hardware claim.

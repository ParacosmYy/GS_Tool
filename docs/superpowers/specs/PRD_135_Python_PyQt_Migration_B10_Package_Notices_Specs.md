# PRD-135 B10 - Python Package Notices Specs

## Scope

Add a package-time compliance gate for the Python/PyQt lane so packaged
`EmbedDebugPy` outputs carry third-party notices and a dependency version
snapshot.

## Requirements

- `uv run package-embeddebug` copies a Python/PyQt third-party notice file into
  the package root as `THIRD_PARTY_NOTICES.md`.
- `uv run package-embeddebug` writes `PYTHON_DEPENDENCIES.txt` with the runtime
  dependency versions used by the local uv environment.
- `uv run verify-package-embeddebug` fails packages that omit either file.
- The notice route remains GPLv3-compatible for PyQt6 unless a future commercial
  PyQt/Qt or PySide6/LGPL decision supersedes it.

## Non-Goals

- No public release claim.
- No SPDX SBOM generator yet.
- No change to the C++ legacy fallback build.

## Verification

```powershell
uv run pytest tests/python/unit/test_pyinstaller_package_tools.py
uv run package-embeddebug --version b10-smoke --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-b10-smoke-windows-x64
```

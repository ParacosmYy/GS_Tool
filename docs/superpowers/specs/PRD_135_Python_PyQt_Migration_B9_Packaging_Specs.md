# PRD-135 B9 - Python/PyQt Side-by-Side Packaging Specs

## Scope

B9 adds the Python packaging lane without changing the C++ baseline:

- `uv run package-embeddebug-py`
- `uv run verify-package-embeddebug-py`
- PyInstaller `onedir` packaging for `EmbedDebugPy`
- final output under `dist/EmbedDebugPy-<version>-windows-x64/`
- PyInstaller temporary `workpath`, `distpath`, and `specpath` outside repository `build/`

## Non-Goals

- No `EmbedDebug.bat` routing change.
- No reuse of `uv run package-embeddebug`.
- No CMake or C++ source changes.
- No public release claim; PyQt GPL/commercial release notices and SBOM are still future gates.
- No onefile package.

## Architecture

The command implementation lives under `python/embeddebug/devtools/` and is exposed only through
`pyproject.toml` scripts. Runtime product code remains under `python/embeddebug/`.

The package command may call PyInstaller internally, but user workflow stays project-specific:

```powershell
uv run package-embeddebug-py
uv run verify-package-embeddebug-py
```

## Acceptance

- `uv run pytest tests/python/unit/test_pyinstaller_package_tools.py`
- `uv run package-embeddebug-py --help`
- `uv run verify-package-embeddebug-py --help`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug-py --smoke`

Packaging a full executable is environment-sensitive, so the scripted command must support `--dry-run`
for deterministic command verification before full binary creation.

## E/U/D Target

- Engineering: `E4` for packaging command construction and verifier tests.
- User: `U2`, a local user has a stable uv command path for Python packaging.
- Device: `D2`, no hardware claim; package only covers local fake/PyQt workflow.

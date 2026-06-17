# PRD-136 - Python/PyQt Default Cutover Specs

## Scope

Replace default C++ startup mode with Python/PyQt:

- `EmbedDebug.bat`
- `tools/launch_embeddebug.ps1`
- `pyproject.toml` default scripts
- Python tool tests
- active constraint docs that still declare the C++ baseline as default

## Required Default Commands

```powershell
uv run start-embeddebug --smoke
uv run package-embeddebug --version cutover-smoke --clean
uv run verify-package-embeddebug --package-dir dist/EmbedDebugPy-cutover-smoke-windows-x64
cmd /c EmbedDebug.bat --smoke
```

## Non-Goals

- Do not delete C++ source.
- Do not create another build directory.
- Do not claim hardware readiness.

## Acceptance

- Default uv script names point to Python/PyQt modules.
- `EmbedDebug.bat` reaches Python/PyQt and does not build C++.
- C++ package scripts are no longer the default commands.
- Python smoke, tests, and package verifier pass.

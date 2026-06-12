# Specs - PRD_099 UV Package Tool

## 1. Scope

Add a uv-friendly packaging entry for the C++/Qt EmbedDebug application.

Allowed paths:

- `pyproject.toml`
- `tools/package_embeddebug.py`
- `.gitignore`
- `README.md`
- `docs/tracking/SCORE_TRACKING.md`

No production C++ changes.

## 2. Command Contract

Primary command:

```powershell
uv run package-embeddebug
```

Supported options:

- `--skip-build`: reuse `build/EmbedDebug.exe`
- `--zip`: also create a zip archive
- `--clean`: remove existing package directory before packaging
- `--version <text>`: override package version suffix

The script must use `build/` only.

## 3. Packaging Behavior

Steps:

1. Resolve repository root.
2. Read `local_env.bat` if present.
3. Resolve CMake, Ninja, Qt prefix, MinGW bin.
4. Configure/build `EmbedDebug` unless `--skip-build` is passed.
5. Create `dist/EmbedDebug-<version>-windows-x64/`.
6. Copy `build/EmbedDebug.exe`.
7. Run `windeployqt.exe` against copied exe.
8. Copy README and selected launch/config docs.
9. Optionally zip the package directory.

## 4. Validation Commands

```powershell
uv run package-embeddebug --help
uv run package-embeddebug --skip-build --clean
uv run package-embeddebug --skip-build --clean --zip
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 5. README Requirement

README must explain:

- PyInstaller is not the right tool for the C++/Qt app body.
- `uv run package-embeddebug` is the supported shortcut.
- output lives under `dist/`, which is ignored by Git.

## 6. Commit Plan

1. PRD/Specs/score tracking.
2. uv packaging tool/README/.gitignore/score tracking after validation.

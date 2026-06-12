# Specs - PRD_100 UV Start Tool

## 1. Scope

Add a uv-friendly start command that delegates to the existing `EmbedDebug.bat`.

Allowed paths:

- `pyproject.toml`
- `tools/start_embeddebug.py`
- `README.md`
- `docs/tracking/SCORE_TRACKING.md`

No C++ changes.

## 2. Command Contract

Primary command:

```powershell
uv run start-embeddebug
```

Options:

- `--dry-run`: print the command without launching.
- `--wait`: wait for the batch process and return its exit code.

## 3. Behavior

- Resolve repo root from the Python module path.
- Locate `EmbedDebug.bat` at repo root.
- Launch it with `subprocess.Popen`.
- Default mode returns after the batch process is spawned.
- `--wait` returns the batch process exit code.
- Missing bat path returns a clear non-zero error.

## 4. Validation Commands

```powershell
uv run start-embeddebug --help
uv run start-embeddebug --dry-run
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 5. README Requirement

README Quick Start must show:

- `uv run start-embeddebug`
- existing `EmbedDebug.bat`
- `uv run package-embeddebug`

## 6. Commit Plan

1. PRD/Specs/score tracking.
2. start tool/README/score tracking after validation.

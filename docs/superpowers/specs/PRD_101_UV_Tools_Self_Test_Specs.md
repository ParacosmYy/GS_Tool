# Specs - PRD_101 UV Tools Self Test

## 1. Scope

Add a lightweight uv self-test command for Python tooling.

Allowed paths:

- `pyproject.toml`
- `tools/test_embeddebug_tools.py`
- `README.md`
- `docs/tracking/SCORE_TRACKING.md`

No C++ changes.

## 2. Command Contract

```powershell
uv run test-embeddebug-tools
```

The command must run `unittest` cases and return non-zero on failure.

## 3. Required Coverage

Test cases must cover:

- `tools.start_embeddebug.repo_root()`
- `tools.start_embeddebug.embeddebug_bat()`
- `tools.start_embeddebug --dry-run` through direct `main([...])`
- `tools.package_embeddebug.package_name()`
- `tools.package_embeddebug.read_local_env()`
- `tools.package_embeddebug.create_zip()` on a temp directory

## 4. Validation Commands

```powershell
uv run test-embeddebug-tools
uv run start-embeddebug --dry-run
uv run package-embeddebug --help
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 5. Commit Plan

1. PRD/Specs/score tracking.
2. test command/README/score tracking after validation.

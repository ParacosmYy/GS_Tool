# PRD-136 B23 - Remove Legacy Native Project

## Goal

Finish the repository-level Python/PyQt cutover by removing the legacy native project shape:

- no root `CMakeLists.txt`
- no `cmake/` directory
- no `src/` native runtime tree
- no C++ test entry under `tests/serial_station/`
- no `tests/CMakeLists.txt`

## Scope

- Delete the legacy native project files and directories.
- Update root and constraint docs so future work cannot restore C++/CMake as the mainline.
- Keep `uv + PyQt6 + PyInstaller` as the only active launch/test/package workflow.
- Add a regression test in `tests/python/unit/test_python_default_cutover.py`.

## Acceptance

- `test_repository_no_longer_contains_legacy_cpp_cmake_project` fails before deletion and passes after deletion.
- `uv run test-embeddebug-py` passes.
- `uv run test-embeddebug-tools` passes.
- `uv run start-embeddebug --smoke` passes.
- `cmd /c EmbedDebug.bat --smoke` passes.
- Active docs describe Python/PyQt as the only current product path.

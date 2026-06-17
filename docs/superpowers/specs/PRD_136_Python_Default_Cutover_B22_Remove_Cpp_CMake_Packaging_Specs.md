# PRD-136 B22 - Remove C++/CMake Packaging Semantics

## Goal

Make the active EmbedDebug workflow Python/PyQt only:

- `EmbedDebug.bat` launches `uv run start-embeddebug`.
- `uv run package-embeddebug` uses PyInstaller.
- Active tool wrappers must not invoke or describe C++/CMake packaging.
- Active docs must describe `uv + PyQt + PyInstaller` as the default workflow.

## Scope

- Update active tools:
  - `tools/package_embeddebug.py`
  - `tools/verify_package_embeddebug.py`
  - `tools/doctor.ps1`
  - `tools/project-audit/project_audit.py`
- Update Python-only cutover tests.
- Update active project docs:
  - `README.md`
  - `docs/constraints/01-project-overview.md`
  - `docs/constraints/07-directory-structure.md`
  - `docs/serial_station_architecture.md`

## Non-Goals

- Do not delete historical C++ source in this batch.
- Do not add new C++ build or package behavior.
- Do not commit generated package output.

## Acceptance

- `tools/package_embeddebug.py` is a compatibility wrapper for the PyInstaller package module.
- `tools/verify_package_embeddebug.py` is a compatibility wrapper for the PyInstaller verifier.
- `tools/doctor.ps1` checks uv/Python/PyQt launch and test paths, not native C++ toolchains.
- `tools/project-audit/project_audit.py` reports Python/PyQt runtime and test counts.
- `tests/python/unit/test_python_default_cutover.py` rejects active C++ packaging terms in active tools.
- `uv run test-embeddebug-tools` passes.
- `uv run test-embeddebug-py` passes.
- `uv run start-embeddebug --smoke` passes.
- `cmd /c EmbedDebug.bat --smoke` passes.
- PyInstaller package creation and verification pass.

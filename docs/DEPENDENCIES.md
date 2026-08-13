# Dependency and license record

Use `uv add` and `uv remove` to change dependencies. Keep `uv.lock` committed. This record is a project-level inventory and must be reviewed before public distribution.

| Dependency | Role | Initial license / review note |
|---|---|---|
| Python 3.12 | runtime | uv-managed CPython distribution; record distribution notices for releases |
| PyQt6 | Qt Python bindings | GPLv3 or Riverbank commercial license; closed-source distribution requires license review |
| PyQt6-QScintilla | editor control bindings | follows the PyQt/QScintilla licensing model; review before redistribution |
| PyInstaller | EXE bundling | GPLv2 with bootloader exception; verify current notices in the release artifact |
| Ruff | formatting and static checks | development-only dependency; license review still applies to redistributed tooling |
| Qt 6 libraries | GUI runtime | license depends on the selected Qt/PyQt distribution and modules |

## Policy

- Do not add a dependency only to avoid a small local abstraction.
- Prefer well-maintained packages with a clear license and Windows wheels.
- Pin compatible ranges in `pyproject.toml`; let `uv.lock` capture exact resolved versions.
- Review transitive dependencies before a public release.
- Store required notices under `docs/third_party/` when the release process needs them.
- `docs/third_party/NOTICE.md` inventories every non-editable package resolved by
  `uv.lock`; `scripts/verify_notice_inventory.py` mechanically checks exact
  version coverage without making a license-clearance decision.

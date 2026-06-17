# Python/PyQt Third-Party Notices

This notice file applies to the Python/PyQt migration lane packaged as
`EmbedDebugPy`.

## License Route

The current Python/PyQt lane uses a GPLv3-compatible development and packaging
route for PyQt6. Public distribution must keep this notice, the dependency
version snapshot, and the corresponding source/license obligations aligned.

If a proprietary or MIT-only binary is required later, the project must record a
separate Riverbank commercial PyQt/Qt decision or open a PySide6/LGPL migration
decision before release.

## Runtime Dependencies

| Dependency | Purpose | License Route |
| --- | --- | --- |
| PyQt6 | Qt GUI bindings and QtSerialPort access | Riverbank commercial or GPLv3 |
| PyQt6-Qt6 | Qt runtime bundled with PyQt6 wheels | Covered by the selected Qt/PyQt route |
| PyQt6-sip | PyQt binding support package | SIP/PyQt-compatible license terms |
| numpy | Typed channel batches and ring buffers | BSD-style license |
| pyqtgraph | 2D waveform plotting | MIT license |

## Packaging Tooling

PyInstaller is used only as the Python lane packager. Its work, dist, and spec
directories are kept outside the repository `build/` directory by
`uv run package-embeddebug`.

## Verification

Every `EmbedDebugPy-*-windows-x64` package must include:

- `THIRD_PARTY_NOTICES.md`
- `PYTHON_DEPENDENCIES.txt`
- `EmbedDebugPy.exe`
- Qt platform plugin files required for startup

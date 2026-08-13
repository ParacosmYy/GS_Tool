# QuillForge third-party notice inventory

This file is the release-candidate inventory for the dependency set resolved by
`uv.lock`. It is not a legal clearance, a substitute for the full license
texts, or a claim that the current executable is ready for public redistribution.
The release owner must verify the applicable license path and include the
required notices before signing or distributing a release.

## Runtime and packaged components

| Component | Resolved version | Role | License / release action |
| --- | ---: | --- | --- |
| CPython | 3.12.x runtime family | Python runtime | Record the exact redistributable runtime notice used by the release image. |
| altgraph | 0.17.5 | PyInstaller dependency | Release-owner license and notice verification required before redistribution. |
| macholib | 1.16.4 | PyInstaller dependency | Release-owner license and notice verification required before redistribution. |
| packaging | 26.3 | Version/specifier dependency | Release-owner license and notice verification required before redistribution. |
| pefile | 2024.8.26 | PyInstaller dependency | Release-owner license and notice verification required before redistribution. |
| PyQt6 | 6.11.0 | Qt Python bindings | Select and document the GPL/commercial license path; legal review required. |
| PyQt6-QScintilla | 2.14.1 | QScintilla editor bindings | Review the applicable PyQt/QScintilla license path and notices before redistribution. |
| PyQt6-Qt6 | 6.11.1 | Qt 6 Windows runtime | Record the selected Qt GPL/LGPL/commercial path and its complete notices. |
| pyqt6-sip | 13.12.0 | PyQt6 binding support | Release-owner license and notice verification required before redistribution. |
| PyInstaller | 6.22.0 | EXE bundling | Verify the PyInstaller and bootloader exception notices in the release package. |
| pyinstaller-hooks-contrib | 2026.6 | PyInstaller hook collection | Release-owner license and notice verification required before redistribution. |
| pywin32-ctypes | 0.2.3 | Windows packaging support | Release-owner license and notice verification required before redistribution. |
| setuptools | 84.0.0 | Build/package support | Release-owner license and notice verification required before redistribution. |

## Development-only components

| Component | Resolved version | Role | Release action |
| --- | ---: | --- | --- |
| Ruff | 0.16.2 | Formatting and static checks | Not part of the application runtime; retain inventory for build provenance. |

## Resolution and open gates

- Exact transitive versions and hashes are authoritative in `uv.lock`.
- The mechanical inventory check covers every non-editable package resolved by
  `uv.lock`; the editable `quillforge` project entry is intentionally excluded.
- Run `uv run python scripts/verify_notice_inventory.py` before packaging. The
  check proves version coverage only; it does not select a license path or
  provide legal clearance.
- This project currently builds an unsigned portable one-file EXE and does not
  claim an installer or update channel.
- The release candidate must attach this inventory beside the EXE and replace
  the review-required rows with the exact license texts/notices selected by the
  release owner.
- Until that review, S13 remains `environment_pending` and no commercial or
  public redistribution claim is made.

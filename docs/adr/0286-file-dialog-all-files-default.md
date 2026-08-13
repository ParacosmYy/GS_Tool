# ADR-0286: File dialog all-files default

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D242 / ARCH-223

## Decision

Make the existing localized `dialog.text_filter` start with Qt's `All files
(*)` filter and retain the existing text/source filter as the second option.
The same filter contract continues to serve the existing open and save dialog
surface; no second picker, document service, path policy, or asynchronous
boundary is introduced.

This repairs the default visibility gap for extensionless files and common
source/configuration files that were absent from the finite first filter. The
open dialog remains responsible for selecting an existing file, while the
application document service remains responsible for decoding and reporting
unsupported content.

## Public-source applicability

Qt's first-party `QFileDialog` documentation is the applicable public
engineering reference. It documents name filters separated by `;;`, the
`getOpenFileName`/`getSaveFileName` filter argument, and `All files (*)` as the
unrestricted wildcard form:

- Qt, *QFileDialog Class*, Qt 6.11.1 documentation, accessed 2026-08-12:
  <https://doc.qt.io/qt-6/qfiledialog.html>
- Qt, *QDir Class*, Qt 6.11.1 documentation, accessed 2026-08-12:
  <https://doc.qt.io/qt-6/qdir.html>

These are framework references, not manufacturer requirements. No private
ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or compliance
claim is made. Embedded C/C++, MCU, BSP/HAL, RTOS, and manufacturer
requirements are not applicable.

## Evidence and limits

- `D242-AST=PASS`; `D242-COMPILEALL=PASS`; `D242-RUFF=PASS`;
  `D242-FORMAT=PASS`
- `D242-FILE-FILTER-CONTRACT=PASS`
- `D242-QT-WILDCARD-CONTRACT=PASS`
- `D242-EXTENSIONLESS-REGRESSION-PROBE=PASS`
- `D242-SOURCE-STARTUP-DIAGNOSTIC=PASS`
- PS5.1 and PS7 package builds passed; final identity is
  `DF7F364FA81AF057F8EC2E076A0FF6D6D9123184C527C806F8B7215B8DD867DE`,
  38,575,619 bytes.
- `D242-FROZEN-RESOURCES=PASS`; `D242-FROZEN-APP-MODULE=PASS`;
  `D242-PYINSTALLER-WARNING-SCOPE=PASS`
- Parent review and simplification assessment: `PASS`.
- Architecture and independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits; no child PASS is claimed.
- Native EXE/Qt startup, native file-dialog rendering, and clean-machine
  behavior remain unrun under the permanent no-launch policy.

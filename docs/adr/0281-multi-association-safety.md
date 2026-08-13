# ADR-0281: Safe multi-extension association ownership

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D237 / ARCH-217

## Decision

Keep the existing opt-in, HKCU-only distribution boundary, but make shared
`QuillForge.Document` ownership explicit within one install operation:

- the first requested extension creates the shared ProgId;
- later extensions may reuse it only through the installer-owned reuse flag and
  only when its recorded open command still matches exactly;
- each extension remains independently checked and recorded;
- rollback removes recorded extension keys first, then removes the shared
  ProgId only when no remaining class key references it;
- uninstall and failed-install cleanup preserve the executable and state when
  association cleanup is incomplete, preventing a dangling command target.

The portable artifact remains association-free. Registry mutation remains an
explicit installer choice and is never performed by the application.

## Boundaries

`QuillForge.Distribution.psm1` owns association creation, ownership checks,
reference scanning, and cleanup. `install.ps1` owns the single-operation
ProgId ownership marker and rollback decision. `uninstall.ps1` owns the
stop-before-file-removal safety boundary. No application Python, UI, locale,
font, theme, or runtime behavior changes are included.

## Public-source applicability

Microsoft PowerShell 5.1/7 `SupportsShouldProcess` and Registry provider
behavior are applicable engineering references:

- `about_ShouldProcess`: <https://learn.microsoft.com/powershell/module/microsoft.powershell.core/about/about_shouldprocess>
- `about_Registry_Provider`: <https://learn.microsoft.com/powershell/module/microsoft.powershell.core/about/about_registry_provider>

These are public first-party PowerShell references, not manufacturer
requirements. No private ByteDance standard, certification, MISRA, ISO 26262,
ASPICE, or compliance claim is made. Embedded C/C++, MCU, BSP/HAL, RTOS, and
manufacturer requirements are not applicable.

## Evidence and limits

- `D237-DISTRIBUTION-STATIC-CONTRACT=PASS`
- `D237-PS51-AST=PASS`; `D237-PS7-AST=PASS`
- `D237-PACKAGE-IDENTITY-PROBE=PASS`
- `D237-PACKAGE-ARCHIVE=PASS`
- `D237-SOURCE-REVISION-BOUNDARY=PASS`
- The first bounded independent review found the pre-fix multi-extension
  failure; the correction was applied before this delivery.
- The post-fix independent window returned `NO_CONCLUSION` after bounded
  waiting. Native registry execution, installer execution, and EXE/Qt startup
  remain unrun by policy.

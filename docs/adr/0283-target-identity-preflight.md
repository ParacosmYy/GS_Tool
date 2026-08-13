# ADR-0283: Target identity preflight for update and rollback

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D239 / ARCH-220

## Decision

Before any updater file move, validate the current executable against the
install state's recorded SHA-256. For explicit rollback, validate both the
current target and the recorded backup against their respective state hashes,
and reject a state where the two paths are identical.

Use the existing `Get-QfArtifactInfo` boundary after `ShouldProcess`; do not
add a second hash implementation, state schema, lock service, or transaction
framework. A missing file, malformed digest, or mismatch stops the operation
before the first `Move-Item`, preserving the externally changed file for
operator inspection.

## Public-source applicability

Python/PyInstaller are not changed in this slice. Microsoft PowerShell
5.1/7 `Get-FileHash`, `Move-Item`, `about_ShouldProcess`, and the existing
Registry provider references are applicable public first-party engineering
references:

- <https://learn.microsoft.com/powershell/module/microsoft.powershell.management/get-filehash>
- <https://learn.microsoft.com/powershell/module/microsoft.powershell.management/move-item>
- <https://learn.microsoft.com/powershell/module/microsoft.powershell.core/about/about_shouldprocess>

No manufacturer requirement, private ByteDance standard, certification, MISRA,
ISO 26262, ASPICE, or compliance claim is made. Embedded C/C++, MCU, BSP/HAL,
RTOS, and manufacturer requirements are not applicable.

## Evidence and limits

- `D239-STATE-IDENTITY-STATIC-CONTRACT=PASS`
- `D239-PS51-AST=PASS`; `D239-PS7-AST=PASS`
- `D239-PACKAGE-IDENTITY-PROBE=PASS`
- `D239-PACKAGE-ARCHIVE=PASS`
- Architecture and post-fix independent bounded windows returned
  `NO_CONCLUSION`; parent review and simplification are the only PASS claims.
- Updater/rollback execution, EXE/Qt startup, and clean-machine behavior remain
  unrun by policy.

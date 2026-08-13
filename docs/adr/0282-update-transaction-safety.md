# ADR-0282: Transaction-safe local update and rollback

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D238 / ARCH-219

## Decision

Keep `update.ps1` as a local, explicit, `ShouldProcess`-guarded operation, but
make the executable/state exchange recoverable when state persistence fails:

- ordinary update records that the previous target moved and restores it only
  when the current target is either the expected new artifact or the expected
  previous artifact;
- explicit rollback records both moves and restores the original target and
  original backup path only when hashes and exact state-owned paths still
  match;
- external modification, occupied backup paths, permission failures, and
  uncertain hashes cause a warning and preservation of the current state rather than a
  forced overwrite.

No new state schema, process launcher, registry operation, or update channel is
introduced. The existing `Write-QfState` remains the commit boundary.

## Public-source applicability

Microsoft PowerShell 5.1/7 `about_ShouldProcess`, `about_Registry_Provider`
(for the surrounding distribution boundary), `Move-Item`, and
`Get-FileHash` documentation are public first-party engineering references:

- <https://learn.microsoft.com/powershell/module/microsoft.powershell.core/about/about_shouldprocess>
- <https://learn.microsoft.com/powershell/module/microsoft.powershell.management/get-filehash>
- <https://learn.microsoft.com/powershell/module/microsoft.powershell.management/move-item>

They are not manufacturer requirements. No private ByteDance standard,
certification, MISRA, ISO 26262, ASPICE, or compliance claim is made. Embedded
C/C++, MCU, BSP/HAL, RTOS, and manufacturer requirements are not applicable.

## Evidence and limits

- `D238-UPDATE-TRANSACTION-STATIC-CONTRACT=PASS`
- `D238-PS51-AST=PASS`; `D238-PS7-AST=PASS`
- `D238-PACKAGE-IDENTITY-PROBE=PASS`
- `D238-PACKAGE-ARCHIVE=PASS`
- The architecture and post-fix independent bounded windows returned
  `NO_CONCLUSION`; parent review and simplification are the only PASS claims.
- Update/rollback execution, native EXE/Qt startup, clean-machine, and release
  gates remain unrun or open under policy.

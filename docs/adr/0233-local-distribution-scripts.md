# ADR-0233: Local hash-gated distribution scripts

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D184 / ARCH-171

## Decision

Keep `dist/QuillForge.exe` as the portable release artifact and add a small,
local-only PowerShell distribution boundary under `packaging/`:

- `QuillForge.Distribution.psm1` owns path normalization/containment, SHA-256
  artifact validation, install-state persistence, bounded rollback names, and
  exact owned file-association cleanup.
- `install.ps1` copies only a caller-supplied `.exe` whose expected SHA-256
  matches, into the current user's local Programs directory by default.
- `update.ps1` stages and validates a replacement, retains the previous
  executable, and exposes an explicit rollback operation.
- `uninstall.ps1` removes only state-owned executable/rollback/state files and
  associations whose recorded values still match; user-modified associations
  are left in place.

File associations are opt-in through `-RegisterFileAssociations`, restricted
to safe explicit extensions, and written only under HKCU. Existing extension
or QuillForge ProgId keys are refused rather than overwritten. All mutating
entry points use `SupportsShouldProcess` so a caller can preview the operation.

## Scope and non-goals

The scripts do not download, elevate, launch, flash, modify machine-wide
registry state, or claim a signed installer/update channel. They do not change
Python application behavior, settings, UI, file-open policy, or the portable
release manifest. Runtime script execution, registry behavior, rollback
durability, clean-machine behavior, cross-machine repeatability, signing, and
release-owner approval remain open.

## Evidence

- PowerShell parser: `D184-POWERSHELL-PARSE=PASS`.
- Static safety/contract probes:
  `D184-STATIC-DISTRIBUTION-PROBE=PASS`,
  `D184-SHA-CONTRACT-PROBE=PASS`, `D184-CONTAINMENT-PROBE=PASS`, and
  `D184-OPT-IN-HKCU-PROBE=PASS`.
- Parent review: `docs/agent-team/reviews/D184-local-distribution-scripts-parent-review.md`.
- Independent review record:
  `docs/agent-team/reviews/D184-local-distribution-scripts-independent-review.md`.
- Handoff: `docs/handoffs/2026-08-11-d184-local-distribution-scripts/handoff.md`.

## Applicability

This is a Python/PyQt6 Windows distribution contract. No embedded C/C++,
MCU, BSP/HAL, RTOS, or manufacturer requirement applies. Shipping and
security guidance is treated as engineering guidance; public CloudWeGo
material, where referenced by project records, is not a private ByteDance
standard and does not support a certification/compliance claim.

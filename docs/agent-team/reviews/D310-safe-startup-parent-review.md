# D310 parent review — safe startup recovery path

## Scope

Reviewed the `--safe-mode` dispatcher flag, argument stripping before Qt
parsing, composition-root boolean boundary, default-settings projection,
plugin/session/recovery bypass, explicit startup-path preservation, cleanup,
and static contract.

## Findings

- PASS — the flag is handled at the application boundary and is not passed to
  `QApplication` as an unknown Qt option.
- PASS — normal startup retains its existing plugin activation, session/recovery
  restore, command refresh, show, file-open, and cleanup order.
- PASS — safe mode reuses `DEFAULT_SETTINGS`, creates an initial document,
  preserves explicit file paths, and avoids plugin/session/recovery activation
  without adding a second composition root.
- PASS — the source composition probe, startup/file-open diagnostics, static
  audit, package identity, PE header, and frozen archive inventory pass.

## Simplification assessment

PASS. One boolean at the existing `DesktopRuntime` boundary is the smallest
complete recovery seam. A separate safe-mode service, settings schema field,
or widget-level bypass would add coupling and persistence risk.

## Limits and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. Native startup, window rendering, clean-machine behavior, and
safe-mode interaction remain unverified. This is Python/PyQt6 desktop code;
embedded vendor-source applicability is N/A.


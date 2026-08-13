# ADR-0021: Windows packaging provenance and variant isolation

日期：2026-08-09  
状态：Accepted for engineering builds

## Context

SerialForge has two optional packaging dimensions: capability variant (core or BLE)
and PyInstaller mode (onedir or onefile). Reusing one dist directory makes it
easy to start an old executable, hide optional dependencies, or lose the exact
source/toolchain evidence for a bundle. J-Link RTT is intentionally deferred and
must not introduce a driver, SDK, DLL, or vendor executable during this slice.

## Decision

- scripts/package.ps1 is the only Windows packaging entry point.
- The project version is read from src/serialforge/__init__.py and is used both
  for the output path and the generated PE version resource.
- Every build is isolated at
  dist/release/<version>/<variant>/<mode>/app. The generated PyInstaller work
  and spec files live below build/pyinstaller/provenance/<version>/<variant>/<mode>.
- Each mode writes a provenance manifest, SHA256 sidecar, archive listing,
  locked dependency tree, engineering NOTICE, and third-party inventory.
- The manifest records source revision, uv.lock hash, Python/PyInstaller versions,
  PE metadata, Authenticode status, capabilities, hardware status, and license
  inventory status. It is an engineering-build record, not a release signature.
- Core content is rejected when Bleak/WinRT appears in the archive or payload.
  Every variant is rejected when J-Link/SEGGER/probe-rs vendor binaries appear.
- CI executes the four combinations independently on fresh Windows runners and
  uploads a unique artifact name for each combination.

## Consequences

The release directory is larger because evidence travels with each package, but
the result is diagnosable and cannot silently mix core/BLE or onedir/onefile
outputs. A future signed release still needs a complete license-text bundle,
legal review, authorized hardware acceptance, and signing infrastructure. RTT
remains the last functional milestone and is not enabled by this packaging ADR.

## Verification

Local verification must include locked static/compile checks, all four package
matrix entries, manifest verification, archive content scans, PE version fields,
unsigned status, and startup/close for each executable. No COM, BLE radio, J-Link
driver, target board, or formal release claim is implied.

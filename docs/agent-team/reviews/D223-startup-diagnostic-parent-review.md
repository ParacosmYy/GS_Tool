# D223 / ARCH-206 parent review: no-window packaged startup diagnostic

## Decision

`PASS` for the bounded diagnostic contract, accepted with explicit runtime and
release limits.

## Evidence

- The write set is limited to `src/quillforge/app.py`.
- The branch is dispatched before `QApplication` construction and only when
  the explicit `--diagnose-startup` switch is present.
- A required report path, stable JSON schema, execution mode, artifact
  identity, applicable resource checks, and deterministic exit code provide a
  useful handoff for the actual EXE failure investigation.
- The source probe passed and the final PyInstaller archive statically contains
  the expected Qt plugin, QScintilla binary, entry point, and authored icon.
- Runtime-module coverage was reconciled against the archive: all 136 runtime
  modules are present; the two apparent exceptions are the intentional
  top-level PyInstaller `__main__` mapping and the packaging-script-only
  release-manifest store.
- Both PowerShell hosts passed `scripts/check.ps1` and
  `scripts/verify_handoff.ps1`; the final root/dist hashes and release manifest
  agree.
- The normal GUI path remains the existing `QApplication` and composed runtime
  lifecycle.

## Simplification assessment

`PASS`: one preflight branch in the existing dispatcher is the smallest
complete boundary. A new diagnostic service, a console-mode packaging change,
or probes embedded in every composition layer would increase coupling and
alter the user-facing startup shape without adding evidence for this failure
mode.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
No EXE launch, frozen execution, native Qt initialization, or clean-machine
check was performed. The parent conclusion is limited to source behavior,
static package contents, and the no-window source probe.

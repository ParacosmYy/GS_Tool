# D222 / ARCH-205 parent review: windowed startup-failure boundary

## Decision

`PASS` for the bounded entry-point reliability fix, accepted with explicit
runtime and release limits.

## Evidence

- The write set is limited to `src/quillforge/__main__.py`.
- Startup exceptions are handled only at the executable entry boundary; no
  business or presentation policy is changed.
- A user-local UTF-8 traceback is written before the process exits, and a
  Windows-native message is attempted when a Qt shell may not exist.
- Reporting failures are best-effort and cannot replace the original startup
  failure with a second crash.

## Simplification assessment

`PASS`: one small entry-point boundary is the smallest complete fix. Adding
logging to every composition service, changing the PyInstaller console mode,
or introducing a new application service would widen ownership without
improving early-startup diagnosability.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
The actual Qt event loop, native message rendering, Windows user-data ACLs,
and EXE startup remain unrun under the active no-launch policy.

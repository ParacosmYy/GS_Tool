# ADR-0278: Guard the entry-point application import

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D232 / ARCH-214
- Scope: `src/quillforge/__main__.py`

## Context

The windowed entry point previously imported `app.main` at module scope before
the bottom-level exception boundary ran. If that import failed because of a
missing or incompatible packaged module, the process could terminate before
the existing startup log, native fallback, or stderr path was available.

## Decision

Keep `main(argv)` as the public entry contract, but resolve `app.main` inside
that function. The existing `if __name__ == "__main__"` guard therefore covers
both application dispatch and its import boundary. Preserve the existing
package-relative import for installed/bundled execution and the direct-source
`sys.path` compatibility branch.

No Qt construction, composition ownership, command-line dispatch, message
text, normal return value, or final startup exit code changes.

## Boundary and privacy

The change only broadens the existing diagnostics-only entry boundary. It does
not add a logger, environment dump, settings/document logging, or new runtime
dependency. Native rendering, clean-machine behavior, and release evidence
remain unverified under the no-launch policy.

## Review and simplification

Parent review and simplification are `PASS`. The bounded architecture role and
independent role are recorded as `NO_CONCLUSION` when their windows close
without a conclusion. A single lazy-import wrapper is the smallest compatible
change that keeps the public `main(argv)` entry point and catches import-time
startup failures.

## Public-source applicability

Python 3.12 public import, module execution, exception, and `collections.abc`
callable contracts are the applicable engineering references. PyInstaller's
public one-file entry behavior is the packaging engineering reference; no
dependency changed. Public CloudWeGo material is engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

# ADR-0276: Keep startup fallback display best-effort

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D230 / ARCH-212
- Scope: `src/quillforge/__main__.py`

## Context

D229 prevented report-path and log-write failures from masking an early
startup exception. The remaining fallback path still interpolated the
exception with `str(error)`, called the native Windows MessageBox, and wrote
to stderr without a final fail-open boundary. An unusual exception object,
native API failure, or broken stderr could therefore obscure the original
failure after the log boundary had already completed.

## Decision

Keep fallback presentation at the existing entry boundary and make each
presentation step best-effort:

1. use a stable generic message when exception stringification fails;
2. catch native fallback display failures broadly enough to fall through; and
3. treat stderr output as optional so the original exception remains the
   authoritative exit cause.

For ordinary exceptions and working Windows APIs, the existing message text,
native MessageBox title/icon, and exit code remain unchanged. No Qt import,
application composition, settings, or second diagnostics service is added.

## Boundary and privacy

The fallback continues to expose only the exception summary and optional log
path. It does not add arguments, environment dumps, settings, secrets, or
document contents. Paths may contain usernames or network roots. Native EXE
startup and rendering remain outside the no-launch proof boundary.

## Review and simplification

Parent review and behavior-preserving simplification are `PASS`. The bounded
`Copernicus the 6th / Luna max` architecture role returned `NO_CONCLUSION`;
the `Mendel the 6th / Luna max` independent source review returned `PASS`.
The three small best-effort stages remain local to the entry boundary; no
abstraction is justified. Interpreting a zero `MessageBoxW` return is a
separate follow-up and is not claimed by D230.

## Public-source applicability

Python 3.12 public built-in exception, `ctypes`, `sys`, and stderr behavior are
the applicable first-party engineering references. No dependency changed.
Public CloudWeGo material is engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

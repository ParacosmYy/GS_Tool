# ADR-0277: Fall through when the native startup MessageBox reports failure

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D231 / ARCH-213
- Scope: `src/quillforge/__main__.py`

## Context

D230 made native fallback display best-effort, but treated every non-throwing
`MessageBoxW` call as successful. Win32 reports API failure by returning zero;
that result could leave a console-disabled process with no visible fallback
even though stderr remained available.

## Decision

Capture the `MessageBoxW` return value. Preserve the existing direct return for
nonzero success, and continue to the existing stderr fallback when the return
value is zero. Keep exception handling unchanged and local to the startup
diagnostic boundary.

No Qt, application composition, message text, title/icon request, normal
return value, or final startup exit code changes.

## Boundary and privacy

The fallback still records/displays only the existing exception summary and
optional diagnostic path. No arguments, environment dump, settings, secrets,
or document contents are added. Paths may contain usernames or network roots.
Native rendering remains unverified under the no-launch policy.

## Review and simplification

Parent review and simplification are `PASS`. The bounded architecture role and
independent role are recorded as `NO_CONCLUSION` when their windows close
without a conclusion. The single result check is the smallest compatible
change; no abstraction is introduced.

## Public-source applicability

Python 3.12 public `ctypes` and exception behavior, plus the public Win32
`MessageBoxW` return contract, are the applicable engineering references. No
dependency changed. Public CloudWeGo material is engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

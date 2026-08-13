# ADR-0308: Secondary startup fallback localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D264 / ARCH-242

## Context

The first startup-fallback localization slice covered the normal construction
path, but its outer fail-open branch still used the English message
`Startup diagnostic details unavailable.`. That branch is intentionally rare,
yet it is the final message shown when error formatting or diagnostic context
itself fails.

## Decision

Inside the existing outer exception handler, make one additional guarded call
to the Qt-free `_startup_locale()` resolver. Use Chinese fallback labels for
`zh-CN`, retain the original English fallback for other locales, and retain a
second nested English literal if the locale probe itself fails. No diagnostic
file, exception detail, MessageBox flag, stderr behavior, process exit code, or
normal startup path changes.

## Boundaries and alternatives

The final English literal remains the last-resort fail-open behavior. Loading
settings, Qt, or the presentation catalog here would violate the early startup
boundary and could hide the original failure. The change therefore stays local
to `__main__.py` and does not introduce a new service.

## Public-source applicability and review

Python's first-party [`locale.getlocale` documentation](https://docs.python.org/3/library/locale.html#locale.getlocale)
is applicable to the guarded language hint, and Python's exception handling
contract is applicable to preserving the nested fail-open boundary. Microsoft's
first-party [`MessageBoxW` documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw)
remains applicable to the unchanged native fallback call and flags. The
project startup-diagnostic contract is the applicable engineering reference.
No manufacturer requirement changed. Public CloudWeGo/ByteDance material
remains an engineering reference only; no private corporate standard,
certification, MISRA, ISO 26262, ASPICE, or embedded C/C++/MCU/RTOS claim is
made.


# ADR-0306: Early startup fallback localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D262 / ARCH-240

## Context

When the desktop process fails before the normal Qt shell can load, the
existing native fallback showed `QuillForge could not start`, `Diagnostic log`,
and `QuillForge startup error` only in English. That made the very error path
needed to diagnose an unopened EXE inconsistent with the selected Windows
language experience.

## Decision

Keep the early fallback in `src/quillforge/__main__.py`, which is already
Qt-free, and add a small two-locale system-language projection. Environment
locale hints are checked first, followed by Python's `locale.getlocale()`;
Chinese-like values select `zh-CN`, and every other value safely falls back to
the existing English text. Only the user-facing fallback labels are localized:
the original exception type/message, diagnostic path, log format, MessageBox
flags, stderr fallback, and process exit code remain unchanged.

This is deliberately independent from persisted application settings because
the settings service and Qt presentation catalog may be unavailable at this
stage of startup.

## Boundaries and alternatives

No Qt import, translator, settings read, or registry lookup is introduced into
the early exception path. A full catalog dependency would add startup coupling
and could hide the original failure. A larger OS-locale matrix is out of scope;
the current product contract supports `en-US` and `zh-CN` only.

## Public-source applicability and review

Python's first-party [`locale.getlocale` documentation](https://docs.python.org/3/library/locale.html#locale.getlocale)
is applicable to the best-effort pre-Qt language hint. Microsoft's first-party
[`MessageBoxW` documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw)
is applicable to preserving the existing native fallback call and flags. The
project startup-diagnostic and supported-locale contracts are the applicable
engineering references. No manufacturer requirement changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.


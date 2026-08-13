# ADR-0280: Close startup-path argument and admission edge cases

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D235 / ARCH-216
- Scope: `src/quillforge/application/desktop_launch.py`, `src/quillforge/presentation/main_window.py`

## Context

D233 routed explicit paths into the existing startup open boundary. Two small
edge cases remained: Qt value options are commonly written with either one or
two leading dashes, and a startup directory could be removed from the queue
without feedback when workspace admission was unavailable. Those cases made a
valid launch request appear to be ignored.

## Decision

Normalize only the leading-dash spelling when identifying the existing bounded
Qt value-option set. Preserve the original option text and its value in the Qt
argument tuple. In the startup-path drain, retain the existing temporary busy
and session-restore retry behavior; after a non-temporary admission rejection,
emit a warning before advancing to the next queued path.

No new queue, document service, workspace service, registry owner, or startup
policy is introduced. The portable artifact remains without default Windows
file associations; the existing opt-in HKCU-only packaging scripts remain the
separate install boundary.

## Review and simplification

Parent review and simplification are `PASS`. The bounded architecture role
(`Schrodinger the 7th / Luna max`) and independent role (`Tesla the 7th / Luna
max`) returned `NO_CONCLUSION` after their windows closed. The fix is limited to
one parser predicate and one existing presentation warning branch; extracting
another coordinator would increase coupling without improving ownership.

## Public-source applicability

Python 3.12 public process-argument and `pathlib` behavior, PyQt6 argument
forwarding, and PyInstaller one-file packaging are applicable engineering
references. No dependency or vendor SDK changed. Public CloudWeGo material is
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made. Embedded C/C++, MCU, RTOS, and manufacturer
requirements are not applicable.

## Limits

No QApplication, native EXE, Windows shell drag-and-drop, registry, installer,
clean-machine, or cross-machine operation was performed. Static and package
evidence does not prove native rendering, shell association, or release
readiness.

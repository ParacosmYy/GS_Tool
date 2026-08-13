# ADR-0274: Startup failure context record

- Status: accepted-with-limits
- Delivery: D228 / ARCH-210
- Date: 2026-08-12

## Context

The console-disabled portable EXE already recorded the exception type,
message, and traceback when normal startup failed. That evidence identified
the D225 missing accessor, but it did not identify which executable, working
directory, frozen bundle, or Python runtime produced the log. This makes an
old package or a different user-local launch context difficult to distinguish
from a new regression.

## Decision

Keep startup diagnostics at the existing `src/quillforge/__main__.py` entry
boundary and add a fixed, path-only execution-context block to the existing
UTF-8 startup log. The fields are executable path, working directory, frozen
flag, Python major/minor/micro version, and frozen bundle root. Path resolution
uses a small fail-open helper, and the entire context collection is guarded so
it cannot replace the original exception or alter the native fallback message
and exit code.

## Boundaries and alternatives

- No document contents, command-line arguments, environment dump, settings
  payload, secrets, or user file names are recorded by the new fields.
- Path strings can still contain usernames or network roots; this is an
  explicit diagnostic tradeoff for a user-local troubleshooting log.
- No Qt, QApplication, MainWindow, application policy, or normal GUI path was
  changed.
- A full structured crash-reporting service was rejected as disproportionate
  and would add privacy, transport, and dependency scope.

## Review and simplification

- Parent review: `PASS` for the bounded entry-boundary source scope.
- Architecture role `Avicenna the 6th / Luna max`: `NO_CONCLUSION` after a
  bounded window; the direction and risks were supplied, but no patch review
  conclusion was produced.
- Independent review `Epicurus the 6th / Luna max`: `NO_CONCLUSION` after a
  bounded window and closure.
- Simplification: `PASS`; one safe path helper, one fixed context tuple, and
  one fail-open call site are the smallest clear implementation.

## Public-source applicability

Python 3.12 public `pathlib`, `sys`, `traceback`, and exception behavior are
the applicable first-party engineering references. No dependency changed.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded C/
C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Evidence and limits

The context probe, fail-open fallback probe, compileall, Ruff, formatting,
presentation audit, dual-shell package builds, archive inspection, package
identity, project check, and handoff verification are the applicable
non-destructive evidence. Actual EXE/Qt startup, native message-box rendering,
clean-machine, cross-machine, signing, installer/update, legal, support,
permission/disk-pressure, hard-power, and release-owner evidence remain open.


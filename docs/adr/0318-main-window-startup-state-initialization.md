# ADR-0318: MainWindow startup-state initialization

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D282 / ARCH-252

## Context

`MainWindow.__init__` wires many Qt-free coordinator ports and callback
closures before the complete shell exists. The `_busy` callbacks were created
before the default `_busy = False` assignment. Current constructors do not
invoke those callbacks, but a future construction-time callback could read an
uninitialized attribute and fail before a native window appears.

## Decision

Initialize `_busy` alongside `_startup_restore_inflight` at the start of
`MainWindow.__init__`, before any coordinator or callback wiring. Leave the
existing `True`/`False` assignments used by actual operations unchanged. Add
a Qt-free presentation contract that checks, within `MainWindow.__init__`,
that exactly one default `_busy = False` assignment exists and that it appears
before the first `is_busy=lambda: self._busy` capture.

The contract is intentionally narrow: it does not forbid later runtime state
transitions and does not attempt to infer whether arbitrary callbacks execute
inside third-party Qt constructors.

## Consequences

Any future construction-time read of `_busy` observes the same initial false
state used by the prior runtime path. The change removes an avoidable
uninitialized-state window without changing operation admission semantics.
The audit turns the ordering assumption into a maintenance check.

## Public-source applicability

Python 3.12 `ast` syntax-tree traversal is the applicable first-party
reference for the static contract:
<https://docs.python.org/3/library/ast.html>. Qt 6 is only part of the
surrounding presentation composition; no new Qt API is introduced. No
manufacturer requirement applies. This is not embedded C/C++, MCU, BSP/HAL,
RTOS, ISR/DMA, driver, bootloader, or firmware work; no certification claim
is made.

## Verification boundary

The presentation audit, Ruff, formatting, compileall, source startup
diagnostic, project checks, PyInstaller PE/archive inspection, and package
identity passed. EXE/Qt launch, clean-machine startup, signing, installer,
updater, registry, and release-go evidence remain unrun or open.

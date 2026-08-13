# ADR-0317: Startup diagnostic user guidance

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D281 / ARCH-251

## Context

The portable candidate already exposes `--diagnose-startup`, including the
D280 settings preflight, but the README described only the older runtime and
resource checks. A user who sees no window could not tell what the settings
record meant or where the normal-startup traceback was written.

## Decision

Document the existing command and its evidence boundary in `README.md`. The
documentation names the `settings_preflight` metadata fields, states that the
diagnostic does not write or expose preference values, explains exit codes,
and distinguishes source/package preflight from native window creation and
release acceptance.

No runtime code, settings schema, packaging behavior, or launch policy changes
are part of this slice.

## Consequences

Users have a reproducible path for collecting a startup report and a clear
interpretation of a passing report. The README cannot prove native Windows
startup; it points to the existing startup-error log and preserves the
no-launch boundary.

## Public-source applicability

The applicable references are the project's own `--diagnose-startup` contract
in `src/quillforge/app.py` and Python 3.12 command-line/file-path behavior;
the Python standard-library documentation is public and first-party:
<https://docs.python.org/3/library/argparse.html> and
<https://docs.python.org/3/library/pathlib.html>. No manufacturer requirement
applies. This is not embedded C/C++, MCU, BSP/HAL, RTOS, ISR/DMA, driver,
bootloader, or firmware work; no certification claim is made.

## Verification boundary

README contract assertion, source startup diagnostic, compileall, Ruff,
formatting, and project checks passed. No EXE/Qt launch, clean-machine run,
signing, installer, updater, registry, or external release action was taken.

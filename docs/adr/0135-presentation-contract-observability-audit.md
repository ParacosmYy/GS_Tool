# ADR-0135: Presentation contract and observability audit

- **Status:** accepted-with-limits; D108/ARCH-80 bounded static-gate slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The existing `scripts/check.ps1` enforces lower-layer import direction, but
three presentation contracts were only documented and manually reviewed:
Qt-free coordinators must not acquire Qt shell or concrete worker-owner
dependencies; MainWindow notifications must identify their semantic level; and
TaskRunner pending work must remain visible through the status phase while
worker execution or queued completion delivery is retained.

These are cross-module contracts. A future edit can break them without
violating the domain/application/infrastructure rules already present in
`check.ps1`.

## Decision

Add `scripts/audit_presentation_contracts.py` and invoke it from
`scripts/check.ps1`. The gate parses source with Python's standard `ast`
module and checks:

1. every `presentation/*_coordinator.py` import list for `PyQt6`,
   `task_runner`, `main_window`, or `status_surface` dependencies;
2. every direct `self.notify(...)` call in MainWindow for an explicit `level=`
   keyword;
3. TaskRunner's `pending_changed`, `pending_count`,
   `has_pending_work`, and add/release emission contract, plus MainWindow's
   signal connection, callback, and `_runner.has_pending_work()` projection.

The gate reports file/line or contract-specific diagnostics and returns
non-zero on violation. It deliberately does not infer runtime event ordering,
error completeness, visual output, or performance from source names.

## Invariants

1. No application behavior, notification text, status level, TaskRunner
   scheduling, coordinator state, close policy, or public API changes.
2. The gate remains stdlib-only and runs before the existing lock/Ruff/compile
   checks in `scripts/check.ps1`.
3. Relative imports are evaluated by their AST module name and diagnostics use
   `Path.as_posix()` so the gate remains readable on Windows.
4. The gate is a source contract audit, not a unit-test harness or runtime
   observability claim.

## Alternatives considered

- **Keep these rules only in review prose:** rejected; they are cheap,
  deterministic regressions to catch statically.
- **Add runtime assertions or a Qt smoke test:** rejected; this would widen a
  source-contract slice into runtime behavior and violate the current
  no-launch/test-asset boundary.
- **Duplicate the checks as more ripgrep patterns in PowerShell:** rejected;
  AST parsing gives clearer call/import semantics and avoids quoting/path
  ambiguity.
- **Audit every error path automatically:** rejected; source names cannot prove
  business-level error completeness, and a broad heuristic would create false
  confidence.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop tooling and presentation code. No MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, or manufacturer requirement applies. The
mandatory embedded assurance gate is `N/A`; the embedded workflow and
simplifier were reviewed for applicability and no embedded source was changed.
Public CloudWeGo material remains an engineering reference only. No private
ByteDance standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or
compliance claim is made.

## Review and simplification

- Architect: Laplace the 3rd / Luna max; bounded read-only wait timed out and
  the agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS
  is claimed.
- Independent review: Mendel the 3rd / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for AST portability, explicit diagnostics,
  dependency scope, notification-level detection, TaskRunner add/release
  coverage, and check-script integration.
- Simplification assessment: PASS. One stdlib script centralizes three
  existing contracts; no helper package, runtime layer, test harness, or
  duplicated PowerShell heuristic was added.

## Verification target and limits

- Required and authorized here: positive audit, in-memory AST negative-case
  probe, compileall, Ruff, format, package identity, check/handoff/index
  synchronization, and expected release NO-GO evidence.
- Not proven: runtime error completeness, native queued timing, QApplication
  startup, visual/accessibility output, performance, clean-machine,
  cross-machine, signing, installer, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.

# D40 parent review — plugin operation-state boundary

- **Delivery:** D40 / ARCH-30 / UI-26
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D40 extracts only the duplicated lifecycle state for three independent plugin
operation domains into the Qt-free
`presentation.plugin_operation_tracker.PluginOperationTracker`:
`catalog-scan`, `catalog-governance`, and `host-probe`. Each domain retains a
separate monotonic ID sequence and active callback boundary. A completion is
accepted only when its ID is current for the same domain.

MainWindow still owns the PluginCatalog/Approval/Runtime/Host services,
TaskRunner submission, notifications and severity mapping, PluginSurface
projection, command refresh, synchronous runtime enablement policy, and all
trust/approval/containment/execution decisions. `closeEvent()` now asks the
tracker about all three domains, preserving the existing close guard.

The D39 single-active `OperationTracker` is intentionally not reused: plugin
catalog scan, governance, and host diagnostics have independent concurrency
domains and must not cancel or stale each other.

The required architecture consultation was attempted with Maxwell the 2nd /
Terra max for a cross-module plugin lifecycle trace. Two bounded waits returned
no conclusion and the agent was closed; no architecture PASS is claimed. An
independent read-only review was attempted with Peirce the 2nd / Luna max;
two bounded waits also returned no conclusion and it was closed. No child
review PASS is claimed.

## Static review findings

- The tracker exposes a closed `Literal` kind set rather than a free-form
  string map.
- `begin()` increments only the selected kind's counter and marks only that
  slot active.
- `complete()` rejects stale IDs and clears only the matching kind.
- Catalog scan success/failure, governance success/failure, and host
  diagnostic success/failure all use the matching kind.
- No old plugin inflight or operation-counter fields remain in MainWindow.
- PluginSurface callbacks, TaskRunner, PluginFailed handling, notification
  mapping, and `refresh_command_menus()` remain in their existing owners.
- The close guard still blocks while any of the three plugin domains is in
  flight.

## Simplification assessment

The smallest behavior-preserving extraction is one typed tracker with three
fixed slots. It removes six coordinator fields without introducing a generic
service, Qt signal, callback bus, or second plugin policy model. Keeping the
tracker presentation-local avoids expanding application contracts and keeping
the three slots independent avoids weakening concurrency semantics. No further
safe simplification is required for D40.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation lifecycle code; embedded C/C++, MCU, BSP/HAL,
RTOS, ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements
are not applicable. Public CloudWeGo material remains an engineering
reference only; no private ByteDance standard, certification, or compliance
claim is made. The applicability record is carried in ADR-0065.

## Authorized non-destructive validation

- D40 source/state-boundary probe — PASS: fixed kinds, tracker methods, old
  field removal, per-domain delegation, and close-guard coverage.
- `uv run python -m compileall -q src/quillforge/presentation/plugin_operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- `uv run ruff check src/quillforge/presentation/plugin_operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- `uv run ruff format --check src/quillforge/presentation/plugin_operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, deployment, or hardware
  operation was authorized.

## Handoff and limits

The D40 handoff records the rebuilt package identity after the synchronized
package build. Release verification remains expected NO-GO while historical
runtime reports, clean-machine evidence, signing, installer, update, and
release-owner gates remain open.

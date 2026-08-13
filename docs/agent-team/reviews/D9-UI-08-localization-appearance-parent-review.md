# D9 UI-08 parent architecture review

## Scope

This review covers the localization, appearance preferences, transition
animation, settings migration, and workspace file-activation slice. The parent
requested the Architect role before source changes and retained the parent as
the only writer in the current checkout.

## Architecture result

`PASS WITH LIMITS`.

- Domain owns immutable `AppearanceSettings` and the expanded editor value;
  application owns schema v2 defaults/migration/normalization; infrastructure
  owns only JSON persistence and atomic replacement.
- Presentation owns the bounded i18n catalog, theme token projection, settings
  controls, and the optional opacity transition. No Qt dependency was added to
  domain, application, or settings storage.
- `WorkspacePanel` maps Qt items to semantic file/directory signals. `MainWindow`
  performs containment validation and reuses the existing `TaskRunner` plus
  document service, preserving high cohesion and low coupling.
- The composition root loads the initial settings snapshot once and injects it
  into `MainWindow`; the constructor retains a fallback load for non-root
  callers.

## Independent review

Luna/max Architect agent Plato (`019fe792-e12a-7041-9915-12f097751b94`) gave a
read-only review. It returned `PASS WITH LIMITS` for the file/directory signal chain,
settings boundary, and dialog initialization order. It identified two explicit
limits: native click/double-click timing can still vary by Qt/platform, and
workspace containment is a pre-open check rather than a strong TOCTOU boundary.
The review also recommended menu retranslation and avoiding duplicate settings
loads; both were addressed in this slice.

## Simplification assessment

The smallest complete shape is retained:

- one immutable appearance value instead of separate mutable UI preference
  objects;
- one bounded catalog and one theme token module instead of per-dialog string
  or color branches;
- whitelist/range normalization instead of arbitrary persisted font/theme data;
- one existing async document-open boundary instead of a second workspace file
  reader;
- one optional fade animation instead of a general transition subsystem.

No behavior-preserving simplification is required beyond the current design.

## Public-source applicability

The mandatory embedded enterprise workflow and embedded-code review
simplifier were invoked for the assurance gate and recorded as not applicable:
this project is Python/PyQt6 desktop code with no MCU, firmware, BSP/HAL,
RTOS, ISR/DMA, driver, bootloader, or embedded C/C++ change. Therefore there
is no applicable public first-party MCU/vendor requirement to claim. No
MISRA/ISO/certification claim is made.

## Authorized validation

- `uv run python -m compileall -q src/quillforge` — PASS.
- `uv run ruff check` on all changed Python modules — PASS.
- `uv run ruff format --check` on all changed Python modules — PASS.
- `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and packaging are required
  for final disposition and are recorded in the handoff.
- No QApplication, window, executable, unit test, mock, fixture, harness, or
  target operation was launched or created. Runtime visual, font-installation,
  native-dialog, DPI, and click-timing evidence remain unrun under policy.

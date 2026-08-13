# ADR-0084: Remove the status-phase forwarding alias

- **Status:** accepted-with-limits; D59 / ARCH-48 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._sync_active_document_phase()` had no document-specific behavior;
it immediately called `_sync_status_surface()`. Its callers already ran at
document dirty changes, workspace-open completion, and current-tab changes,
while the unified status method owns working/attention/ready precedence.

## Decision

Delete the forwarding alias and route its existing callers directly to
`_sync_status_surface()`. Keep the status method and its policy unchanged:
pending/active operations remain `working`, dirty active documents remain
`attention`, and the idle clean state remains `ready`. No status surface,
notification, workspace, editor, or document policy moves.

## Invariants

1. `_sync_active_document_phase` has no remaining source reference.
2. Editor modified callbacks still refresh status after dirty-state updates.
3. Workspace open completion still refreshes status after navigation results.
4. Current-tab changes still refresh status after selection and notification.
5. Runner-pending, operation completion, and close/status guards still call the
   same unified status projection method.

## Alternatives considered

- **Keep the alias as a semantic label:** rejected because it falsely implies a
  separate phase owner while the implementation has no additional behavior.
- **Move phase policy into StatusSurface:** rejected; this bounded cleanup does
  not change the existing application-owned precedence policy.
- **Create a document-status coordinator:** rejected; the alias has no state,
  and a new coordinator would add indirection without a consumer.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target and limits

- A source probe confirms the alias is absent, direct status callers remain,
  and the editor/workspace/tab/status method boundaries remain present.
- Compile, Ruff, format, handoff, package identity, and expected release no-go
  evidence are recorded in the D59 handoff.
- Native Qt event timing, runtime startup, accessibility, DPI, clean-machine,
  cross-machine, and release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.

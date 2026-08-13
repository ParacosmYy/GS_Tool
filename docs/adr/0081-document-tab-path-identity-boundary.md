# ADR-0081: Document-tab path identity boundary

- **Status:** accepted-with-limits; D56 / ARCH-45 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._find_tab_by_path()` iterated the complete tab registry and
implemented path normalization beside document/session orchestration. The
existing `DocumentTabSurface` already owns tab record identity, index lookup,
and projection, so the full-registry lookup belonged at that boundary.

## Decision

Add `DocumentTabSurface.find_by_path(path, exclude=None)`. It uses the existing
domain `path_key()` helper, ignores pathless records, preserves object-identity
exclusion, and returns the first matching projected record. MainWindow's
`_find_tab_by_path()` becomes a one-line delegation.

The startup session-restoration subset remains in MainWindow: selecting the
restored active tab from `_session_restore_tabs` is application/policy
orchestration and is not folded into the general tab registry lookup.

## Invariants

1. `None` paths return no record.
2. Path comparison continues to use the canonical `domain.path_identity.path_key`
   normalization and first-match ordering.
3. `exclude` compares by record identity, preserving Save As/open deduplication
   behavior.
4. DocumentTabSurface remains presentation-owned; it does not open files,
   change DocumentService state, or choose session-restore policy.
5. MainWindow no longer traverses the full tab registry for ordinary path
   lookup, while the restore-subset active-tab selection remains explicit.

## Alternatives considered

- **Keep the loop in MainWindow:** rejected because the registry owner would
  expose its internal collection to every coordinator path.
- **Add a second path-index cache:** rejected; one small linear lookup matches
  the existing bounded tab collection and avoids stale index lifecycle.
- **Move all restore selection into the surface:** rejected because the
  restored subset is startup policy, not generic tab projection.

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

- A source/AST probe confirms canonical path lookup and MainWindow delegation,
  while the session-restore subset comparison remains in MainWindow.
- Compile, Ruff, format, handoff, package identity, and expected release no-go
  evidence are recorded in the D56 handoff.
- Native Qt tab interaction, path casing on every Windows filesystem, runtime
  startup, clean-machine, cross-machine, and release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.

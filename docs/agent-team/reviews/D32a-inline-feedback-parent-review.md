# D32a parent review — inline workspace feedback hierarchy

- **Delivery:** D32a / UI-18 / ARCH-22
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

`presentation/feedback.py` is a narrow presentation-only seam. It owns the
`FeedbackLevel` vocabulary and the Qt property/repolish operation required to
make centralized QSS state selectors take effect. `StatusSurface` reuses the
same helper while preserving its import-compatible `StatusMessageLevel` name.
`WorkspacePanel` and `WorkspaceSearchDialog` retain their existing localized
message, result, loading, cancellation, diagnostic, signal, and application
policy ownership; they only project explicit state metadata.

The D32a boundary deliberately excludes FindBar. FindBar has multiple
MainWindow outcome call sites whose semantic mapping needs its own audit and is
tracked as D33 rather than being hidden in this slice.

## Review evidence

The parent source review found no must-fix behavior regression in the changed
files. Existing callbacks, operation IDs, cancellation paths, root handling,
result payloads, and localized text remain in their original owners. The
workspace/search state mapping is explicit: initial/info, working, success,
partial/cancelled/warning, and recoverable error/error.

The required architect consultation was attempted with Gauss the 2nd / Luna
max (read-only, bounded D32a contract question). It returned no conclusion;
this record does not claim an architect PASS. An independent read-only review
was attempted with Dalton the 2nd / Luna max. After two bounded waits and a
request to return a concise conclusion, it also returned no conclusion and was
closed. No child PASS is claimed; the parent review is the integration
decision.

## Simplification assessment

The shared helper is a safe behavior-preserving simplification: it removes
three copies of property/repolish mechanics and keeps the state vocabulary in
one module. Importing `StatusSurface` into inline widgets or deriving state by
parsing localized text would increase coupling and were rejected in ADR-0057.
No further simplification is required for this slice. `FeedbackLevel` includes
`working` because inline surfaces have an in-flight state, while the D31
status-message type remains a narrower compatible subset.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control concerns are not
applicable. The public CloudWeGo pages recorded in ADR-0057 are transferable
engineering references only. They do not establish a private ByteDance
standard, certification, or compliance claim.

## Authorized non-destructive validation

- `uv run python -m compileall -q src` — PASS.
- Ruff check over all five changed presentation modules — PASS.
- Ruff format check over all five changed presentation modules — PASS.
- Static contract/dependency probe — PASS: shared helper exists, each target
  surface imports it, and the helper has no application-layer dependency.
- Static contrast probe — PASS: 60 pairs (3 themes × 4 accents × 5 states),
  each at or above 4.5:1.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive visual review,
  screen-reader, DPI, clean-machine, deployment, or hardware operation was
  authorized or performed.

## Handoff and release limits

The D32a handoff and package manifest are updated after packaging. The release
verifier is expected to remain NO-GO because the project still lacks the
release-owner, clean-machine, interactive, and fresh packaged-report evidence
listed in `docs/RELEASE_HANDOFF.md`.

The D32a package identity is root/dist SHA-256
`CE32F1DD1876F4AB108A560E5569E70998163527FFD1420115F58C78D6859119`,
`38,403,655` bytes, with source revision
`tree-sha256:d76954b9b1536e3f219dc17d932eb4e1528e204d56ec1a6c54dfa97247088723`.

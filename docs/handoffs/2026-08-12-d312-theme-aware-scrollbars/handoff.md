# Handoff: 2026-08-12-d312-theme-aware-scrollbars

| Field | Value |
|---|---|
| ID | `2026-08-12-d312-theme-aware-scrollbars` |
| Delivery / slice | `D312 / UI-130 / ARCH-282 Theme-aware scrollbars` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:59:00+08:00` |

## User outcome

Vertical and horizontal scrollbars now match the selected theme, use compact
rounded geometry, and expose readable hover/pressed highlights, making the
editor and workspace feel more modern without changing scrolling behavior.

## Scope and boundaries

Changed only the centralized presentation stylesheet and its static contract
audit. No scrollbar range/page logic, editor behavior, widget callbacks,
layout ownership, commands, locale, settings, persistence, startup, file-open,
or plugin behavior changed.

## Team roles and ownership

- Architect: parent agent; architecture consultation timeboxed with no
  conclusion.
- Developer: parent agent; sole writer in the supplied checkout.
- Independent reviewer: Luna/max read-only pass; no conclusion after three
  bounded waits.
- Project manager/product/QA evidence: represented by the parent review,
  acceptance record, package checks, and release handoff limits.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized scrollbar tokens and
  QSS state selectors.
- `scripts/audit_presentation_contracts.py` — scrollbar source contract and
  3-theme/4-accent contrast audit.
- `docs/adr/0348-theme-aware-scrollbars.md` — decision record.
- `docs/agent-team/reviews/D312-theme-aware-scrollbars-parent-review.md` —
  parent review and simplification assessment.
- `docs/agent-team/reviews/D312-theme-aware-scrollbars-independent-review.md` —
  independent review status.

## Decisions and constraints

Use the existing `_stylesheet()` owner and readable-edge helper. Normal,
hover, and pressed handles must clear the 3.0 non-text contrast floor on
`surface_1` and `surface_2`. Hide only arrow-line controls; preserve native
range, page, wheel, keyboard, and drag semantics. No native EXE/Qt startup is
authorized, and no unit-test or test-only asset was created or run.

## Verification commands and results

- `uv run ruff format src/quillforge/presentation/theme.py scripts/audit_presentation_contracts.py` — PASS.
- `uv run python -m compileall -q src scripts` — PASS.
- `uv run ruff check src scripts` — PASS.
- `uv run python scripts/audit_presentation_contracts.py` — PASS.
- `D312-SCROLLBAR-MATRIX=PASS themes=3 accents=4 handle_min=12.01 hover_min=3.93 pressed_min=3.93`.
- `D312-QSS-PROBE=PASS matrices=12`.
- `D312-SOURCE-DIAGNOSTIC=PASS status=passed window_shown=False event_loop_entered=False`.
- `D312-MANIFEST-COPY=PASS bytes=38600928 sha=74EBA0B827ABFA466740ABDF254C8C98AA52BED6FD5922EF5CECF1B159CB149B`.
- `D312-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D312-PE-ARCHIVE=PASS outer_entries=166 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1`.
- Root `QuillForge.exe` and `dist/QuillForge.exe` are byte-identical.

## Unrun checks and reason

Native EXE launch, Qt scrollbar rendering, interactive hover/pressed/drag
behavior, screen-reader output, DPI/scaling, alternate style engines,
clean-machine behavior, real DLL-loader behavior, signing, installer,
updater, and release owner gates remain unrun because the project policy sets
`software_start_allowed=false` and this change does not authorize release
operations.

## Known risks and limits

QSS rendering can vary by native Qt style engine and platform metrics. The
static matrix proves token contrast, not pixel-level appearance or interaction
behavior. The source diagnostic still emits the existing non-fatal Qt
font-directory warning because the development PyQt6 installation does not
ship fonts. The package remains an unsigned portable candidate and the
release verifier is expected to remain NO-GO on its existing report-binding and
native startup gates.

## Acceptance and evidence IDs

- Delivery: `D312 / UI-130 / ARCH-282`.
- Scenario: `S352`.
- Reviews: parent `PASS`, independent `NO_CONCLUSION`.
- Simplification: `PASS`.
- Applicability: Python/PyQt6 desktop engineering reference; embedded vendor
  applicability N/A.

## Next owner and next action

Architect/project manager should authorize a separate native-rendering and
clean-machine verification pass before treating the portable candidate as a
release. The next UI slice should continue reusing the centralized theme
owner, not introduce widget-local styling.

## Artifact information

- `QuillForge.exe` and `dist/QuillForge.exe`
- SHA-256: `74EBA0B827ABFA466740ABDF254C8C98AA52BED6FD5922EF5CECF1B159CB149B`
- Size: `38,600,928` bytes
- Source revision: `tree-sha256:3af09bdc594dcba4633bc58bf9a3ce5574d78011d22f12a3e57db4cafb3fa8f4`
- Packaging: PyInstaller 6.22.0, Python 3.12.13, Windows x64 portable one-file candidate

## Disposition

Accepted with limits. The D312 source contract and package are recorded, but
the artifact is not a release approval while native startup and existing
enterprise release gates remain open.


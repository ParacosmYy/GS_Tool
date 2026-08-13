# Handoff: 2026-08-12-d311-theme-aware-shell-separators

| Field | Value |
|---|---|
| ID | `2026-08-12-d311-theme-aware-shell-separators` |
| Delivery / slice | `D311 / UI-129 / ARCH-281 Theme-aware shell separators` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:59:00+08:00` |

## User outcome

The main window and dock-panel split boundaries now use the selected theme's
readable edge color and an alternate-accent hover highlight, improving shell
hierarchy without changing editor or file behavior.

## Scope and boundaries

Changed only the centralized presentation stylesheet and its presentation
contract audit. No widget logic, layout ownership, commands, locale,
settings, persistence, startup, file-open, or plugin behavior changed.

## Team roles and ownership

- Architect: parent agent; architecture consultation timeboxed with no
  conclusion.
- Developer: parent agent; sole writer in the supplied checkout.
- Independent reviewer: Luna/max read-only pass; no conclusion after three
  bounded waits.
- Project manager/product/QA evidence: represented by the parent review,
  acceptance record, package checks, and release handoff limits.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — derived separator edge and shared
  main-window/dock QSS selectors.
- `scripts/audit_presentation_contracts.py` — separator source contract and
  3-theme/4-accent contrast audit.
- `docs/adr/0347-theme-aware-shell-separators.md` — decision record.
- `docs/agent-team/reviews/D311-theme-aware-shell-separators-parent-review.md`
  — parent review and simplification assessment.
- `docs/agent-team/reviews/D311-theme-aware-shell-separators-independent-review.md`
  — independent review status.

## Decisions and constraints

Use the existing `_stylesheet()` owner and token helpers. A normal edge must
clear the 3.0 non-text contrast floor on `surface_0`–`surface_2`; hover uses
the existing `accent_alt`. No native EXE/Qt startup is authorized, and no
unit-test or test-only asset was created or run.

## Verification commands and results

- `uv run ruff format src/quillforge/presentation/theme.py scripts/audit_presentation_contracts.py` — PASS.
- `uv run python -m compileall -q src scripts` — PASS.
- `uv run ruff check src scripts` — PASS.
- `uv run python scripts/audit_presentation_contracts.py` — PASS.
- `D311-SEPARATOR-MATRIX=PASS themes=3 accents=4 edge_min=10.00 hover_min=3.93`.
- `D311-QSS-RENDER-PROBE=PASS matrices=12 selector_pairs=2`.
- `D311-SOURCE-DIAGNOSTIC=PASS status=passed window_shown=False event_loop_entered=False`.
- `D311-MANIFEST-COPY=PASS bytes=38600780 sha=2C895413652A9D3798129A506B78D80418FC81D74BF178CD21D2907A34975808`.
- `D311-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D311-PE-ARCHIVE=PASS outer_entries=166 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1`.
- Root `QuillForge.exe` and `dist/QuillForge.exe` are byte-identical.

## Unrun checks and reason

Native EXE launch, Qt window rendering, interactive separator dragging/hover,
screen-reader output, DPI/scaling, alternate style engines, clean-machine
behavior, real DLL-loader behavior, signing, installer, updater, and release
owner gates remain unrun because the project policy sets
`software_start_allowed=false` and the release gates are not authorized by
this change.

## Known risks and limits

QSS rendering can vary by native Qt style engine and platform metrics. The
static matrix proves token contrast, not pixel-level appearance or interaction
behavior. The source diagnostic emitted the existing non-fatal Qt font-directory
warning because the development PyQt6 installation does not ship fonts. The
package remains an unsigned portable candidate and the release verifier is
expected to remain NO-GO on its existing report-binding and native startup
gates.

## Acceptance and evidence IDs

- Delivery: `D311 / UI-129 / ARCH-281`.
- Scenario: `S351`.
- Reviews: parent `PASS`, independent `NO_CONCLUSION`.
- Simplification: `PASS`.
- Applicability: Python/PyQt6 desktop engineering reference; embedded vendor
  applicability N/A.

## Next owner and next action

Architect/project manager should authorize a separate native-rendering and
clean-machine verification pass before treating the portable candidate as a
release. The next implementation slice should retain the centralized theme
owner and avoid widening this separator change into startup or widget logic.

## Artifact information

- `QuillForge.exe` and `dist/QuillForge.exe`
- SHA-256: `2C895413652A9D3798129A506B78D80418FC81D74BF178CD21D2907A34975808`
- Size: `38,600,780` bytes
- Source revision: `tree-sha256:431fa49540508664ed88fc258500a20e761a3299e842eed646ffed78d35ac02b`
- Packaging: PyInstaller 6.22.0, Python 3.12.13, Windows x64 portable one-file candidate

## Disposition

Accepted with limits. The D311 source contract and package are recorded, but
the artifact is not a release approval while native startup and existing
enterprise release gates remain open.

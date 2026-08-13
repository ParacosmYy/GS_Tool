# Handoff: 2026-08-12-d315-readable-checkbox-indicator

| Field | Value |
|---|---|
| ID | `2026-08-12-d315-readable-checkbox-indicator` |
| Delivery / slice | `D315 / UI-133 / ARCH-285 Readable checkbox indicator states` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T00:20:00+08:00` |

## User outcome

Checked behavior toggles and other checkboxes retain Qt's native tick while
remaining readable across the supported themes and accent choices, including
the amber/砂金 palette.

## Scope and boundaries

Changed only the centralized presentation stylesheet and its static contract
audit. No checkbox signal, state model, keyboard behavior, accessibility
semantics, Settings role, persistence, locale, editor, startup, or application
boundary changed.

## Team roles and ownership

- Architect: parent agent; Luna/max architecture consultation timeboxed with
  no conclusion.
- Developer: parent agent; sole writer in the supplied checkout.
- Independent reviewer: Luna/max read-only pass; no conclusion after three
  bounded waits.
- Project manager/product/QA evidence: represented by the parent review,
  acceptance record, package checks, and release handoff limits.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — readable checked indicator surfaces
  and derived edge tokens.
- `scripts/audit_presentation_contracts.py` — checkbox source and contrast
  contract.
- `docs/adr/0351-readable-checkbox-indicator-states.md` — decision record.
- `docs/agent-team/reviews/D315-readable-checkbox-indicator-parent-review.md` —
  parent review and simplification assessment.
- `docs/agent-team/reviews/D315-readable-checkbox-indicator-independent-review.md` —
  independent review status.

## Decisions and constraints

Keep Qt's native QCheckBox indicator and check-state semantics. Use token-bound
pressed/hover/muted surfaces plus readable edge fallbacks; do not introduce a
custom SVG, resource path, widget subclass, or event handler. Native EXE/Qt
startup is not authorized, and no unit-test or test-only asset was created or
run.

## Verification commands and results

- `uv run ruff format src/quillforge/presentation/theme.py scripts/audit_presentation_contracts.py` — PASS.
- `uv run python -m compileall -q src scripts` — PASS.
- `uv run ruff check src scripts` — PASS.
- `uv run python scripts/audit_presentation_contracts.py` — PASS.
- `D315-CHECKBOX-RENDER-PROBE=PASS matrices=12 checked_surface_pixels=present`.
- `D315-CHECKBOX-MATRIX=PASS themes=3 accents=4 mark_min=8.64`.
- `D315-SOURCE-DIAGNOSTIC=PASS status=passed window_shown=False event_loop_entered=False`.
- `D315-PACKAGE-IDENTITY=PASS bytes=38602010 sha=68D86AA6FB5756AE0E28E737A0F2418DA98668DC442AFD2AB3FCBE110680070C`.
- `D315-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D315-PE-ARCHIVE=PASS outer_entries=166 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1`.
- Root `QuillForge.exe` and `dist/QuillForge.exe` are byte-identical.

## Unrun checks and reason

Native EXE launch, Qt pixel rendering on Windows, keyboard interaction,
screen-reader output, DPI/scaling, alternate style engines, clean-machine
behavior, real DLL-loader behavior, signing, installer, updater, and release
owner gates remain unrun because the project policy sets
`software_start_allowed=false` and this change does not authorize release
operations.

## Known risks and limits

The offscreen probe confirms the resolved checked surfaces and native rendering
path in the development environment, not Windows native pixel parity. QSS
state propagation and platform metrics may vary by style engine. The source
diagnostic still emits the existing non-fatal Qt font-directory warning. The
package remains an unsigned portable candidate and the release verifier is
expected to remain NO-GO on its existing report-binding and native-startup
gates.

## Acceptance and evidence IDs

- Delivery: `D315 / UI-133 / ARCH-285`.
- Scenario: `S355`.
- Reviews: parent `PASS`, independent `NO_CONCLUSION`.
- Simplification: `PASS`.
- Public source: Qt 6 QCheckBox/QSS references and WCAG-style contrast;
  embedded vendor applicability N/A.

## Next owner and next action

Architect/project manager should authorize a separate native-rendering and
clean-machine verification pass before treating the portable candidate as a
release. Continue auditing remaining interactive controls through semantic
presentation roles rather than widget-local overrides.

## Artifact information

- `QuillForge.exe` and `dist/QuillForge.exe`
- SHA-256: `68D86AA6FB5756AE0E28E737A0F2418DA98668DC442AFD2AB3FCBE110680070C`
- Size: `38,602,010` bytes
- Source revision: `tree-sha256:2d1144c3be56e68fd7b2ed615caa7ffbe807a0104f0a712995605b7975c711cf`
- Packaging: PyInstaller 6.22.0, Python 3.12.13, Windows x64 portable one-file candidate

## Disposition

Accepted with limits. The D315 source contract and package are recorded, but
the artifact is not a release approval while native startup and existing
enterprise release gates remain open.


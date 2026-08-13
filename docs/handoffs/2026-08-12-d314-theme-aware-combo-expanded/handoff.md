# Handoff: 2026-08-12-d314-theme-aware-combo-expanded

| Field | Value |
|---|---|
| ID | `2026-08-12-d314-theme-aware-combo-expanded` |
| Delivery / slice | `D314 / UI-132 / ARCH-284 Theme-aware combo-box expanded affordance` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T00:05:00+08:00` |

## User outcome

The language, theme, accent, UI-font, editor-font, and font-style combo boxes
now make their expanded/open state visible through the same theme surface and
accent boundary as the surrounding field and arrow.

## Scope and boundaries

Changed only the centralized presentation stylesheet and its static contract
audit. No combo-box item data, settings role, locale catalog, popup model,
keyboard navigation, persistence, editor policy, startup path, or application
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

- `src/quillforge/presentation/theme.py` — shared combo-box subcontrol
  positioning and popup-open state.
- `scripts/audit_presentation_contracts.py` — source and contrast contract.
- `docs/adr/0350-theme-aware-combo-expanded-affordance.md` — decision record.
- `docs/agent-team/reviews/D314-theme-aware-combo-expanded-parent-review.md` —
  parent review and simplification assessment.
- `docs/agent-team/reviews/D314-theme-aware-combo-expanded-independent-review.md` —
  independent review status.

## Decisions and constraints

Use Qt's documented `QComboBox::drop-down` and `::down-arrow` subcontrols in
the existing `_stylesheet()` owner. Preserve the native arrow, popup, item
data, and keyboard semantics. The open-state foreground must clear the 4.5
contrast floor on the pressed token surface. No native EXE/Qt startup is
authorized, and no unit-test or test-only asset was created or run.

## Verification commands and results

- `uv run ruff format src/quillforge/presentation/theme.py scripts/audit_presentation_contracts.py` — PASS.
- `uv run python -m compileall -q src scripts` — PASS.
- `uv run ruff check src scripts` — PASS.
- `uv run python scripts/audit_presentation_contracts.py` — PASS.
- `D314-COMBO-EXPANDED-MATRIX=PASS themes=3 accents=4 text_min=8.64`.
- `D314-QSS-PROBE=PASS matrices=12 states=normal,hover,pressed,on,disabled`.
- `D314-SOURCE-DIAGNOSTIC=PASS status=passed window_shown=False event_loop_entered=False`.
- `D314-MANIFEST-COPY=PASS bytes=38601248 sha=37C7893A6446F83F4289B83DB38CC11F46AB7CA9691EA4DB568069CA904ACACD`.
- `D314-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D314-PE-ARCHIVE=PASS outer_entries=166 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1`.
- Root `QuillForge.exe` and `dist/QuillForge.exe` are byte-identical.

## Unrun checks and reason

Native EXE launch, Qt popup rendering, keyboard navigation, accessibility
output, DPI/scaling, alternate style engines, clean-machine behavior, real
DLL-loader behavior, signing, installer, updater, and release owner gates
remain unrun because the project policy sets `software_start_allowed=false`
and this change does not authorize release operations.

## Known risks and limits

QSS subcontrol painting and pseudo-state propagation can vary by native Qt
style engine and platform metrics. The static matrix proves token contrast,
not pixel-level arrow/popup rendering or click timing. The source diagnostic
still emits the existing non-fatal Qt font-directory warning because the
development PyQt6 installation does not ship fonts. The package remains an
unsigned portable candidate and the release verifier is expected to remain
NO-GO on its existing report-binding and native-startup gates.

## Acceptance and evidence IDs

- Delivery: `D314 / UI-132 / ARCH-284`.
- Scenario: `S354`.
- Reviews: parent `PASS`, independent `NO_CONCLUSION`.
- Simplification: `PASS`.
- Public source: Qt 6 QComboBox/QSS references and WCAG-style contrast;
  embedded vendor applicability N/A.

## Next owner and next action

Architect/project manager should authorize a separate native-rendering and
clean-machine verification pass before treating the portable candidate as a
release. The next UI slice should continue reusing semantic presentation roles
and the centralized theme owner.

## Artifact information

- `QuillForge.exe` and `dist/QuillForge.exe`
- SHA-256: `37C7893A6446F83F4289B83DB38CC11F46AB7CA9691EA4DB568069CA904ACACD`
- Size: `38,601,248` bytes
- Source revision: `tree-sha256:85674b7aa74f4baaad45ab4902b39bd8b7f9d471a5b810ac7ff1152ed1308e68`
- Packaging: PyInstaller 6.22.0, Python 3.12.13, Windows x64 portable one-file candidate

## Disposition

Accepted with limits. The D314 source contract and package are recorded, but
the artifact is not a release approval while native startup and existing
enterprise release gates remain open.


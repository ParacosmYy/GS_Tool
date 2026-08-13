# Handoff: 2026-08-12-d313-theme-aware-typography-steppers

| Field | Value |
|---|---|
| ID | `2026-08-12-d313-theme-aware-typography-steppers` |
| Delivery / slice | `D313 / UI-131 / ARCH-283 Theme-aware typography steppers` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:59:00+08:00` |

## User outcome

The Settings interface-font and editor-font size controls now have compact,
theme-aware up/down step buttons with readable normal, hover, pressed, and
disabled states while preserving their existing value behavior.

## Scope and boundaries

Changed only the centralized presentation stylesheet and its static contract
audit. No QSpinBox range, keyboard stepping, persistence, settings schema,
locale, editor policy, layout ownership, startup, or application behavior
changed.

## Team roles and ownership

- Architect: parent agent; architecture consultation timeboxed with no
  conclusion.
- Developer: parent agent; sole writer in the supplied checkout.
- Independent reviewer: Luna/max read-only pass; no conclusion after three
  bounded waits.
- Project manager/product/QA evidence: represented by the parent review,
  acceptance record, package checks, and release handoff limits.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — role-scoped QSpinBox stepper tokens
  and QSS subcontrols.
- `scripts/audit_presentation_contracts.py` — typography-stepper source
  contract and 3-theme/4-accent contrast audit.
- `docs/adr/0349-theme-aware-typography-steppers.md` — decision record.
- `docs/agent-team/reviews/D313-theme-aware-typography-steppers-parent-review.md`
  — parent review and simplification assessment.
- `docs/agent-team/reviews/D313-theme-aware-typography-steppers-independent-review.md`
  — independent review status.

## Decisions and constraints

Use Qt's documented `QSpinBox::up-button`/`down-button` subcontrols through
the existing `_stylesheet()` owner. Keep native arrows and value semantics;
do not add icons, event handlers, or settings state. Normal, hover, and
pressed text must clear the 4.5 contrast floor on their token surfaces. No
native EXE/Qt startup is authorized, and no unit-test or test-only asset was
created or run.

## Verification commands and results

- `uv run ruff format src/quillforge/presentation/theme.py scripts/audit_presentation_contracts.py` — PASS.
- `uv run python -m compileall -q src scripts` — PASS.
- `uv run ruff check src scripts` — PASS.
- `uv run python scripts/audit_presentation_contracts.py` — PASS.
- `D313-TYPOGRAPHY-STEPPER-MATRIX=PASS themes=3 accents=4 text_min=8.64`.
- `D313-QSS-PROBE=PASS matrices=12 states=normal,hover,pressed,disabled`.
- `D313-SOURCE-DIAGNOSTIC=PASS status=passed window_shown=False event_loop_entered=False`.
- `D313-MANIFEST-COPY=PASS bytes=38601532 sha=BD35A8812EBE6FA411BB74B387C698EA00F5F73C2EECA79A864A2587F557D1A4`.
- `D313-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D313-PE-ARCHIVE=PASS outer_entries=166 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1`.
- Root `QuillForge.exe` and `dist/QuillForge.exe` are byte-identical.

## Unrun checks and reason

Native EXE launch, Qt arrow rendering, stepper clicks, keyboard/accessibility
output, DPI/scaling, alternate style engines, clean-machine behavior, real
DLL-loader behavior, signing, installer, updater, and release owner gates
remain unrun because the project policy sets `software_start_allowed=false`
and this change does not authorize release operations.

## Known risks and limits

QSS subcontrol painting can vary by native Qt style engine and platform
metrics. The static matrix proves token contrast, not pixel-level arrow
appearance or click timing. The source diagnostic still emits the existing
non-fatal Qt font-directory warning because the development PyQt6 installation
does not ship fonts. The package remains an unsigned portable candidate and
the release verifier is expected to remain NO-GO on its existing report-binding
and native startup gates.

## Acceptance and evidence IDs

- Delivery: `D313 / UI-131 / ARCH-283`.
- Scenario: `S353`.
- Reviews: parent `PASS`, independent `NO_CONCLUSION`.
- Simplification: `PASS`.
- Public source: Qt 6.11 QSpinBox/QSS references and WCAG-style contrast;
  embedded vendor applicability N/A.

## Next owner and next action

Architect/project manager should authorize a separate native-rendering and
clean-machine verification pass before treating the portable candidate as a
release. The next UI slice should continue reusing semantic presentation
roles and the centralized theme owner.

## Artifact information

- `QuillForge.exe` and `dist/QuillForge.exe`
- SHA-256: `BD35A8812EBE6FA411BB74B387C698EA00F5F73C2EECA79A864A2587F557D1A4`
- Size: `38,601,532` bytes
- Source revision: `tree-sha256:331b143af6ce381542d2a4fb4d3a0766072670a8e4a8f2e0272b94a693525c44`
- Packaging: PyInstaller 6.22.0, Python 3.12.13, Windows x64 portable one-file candidate

## Disposition

Accepted with limits. The D313 source contract and package are recorded, but
the artifact is not a release approval while native startup and existing
enterprise release gates remain open.

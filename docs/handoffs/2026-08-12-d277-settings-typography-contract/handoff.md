# Handoff: 2026-08-12-d277-settings-typography-contract

| Field | Value |
|---|---|
| ID | `2026-08-12-d277-settings-typography-contract` |
| Delivery / slice | `D277 / ARCH-247 Settings typography contract audit` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The requested language, theme/accent, interface/editor font family, size,
style, persistence, live application, and motion-transition paths now have an
end-to-end regression contract.

## Scope and boundaries

- Added `_audit_settings_typography_contract()` to the existing Qt-free
  presentation audit.
- Covered nine source routes from settings validation/storage through the
  settings form, save projection, global QSS, editor adapter, and localized
  labels.
- No settings values, schema behavior, save order, QSS selectors, editor
  behavior, or motion policy changed.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Dirac consultation | Boundary, ownership, and integration review |
| Developer | parent | Qt-free settings/typography contract audit |
| QA | parent | Contract, static, package, and handoff verification |
| Independent reviewer | Mencius consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- D277 ADR, review, plan, roadmap, register, acceptance, and handoff records

## Decisions and constraints

- Keep settings validation/persistence in the application/infrastructure
  boundary and UI projection in presentation modules.
- Keep QSS typography in `theme.py` and editor typography in `EditorWidget`.
- No new tests, mocks, fixtures, harnesses, or test-only assets were created.
- Embedded workflow is not applicable to this Python/PyQt6 desktop change.

## Verification commands and results

| Evidence | Result |
|---|---|
| Typography/settings contract | `D277-TYPOGRAPHY-CONTRACT=PASS routes=9` |
| Presentation contract audit | `PASS` |
| Compileall | `D277-COMPILEALL=PASS` |
| Ruff and formatting | `PASS` |
| Project check | `PASS` |
| PE/archive | `D277-PE-ARCHIVE=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI required=8` |
| Root/dist identity | `D277-MANIFEST-COPY=PASS bytes=38584188 sha=166FDFBB9A294793720EB9E9384DB19F9E03B9857A7CA737274C91DF9F587DCD` |
| Release handoff | Current artifact identity matches; expected no-go retains only unrun artifact-bound startup/performance checks |

## Unrun checks and reason

Native EXE/Qt startup, native font rendering, installed-font fallback,
accessibility, clean-machine behavior, signing, installer/update, registry,
cross-machine repeatability, and release-owner acceptance remain unrun under
the active no-launch and non-destructive policy.

## Known risks and limits

The source contract does not prove native font metrics, DPI behavior, widget
selector specificity, screen-reader output, or startup success. Independent
review returned `NO_CONCLUSION`; no independent PASS is claimed.

## Acceptance and evidence IDs

- Acceptance: `S317`.
- Architecture slice: `ARCH-247`.
- Evidence: `D277-TYPOGRAPHY-CONTRACT=PASS`, `D277-CHECK=PASS`,
  `D277-MANIFEST-IDENTITY=PASS`,
  `D277-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D277-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D277-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: perform native settings changes and visual/font confirmation with
  the current root candidate; attach the diagnostic if startup fails.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `166FDFBB9A294793720EB9E9384DB19F9E03B9857A7CA737274C91DF9F587DCD` /
  `38,584,188` bytes.
- Source revision:
  `tree-sha256:bb6627070a1e74f44af9aa88242eef79eec3da5229ced50afc121b4ce3b2820a`.

## Disposition

`accepted-with-limits`: settings/typography source boundaries are guarded and
the candidate is package-bound; native runtime and enterprise release gates
remain open.

## Review and architecture

- Parent review: `PASS`.
- Architecture role `Dirac the 7th / Luna max`: `NO_CONCLUSION`.
- Independent review `Mencius the 7th / Luna max`: `NO_CONCLUSION`.
- Simplification assessment: `PASS`.
- Public-source applicability: Python 3.12 first-party dataclasses/typing
  behavior and existing settings/theme/editor port contracts; no manufacturer
  requirement applies.

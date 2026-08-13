# Handoff: 2026-08-12-d275-qss-semantic-contrast-contract

| Field | Value |
|---|---|
| ID | `2026-08-12-d275-qss-semantic-contrast-contract` |
| Delivery / slice | `D275 / ARCH-245 QSS semantic contrast contract` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The 砂金/readability protection now covers the actual derived QSS foregrounds,
not only the base theme tokens. Accent-alt, selection, warning, success,
working, and error surfaces use one contrast-safe resolver across every
supported theme and accent choice.

## Scope and boundaries

- Added `QssForegroundTokens` and `qss_foreground_tokens()` to
  `src/quillforge/presentation/theme_tokens.py`.
- Reused that resolver in `theme.py` palette/QSS projection and the existing
  Qt-free presentation audit.
- Covered 7 QSS semantic foreground/background pairs × 3 themes × 4 accents.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Leibniz consultation | Boundary, ownership, and integration review |
| Developer | parent | Token resolver and audit integration |
| QA | parent | Contrast matrix, static checks, package, and handoff verification |
| Independent reviewer | Planck consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/theme_tokens.py`
- `src/quillforge/presentation/theme.py`
- `scripts/audit_presentation_contracts.py`
- D275 ADR, review, plan, roadmap, register, acceptance, and handoff records

## Decisions and constraints

- Keep the visual policy in the framework-neutral token layer.
- Preserve QSS selectors, palette roles, settings, editor behavior, and
  application/service seams.
- No new tests, mocks, fixtures, harnesses, or test-only assets were created.
- Embedded workflow is not applicable to this Python/PyQt6 desktop change.

## Verification commands and results

| Evidence | Result |
|---|---|
| QSS semantic contrast matrix | `D275-QSS-CONTRAST=PASS checks=84 failures=0` |
| Presentation contract audit | `PASS` |
| Compileall | `D275-COMPILEALL=PASS` |
| Ruff and formatting | `PASS` |
| Project check | `PASS` |
| PE/archive | `D275-PE-ARCHIVE=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI required=8` |
| Root/dist identity | `D275-MANIFEST-COPY=PASS bytes=38583820 sha=CBDA1FDADCF25C143D0B24F36230DB9B5194722F63BBEC87670E4DA29340C944` |
| Release handoff | Current artifact identity matches; expected no-go retains only unrun artifact-bound startup/performance checks |

## Unrun checks and reason

Native EXE/Qt startup, native rendering, native dialogs, clean-machine
behavior, signing, installer/update, registry, cross-machine repeatability,
and release-owner acceptance remain unrun under the active no-launch and
non-destructive policy.

## Known risks and limits

Token-level contrast does not prove native Qt selector specificity, font
metrics, DPI behavior, installed-font coverage, or accessibility output.
Independent review returned `NO_CONCLUSION`; no independent PASS is claimed.

## Acceptance and evidence IDs

- Acceptance: `S315`.
- Architecture slice: `ARCH-245`.
- Evidence: `D275-QSS-CONTRAST=PASS`, `D275-CHECK=PASS`,
  `D275-MANIFEST-IDENTITY=PASS`,
  `D275-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D275-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D275-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: perform native startup and visual checks with the current root
  candidate; if startup still fails, attach the newest startup diagnostic.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `CBDA1FDADCF25C143D0B24F36230DB9B5194722F63BBEC87670E4DA29340C944` /
  `38,583,820` bytes.
- Source revision:
  `tree-sha256:bbba300cd7a84d7ac7e06e3aff10c6dd028d8d9304ecb97cecbef54075a5c99d`.

## Disposition

`accepted-with-limits`: QSS semantic contrast ownership and static/package
verification are complete; native runtime and enterprise release gates remain
open.

## Review and architecture

- Parent review: `PASS`.
- Architecture role `Leibniz the 7th / Luna max`: `NO_CONCLUSION`.
- Independent review `Planck the 7th / Luna max`: `NO_CONCLUSION`.
- Simplification assessment: `PASS`.
- Public-source applicability: Python 3.12 first-party dataclasses/typing
  behavior and the existing project theme-token contract; no manufacturer
  requirement applies.

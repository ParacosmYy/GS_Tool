# Handoff: 2026-08-11-d178-message-dialog-action-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d178-message-dialog-action-hierarchy` |
| Delivery / slice | `D178 / UI-90 / ARCH-165 Message dialog action hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T21:00:00+08:00` |

## User outcome

Common message dialogs now expose a readable action hierarchy: Save is the
primary action, Discard is visibly cautionary, Cancel is quiet, and About/Error
dialogs have an explicit quiet OK action. Existing message decisions and
localization remain unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/message_surface.py` standard-button role
  projection.
- Reuse of existing `primaryAction`, `warningAction`, and `quietAction`
  selectors in `src/quillforge/presentation/theme.py`.
- Source, static, package, manifest, handoff, and expected release no-go
  evidence.

### Out of scope

- No close policy, recovery policy, notification policy, locale catalog,
  application/domain coordinator, theme-token derivation, or business
  decision changed.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Confucius the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Boyle the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/message_surface.py` — standard-button role
  projection and explicit OK actions.
- D178 ADR, review records, handoff, and traceability files.

## Decisions and constraints

- `MessageSurface` owns message-box composition; `theme.py` owns visual role
  projection; close/recovery/application policy stays where it was.
- Existing token-derived foregrounds remain authoritative, including the
  砂金/warning role.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D178-MESSAGE-ACTION-SOURCE-PROBE=PASS`.
- `D178-MESSAGE-THEME-ROLE-CONTRACT-PROBE=PASS`.
- `D178-MESSAGE-BEHAVIOR-SOURCE-PROBE=PASS`.
- `D178-COMPILE-RUFF-FORMAT=PASS`.
- `D178-CHECK=PASS`, `D178-VERIFY-HANDOFF=PASS`.
- `D178-PACKAGE-BUILD=PASS`.
- `D178-PACKAGE-IDENTITY-PROBE=PASS`.
- `D178-MANIFEST-TRACEABILITY-PROBE=PASS`.
- `D178-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`; release verifier remains
  expected `NO-GO` with ten open gates and three mechanical consistency
  failures.

## Unrun checks and reason

- Native Qt message-box layout/painting, metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, release-owner, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt style/layout engines may vary in button order, width, wrapping, or
  role painting across platforms; native visual review remains open.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S231`, `D178-AC01`.
- Evidence: ADR-0227, parent/independent review records, D178 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of confirmation/error/about
  button order, localized widths, keyboard default/focus, DPI behavior, and
  screen-reader naming before closing runtime gates.

## Artifact information

The D178 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `5D7946223BEC98EF8178DABAA874549320436CE279D86AD70EC636F116561C90`.
- Size: `38549008` bytes.
- Source revision: `tree-sha256:48accb0eaf47a97e606323062c33bc331692a60e376e91050633292453732007`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: message-dialog action hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.

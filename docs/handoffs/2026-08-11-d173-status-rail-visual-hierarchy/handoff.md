# Handoff: 2026-08-11-d173-status-rail-visual-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d173-status-rail-visual-hierarchy` |
| Delivery / slice | `D173 / UI-85 / ARCH-160 Status-rail visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T16:30:00+08:00` |

## User outcome

The bottom status region now has a calmer shell surface, a distinct transient
message capsule, and a clearer permanent shell-status rail. Phase pills have
more breathing room while all message and phase meanings remain unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` status-bar/message/rail QSS.
- Outer status surface, message capsule, phase capsule, and context spacing.
- Static selector and contrast projection across supported themes and accents.

### Out of scope

- No `status_surface.py`, `status_bar.py`, notification contract, timer,
  localization, phase precedence, state machine, application, or persistence
  change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Chandrasekhar the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Poincare the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized status visual hierarchy
  only.
- `docs/adr/0222-status-rail-visual-hierarchy.md`.
- D173 parent/independent review records and traceability files.

## Decisions and constraints

- `StatusSurface`/`StatusRail` remain the owners of status projection and
  lifecycle semantics; `theme.py` owns only visual rules.
- Existing readable foreground derivation remains authoritative for 砂金 and
  paper-sand endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D173-COMPILEALL=PASS`.
- `D173-RUFF=PASS` and `D173-FORMAT=PASS`.
- `D173-QSS-CONTRACT-PROBE=PASS`.
- `D173-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective minimum
  was 4.97 for message and phase state projections.
- `D173-PACKAGE-BUILD=PASS`.
- `D173-PACKAGE-IDENTITY-PROBE=PASS`.
- `D173-CHECK=PASS`, `D173-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt status-bar painting/layout, metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different status-bar height and capsule spacing
  than the static projection; runtime visual review remains open.
- Larger user-selected fonts may change the status message width; native DPI
  and font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S226`, `D173-AC01`.
- Evidence: ADR-0222, parent/independent review records, D173 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of status-bar height, message
  truncation, localized widths, screen-reader output, and DPI behavior before
  closing runtime gates.

## Artifact information

The D173 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `315FD62225BA2D3402EAB8697C870BA6C514BC0D5FDA020230180E4B153FA602`.
- Size: `38551243` bytes.
- Source revision: `tree-sha256:19dd315c12b2af54791344fa571be59df59187dbd6998caa968d21a83154f0c9`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: status-rail visual hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.

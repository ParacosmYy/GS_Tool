# Handoff: 2026-08-11-d164-status-message-visual-weight

| Field | Value |
|---|---|
| ID | `2026-08-11-d164-status-message-visual-weight` |
| Delivery / slice | `D164 / UI-77 status-message visual weight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T04:20:00+08:00` |

## User outcome

Transient status messages now have a readable semibold visual anchor in the
status rail, making information and feedback easier to distinguish without
changing language, message content, severity, or notification behavior.

## Scope and boundaries

### In scope

- One centralized `QLabel#statusMessage` QSS base declaration.
- Static scope, AST, independent review, simplification, package, and
  traceability evidence.

### Out of scope

- No status text, locale catalog, feedback state, timer, visibility, tooltip,
  layout, size policy, notification policy, or application behavior changed.
- No QApplication/EXE launch, screenshot, native rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Faraday the 5th / Luna max | `PASS` for bounded presentation scope |
| Independent review | Hubble the 5th / Luna max | `PASS` for source/state/lifecycle preservation |
| Parent | Architect | `PASS`; sole writer, integration, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — shared status-message QSS weight.
- `docs/adr/0213-status-message-visual-weight.md`.
- `docs/agent-team/reviews/D164-status-message-visual-weight-parent-review.md`.
- `docs/agent-team/reviews/D164-status-message-visual-weight-independent-review.md`.
- `docs/support/HANDOFF.md` — local support packet prepared during the release audit.
- Traceability files under `docs/`, `tasks/`, and `docs/handoffs/index.json`.

## Decisions and constraints

- The existing `statusMessage` selector and state-property contract remain the
  single presentation boundary.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, package, and handoff
  checks are the authorized validation boundary.

## Verification commands and results

- `D164-AST-PROBE=PASS`.
- `D164-STATUS-MESSAGE-QSS-PROBE=PASS`.
- `D164-SINGLE-SCOPE-PROBE=PASS`.
- `D164-INDEPENDENT-REVIEW=PASS`.
- `D164-SIMPLIFICATION-ASSESSMENT=PASS`.
- `scripts/check.ps1` and package identity are recorded after the final docs
  and artifact synchronization.
- Expected release `NO-GO`; stale artifact-bound runtime reports and external
  release gates remain explicit.

## Unrun checks and reason

- Qt/EXE startup, native font rendering, screenshots, DPI, size hints, text
  clipping, and cross-machine appearance — prohibited by the active no-launch
  policy or outside the current checkout.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  project policy and not created.
- Signing, installer, updater, clean-machine, legal, support acceptance,
  permission/disk-pressure, hard-power, and cross-machine gates — require
  external authorization, tools, or environments not present here.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

The semibold weight may affect native font metrics or wrapping on a specific
platform. It does not prove rendered visual quality, but it is isolated enough
to roll back as one declaration if authorized runtime review finds clipping or
unexpected status-bar growth. No formal accessibility, certification, or
release-readiness claim is made.

## Acceptance and evidence IDs

- Acceptance: `S217`, `D164-AC01`.
- Evidence: ADR-0213, parent/independent review records, D164 probes,
  `scripts/check.ps1`, package identity, handoff/index/register checks, and
  expected release `NO-GO`.

## Next owner and next action

- Owner: Architect.
- Action: perform the authorized runtime visual review when the no-launch
  policy is explicitly reversed; otherwise continue only with bounded static
  UI/architecture increments and retain the release no-go boundary.

## Artifact information

The portable candidate was rebuilt after source and traceability
synchronization without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `932E32F2520C61A145CCB5E24CBFD803A0273DFC3AB67C08ECA43F6631D98266`.
- Size: `38547674` bytes.
- Source revision: `tree-sha256:cf88c0bf45dad502147be84bbf75abe75c366bdbe61140ca0982588ee42ab449`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: status-message hierarchy is improved through one
centralized semibold rule; native rendering and all external release gates
remain open.

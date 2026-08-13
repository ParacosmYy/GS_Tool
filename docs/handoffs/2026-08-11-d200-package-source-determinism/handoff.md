# Handoff: 2026-08-11-d200-package-source-determinism

| Field | Value |
|---|---|
| ID | `2026-08-11-d200-package-source-determinism` |
| Delivery / slice | `D200 / ARCH-186 Package source-revision determinism` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Packaging now produces the same source-revision hash under Windows PowerShell
5.1 and PowerShell 7 for the same source contents. The earlier culture-sort
drift that weakened artifact traceability is removed; application behavior is
unchanged.

## Scope and boundaries

### In scope

- Stable ordinal ordering of existing `relative-path=SHA-256` source lines in
  `scripts/package.ps1`.
- Dual-shell parser/build, source-line identity, manifest, and artifact checks.

### Out of scope

- EXE launch, GUI/QApplication, screenshots, runtime measurement, installer,
  updater, registry/file association, network, signing, legal, support,
  clean-machine, cross-machine, permission/disk-pressure, hard-power, and
  release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Reproducible enterprise package provenance |
| Developer | `parent` | PowerShell source-revision boundary |
| QA | `parent` | Cross-shell parser, provenance, package, and handoff checks |

## Changed files and modules

- `scripts/package.ps1` — ordinal source-line ordering.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized release records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded packaging slice.
- Runtime launch policy: not allowed; no GUI, QApplication, EXE, screenshot,
  or test-only asset was run/created.
- Architecture window: `Carver the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Independent review: `Hubble the 6th / Luna max` — `PASS` for the bounded
  packaging-provenance slice; the review confirmed the ordinal comparer,
  166-entry manifest revision, and unchanged cleanup/atomic-copy boundaries.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Cross-shell source-line reproduction | `PASS` then `FAIL` before fix | Prior run identified 11 culture-order differences. |
| Ordinal-sort prototype | `PASS` | `D200-STABLE-SORT-PROTOTYPE=PASS shells_identical`. |
| Windows PowerShell 5.1 parser | `PASS` | `D200-PS51-PARSE-PROBE=PASS`. |
| PowerShell 7 parser | `PASS` | `D200-PS7-PARSE-PROBE=PASS`. |
| Deterministic-sort source probe | `PASS` | `D200-DETERMINISTIC-SORT-SOURCE-PROBE=PASS`. |
| `scripts/package.ps1` under Windows PowerShell 5.1 | `PASS` | Artifact SHA `0E71019B81B5DD21B294D5634D785C04615417091C81A9606B617C3C0A0A205E`, 38,562,010 bytes; source revision `tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`. |
| `scripts/package.ps1` under PowerShell 7 | `PASS` | Artifact SHA `DC3C33DE56AA0FED10E04771A539AC5C0B2869996ABCA58E7CFE12BB42E3FC31`, 38,561,232 bytes; same source revision. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |

## Unrun checks and reason

- EXE startup, GUI/native rendering, packaged measurement, clean-machine,
  cross-machine, installer/updater, registry/file association, signing, legal,
  support, permission/disk-pressure, hard-power, and release-owner checks —
  prohibited by the active no-launch policy or require external authorization.

## Known risks and limits

- PyInstaller artifacts from separate shell invocations are not byte-identical;
  each manifest binds its own artifact and both source revisions now agree.
- The checkout has no Git baseline; the architecture consultation returned
  `NO_CONCLUSION`, while the independent review is a bounded `PASS` only.
- Artifact-bound runtime reports remain stale and keep release status at no-go.

## Acceptance and evidence IDs

- Acceptance: `S252`
- Evidence: `D200-STABLE-SORT-PROTOTYPE=PASS shells_identical`,
  `D200-PS51-PARSE-PROBE=PASS`, `D200-PS7-PARSE-PROBE=PASS`,
  `D200-DETERMINISTIC-SORT-SOURCE-PROBE=PASS`,
  `D200-COMPILE-RUFF-FORMAT=PASS`, `D200-PACKAGE-BUILD-PS51=PASS`,
  `D200-PACKAGE-BUILD-PS7=PASS`, `D200-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D200-INDEPENDENT-REVIEW=PASS bounded packaging-provenance slice`.

## Next owner and next action

- Owner: QA / Product
- Action: use the shared source revision when binding future runtime reports;
  authorize runtime and external release evidence separately.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `DC3C33DE56AA0FED10E04771A539AC5C0B2869996ABCA58E7CFE12BB42E3FC31` /
  `38,561,232` bytes
- Source revision: `tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`
- Packaging note: the final current candidate is the PowerShell 7 package;
  the PowerShell 5.1 package identity is recorded as an independently bound
  successful build.

## Disposition

`accepted-with-limits`: cross-shell source-revision determinism and current
package identity are recorded; runtime and enterprise release gates remain
open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

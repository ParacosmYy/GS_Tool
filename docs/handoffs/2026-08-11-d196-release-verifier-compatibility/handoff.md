# Handoff: 2026-08-11-d196-release-verifier-compatibility

| Field | Value |
|---|---|
| ID | `2026-08-11-d196-release-verifier-compatibility` |
| Delivery / slice | `D196 / ARCH-182 Release verifier PowerShell compatibility` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The release dossier verifier now runs under Windows PowerShell 5.1 and
PowerShell 7, correctly decodes the existing UTF-8 reports, writes the current
dossier, and returns the same expected no-go mechanical failures in both
shells. It does not refresh historical reports or close release gates.

## Scope and boundaries

### In scope

- `ConvertFrom-JsonCompat` in `scripts/verify_release_handoff.ps1`.
- Explicit UTF-8 reads for the manifest and four JSON reports.
- Two-shell verifier, parser, static, dossier, and package evidence.

### Out of scope

- Report refresh, EXE startup, GUI/native rendering, installer, updater,
  registry/file associations, network, clean-machine, cross-machine, signing,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Consistent Windows-first release evidence tooling |
| Developer | `parent` | JSON compatibility and encoding boundary |
| QA | `parent` | Two-shell verifier, static, dossier, and package verification |

## Changed files and modules

- `scripts/verify_release_handoff.ps1` — JSON compatibility adapter and UTF-8
  report reads.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized architecture/release
  records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded tooling slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, registry, or
  test-only asset was run/created.
- Architecture consultation: `Helmholtz the 6th / Luna max` and follow-up
  `Franklin the 6th / Luna max` — both `NO_CONCLUSION` after two bounded waits;
  no child PASS is claimed.
- Independent review: `Euler the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Windows PowerShell 5.1 parser probe | `PASS` | `D196-PS51-PARSE-PROBE=PASS`. |
| PowerShell 7 parser probe | `PASS` | `D196-PS7-PARSE-PROBE=PASS`. |
| Compatibility source probe | `PASS` | `D196-COMPAT-SOURCE-PROBE=PASS`; one adapter and five UTF-8 reads. |
| Windows PowerShell 5.1 verifier | `EXPECTED NO-GO` | UTF-8 Chinese JSON decoded; three mechanical failures, ten open gates. |
| PowerShell 7 verifier | `EXPECTED NO-GO` | Same three mechanical failures and ten open gates. |
| Same-failure/dossier invariant probe | `PASS` | `D196-SAME-MECHANICAL-FAILURES-PROBE=PASS`; current identity bound. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `scripts/check.ps1` under 5.1 and 7 | `PASS` | Existing static and handoff checks passed. |
| `scripts/package.ps1` under 5.1 and 7 | `PASS` | Package refreshed after source change; EXE not launched. |

## Unrun checks and reason

- EXE startup, GUI/native rendering, performance-report refresh, installer/
  updater, registry/file association, accessibility, clean-machine,
  cross-machine, signing, legal, support, permission/disk-pressure,
  hard-power, and release-owner checks — prohibited or require external
  authorization/evidence.

## Known risks and limits

- Windows PowerShell 5.1's JSON conversion differs from PowerShell 7 for date
  handling; the adapter keeps 7's string behavior and avoids date fields in
  5.1 while preserving all fields used by the verifier.
- The existing historical reports remain artifact-stale by design; their
  mechanical failures are recorded, not suppressed.
- Existing enterprise release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S248`
- Evidence: `D196-PS51-PARSE-PROBE=PASS`, `D196-PS7-PARSE-PROBE=PASS`,
  `D196-COMPAT-SOURCE-PROBE=PASS`, `D196-PS51-VERIFIER=EXPECTED-NO-GO`,
  `D196-PS7-VERIFIER=EXPECTED-NO-GO`,
  `D196-SAME-MECHANICAL-FAILURES-PROBE=PASS`,
  `D196-COMPILE-RUFF-FORMAT=PASS`, `D196-PRESENTATION-AUDIT=PASS`,
  `D196-PACKAGE-IDENTITY-PROBE=PASS`,
  `D196-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D196-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize fresh artifact-bound runtime/performance evidence and
  external release-owner gates before enterprise release acceptance.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `C4453735E181565DD6B0A584FA6A49009CA098B82CE8BE531106B9B029692713` /
  `38,562,113` bytes
- Source revision: `tree-sha256:3cf08017d895dc48ad0fcde924eb1e4917fbe9e13993dca82bd8eacca5eb48c1`
- Packaging note: portable PyInstaller one-file candidate rebuilt under both
  shells after the verifier source change; no installer or updater artifact is
  claimed.

## Disposition

`accepted-with-limits`: cross-shell UTF-8 JSON parsing, identical expected
no-go verifier behavior, package identity, and static handoff evidence are
recorded; runtime and enterprise release gates remain open.

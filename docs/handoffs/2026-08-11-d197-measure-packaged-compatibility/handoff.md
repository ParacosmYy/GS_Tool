# Handoff: 2026-08-11-d197-measure-packaged-compatibility

| Field | Value |
|---|---|
| ID | `2026-08-11-d197-measure-packaged-compatibility` |
| Delivery / slice | `D197 / ARCH-183 Packaged measurement PowerShell compatibility` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The packaged-capture evidence script now has the same Windows PowerShell 5.1/
PowerShell 7 JSON compatibility boundary as the package and release verifier
tools. It is statically validated and packaged, but no EXE or measurement run
was started.

## Scope and boundaries

### In scope

- `ConvertFrom-JsonCompat` in `scripts/measure_packaged.ps1`.
- Explicit UTF-8 reads for the release manifest and per-run capture report.
- Static, parser, handoff, and package evidence.

### Out of scope

- EXE launch, Qt offscreen capture, performance measurement, report refresh,
  cleanup execution, installer, updater, registry/file association,
  clean-machine, cross-machine, signing, legal, support, and release-owner
  acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Complete Windows-first evidence tooling |
| Developer | `parent` | Packaged measurement JSON compatibility boundary |
| QA | `parent` | Parser, static, handoff, and package verification |

## Changed files and modules

- `scripts/measure_packaged.ps1` — JSON compatibility adapter and UTF-8 reads.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized architecture/release
  records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded tooling slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, process
  measurement, or test-only asset was run/created.
- Architecture window: `Kierkegaard the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child PASS is claimed.
- Independent review: `Planck the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Windows PowerShell 5.1 parser probe | `PASS` | `D197-PS51-PARSE-PROBE=PASS`. |
| PowerShell 7 parser probe | `PASS` | `D197-PS7-PARSE-PROBE=PASS`. |
| Compatibility source probe | `PASS` | `D197-COMPAT-SOURCE-PROBE=PASS`; one adapter and two UTF-8 reads. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `scripts/check.ps1` under 5.1 and 7 | `PASS` | Existing static and handoff checks passed. |
| `scripts/verify_handoff.ps1` under 5.1 and 7 | `PASS` | Handoff checks passed. |
| `scripts/package.ps1` under 5.1 and 7 | `PASS` | Package refreshed; EXE not launched. |
| `scripts/measure_packaged.ps1` | `NOT RUN` | Launch/measurement prohibited by current policy. |

## Unrun checks and reason

- Packaged capture, EXE startup, GUI/native rendering, performance reports,
  clean-machine, cross-machine, installer/updater, registry/file association,
  accessibility, signing, legal, support, permission/disk-pressure,
  hard-power, and release-owner checks — prohibited or require external
  authorization/evidence.

## Known risks and limits

- PowerShell 5.1 and 7 can differ in JSON date materialization; the only
  runtime-consumed fields in this script are artifact/capture data, and the
  7+ string-date behavior is preserved.
- Existing historical performance reports remain stale and are not refreshed.
- Existing enterprise release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S249`
- Evidence: `D197-PS51-PARSE-PROBE=PASS`, `D197-PS7-PARSE-PROBE=PASS`,
  `D197-COMPAT-SOURCE-PROBE=PASS`, `D197-COMPILE-RUFF-FORMAT=PASS`,
  `D197-PRESENTATION-AUDIT=PASS`, `D197-PACKAGE-IDENTITY-PROBE=PASS`,
  `D197-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D197-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize packaged runtime capture and artifact-bound performance
  evidence before treating this tool as runtime validated.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `D07A3578ACF9646DB701FF3895BEADD234F793A8E401C774070406E93D29034F` /
  `38,562,232` bytes
- Source revision: `tree-sha256:ff054295d2fe9c610e98c034b876e9929e98e0d86a297545c1e733a79bba56bf`
- Packaging note: portable PyInstaller one-file candidate rebuilt under both
  shells; no installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: packaged measurement JSON compatibility, static checks,
and package identity are recorded; measurement runtime, native UI, and
enterprise release gates remain open.

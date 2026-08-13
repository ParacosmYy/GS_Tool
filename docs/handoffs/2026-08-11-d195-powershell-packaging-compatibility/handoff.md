# Handoff: 2026-08-11-d195-powershell-packaging-compatibility

| Field | Value |
|---|---|
| ID | `2026-08-11-d195-powershell-packaging-compatibility` |
| Delivery / slice | `D195 / ARCH-181 Windows PowerShell packaging compatibility` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The documented packaging entry point now completes under both Windows
PowerShell 5.1 and PowerShell 7. The release manifest retains the same source
inventory and SHA-256 provenance contract; no installer, registry, network, or
runtime behavior was added.

## Scope and boundaries

### In scope

- Compatibility replacements in `scripts/package.ps1` for relative paths and
  source-revision hashing.
- PowerShell parser/package probes and the refreshed portable artifact.

### Out of scope

- Application behavior, UI, installer, updater, file associations, registry,
  network, EXE startup, clean-machine, cross-machine, signing, legal, support,
  permission/disk-pressure, hard-power, and release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Reliable Windows-first delivery entry point |
| Developer | `parent` | Packaging script compatibility boundary |
| QA | `parent` | Read-only parser, package, manifest, and handoff verification |

## Changed files and modules

- `scripts/package.ps1` — .NET Framework-compatible relative path and SHA-256
  helpers.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized architecture/release
  records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded tooling slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, registry, or
  test-only asset was run/created.
- Architecture window: `Archimedes the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child PASS is claimed.
- Independent review: `Sartre the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Windows PowerShell 5.1 parser probe | `PASS` | `D195-PS51-PARSE-PROBE=PASS`. |
| PowerShell 7 parser probe | `PASS` | `D195-PS7-PARSE-PROBE=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | `D195-PS51-PACKAGE=PASS`; EXE built and manifest written, not launched. |
| PowerShell 7 package | `PASS` | `D195-PS7-PACKAGE=PASS`; EXE built and manifest written, not launched. |
| Compatibility source probe | `PASS` | `D195-POWERSHELL-COMPAT-SOURCE-PROBE=PASS`; unavailable APIs absent. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `scripts/check.ps1` under 5.1 and 7 | `PASS` | Existing static and handoff checks passed in both shells. |
| `scripts/verify_handoff.ps1` under 5.1 and 7 | `PASS` | Handoff checks passed in both shells. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | 10 external/open gates and three stale artifact-bound consistency failures remain. |

## Unrun checks and reason

- EXE startup, GUI/native rendering, installer/update, registry/file
  association, accessibility, clean-machine, cross-machine, signing, legal,
  support, permission/disk-pressure, hard-power, and release-owner checks —
  prohibited or require external authorization/evidence.

## Known risks and limits

- PyInstaller artifact bytes can vary between separate local rebuilds; each
  shell run still verified its own root/dist identity and manifest binding.
- URI-relative path generation assumes the existing source inventory remains
  under the checkout root; the script already constructs that inventory from
  root-owned paths.
- Existing enterprise release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S247`
- Evidence: `D195-PS51-PARSE-PROBE=PASS`, `D195-PS7-PARSE-PROBE=PASS`,
  `D195-PS51-PACKAGE=PASS`, `D195-PS7-PACKAGE=PASS`,
  `D195-POWERSHELL-COMPAT-SOURCE-PROBE=PASS`,
  `D195-COMPILE-RUFF-FORMAT=PASS`, `D195-PRESENTATION-AUDIT=PASS`,
  `D195-PACKAGE-IDENTITY-PROBE=PASS`,
  `D195-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D195-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize external release-owner, clean-machine, signing,
  installer/update, and runtime evidence before release acceptance.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `01D055FA3790DCF2B0EB8D6C6378B14BCEE54D4073743CD5DD430435DBB2D499` /
  `38,557,931` bytes
- Source revision: `tree-sha256:8f3f60ff67876032ad7407f2f26a3a34bcc452b3c7750476bd2ade188f1de5ca`
- Packaging note: portable PyInstaller one-file candidate rebuilt under both
  shells; no installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: Windows PowerShell 5.1/7 packaging compatibility,
manifest identity, and static handoff evidence are recorded; runtime and
enterprise release gates remain open.

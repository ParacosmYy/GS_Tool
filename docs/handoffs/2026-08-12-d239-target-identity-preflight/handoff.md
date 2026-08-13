# Handoff: 2026-08-12-d239-target-identity-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d239-target-identity-preflight` |
| Delivery / slice | `D239 / ARCH-220 Target identity preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D239 / ARCH-220 handoff — target identity preflight

## User outcome

The local updater now refuses to replace an executable that no longer matches
the install state. Rollback likewise refuses a changed target or backup before
performing any file exchange.

## Scope and boundaries

Changed only `packaging/update.ps1`. Existing D238 recovery, local path
containment, SHA-256 validation, state schema, portable EXE, UI, locale, font,
theme, and association policy remain otherwise unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Architecture window: Parfit the 7th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Post-fix independent window: Dalton the 7th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `packaging/update.ps1`: target/backup identity preflight and path separation
  before any update or rollback move.
- `docs/adr/0283-target-identity-preflight.md` and review records.

## Decisions and constraints

The existing `Get-QfArtifactInfo` function remains the single hash validator.
Preflight runs after `ShouldProcess` and before `Move-Item`; a mismatch stops
without attempting recovery moves because no move has started.

## Verification commands and results

| Evidence | Result |
|---|---|
| PowerShell 5.1 AST parse | `D239-PS51-AST=PASS` |
| PowerShell 7 AST parse | `D239-PS7-AST=PASS` |
| Target identity static contract | `D239-STATE-IDENTITY-STATIC-CONTRACT=PASS` |
| Distribution boundary audit | `D239-DISTRIBUTION-BOUNDARY-AUDIT=PASS` |
| Startup archive audit | `D239-STARTUP-ARCHIVE-AUDIT=PASS` |
| Python compile | `D239-COMPILEALL=PASS` |
| Ruff | `D239-RUFF=PASS` |
| Presentation audit | `D239-PRESENTATION-AUDIT=PASS` |
| PS5.1 package build | `PASS`, SHA `C0CD76F3F60AE98F1A44CB803DD52C9DCE309AFE3BFDB39EC05BE095AC31581F` |
| PS7 package build | `PASS`, final SHA `D1F6308C5EDD45B79BA9131B784029483C01FD603E121958B29365F5779B9C37` |
| Package identity | `D239-PACKAGE-IDENTITY-PROBE=PASS`, 38,572,229 bytes, dist/root match |
| PyInstaller archive/PYZ | `D239-PACKAGE-ARCHIVE=PASS`, 432 entries, 142 QuillForge modules |
| PyInstaller warning scope | `D239-PYINSTALLER-WARNING-SCOPE=PASS` |
| Project checks | `D239-CHECK-PS51=PASS`; `D239-CHECK-PS7=PASS` |
| Handoff verifier | `D239-HANDOFF-PS51=PASS`; `D239-HANDOFF-PS7=PASS` |
| Release verifier | `D239-RELEASE-VERIFY=EXPECTED-NO-GO`, 10 open gates; three artifact-bound report consistency failures require authorized startup |

## Unrun checks and reason

Updater/rollback execution, changed-target failure injection, file-lock and
permission pressure, concurrent modification, native EXE/Qt startup,
clean-machine, signing, and release-owner checks remain unrun under the
no-launch/non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

The identity check narrows the race window but cannot provide an OS-level lock
between hashing and `Move-Item`. If a file changes during that window, D238's
recovery guards still preserve uncertain content and warn rather than silently
overwriting it.

## Acceptance and evidence IDs

`S287`, `ARCH-220`, `D239-STATE-IDENTITY-STATIC-CONTRACT=PASS`,
`D239-PACKAGE-IDENTITY-PROBE=PASS`, `D239-PACKAGE-ARCHIVE=PASS`,
`D239-SIMPLIFICATION-ASSESSMENT=PASS`,
`D239-ARCHITECT-REVIEW=NO_CONCLUSION`,
`D239-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next authorized disposable-profile operational run of
update, rollback, and changed-target recovery.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `D1F6308C5EDD45B79BA9131B784029483C01FD603E121958B29365F5779B9C37`
- Bytes: `38,572,229`
- Source revision: `tree-sha256:3a7924bf47e0fc76cf09f923565c0334c9a132985ddb466900a34c890720cde0`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The preflight source boundary and portable package are
traceable; updater execution and native release gates remain open.

# Handoff: 2026-08-12-d238-update-transaction-safety

| Field | Value |
|---|---|
| ID | `2026-08-12-d238-update-transaction-safety` |
| Delivery / slice | `D238 / ARCH-219 Update transaction safety` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D238 / ARCH-219 handoff — update transaction safety

## User outcome

Local update and explicit rollback now preserve a consistent executable/backup
layout when state persistence fails, while refusing to overwrite a target that
changed outside the transaction.

## Scope and boundaries

Changed only `packaging/update.ps1`. The portable application, UI, locale,
font, theme, startup routing, association policy, and state schema are
unchanged. No installer/update channel or registry behavior is claimed.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Architecture window: Laplace the 7th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Post-fix independent window: Ramanujan the 7th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `packaging/update.ps1`: move/commit markers, hash-guarded ordinary-update
  recovery, and hash/path-guarded rollback recovery.
- `docs/adr/0282-update-transaction-safety.md` and review records.

## Decisions and constraints

`Write-QfState` remains the commit boundary. Before commit, recovery may move
only the known target/backup paths and only when the observed hash is the
expected new or previous artifact. An uncertain or externally changed file is
preserved with a warning.

## Verification commands and results

| Evidence | Result |
|---|---|
| PowerShell 5.1 AST parse | `D238-PS51-AST=PASS` |
| PowerShell 7 AST parse | `D238-PS7-AST=PASS` |
| Update transaction static contract | `D238-UPDATE-TRANSACTION-STATIC-CONTRACT=PASS` |
| Python compile | `D238-COMPILEALL=PASS` |
| Ruff | `D238-RUFF=PASS` |
| Presentation contract audit | `D238-PRESENTATION-AUDIT=PASS` |
| PS5.1 package build | `PASS`, SHA `BEB2A262E2031C68BB2169FAA8F8891D0ABDF608F23D247C76470E54484FB949` |
| PS7 package build | `PASS`, final SHA `587E8F437114474980D571BCD6A1CAB75E80E90386F1F47CA28C0CCF33F1AC15` |
| Package identity | `D238-PACKAGE-IDENTITY-PROBE=PASS`, 38,574,336 bytes, dist/root match |
| PyInstaller archive/PYZ | `D238-PACKAGE-ARCHIVE=PASS`, 432 entries, 142 QuillForge modules |
| PyInstaller warning scope | `D238-PYINSTALLER-WARNING-SCOPE=PASS` |
| Project checks | `D238-CHECK-PS51=PASS`; `D238-CHECK-PS7=PASS` |
| Handoff verifier | `D238-HANDOFF-PS51=PASS`; `D238-HANDOFF-PS7=PASS` |
| Release verifier | `D238-RELEASE-VERIFY=EXPECTED-NO-GO`, 10 open gates; three artifact-bound report consistency failures require authorized startup |

## Unrun checks and reason

Updater/rollback execution, state-write failure injection, permission pressure,
concurrent external modification, native EXE/Qt startup, clean-machine,
signing, and release-owner checks remain unrun under the no-launch and
non-destructive validation policy. No unit-test assets were created or run.

## Known risks and limits

PowerShell file moves and permissions can race with external processes. The
new guards reduce destructive recovery but cannot provide an OS-level
transaction across files and JSON state. In uncertain cases the script leaves
recoverable artifacts and requires operator inspection.

## Acceptance and evidence IDs

`S286`, `ARCH-219`, `D238-UPDATE-TRANSACTION-STATIC-CONTRACT=PASS`,
`D238-PACKAGE-IDENTITY-PROBE=PASS`, `D238-PACKAGE-ARCHIVE=PASS`,
`D238-SIMPLIFICATION-ASSESSMENT=PASS`,
`D238-ARCHITECT-REVIEW=NO_CONCLUSION`,
`D238-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next authorized disposable-profile operational run
for update, rollback, and state-write failure recovery.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `587E8F437114474980D571BCD6A1CAB75E80E90386F1F47CA28C0CCF33F1AC15`
- Bytes: `38,574,336`
- Source revision: `tree-sha256:cc55340d7d4a38b7e0e479ca545d10bba57692c979a081dc1ef5ef0d81f2a80e`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The updater source boundary and portable package are
traceable; updater execution and native release gates remain open.

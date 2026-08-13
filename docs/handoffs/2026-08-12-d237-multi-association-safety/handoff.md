# Handoff: 2026-08-12-d237-multi-association-safety

| Field | Value |
|---|---|
| ID | `2026-08-12-d237-multi-association-safety` |
| Delivery / slice | `D237 / ARCH-217 Multi-association safety` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D237 / ARCH-217 handoff — multi-association safety

## User outcome

The optional local installer can now register multiple extensions against one
QuillForge executable without failing on the second extension. User-modified
associations and unknown references are preserved, and incomplete cleanup no
longer deletes the executable that an association may still target.

## Scope and boundaries

Changed only the local PowerShell distribution boundary:
`packaging/QuillForge.Distribution.psm1`, `packaging/install.ps1`, and
`packaging/uninstall.ps1`. The portable EXE remains
`file_associations: not-configured`; the application never writes registry
state. No Python/UI/locale/font/theme behavior changed.

## Team roles and ownership

- Parent architect: integration, review, simplification, packaging, and handoff.
- Architecture window: Franklin the 7th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Initial independent window: Meitner the 7th / Luna max — found the pre-fix
  multi-extension and rollback defects; evidence used to drive the correction.
- Post-fix independent window: Anscombe the 7th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `packaging/QuillForge.Distribution.psm1`: explicit owned-ProgId reuse,
  idempotent extension cleanup, and HKCU reference scan before shared-key
  deletion.
- `packaging/install.ps1`: records the operation-owned ProgId and preserves the
  target on incomplete rollback.
- `packaging/uninstall.ps1`: stops before removing files/state when association
  cleanup is incomplete.
- `docs/adr/0281-multi-association-safety.md` and review records.

## Decisions and constraints

The first extension creates `QuillForge.Document`. Later extensions reuse it
only when the installer has already created it in the same operation and its
open command still matches. Cleanup is extension-first, ProgId-last, and
reference-aware. Registry mutation stays opt-in, HKCU-only, and
`SupportsShouldProcess`-guarded.

## Verification commands and results

| Evidence | Result |
|---|---|
| PowerShell 5.1 AST parse | `D237-PS51-AST=PASS` |
| PowerShell 7 AST parse | `D237-PS7-AST=PASS` |
| Distribution static ownership/rollback contract | `D237-DISTRIBUTION-STATIC-CONTRACT=PASS` |
| Python compile | `D237-COMPILEALL=PASS` |
| Ruff | `D237-RUFF=PASS` |
| Presentation contract audit | `D237-PRESENTATION-AUDIT=PASS` |
| PS5.1 package build | `PASS`, SHA `44E96C3443797D4D87E7F886B46DF4E7F3CBCA85A060155438C8B1A9D8AA5B78` |
| PS7 package build | `PASS`, final SHA `C895DAA3B24AA5CF97169E51DC1C0C7DA5188A1BCB36739C4755CF7F88FC0230` |
| Package identity | `D237-PACKAGE-IDENTITY-PROBE=PASS`, 38,574,276 bytes, dist/root match |
| PyInstaller archive/PYZ | `D237-PACKAGE-ARCHIVE=PASS`, 432 entries, 142 QuillForge modules |
| PyInstaller warning scope | `D237-PYINSTALLER-WARNING-SCOPE=PASS` |
| Source/artifact boundary | `D237-SOURCE-REVISION-BOUNDARY=PASS` |
| Project checks | `D237-CHECK-PS51=PASS`; `D237-CHECK-PS7=PASS` |
| Handoff verifier | `D237-HANDOFF-PS51=PASS`; `D237-HANDOFF-PS7=PASS` |
| Release verifier | `D237-RELEASE-VERIFY=EXPECTED-NO-GO`, 10 open gates; three artifact-bound report consistency failures require authorized startup |

## Unrun checks and reason

Native EXE/Qt startup, installer and uninstaller execution, Windows Registry
provider behavior, shell double-click/file association, clean-machine startup,
cross-machine repeatability, signing, and release-owner approval remain
unrun because the permanent project policy forbids launch/registry actions.
No unit-test assets were created or run.

## Known risks and limits

The reference scan and cleanup behavior are statically verified only. A
Windows registry provider may expose platform-specific permission or race
behavior that requires an authorized disposable-user-profile run. Preserving
the executable on incomplete cleanup intentionally leaves a recoverable,
explicitly incomplete installation rather than risking a dangling association.

## Acceptance and evidence IDs

`S285`, `ARCH-217`, `D237-DISTRIBUTION-STATIC-CONTRACT=PASS`,
`D237-PACKAGE-IDENTITY-PROBE=PASS`, `D237-PACKAGE-ARCHIVE=PASS`,
`D237-SOURCE-REVISION-BOUNDARY=PASS`, `D237-SIMPLIFICATION-ASSESSMENT=PASS`,
`D237-ARCHITECT-REVIEW=NO_CONCLUSION`,
`D237-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next authorized Windows-only validation decision.
Before enabling file associations for release, run the installer/uninstaller
matrix in a disposable user profile with explicit approval and capture rollback
and user-modification evidence.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `C895DAA3B24AA5CF97169E51DC1C0C7DA5188A1BCB36739C4755CF7F88FC0230`
- Bytes: `38,574,276`
- Source revision: `tree-sha256:15cd48ea375736b2023b270e2f7ddaad6108a12c11169df154fd417b9ee7a54c`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The package is rebuilt and traceable; native startup and
registry gates remain open, so this is not a claim of release readiness.

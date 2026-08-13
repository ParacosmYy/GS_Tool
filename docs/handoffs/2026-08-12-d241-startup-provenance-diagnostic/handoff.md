# Handoff: 2026-08-12-d241-startup-provenance-diagnostic

| Field | Value |
|---|---|
| ID | `2026-08-12-d241-startup-provenance-diagnostic` |
| Delivery / slice | `D241 / ARCH-222 Startup provenance diagnostic` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D241 / ARCH-222 handoff — startup provenance diagnostic

## User outcome

The existing no-window startup diagnostic now shows whether the packaged
application import resolved as expected and whether the frozen Qt plugin root
and Windows platform plugin are present. This gives the next authorized
runtime run actionable evidence without changing ordinary GUI startup.

## Scope and boundaries

Changed only `src/quillforge/app.py`. Normal `QApplication` construction,
composition, file/folder routing, locale, theme, font, motion, update, and
association behavior remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Helmholtz the 7th / Luna max initial architecture window: `NO_CONCLUSION`.
- Pauli the 7th / Luna max follow-up architecture window: `PASS`.
- Confucius the 7th / Luna max final independent review: `PASS`.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/app.py`: diagnostic import provenance, frozen Qt plugin
  presence, and single-source platform-plugin status.
- ADR and review records under `docs/adr/` and `docs/agent-team/reviews/`.

## Decisions and constraints

The diagnostic remains behind the existing explicit `--diagnose-startup`
option and does not run during ordinary GUI startup. It records import and
resource provenance only; it never mutates `QT_PLUGIN_PATH`, constructs a
`QApplication`, or treats resource presence as proof of native plugin loading.

## Verification commands and results

| Evidence | Result |
|---|---|
| Python AST | `D241-AST=PASS` |
| Python compile | `D241-COMPILEALL=PASS` |
| Ruff | `D241-RUFF=PASS` |
| Ruff format | `D241-FORMAT=PASS`, 149 files already formatted |
| Source no-window diagnostic | `D241-SOURCE-STARTUP-DIAGNOSTIC=PASS` |
| Desktop file/folder parser | `D241-DESKTOP-LAUNCH-PARSER=PASS` |
| PS5.1 package build | `PASS`, SHA `BA5DFA009955810E34585FF845A401BC002903B015AE8A3B24EBEAB28C6980BC` |
| PS7 package build | `PASS`, final SHA `59A05DCADED8C83917C5E6750FE58403C2C0B67C535AABDEB3BC246037F9023C` |
| Frozen archive/entrypoint | `D241-FROZEN-ARCHIVE-ENTRYPOINT=PASS`; `D241-PYINSTALLER-WARNING-SCOPE=PASS` |
| Package identity | `D241-PACKAGE-IDENTITY=PASS`, 38,575,419 bytes, dist/root match |
| Project checks | `D241-CHECK-PS51=PASS`; `D241-CHECK-PS7=PASS` |
| Handoff verifier | `D241-HANDOFF-PS51=PASS`; `D241-HANDOFF-PS7=PASS` |
| Release verifier | `D241-RELEASE-VERIFY=EXPECTED-NO-GO`, 10 open gates; three artifact-bound startup/report checks remain open |

## Unrun checks and reason

Native EXE/Qt startup, frozen `--diagnose-startup`, clean-machine startup,
native file/folder dialogs, signing, installer/update/rollback, registry, and
release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

The new diagnostic reports plugin presence and import provenance but does not
load the platform plugin or construct a native window. The final release
candidate therefore remains accepted with limits rather than release-ready.

## Acceptance and evidence IDs

`S289`, `ARCH-222`, `D241-SOURCE-STARTUP-DIAGNOSTIC=PASS`,
`D241-DESKTOP-LAUNCH-PARSER=PASS`, `D241-FROZEN-ARCHIVE-ENTRYPOINT=PASS`,
`D241-PACKAGE-IDENTITY=PASS`, `D241-SIMPLIFICATION-ASSESSMENT=PASS`,
`D241-ARCHITECTURE-CONSULTATION=PASS`, `D241-INDEPENDENT-REVIEW=PASS`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and
disposable-profile diagnostic run; this handoff does not authorize it.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `59A05DCADED8C83917C5E6750FE58403C2C0B67C535AABDEB3BC246037F9023C`
- Bytes: `38,575,419`
- Source revision: `tree-sha256:2297f6059b3e52f12a37cb3a28ccfea563ff623dbfaaa13e968bcfaac2e6c7a5`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Startup provenance and frozen resource evidence are
traceable; native startup and remaining enterprise release gates remain open.

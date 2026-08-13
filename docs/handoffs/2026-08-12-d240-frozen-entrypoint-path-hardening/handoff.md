# Handoff: 2026-08-12-d240-frozen-entrypoint-path-hardening

| Field | Value |
|---|---|
| ID | `2026-08-12-d240-frozen-entrypoint-path-hardening` |
| Delivery / slice | `D240 / ARCH-221 Frozen entrypoint path hardening` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D240 / ARCH-221 handoff — frozen entrypoint path hardening

## User outcome

The packaged entrypoint no longer performs source-style `__file__` parent
path surgery when PyInstaller marks the process as frozen. Direct source-file
diagnostics retain their existing import convenience, while the packaged
importer remains the owner of frozen package resolution.

## Scope and boundaries

Changed only `src/quillforge/__main__.py` in the implementation slice. The
public `main(argv)` contract, startup diagnostics, Qt argument/path routing,
application composition, UI, locale, font, theme, and update/association
boundaries remain otherwise unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Mendel the 7th / Luna max architecture window: `NO_CONCLUSION`.
- Galileo the 7th / Terra max escalation window: `NO_CONCLUSION`.
- James the 7th / Luna max independent review: `NO_CONCLUSION`.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/__main__.py`: guard the direct-source `sys.path` insertion
  with `not getattr(sys, "frozen", False)`.
- ADR and review records under `docs/adr/` and `docs/agent-team/reviews/`.

## Decisions and constraints

The package-relative import remains unchanged. The direct-source branch keeps
its compatibility path only when the process is not frozen. This is a
single-boundary entrypoint change; it does not add an importer abstraction or
move application composition across layers.

## Verification commands and results

| Evidence | Result |
|---|---|
| Python AST entrypoint contract | `D240-FROZEN-ENTRYPOINT-STATIC-CONTRACT=PASS` |
| Python compile | `D240-COMPILEALL=PASS` |
| Ruff | `D240-RUFF=PASS` |
| Ruff format | `D240-FORMAT=PASS`, 149 files already formatted |
| Frozen archive essentials | `D240-FROZEN-ARCHIVE-ESSENTIALS=PASS` |
| PS5.1 package build | `PASS`, SHA `50FF287605CBFC1F638DDC777B9D78A0BA75D31401C81E00CA12011802E428C2` |
| PS7 package build | `PASS`, final SHA `5E8DE0300E1ED750B155A2D751687907A449C7535FA38DE79B9153AA487C89DB` |
| Package identity | `D240-PACKAGE-IDENTITY=PASS`, 38,573,963 bytes, dist/root match |
| Project checks | `D240-CHECK-PS51=PASS`; `D240-CHECK-PS7=PASS` |
| Handoff verifier | `D240-HANDOFF-PS51=PASS`; `D240-HANDOFF-PS7=PASS` |
| Release verifier | `D240-RELEASE-VERIFY=EXPECTED-NO-GO`, 10 open gates; artifact/report identity failures remain until authorized native reports exist |

## Unrun checks and reason

Native EXE/Qt startup, `--diagnose-startup`, clean-machine startup, native
file/folder dialog behavior, signing, installer/update/rollback, registry,
and release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

The frozen branch is covered by static entrypoint and archive evidence, not
by native process execution. A real PyInstaller bootloader/importer run is
still required before an enterprise release decision.

## Acceptance and evidence IDs

`S288`, `ARCH-221`, `D240-FROZEN-ENTRYPOINT-STATIC-CONTRACT=PASS`,
`D240-FROZEN-ARCHIVE-ESSENTIALS=PASS`, `D240-PACKAGE-IDENTITY=PASS`,
`D240-SIMPLIFICATION-ASSESSMENT=PASS`,
`D240-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D240-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and
disposable-profile operational run; it is not authorized by this handoff.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `5E8DE0300E1ED750B155A2D751687907A449C7535FA38DE79B9153AA487C89DB`
- Bytes: `38,573,963`
- Source revision: `tree-sha256:1b1bdd8651880ea20168e086503ff5f91362f9563919db59b7c99b6fae3d050f`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The frozen entrypoint source boundary and portable
package are traceable; native startup and remaining enterprise release gates
remain open.

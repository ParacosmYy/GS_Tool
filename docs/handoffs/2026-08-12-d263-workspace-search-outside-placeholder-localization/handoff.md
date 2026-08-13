# Handoff: 2026-08-12-d263-workspace-search-outside-placeholder-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d263-workspace-search-outside-placeholder-localization` |
| Delivery / slice | `D263 / ARCH-241 Workspace-search outside-workspace placeholder localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Workspace-search diagnostics now show `<位于所选工作区之外>` in Simplified
Chinese when an issue path cannot be projected relative to the selected root;
English retains `<outside selected workspace>`.

## Scope and boundaries

The change adds one catalog key and uses it only in
`WorkspaceSearchDialog._relative_path`. Existing diagnostic path data,
containment checks, reasons, result ordering, expansion state, locale refresh,
and search policy remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Herschel the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- Independent-review window: `NO_CONCLUSION`; no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add the two-locale placeholder.
- `src/quillforge/presentation/workspace_search_dialog.py` — project the
  localized placeholder at the existing relative-path fallback.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The angle-bracket placeholder remains a display-only fallback and must not be
used as a containment decision. No EXE/Qt launch, native dialog, registry,
installer, updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Placeholder catalog/projection probe | `D263-OUTSIDE-WORKSPACE-LOCALIZATION=PASS locales=2 placeholder=preserved` |
| Diagnostic refresh wiring probe | `D263-DIAGNOSTIC-REFRESH-WIRING=PASS locale_refresh=1` |
| Python compile | `D263-COMPILEALL=PASS` |
| Ruff | `D263-RUFF=PASS` |
| Ruff format | `D263-FORMAT=PASS` |
| Source startup diagnostic | `D263-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `3FADD4DD6C642622D783D6BDA2A9417D57ACE23E0B8E9169B740575E198A4FFB` |
| PS7 package build | `PASS`, SHA `A6F372EC2572FA1A79B77F9E44D7DC08880D488E0C1E60B932C173EA0235E0CD` |
| Package identity | root/dist match, 38,583,245 bytes |
| Source revision | `tree-sha256:38e4cc78a05d70ea28f1e948636b3aa365bddaa918e5afefd73127c43da48893` |
| Frozen runtime dependencies | `D263-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D263-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D263-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |
| Project checks | `D263-CHECK-PS51=PASS`, `D263-CHECK-PS7=PASS` |
| Release verifier | `D263-RELEASE-VERIFY=EXPECTED-NO-GO open_gates=10` |

## Unrun checks and reason

Native EXE/Qt startup, native search dialog, desktop association launch,
clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probes verify catalog projection and locale-refresh wiring but not
native diagnostic rendering, real filesystem containment events, DPI behavior,
or GUI startup. The current release dossier remains `no-go` because
artifact-bound startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S311`, `ARCH-241`,
`D263-OUTSIDE-WORKSPACE-LOCALIZATION=PASS locales=2 placeholder=preserved`,
`D263-SIMPLIFICATION-ASSESSMENT=PASS`,
`D263-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D263-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `A6F372EC2572FA1A79B77F9E44D7DC08880D488E0C1E60B932C173EA0235E0CD`
- Bytes: `38,583,245`
- Source revision: `tree-sha256:38e4cc78a05d70ea28f1e948636b3aa365bddaa918e5afefd73127c43da48893`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The workspace-search placeholder is statically and
package verified; native rendering and remaining release gates remain open.

# Handoff: 2026-08-11-d120-workspace-navigation-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d120-workspace-navigation-ports |
| Delivery / slice | D120 / ARCH-94 workspace-navigation ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T09:00:00+08:00 |

## User outcome

Workspace open/directory completion policy now has an explicit typed ports
boundary. This makes stale/invalidation/error/session-restore wiring easier to
extend and audit while preserving the existing workspace behavior.

## Scope and boundaries

### In scope

- Add `WorkspaceNavigationPorts`.
- Convert the existing coordinator and MainWindow construction to named
  callback mapping.
- Preserve submit dispatch, generation tracking, surface projection,
  notifications, restore continuation, and close policy.

### Out of scope

- No WorkspaceService, WorkspacePanel, WorkspaceSurface, operation tracker,
  TaskRunner, filesystem, document, startup, close, or persistence semantics
  changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, clean-machine, cross-machine, signing, installer,
  updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Copernicus the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Zeno the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_coordinator.py` — typed
  ports contract and callback projection.
- `src/quillforge/presentation/main_window.py` — named composition.
- `docs/adr/0156-workspace-navigation-ports-contract.md`
- `docs/agent-team/reviews/D120-workspace-navigation-ports-parent-review.md`
- `docs/agent-team/reviews/D120-workspace-navigation-ports-independent-review.md`

## Decisions and constraints

- The ports object is callback-only apart from the existing tracker reference;
  it is not a second state owner.
- MainWindow remains the composition/policy owner; the coordinator remains
  Qt-free and completion-focused.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D120-WORKSPACE-NAVIGATION-BEHAVIOR-PROBE=PASS` | PASS | Open success, invalid directory, and failure projection ordering. |
| `D120-WORKSPACE-NAVIGATION-CONTRACT-PROBE=PASS` | PASS | Ports shape, named composition, Qt-free imports, and callback mapping. |
| `D120-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D120-RUFF=PASS` | PASS | All checks passed. |
| `D120-FORMAT=PASS` | PASS | Both target files already formatted. |
| `D120-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `EF47BACDC8618053029F2FE8B95D931A51FA7D995DE642E4310EBEBDF67C688E`, 38,503,763 bytes, source `tree-sha256:7451e87c1b7a62f738a064c6296618dd4149c005d9e9d98313bf7bd24348c626`. |
| `D120-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D120-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D120-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D120 artifact identity. |
| `D120-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Independent review conclusion — child windows timed out twice; recorded as
  `NO_CONCLUSION`, not PASS.
- Native workspace rendering, QApplication startup, real worker interleaving,
  filesystem timing, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native rendering or real callback timing.
- Copernicus architecture and Zeno independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S160`, `D120-AC01`.
- Evidence: ADR-0156, source/behavior probes, parent and independent review
  records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after source
and record synchronization:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `EF47BACDC8618053029F2FE8B95D931A51FA7D995DE642E4310EBEBDF67C688E`
- Size: `38503763` bytes
- Source revision: `tree-sha256:7451e87c1b7a62f738a064c6296618dd4149c005d9e9d98313bf7bd24348c626`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace navigation composition is explicit while
native/runtime/release evidence remains open.

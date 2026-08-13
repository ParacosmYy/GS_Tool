# Handoff: 2026-08-11-d119-workspace-file-activation

| Field | Value |
|---|---|
| ID | 2026-08-11-d119-workspace-file-activation |
| Delivery / slice | D119 / ARCH-93 workspace file activation boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T08:00:00+08:00 |

## User outcome

Workspace file intents now pass through an explicit Qt-free activation seam.
The existing first-click file behavior, folder navigation, keyboard opening,
duplicate-tab focus, workspace containment, and asynchronous document opening
remain connected while the historical “only folders open” path is easier to
verify and extend.

## Scope and boundaries

### In scope

- Add `WorkspaceFileActivationPorts[TabT]` and
  `WorkspaceFileActivationCoordinator[TabT]`.
- Bind `WorkspaceSurface.file_requested` directly to `activate`.
- Remove the mixed `MainWindow._open_workspace_file` method while retaining
  `_start_open` as the only asynchronous document-open entry.

### Out of scope

- No WorkspacePanel signal, item-kind, click/double-click/keyboard behavior,
  WorkspaceService containment, DocumentTabSurface lookup, DocumentService,
  TaskRunner, startup/close, or persistence semantics changed.
- No new file dialog, worker, filesystem path, or document-open implementation.
- No QApplication launch, clean-machine, cross-machine, signing, installer,
  updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Newton the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Mendel the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_file_activation_coordinator.py` —
  typed Qt-free activation boundary.
- `src/quillforge/presentation/main_window.py` — composition and direct file
  intent binding.
- `docs/adr/0155-workspace-file-activation-boundary.md`
- `docs/agent-team/reviews/D119-workspace-file-activation-parent-review.md`
- `docs/agent-team/reviews/D119-workspace-file-activation-independent-review.md`

## Decisions and constraints

- The coordinator is callback-only; WorkspaceService remains the containment
  authority and DocumentTabSurface remains the tab identity authority.
- MainWindow remains the concrete Qt/application composition root and owns the
  async `_start_open` policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D119-WORKSPACE-FILE-ACTIVATION-BEHAVIOR-PROBE=PASS` | PASS | Invalid, startup, busy, outside, duplicate, and new-file paths. |
| `D119-WORKSPACE-FILE-CONTRACT-PROBE=PASS` | PASS | Qt-free imports, direct surface binding, removed forwarding method, and Panel signal semantics. |
| `D119-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D119-RUFF=PASS` | PASS | All checks passed. |
| `D119-FORMAT=PASS` | PASS | Both target files already formatted. |
| `D119-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| `D119-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | PyInstaller package completed without launching QuillForge; no process was started by validation. |
| `D119-RELEASE-DOSSIER-PROBE=PASS` | PASS | Current dossier binds the rebuilt artifact and records the expected no-go gates. |
| `D119-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Verifier remains non-zero for the three known mechanical report bindings and ten open gates. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and index traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository notice, formatting, lint, compilation, and presentation checks passed. |

## Unrun checks and reason

- Independent review conclusion — child windows timed out twice; recorded as
  `NO_CONCLUSION`, not PASS.
- Native tree interaction, QApplication startup, real worker interleaving,
  filesystem timing, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native item event ordering or real worker
  timing.
- Newton architecture and Mendel independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S159`, `D119-AC01`.
- Evidence: ADR-0155, source/behavior/signal probes, parent and independent
  review records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `AB0BE369FE018B952F090793B434DB2ECA21C83EACA8B489446213B2B1BDECF2`
- Size: `38502934` bytes
- Source revision: `tree-sha256:0bde49a9f8f250e31b7f9a5d66c3f788249b7acf36a6580d45e1712516275217`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace file activation is explicitly bounded and
the existing file-opening route is preserved; native/runtime/release evidence
remains open.

# Handoff: 2026-08-11-d121-close-guard-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d121-close-guard-ports |
| Delivery / slice | D121 / ARCH-97 close-guard ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T12:00:00+08:00 |

## User outcome

Close-readiness wiring now has an explicit typed boundary. Future close gates
can be added by named ports while the existing operation/search/dirty/background/
pending precedence stays auditable and behaviorally stable.

## Scope and boundaries

### In scope

- Add frozen/slotted `CloseGuardPorts`.
- Replace the eight positional MainWindow constructor callbacks with named
  mapping.
- Preserve close precedence, cancellation, session-save, pending-work, timer,
  QCloseEvent, and message policy.

### Out of scope

- No close guard decision type, service, tracker, timer, TaskRunner,
  persistence, dialog, startup, or application policy semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native event capture, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Avicenna the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Gibbs the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/close_guard_coordinator.py` — typed ports and
  named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `docs/adr/0159-close-guard-ports-contract.md`
- `docs/agent-team/reviews/D121-close-guard-ports-parent-review.md`
- `docs/agent-team/reviews/D121-close-guard-ports-independent-review.md`

## Decisions and constraints

- `CloseGuardPorts` is callback-only and frozen/slotted; it is not a new state
  or policy owner.
- MainWindow retains the QCloseEvent boundary and concrete close policy; the
  coordinator remains Qt-free and classification-focused.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D121-CLOSE-GUARD-BEHAVIOR-PROBE=PASS` | PASS | All five blocked/allowed branches and side-effect order. |
| `D121-CLOSE-GUARD-CONTRACT-PROBE=PASS` | PASS | Frozen ports, named MainWindow mapping, and public export. |
| `D121-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D121-RUFF=PASS` | PASS | Target source passed lint. |
| `D121-FORMAT=PASS` | PASS | Both target files already formatted. |
| `D121-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `9DFBA8C96B3FC745746E4EF6D15E58C58181737F4E9C8F44E43E904F29449167`, 38,505,334 bytes, source `tree-sha256:cba7a01f0f5e92c9af1fff80f2f787c9c8941a457e51f2c9389720100bb8c5a3`. |
| `D121-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D121-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D121-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D121 artifact identity. |
| `D121-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native QCloseEvent timing, QApplication startup, worker interleaving,
  filesystem timing, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native close-event timing or real worker
  interleaving.
- Avicenna architecture and Gibbs independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S163`, `D121-AC01`.
- Evidence: ADR-0159, source/behavior probes, parent and independent review
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
- SHA-256: `9DFBA8C96B3FC745746E4EF6D15E58C58181737F4E9C8F44E43E904F29449167`
- Size: `38505334` bytes
- Source revision: `tree-sha256:cba7a01f0f5e92c9af1fff80f2f787c9c8941a457e51f2c9389720100bb8c5a3`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: close readiness has an explicit typed composition
boundary while native/runtime/release evidence remains open.

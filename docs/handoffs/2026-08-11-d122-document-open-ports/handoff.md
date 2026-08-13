# Handoff: 2026-08-11-d122-document-open-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d122-document-open-ports |
| Delivery / slice | D122 / ARCH-98 document-open ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T13:00:00+08:00 |

## User outcome

Document opening now has an explicit typed composition boundary for ordinary
and session-restored documents. Future open-result policy can evolve through
named ports without obscuring restore identity or continuation wiring.

## Scope and boundaries

### In scope

- Add frozen/slotted `DocumentOpenPorts`.
- Replace the seven positional MainWindow constructor callbacks with named
  mapping.
- Preserve ordinary/session-restore result, failure, projection, notification,
  line-number, and continuation behavior.

### Out of scope

- No DocumentService, TaskRunner, DocumentOpenProjectionCoordinator,
  tab/editor, persistence, startup, close, locale, or application policy
  semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Rawls the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Goodall the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_open_coordinator.py` — typed ports and
  named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `docs/adr/0160-document-open-ports-contract.md`
- `docs/agent-team/reviews/D122-document-open-ports-parent-review.md`
- `docs/agent-team/reviews/D122-document-open-ports-independent-review.md`

## Decisions and constraints

- `DocumentOpenPorts` is callback-only and frozen/slotted; it is not a second
  document or restore state owner.
- `DocumentOpenProjectionCoordinator` remains the valid-result projection
  owner; MainWindow retains services, persistence, startup, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D122-DOCUMENT-OPEN-BEHAVIOR-PROBE=PASS` | PASS | Ordinary/restore valid, invalid, failure, stale, and continuation paths. |
| `D122-DOCUMENT-OPEN-CONTRACT-PROBE=PASS` | PASS | Frozen ports, named composition, Qt-free imports, and public export. |
| `D122-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D122-RUFF=PASS` | PASS | Target source passed lint after line-length correction. |
| `D122-FORMAT=PASS` | PASS | Both target files already formatted. |
| `D122-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `7B00D285B7DEE35789142A98F7E047E4467B59890C8243D28B727D83FB1A0028`, 38,503,901 bytes, source `tree-sha256:9f3a49604edaaf39eb3da1772a2e86ff016b869f85a31199f4f819132306a065`. |
| `D122-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D122-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D122-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D122 artifact identity. |
| `D122-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native editor rendering, QApplication startup, worker interleaving,
  filesystem decoding timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native editor rendering or real callback
  timing.
- Rawls architecture and Goodall independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S164`, `D122-AC01`.
- Evidence: ADR-0160, source/behavior probes, parent and independent review
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
- SHA-256: `7B00D285B7DEE35789142A98F7E047E4467B59890C8243D28B727D83FB1A0028`
- Size: `38503901` bytes
- Source revision: `tree-sha256:9f3a49604edaaf39eb3da1772a2e86ff016b869f85a31199f4f819132306a065`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-open composition is explicit while native/
runtime/release evidence remains open.

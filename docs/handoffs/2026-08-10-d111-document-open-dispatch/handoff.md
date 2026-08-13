# Handoff: 2026-08-10-d111-document-open-dispatch

| Field | Value |
|---|---|
| ID | `2026-08-10-d111-document-open-dispatch` |
| Delivery / slice | `D111 / ARCH-83 document-open dispatch callback boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T00:05:00+08:00` |

## User outcome

Ordinary document opens and session-restore opens now use one typed
callback-binding path. The existing Qt-free `DocumentOpenCoordinator` binds
the optional line navigation and routes valid, invalid, stale, and failed
callbacks while MainWindow retains path selection, restore binding,
DocumentService, TaskRunner, status/notification, persistence, and close
behavior.

## Scope and boundaries

### In scope

- Add typed open operation/success/failure/dispatcher contracts.
- Add `DocumentOpenCoordinator.submit(...)` for callback binding and generic
  dispatch.
- Route MainWindow document opens through the existing lifecycle owner.
- Preserve ordinary/session-restore branching, stale/liveness guards,
  invalid/failure projection, restore continuation, document application, and
  line navigation.
- Record public-source applicability, parent review, independent review
  status, simplification, static, inline, package, and release-limit evidence.

### Out of scope

- No DocumentService, DocumentStore, path-selection, session schema, editor,
  workspace, notification, persistence, TaskRunner implementation, or close
  policy changed.
- No new coordinator, state owner, signal, dependency, or test-only asset was
  introduced.
- No QApplication launch, worker timing, filesystem decoding, screenshot,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Hilbert the 3rd / Luna max | Read-only D111 boundary consultation; `NO_CONCLUSION` after bounded wait |
| Independent review | Newton the 3rd / Luna max | Read-only D111 review; `NO_CONCLUSION` after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_open_coordinator.py` — typed
  dispatcher contract and callback-binding method.
- `src/quillforge/presentation/main_window.py` — document open now uses the
  coordinator boundary and no longer owns duplicate open closures.
- `docs/adr/0138-document-open-dispatch-callback-boundary.md` — decision,
  invariants, alternatives, applicability, review, simplification, and limits.
- `docs/agent-team/reviews/D111-arch-83-document-open-parent-review.md` — parent
  review.
- `docs/agent-team/reviews/D111-arch-83-document-open-independent-review.md` —
  independent `NO_CONCLUSION` record.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- The existing `DocumentOpenCoordinator` remains the sole open callback
  lifecycle owner; no parallel dispatcher abstraction was added.
- MainWindow continues to select paths, bind session restore, allocate
  operation IDs, submit TaskRunner work, and control document/close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D111-SOURCE-DEPENDENCY-PROBE=PASS` | `PASS` | Typed submit is present, duplicate MainWindow open closures are absent, and the coordinator has no Qt/TaskRunner/DocumentService/widget imports. |
| `D111-DOCUMENT-OPEN-DISPATCH-PROBE=PASS` | `PASS` | Inline production-class probe covered ordinary/session-restore valid paths, invalid/failure projection, stale suppression, line navigation, continuation, and synchronous dispatcher exception behavior. |
| `uv run python -m compileall -q src scripts` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src scripts` | `PASS` | All checks passed. |
| `uv run ruff format --check src scripts` | `PASS` | All 122 files were already formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | `PASS` | Existing presentation contract/error/observability gate passed. |
| `D111-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | `DADDE931242BD45E921FBD9BE842030623951A762BC8143B952BA1DA71059500`; 38,495,023 bytes; source `tree-sha256:2bed75c8c661f8c49626a44141b3bdc4a73d9a9112e84d9a4fecf3f98b08b5c0`. |
| `D111-NO-LAUNCH-PROBE=PASS` | `PASS` | Package completed and no QuillForge process was running afterward. |
| `D111-JSON-TRACEABILITY-PROBE=PASS` | `PASS` | Acceptance, delivery register, handoff index, and release manifest point to D111/current identity. |
| `D111-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Current dossier is `no-go`, bound to the D111 artifact, with 10 open gates and the three known mechanical report-binding failures. |
| `D111-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | `verify_release_handoff.ps1` remains intentionally non-zero because authorized runtime/report refresh and release gates are still open. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff status/index contract passed. |
| `scripts\check.ps1` | `PASS` | Notice, workflow, acceptance, architecture-boundary, presentation-contract, lock, Ruff, and compile checks passed. |

## Unrun checks and reason

- Native TaskRunner queued timing, actual document open/editor interaction,
  filesystem decoding/durability, startup, screen-reader output, DPI,
  screenshot, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and coordinator lifecycle behavior,
  not native worker scheduling, filesystem decoding, or durable document
  persistence.
- Hilbert and Newton review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S142`, `D111-AC01`.
- Evidence: ADR-0138, D111 source/dispatch probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract slice and
  complete authorized runtime/release gates when authority and environment
  permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `DADDE931242BD45E921FBD9BE842030623951A762BC8143B952BA1DA71059500`
- Size: `38,495,023` bytes
- Source revision: `tree-sha256:2bed75c8c661f8c49626a44141b3bdc4a73d9a9112e84d9a4fecf3f98b08b5c0`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: duplicate document-open callback ownership is
consolidated behind the existing Qt-free coordinator, while native timing,
filesystem decoding, runtime, and enterprise release gates remain open.

# Handoff: 2026-08-10-d99-workspace-navigation-projection

| Field | Value |
|---|---|
| ID | `2026-08-10-d99-workspace-navigation-projection` |
| Delivery / slice | `D99 / ARCH-73 workspace-navigation projection coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Valid workspace-open and directory navigation now have one focused Qt-free
projection coordinator. Existing behavior remains intact: opening a workspace
invalidates the old search root, activates the workspace, projects its root and
directory, and then reports success/persists/finishes restore in the same order;
directory results still use the current root and no-op when the root or surface
is absent.

## Scope and boundaries

### In scope

- `WorkspaceNavigationProjectionCoordinator` valid-open/directory contract.
- Direct D85 `WorkspaceNavigationCoordinator` wiring and removal of the old
  MainWindow valid projection methods.
- Source, contract, static, package, handoff, and release evidence.

### Out of scope

- No WorkspaceService/provider, containment, directory paging, search query or
  cancellation, WorkspaceSurface, session schema, notification contract,
  locale/settings, plugin API, or close policy changed.
- No new worker, retry, cache, mutable shared state, or universal operation
  coordinator.
- No Qt launch, screenshot, native workspace/search review,
  accessibility/DPI/font, clean-machine, cross-machine, signing, installer,
  updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Hypatia the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded windows |
| Independent review | Aristotle the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded windows |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_projection_coordinator.py`
  — Qt-free valid workspace projection sequence.
- `src/quillforge/presentation/workspace_navigation_coordinator.py` — retains
  classification ownership and receives direct projection callbacks.
- `src/quillforge/presentation/main_window.py` — direct D85 wiring and removal
  of `_apply_workspace_opened` / `_apply_workspace_directory`.
- D99 ADR, parent/independent review records, handoff, and synchronized
  acceptance/delivery/roadmap/spec/task/release records.

## Decisions and constraints

- `WorkspaceNavigationCoordinator` remains responsible for completion identity,
  stale/invalidation/loading/invalid/failure classification and failure-time
  restore release.
- MainWindow remains the composition root and supplies WorkspaceService,
  workspace/search surface, notification, session-save, admission, and close
  policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++ assurance
  and vendor manufacturer requirements are `N/A` for this slice.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D99-WORKSPACE-PROJECTION-SOURCE-PROBE=PASS` | `PASS` | Qt-free source boundary, direct D85 wiring, and old method removal. |
| `D99-WORKSPACE-PROJECTION-ORDER-PROBE=PASS` | `PASS` | Open order, directory root pass-through, and missing-surface early return. |
| `python -m compileall -q src/quillforge` | `PASS` | No launch or QApplication instantiation. |
| `uv run ruff check src/quillforge` | `PASS` | No lint errors. |
| `uv run ruff format --check src/quillforge` | `PASS` | 116 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| D99 package identity | `PASS` | SHA-256 `FD69BE84D5D503815B63AF17A1FCA20C08FDC1FDBC2FD378B0C98DE5935C918A`, 38,483,971 bytes. |
| `D99-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | Release verifier reports the expected three mechanical report-binding failures and ten open gates. |

## Unrun checks and reason

- Native workspace/search callback timing, accessibility, DPI, fonts, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static contract probes do not prove Qt signal timing, native workspace/search
  behavior, or close-time interleavings.
- Both delegated D99 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`; three
  mechanical report-binding failures and ten external release gates remain.

## Acceptance and evidence IDs

- Acceptance: `D99-AC01`, `S128`.
- Evidence: ADR-0124, D99 source/order probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: run synchronized handoff/repository/release checks, then continue the
  next smallest MainWindow/application boundary or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `FD69BE84D5D503815B63AF17A1FCA20C08FDC1FDBC2FD378B0C98DE5935C918A` /
  `38,483,971` bytes.
- Source revision: `tree-sha256:f856b2efd82f97d2c6a891737147ac7e4ecc42254f94ca118855b0e30f07f92b`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: valid workspace navigation projection is isolated behind
a Qt-free typed boundary, while native runtime and enterprise release gates
remain open.

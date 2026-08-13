# Handoff: 2026-08-11-d112-settings-save-dispatch

| Field | Value |
|---|---|
| ID | `2026-08-11-d112-settings-save-dispatch` |
| Delivery / slice | `D112 / ARCH-84 settings-save dispatch callback boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T00:20:00+08:00` |

## User outcome

Settings persistence now uses one typed callback-binding path. The existing
Qt-free `SettingsSaveCoordinator` binds the settings operation callbacks
while MainWindow retains tracker admission, SettingsService, TaskRunner,
theme/font/locale/editor/motion projection, notifications, persistence, and
close behavior.

## Scope and boundaries

### In scope

- Add typed settings-save operation/success/failure/dispatcher contracts.
- Add `SettingsSaveCoordinator.submit(...)` for generic dispatch binding.
- Route MainWindow settings saves through the existing lifecycle owner.
- Preserve tracker admission, stale suppression, invalid/failure projection,
  projection order, and synchronous dispatcher exceptions.
- Record public-source applicability, parent review, independent review
  status, simplification, static, inline, package, and release-limit evidence.

### Out of scope

- No SettingsService/SettingsStore, dialog, settings schema, theme, locale,
  font, editor, motion, notification, TaskRunner implementation, persistence,
  or close policy changed.
- No new coordinator, state owner, signal, dependency, or test-only asset was
  introduced.
- No QApplication launch, worker timing, screenshot, rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Boole the 4th / Luna max | Read-only D112 boundary consultation; `NO_CONCLUSION` after bounded wait |
| Independent review | Bernoulli the 4th / Luna max | Read-only D112 review; `NO_CONCLUSION` after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/settings_save_coordinator.py` — typed
  dispatcher contract and callback-binding method.
- `src/quillforge/presentation/main_window.py` — settings save now uses the
  coordinator boundary and no longer owns duplicate callbacks.
- `docs/adr/0139-settings-save-dispatch-callback-boundary.md` — decision,
  invariants, alternatives, applicability, review, simplification, and limits.
- `docs/agent-team/reviews/D112-arch-84-settings-save-parent-review.md` —
  parent review.
- `docs/agent-team/reviews/D112-arch-84-settings-save-independent-review.md` —
  independent `NO_CONCLUSION` record.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- The existing `SettingsSaveCoordinator` remains the sole settings callback
  lifecycle owner; no parallel dispatcher abstraction was added.
- MainWindow continues to edit settings, admit tracker identity, allocate IDs,
  submit TaskRunner work, and control settings/close policy.
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
| `D112-SOURCE-DEPENDENCY-PROBE=PASS` | `PASS` | Typed submit is present, duplicate MainWindow settings callbacks are absent, and the coordinator has no Qt/TaskRunner/SettingsService/widget imports. |
| `D112-SETTINGS-SAVE-DISPATCH-PROBE=PASS` | `PASS` | Inline production-class probe covered valid, invalid, failure, stale, tracker retention, and synchronous dispatcher exception behavior. |
| `uv run python -m compileall -q src scripts` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src scripts` | `PASS` | All checks passed. |
| `uv run ruff format --check src scripts` | `PASS` | All 122 files were already formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | `PASS` | Existing presentation contract/error/observability gate passed. |
| `D112-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | `33B12E168767E1AD5677DCDA49B0B4CD37EDC3194D8DE64DADAF2DDB389DF667`; 38,494,896 bytes; source `tree-sha256:88152f8a4305a6469106bf6fe8498258a4cb6bd4b38eda9fd5804dcb84bf3550`. |
| `D112-NO-LAUNCH-PROBE=PASS` | `PASS` | Package completed and no QuillForge process was running afterward. |
| `D112-JSON-TRACEABILITY-PROBE=PASS` | `PASS` | Acceptance, delivery register, handoff index, and release manifest point to D112/current identity. |
| `D112-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Current dossier is `no-go`, bound to the D112 artifact, with 10 open gates and the three known mechanical report-binding failures. |
| `D112-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | `verify_release_handoff.ps1` remains intentionally non-zero because authorized runtime/report refresh and release gates are still open. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff status/index contract passed. |
| `scripts\check.ps1` | `PASS` | Notice, workflow, acceptance, architecture-boundary, presentation-contract, lock, Ruff, and compile checks passed. |

## Unrun checks and reason

- Native TaskRunner timing, actual settings dialog/theme/font/locale/motion
  rendering, startup, screen-reader output, DPI, screenshot, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-
  owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and tracker lifecycle behavior, not
  native worker scheduling, actual settings rendering, or durable persistence.
- Boole and Bernoulli review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S143`, `D112-AC01`.
- Evidence: ADR-0139, D112 source/dispatch probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract or visual
  quality slice and complete authorized runtime/release gates when authority
  and environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `33B12E168767E1AD5677DCDA49B0B4CD37EDC3194D8DE64DADAF2DDB389DF667`
- Size: `38,494,896` bytes
- Source revision: `tree-sha256:88152f8a4305a6469106bf6fe8498258a4cb6bd4b38eda9fd5804dcb84bf3550`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: duplicate settings-save callback ownership is
consolidated behind the existing Qt-free coordinator, while native timing,
settings rendering, runtime, and enterprise release gates remain open.

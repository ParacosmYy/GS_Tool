# Handoff: 2026-08-10-d67-mainwindow-forwarding-simplification

| Field | Value |
|---|---|
| ID | `2026-08-10-d67-mainwindow-forwarding-simplification` |
| Delivery / slice | `D67 / ARCH-51 MainWindow file-dialog/About forwarding simplification` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:55:00+08:00` |

## User outcome

MainWindow now exposes fewer misleading presentation aliases. Save As and
dirty-tab close call the existing file-dialog surface directly, and About is
registered directly against the existing message surface. User-facing dialog
behavior remains unchanged.

## Scope and boundaries

### In scope

- Remove `_choose_save_path` and `_show_about` forwarding methods.
- Retain direct FileDialogSurface/MessageSurface ownership and all policies.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No save/close/recovery/document policy, command ID/title/menu, file-dialog,
  message surface, i18n, service, persistence, runtime, screenshot,
  accessibility-driver, clean-machine, signing, installer, updater, legal,
  support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Gauss the 3rd / Luna max | Cross-module boundary consultation; no conclusion after two bounded waits |
| Independent review | Lovelace the 3rd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — direct surface calls and
  forwarding-wrapper removal.
- `docs/adr/0092-mainwindow-file-dialog-about-forwarding-simplification.md` —
  decision.
- D67 parent/independent review records, acceptance/register/index, and
  project architecture/spec/roadmap/task traceability.

## Decisions and constraints

- FileDialogSurface remains the native path-picker owner; MessageSurface
  remains the localized About/message owner.
- MainWindow retains Save As, dirty-close, command registry, document,
  recovery, session, and application policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D67 forwarding simplification probe | `PASS` | Alias removal and direct callable/argument retention. |
| Targeted compileall / Ruff / format | `PASS` | Changed MainWindow source. |
| Full compileall / Ruff / format | `PASS` | Source and documentation synchronization completed. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native file-dialog/About interaction, callback timing, accessibility, DPI,
  fonts, runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static direct-call evidence does not prove native callback ordering or
  dialog behavior on every Windows style/DPI combination.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D67-AC01`, `S96`.
- Evidence: ADR-0092, source probe, parent/independent reviews, static checks,
  package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded coordinator/UI slice or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `4B140A66D2C7C752DCBFE15CF2CC280312C3A31157721F92B090B013A108A475` / `38,436,986` bytes.
- Source revision: `tree-sha256:6442c6dcfb0a36e62e8700c253bc694304621ca45011a96a84932d637e077704`.

## Disposition

`accepted-with-limits`: D67 forwarding simplification is integrated and the
package identity is recorded, while runtime and release gates remain open.

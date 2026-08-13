# Handoff: 2026-08-10-d76-recovery-notification-localization

| Field | Value |
|---|---|
| ID | `2026-08-10-d76-recovery-notification-localization` |
| Delivery / slice | `D76 / UI-49 recovery notification dynamic localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Recovery success notifications no longer leave the stable suffix
`content remains unsaved` in English when the UI locale is `zh-CN`. Dynamic
document names, punctuation, notification severity, and recovery behavior are
preserved.

## Scope and boundaries

### In scope

- One bounded dynamic message projection in `presentation.i18n`.
- Chinese projection and exact English fallback evidence.
- Source review, simplification assessment, static checks, package identity,
  handoff registration, and expected release no-go evidence.

### Out of scope

- No RecoveryService, DocumentService, persistence, MainWindow policy, signal,
  notification-level, or filesystem change.
- No new translation service, template registry, dependency, or application
  locale contract.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Lorentz the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Feynman the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — named recovery message
  prefix/suffix projection for `zh-CN`.
- `docs/adr/0101-recovery-notification-localization.md` — boundary decision.
- D76 parent/independent review records and acceptance/roadmap/register/index
  synchronization.

## Decisions and constraints

- Application/recovery messages remain locale-free source contracts.
- The dynamic document name is preserved as opaque display data.
- `en-US` behavior remains source-message identity.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D76 recovery localization probe | `PASS` | `zh-CN`, `en-US` identity, Unicode and punctuation names. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation source. |
| Full compileall / Ruff / format | `PASS` | Final repository checks after synchronization. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native Qt rendering, live language switching, keyboard traversal,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static message-shape coverage does not prove future notification shapes will
  localize; new dynamic messages must add a presentation probe.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D76-AC01`, `S105`.
- Evidence: ADR-0101, D76 localization probe, parent/independent review
  records, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible localization/UI gap or
  obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `AA0A169962FEA8CFE467C3402876C1BBB3589D6416BB03996E816CE894C97E05` / `38,449,595` bytes.
- Source revision: `tree-sha256:f6b3ed3e6379e4f7f8451d31be649ff63de8e615d030e85a92f31a3c5a974da2`.

## Disposition

`accepted-with-limits`: the recovery notification is fully projected through
the existing presentation localization boundary and the package identity is
recorded, while native runtime and release gates remain open.

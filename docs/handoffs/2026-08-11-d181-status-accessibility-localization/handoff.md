# Handoff: 2026-08-11-d181-status-accessibility-localization

| Field | Value |
|---|---|
| ID | `2026-08-11-d181-status-accessibility-localization` |
| Delivery / slice | `D181 / UI-93 / ARCH-168 Status accessibility localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T22:30:00+08:00` |

## User outcome

When the user switches between English and Simplified Chinese, the status
rail's shell status, workspace context, phase, notification name, and dynamic
phase description now follow the selected locale. Visible notification text,
severity, timers, lifecycle state, and theme behavior remain unchanged.

## Scope and boundaries

### In scope

- Five presentation-only accessibility i18n keys/templates in both locales.
- StatusRail accessible names and phase description through its existing
  locale/phase projection.
- StatusSurface notification accessible name through its existing locale path.
- Static, package, manifest, handoff, and expected release no-go evidence.

### Out of scope

- No notification payload, severity, timer, status phase policy, QSS, or
  application/domain contract change.
- No GUI/EXE launch, screenshot, screen-reader execution, unit test, test
  asset, hardware action, flashing, deployment, clean-machine, or cross-machine
  evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent agent | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product scope | User outcome and acceptance |
| Developer 1 | Parent agent, sole writer | i18n/status contract slice |
| Developer 2 | Parent agent, sole writer | Presentation integration/packaging slice |
| QA | Parent agent read-only validation | Deterministic checks and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — accessibility labels and phase
  description for en-US/zh-CN.
- `src/quillforge/presentation/status_bar.py` — localized rail names and
  phase description.
- `src/quillforge/presentation/status_surface.py` — localized notification
  accessible name.

## Decisions and constraints

- Reuse the existing presentation i18n catalog and locale refresh coordinator;
  no second translation pipeline or application-layer dependency was added.
- Public applicability is Python 3.12/PyQt6 only. Public CloudWeGo material is
  an engineering reference, not a private ByteDance standard or compliance
  claim. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
  applicable.
- Shared checkout writer: parent agent only, current local checkout.
- Runtime launch policy: not allowed; no QApplication/window or EXE launch.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Accessibility i18n/source probe | PASS | en-US/zh-CN, four labels and phase template |
| `uv run python -m compileall -q src` | PASS | source compilation |
| Ruff check and format check | PASS | changed status/i18n files |
| `scripts/check.ps1` | PASS | project/handoff/presentation checks |
| `scripts/verify_handoff.ps1` | PASS | indexed handoff and status contract |
| `scripts/package.ps1` | PASS | PyInstaller one-file package |
| Package identity probe | PASS | SHA/size/root-copy/manifest match |
| Release dossier invariant probe | PASS | expected no-go, 10 open gates, 3 mechanical failures |

## Unrun checks and reason

- Native accessibility tree, screen-reader output, Qt rendering, font/DPI,
  screenshots, and runtime startup — prohibited by the active no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  release-owner, and fresh artifact-bound runtime reports — require an
  authorized environment or external decision.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Platform screen readers may expose Qt object names/descriptions differently;
  native accessibility review remains open.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S234`, `D181-AC01`.
- Evidence: ADR-0230, parent/independent review records, D181 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows screen-reader and accessibility-tree review
  of both locales, status updates, and DPI/font combinations.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0` portable candidate.
- SHA-256 / size: `6AE2F8A6F22B1AD28F4AA96CBC57088C469199638D7119283DC5A0418550FF83` / `38553516` bytes.
- Source revision: `tree-sha256:8317c016beb88cbae776e611e959ae75d84499e6e5ddd5308b30be6d405b4fbc`.
- Packaging note: rebuilt PyInstaller one-file portable candidate without
  launching QuillForge.

## Disposition

`accepted-with-limits`: status accessibility labels now follow locale through
the existing presentation boundary; native screen-reader and external release
evidence remain open.

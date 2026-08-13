# Handoff: 2026-08-11-d185-form-control-highlight

| Field | Value |
|---|---|
| ID | `2026-08-11-d185-form-control-highlight` |
| Delivery / slice | `D185 / UI-96 / ARCH-172 Form-control highlight closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Existing settings and form controls now have a clearer focus surface, and
ComboBox popup choices expose visible hover and selected highlight states.
The visual change is centralized and keeps the existing theme/accent system.

## Scope and boundaries

### In scope

- Centralized `presentation.theme` QSS for focus and ComboBox popup states.
- Reuse of existing ThemeColors and object-name/state selectors.
- Static 3-theme × 4-accent source projection and code-quality evidence.

### Out of scope

- Widget construction, values, signals, layout, locale, fonts, motion,
  settings persistence, application policy, GUI startup, and screenshots.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependencies, risks, and status |
| Product | `user outcome` | Readable, modern, visibly highlighted controls |
| Developer 1 | `parent` | Centralized QSS implementation |
| Developer 2 | `parent` | Packaging and handoff integration |
| QA | `parent` | Read-only source, contrast, and package verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — focus surface and ComboBox popup
  state selectors.
- `docs/adr/0234-form-control-highlight-closure.md` — architecture decision.

## Decisions and constraints

- Shared checkout writer: `parent`, one centralized presentation file only.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, or native popup
  rendering was run.
- Architect Hypatia the 5th / Luna max and independent Russell the 5th / Luna
  max returned `NO_CONCLUSION`; parent PASS is the only review conclusion.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| No-GUI QSS projection probe | `PASS` | `D185-QSS-HIGHLIGHT-CONTRACT-PROBE=PASS combos=12`. |
| `uv run python -m compileall -q src` | `PASS` | No runtime startup. |
| `uv run ruff check src` and `ruff format --check src` | `PASS` | `D185-COMPILE-RUFF-FORMAT=PASS`. |
| `scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |

## Unrun checks and reason

- Native ComboBox popup rendering/metrics/accessibility — no GUI policy.
- Screenshot, DPI, clean-machine, cross-machine, and release-owner evidence —
  external/runtime evidence is not authorized or available.

## Known risks and limits

- Qt platform styles may render popup padding or item borders differently.
- The release remains `no-go` with existing signing, installer/update,
  runtime-report, and clean-machine gates open.

## Acceptance and evidence IDs

- Acceptance: `S238`, `D185-AC01`
- Evidence: `D185-QSS-HIGHLIGHT-CONTRACT-PROBE=PASS combos=12`,
  `D185-COMPILE-RUFF-FORMAT=PASS`, `D185-SIMPLIFICATION-ASSESSMENT=PASS`,
  `docs/agent-team/reviews/D185-form-control-highlight-parent-review.md`,
  `docs/agent-team/reviews/D185-form-control-highlight-independent-review.md`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize a native visual review when the no-GUI policy is lifted;
  verify popup metrics and cross-DPI readability before release claims.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `2F93DB9792057ED9BCFF8A47BFF04778BE339E254CD537FFEA827C1D3FC8AD41` / `38,556,809` bytes
- Source revision: `tree-sha256:90b1fe54d7a979259369ca4f73485e3e98bec21dbaeb305682f19e423dc0a939`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no installer
  or updater artifact is claimed.

## Disposition

`accepted-with-limits`: form-control highlight wiring and static evidence are
recorded; native rendering and release gates remain open.

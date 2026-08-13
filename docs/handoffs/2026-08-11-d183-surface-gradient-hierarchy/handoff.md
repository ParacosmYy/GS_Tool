# Handoff: 2026-08-11-d183-surface-gradient-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d183-surface-gradient-hierarchy` |
| Delivery / slice | `D183 / ARCH-170 / UI-95 Surface-gradient hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:30:00+08:00` |

## User outcome

The main window, editor shell, and command rail gain a restrained layered depth
cue from the existing palette, making the modern Sakura/candy and legacy themes
feel less flat while preserving readability and interaction semantics.

## Scope and boundaries

### In scope

- Three ThemeColors-driven QSS linear gradients.
- Main window, editor shell, and command-rail surface selectors only.

### Out of scope

- Editor canvas, syntax, caret, selection, control states, behavior, and signals.
- New visual tokens, assets, animations, settings, or runtime theme services.
- Native Qt runtime, screenshots, DPI, and external release gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependencies, risks, and status |
| Product | `user outcome` | Modern, human-readable surface depth |
| Developer 1 | `parent` | Theme/QSS implementation |
| Developer 2 | `parent` | Presentation integration and packaging |
| QA | `parent` | Read-only static and contrast verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — three scoped QSS surface gradients.
- `docs/adr/0232-surface-gradient-hierarchy.md` — architecture decision.
- `docs/agent-team/reviews/D183-surface-gradient-hierarchy-parent-review.md` — parent review.
- `docs/agent-team/reviews/D183-surface-gradient-hierarchy-independent-review.md` — no-conclusion record.

## Decisions and constraints

- Gradients reuse only `ThemeColors.surface_0/1/2`; no hard-coded visual source.
- The editor canvas remains solid and owns its existing readability tokens.
- Shared checkout writer: parent only; no worktree was created or used.
- Runtime launch policy: not allowed; no Qt window or EXE was started.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge/presentation/theme.py` | `PASS` | Targeted source compilation |
| `uv run ruff check src/quillforge/presentation/theme.py` | `PASS` | Ruff checks passed |
| `uv run ruff format --check src/quillforge/presentation/theme.py` | `PASS` | File already formatted |
| `D183-GRADIENT-CONTRACT-PROBE` | `PASS` | Exactly three gradients across 12 combinations |
| `D183-SURFACE-ENDPOINT-CONTRAST-PROBE` | `PASS` | Text remains readable on all endpoints |
| `scripts/package.ps1` | `PASS` | Portable one-file package rebuilt without launch |
| `D183-PACKAGE-IDENTITY-PROBE` | `PASS` | SHA `5E946F4BE8271F2F5409E18142804E448F4D6AEAC55A000D609C327D818E7BF4`, 38,556,605 bytes |
| `D183-MANIFEST-TRACEABILITY-PROBE` | `PASS` | Manifest and root copy match the package |
| `scripts/check.ps1` | `PASS` | Full project static checks |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff contract synchronized |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Three mechanical report mismatches and ten open release gates remain |
| Architect child review | `NO_CONCLUSION` | Bounded architecture window timed out |
| Independent child review | `NO_CONCLUSION` | Bounded independent window timed out |

## Unrun checks and reason

- Native Qt QSS parsing/painting, DPI, screenshot, font fallback, and visual state review — runtime launch is prohibited.
- Unit tests, mocks, fixtures, and test-only assets — project policy forbids creating or running them by default.
- Clean-machine, cross-machine, signing, installer, updater, legal, and release-owner checks — external evidence remains open.

## Known risks and limits

- Static QSS generation is not native Qt rendering evidence.
- The release dossier remains no-go with three mechanical report mismatches and ten open gates.
- The checkout has no Git baseline; no Git diff or worktree claim is made.

## Acceptance and evidence IDs

- Acceptance: `S236`, `D183-AC01`
- Evidence: `D183-GRADIENT-CONTRACT-PROBE=PASS`, `D183-SURFACE-ENDPOINT-CONTRAST-PROBE=PASS`, `D183-SIMPLIFICATION-ASSESSMENT=PASS`

## Next owner and next action

- Owner: `release owner`
- Action: obtain authorized runtime/release evidence before changing the explicit no-go dossier.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `5E946F4BE8271F2F5409E18142804E448F4D6AEAC55A000D609C327D818E7BF4` / `38,556,605` bytes
- Source revision: `tree-sha256:998e396986d18fb6846cbabb1667029854d0ec040f31b409c1f57daf9127af69`
- Packaging note: portable one-file candidate rebuilt successfully; no launch.

## Disposition

Accepted with limits. The visual projection is statically verified; native
rendering and external release gates remain explicit follow-up work.

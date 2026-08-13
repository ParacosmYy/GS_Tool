# Handoff: 2026-08-10-ui-51-tab-rail

| Field | Value |
|---|---|
| ID | `2026-08-10-ui-51-tab-rail` |
| Delivery / slice | `UI-51 document-tab rail state clarity` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

The document rail now has a more deliberate modern hierarchy: selected tabs
have a strong accent edge, long names elide in the middle, focus is visible,
ordinary tabs use a quiet surface, and close-button hover/pressed states are
explicit. Modified tabs retain their existing marker. Paper/Sand muted text is
also readable across all light surfaces.

## Scope and boundaries

### In scope

- Stable `documentTabBar` identity and native document-mode presentation hints.
- Middle elision and non-expanding tab layout.
- Centralized normal/hover/selected/focus/disabled/close-button QSS states.
- Paper/Sand `text_muted` contrast correction.
- Source, contrast, static, package, handoff, and release evidence.

### Out of scope

- No tab signals, index mapping, close behavior, dirty policy, modified icon
  semantics, document/editor/service/persistence/locale contract changed.
- No custom tab widget, custom painting, widget-local stylesheet, new icon
  asset, worker, or mutable state.
- No Qt launch, screenshot, native rendering, accessibility/DPI/font,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Turing the 3rd / Luna max | Read-only UI-51 boundary consultation; `NO_CONCLUSION` after bounded windows |
| Token confirmation | Archimedes the 3rd / Luna max | Read-only Paper/Sand contrast check; `NO_CONCLUSION` after bounded windows |
| Independent review | Boyle the 3rd / Luna max | Read-only tab rail/QSS review; `NO_CONCLUSION` after bounded windows |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_tab_surface.py` — native tab-bar
  identity, document mode, truncation, and compact layout hints.
- `src/quillforge/presentation/theme.py` — document-rail state selectors and
  Paper/Sand muted-text token correction.
- `docs/adr/0128-document-tab-rail-state-clarity.md` — visual decision and
  invariants.
- UI-51 parent/independent review records, handoff/index, acceptance/delivery
  register, roadmap/spec/task/release records.

## Decisions and constraints

- `DocumentTabSurface` remains the only tab widget owner; `theme.py` remains
  the only QSS/token source.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 UI code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A` for this slice.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `UI-51-TAB-RAIL-SOURCE-PROBE=PASS` | `PASS` | Semantic tab-bar setup and centralized state selectors. |
| `UI-51-TAB-RAIL-CONTRAST-PROBE=PASS` | `PASS` | Three themes × four accents; relevant text/accent pairs at 4.5:1 or better. |
| `python -m compileall -q src/quillforge` | `PASS` | No launch or QApplication instantiation. |
| `uv run ruff check src/quillforge` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge` | `PASS` | 119 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| `UI-51-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | SHA `6EC45C1FC5A070C022BB92797B8E6DD3E8AFFFF1FBD887BE9900A354446C3952`; 38,487,749 bytes; source `tree-sha256:e134a4e118d906d710c1c95dde12619cb0cd4d88fefef757df62ab86fe3876b3`. |
| `scripts\verify_release_handoff.ps1` wrapped expected NO-GO | `PASS` | Dossier reports `no-go`, the three expected mechanical report-binding failures, and 10 open gates. |
| `UI-51-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Dossier artifact SHA/size matches the UI-51 manifest and root/dist candidate. |

## Unrun checks and reason

- Native Qt tab painting, close-button glyph metrics, keyboard focus, screen
  readers, fonts, DPI, startup, clean-machine, cross-machine, hardware,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS and contrast checks do not prove native Qt selector specificity,
  close glyph metrics, font fallback, or visual preference.
- All delegated UI-51 review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; report
  binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `UI-51-AC01`, `S132`.
- Evidence: ADR-0128, UI-51 source/contrast probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, `UI-51-RELEASE-DOSSIER-PROBE=PASS`, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: run synchronized handoff/repository/traceability/release checks and
  continue the next highest-value visual or MainWindow/application slice.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `6EC45C1FC5A070C022BB92797B8E6DD3E8AFFFF1FBD887BE9900A354446C3952` /
  `38,487,749` bytes.
- Source revision: `tree-sha256:e134a4e118d906d710c1c95dde12619cb0cd4d88fefef757df62ab86fe3876b3`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: document-tab rail states and Paper/Sand muted-text
contrast are improved through the existing presentation boundary, while native
runtime and enterprise release gates remain open.

# Handoff: 2026-08-13-d300-settings-choice-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-13-d300-settings-choice-hierarchy` |
| Delivery / slice | `D300 / UI-119 / ARCH-270 Settings choice hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T05:30:00+08:00` |

## User outcome

The Settings theme and accent selectors now stand out as the shell-identity
choices: they use a distinct surface, stronger semantic edge, and explicit
hover/focus/open states. Existing values, swatches, language behavior, and
save/apply contracts remain unchanged.

## Scope and boundaries

### In scope

- Centralized QSS for `settingsTheme` and `settingsAccent`.
- Static object-name/state contract and theme/accent contrast probe.
- Source diagnostics, package rebuild, artifact identity, and delivery records.

### Out of scope

- Settings values/schema/persistence, locale, preview data, keyboard routing,
  custom widgets, native rendering, screenshots, clean-machine behavior,
  signing, installer, updater, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | User-visible hierarchy outcome and acceptance |
| Developer 1 | parent | Central QSS selector implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, contrast, archive, identity, and non-destructive verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — semantic QSS states for the two
  shell-identity selectors.
- `scripts/audit_presentation_contracts.py` — targeted object/state contract.
- ADR, review, acceptance, delivery-register, roadmap, todo, plan, and handoff
  records for D300.

## Decisions and constraints

- Keep the visual owner in centralized `presentation.theme`; do not duplicate
  styles in `SettingsDialog` or introduce a custom combo widget.
- Preserve existing combo item order/data, swatch icons, signals,
  `settings_snapshot()`, locale refresh, persistence, and application wiring.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and matrix probes were authorized.

## Review and source applicability

Architecture consultation: `NO_CONCLUSION` after three bounded waits.
Independent review: `NO_CONCLUSION` after three bounded waits. Parent review:
`PASS`. Simplification assessment: `PASS`.

This is Python 3.12/PyQt6 desktop presentation code. The Qt Company, `Qt Style
Sheets Reference`, Qt 6.11.1, QComboBox box-model and pseudo-state sections,
https://doc.qt.io/qt-6/stylesheet-reference.html, is an engineering reference;
public embedded-vendor source applicability is N/A. No manufacturer, MCU, SDK, RTOS, MISRA, ISO 26262,
ASPICE, certification, or private ByteDance-standard claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run ruff format --check src scripts` | PASS | 149 files formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Choice hierarchy and contrast contracts pass. |
| D300 QSS matrix probe | PASS | 3 themes × 4 accents; normal/hover/focus/on fragments and contrast checked. |
| source `--diagnose-startup --report ...` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md --report ...` | PASS | One startup path opened; no window/exec entered. |
| `scripts/check.ps1` | PASS | Project checks pass after source and handoff synchronization. |
| package identity / PE/archive | PASS | Current root/dist copies and manifest match; PE/archive contents inspected. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and enterprise gates remain open. |

## Unrun checks and reason

- Independent review conclusion — three bounded waits returned no result; no
  independent PASS is claimed.
- Native Qt/EXE rendering, screenshot, focus/accessibility, DPI, and
  clean-machine behavior — prohibited by `software_start_allowed=false`.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this local UI slice.

## Known risks and limits

- Native Qt selector specificity and `:on` painting are statically reasoned
  from the current Qt contract, not visually exercised.
- The stronger accent edge may need adjustment after an authorized native
  visual review at supported DPI/font combinations.
- Release remains no-go until artifact-bound runtime and enterprise gates are
  refreshed by an authorized operator.

## Acceptance and evidence IDs

- Acceptance: `S340`
- Evidence: `D300-SETTINGS-CHOICE-HIERARCHY=PASS`, QSS matrix, source
  diagnostics, package identity, PE/archive checks, and review records.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native visual/focus review if the settings hierarchy needs
  pixel-level acceptance.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `2DB5144231C0F97F4578F11FC8A68C16DCDCC8B1A0ED0246451383B5DFEBD7AC` / `38594315` bytes
- Source revision: `tree-sha256:2de3d3c2eebd7e9b6afd0db4af1b3780d1593af9c19f5dec9369b055cc363120`

## Disposition

Accepted with limits. D300 is source, matrix, diagnostic, package, PE/archive,
and handoff verified; independent review, native visual rendering,
clean-machine behavior, and remaining release gates remain open.

# Handoff: 2026-08-11-d166-font-choice-preview

| Field | Value |
|---|---|
| ID | `2026-08-11-d166-font-choice-preview` |
| Delivery / slice | `D166 / UI-78 / ARCH-153 font-choice preview` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T05:30:00+08:00` |

## User outcome

The Settings page now previews each supported interface and editor font in
its own family inside the existing selectors. Font family and size choices
remain persisted and applied through the existing settings path.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/settings_dialog.py`.
- `QFont` item previews through `Qt.ItemDataRole.FontRole`.

### Out of scope

- No font allowlist, fallback policy, settings schema, persistence, locale,
  theme, size range, motion, editor, service, or application policy change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Epicurus the 5th / Luna max | `PASS` with bounded conditions |
| Independent review | Singer the 5th / Luna max | `NO_CONCLUSION`; no Git baseline for complete-diff proof |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — adds the private
  `_apply_font_previews()` projection and its two call sites.
- `docs/adr/0215-font-choice-preview.md`.
- `docs/agent-team/reviews/D166-font-choice-preview-parent-review.md`.
- `docs/agent-team/reviews/D166-font-choice-preview-independent-review.md`.
- Traceability files under `docs/`, `tasks/`, and `docs/handoffs/index.json`.

## Decisions and constraints

- The existing SettingsDialog remains the sole owner of this visual
  projection; no new font policy, service, persistence path, or global style
  source was introduced.
- `FontRole` is presentation metadata only. Existing `UserRole` values,
  `currentText()` reads, allowlists, and settings application remain the
  source of truth.
- The parent is the sole shared-checkout writer. No Git/worktree operation
  was used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D166-AST-PROBE=PASS`.
- `D166-FONT-ROLE-SHAPE-PROBE=PASS`.
- `D166-SETTINGS-CONTRACT-PROBE=PASS`.
- `D166-COMPILEALL=PASS` through `scripts/check.ps1`.
- `D166-RUFF=PASS`.
- `D166-FORMAT=PASS`.
- `uv run ruff check src/quillforge/presentation/settings_dialog.py` — pass.
- `uv run ruff format --check src/quillforge/presentation/settings_dialog.py` — pass.
- `D166-PACKAGE-BUILD=PASS` through `scripts/package.ps1`.
- `D166-PACKAGE-IDENTITY-PROBE=PASS`.
- `D166-CHECK=PASS` through `scripts/check.ps1`.
- `D166-VERIFY-HANDOFF=PASS` through `scripts/verify_handoff.ps1`.
- `D166-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Settings popup rendering, installed-font fallback metrics, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Qt styles or custom delegates may not honor `FontRole` identically on every
  Windows style; the popup must be visually confirmed in an authorized
  runtime environment.
- An unavailable installed font may fall back to a system font, as already
  stated by the Settings dialog; static evidence does not prove fallback
  metrics or readability at every DPI.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S219`, `D166-AC01`.
- Evidence: ADR-0215, parent/independent review records, D166 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize a native Settings popup review on the target Windows
  environment, record installed-font/DPI evidence, and keep the release
  decision unchanged until the external gates are satisfied.

## Artifact information

The D166 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `83EFCCE2FF49B569C01AE5BEC6B3E700C86AE7F5EBFD2AA473C6038D693403D9`.
- Size: `38549368` bytes.
- Source revision: `tree-sha256:ff675c24d79eebe1b39e52bd1eb521ee8c80a5355adb33896107b63bdef916d3`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: the user-visible font preview is delivered; native
rendering, fallback metrics, and external release evidence remain open.

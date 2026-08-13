# Handoff: 2026-08-11-ui-65-semantic-message-tooltip

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-65-semantic-message-tooltip |
| Delivery / slice | UI-65 semantic message and tooltip chrome |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T11:00:00+08:00 |

## User outcome

About, error, unsaved-close, recovery, and tooltip feedback now use a shared
modern frame: semantic top accents, readable primary/informative text, and
consistent button/tooltip targets across the existing themes.

## Scope and boundaries

### In scope

- Existing `QMessageBox` and `QToolTip` selectors in centralized `theme.py`.
- Semantic object-name accents and informative-label readability.
- Existing button target rhythm and token reuse.

### Out of scope

- No `MessageSurface` or `RecoveryPromptSurface` object names, text, locale,
  button roles, `exec()` flow, return decisions, or application policy changed.
- No new token, widget, signal, runtime effect, async path, worker, or test-only
  asset.
- No QApplication launch, native rendering capture, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Heisenberg the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Noether the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — QMessageBox/QToolTip QSS only.
- `docs/adr/0158-semantic-message-tooltip-chrome.md`
- `docs/agent-team/reviews/UI-65-semantic-message-tooltip-parent-review.md`
- `docs/agent-team/reviews/UI-65-semantic-message-tooltip-independent-review.md`

## Decisions and constraints

- Existing object names and global role selectors remain the visual contract;
  no dialog implementation was duplicated.
- Existing `ThemeColors` and font-size inputs remain the only visual sources.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `UI65-MESSAGE-QSS-SOURCE-PROBE=PASS` | PASS | Existing dialog object names and tooltip/message selectors are covered. |
| `UI65-THEME-CONTRAST-PROBE=PASS` | PASS | Primary/informative text remains readable for every theme/accent combination. |
| `UI65-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `UI65-RUFF=PASS` | PASS | Target source passed lint. |
| `UI65-FORMAT=PASS` | PASS | Target source already formatted. |
| `UI65-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `F4CC0D743DA000C7C49D5B5EB2DB1A682EB812FBDAC5B95986C52EF58124D5C8`, 38,504,475 bytes, source `tree-sha256:69390bfb41f5c2ced05e3ddd89678f1e02e1807943cc74e3f5b48f360acf84e1`. |
| `UI65-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `UI65-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `UI65-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current UI65 artifact identity. |
| `UI65-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native Qt dialog rendering, DPI/font metrics, accessibility, QApplication
  startup, real interaction timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS generation cannot prove native QMessageBox layout or tooltip
  geometry on every Windows style/DPI configuration.
- Heisenberg architecture and Noether independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S162`, `UI-65-AC01`.
- Evidence: ADR-0158, source/contrast probes, parent and independent review
  records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded visual-quality or MainWindow/application
  contract slice and complete authorized runtime/release gates when authority
  and environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after source
and record synchronization:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `F4CC0D743DA000C7C49D5B5EB2DB1A682EB812FBDAC5B95986C52EF58124D5C8`
- Size: `38504475` bytes
- Source revision: `tree-sha256:69390bfb41f5c2ced05e3ddd89678f1e02e1807943cc74e3f5b48f360acf84e1`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: semantic dialog/tooltip chrome is centralized while
native/runtime/release evidence remains open.

# Handoff: 2026-08-11-d180-font-style-settings-contract

| Field | Value |
|---|---|
| ID | `2026-08-11-d180-font-style-settings-contract` |
| Delivery / slice | `D180 / UI-92 / ARCH-167 Font style settings contract` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T22:00:00+08:00` |

## User outcome

D180 delivers actual interface and editor font-style switching. Settings now
offers localized regular/semibold/bold/italic controls for both surfaces,
projects the pending choices in the preview, persists them in schema v3, and
applies them to the global interface stylesheet and every live editor tab after
Save. Existing locale, theme, accent, size, motion, wrapping, line-number,
Save/Cancel, and application-policy behavior remains in its established owner.

## Scope and boundaries

### In scope

- Qt-free `FontStyle` contract with `regular`, `semibold`, `bold`, and `italic`.
- Schema-v3 normalization and v1/v2 fallback for absent style fields.
- Atomic JSON encode/decode for interface and editor style values.
- Localized settings controls, pending preview, QSS projection, QFont
  projection, and editor-adapter application.
- Static, package, manifest, handoff, and expected release no-go evidence.

### Out of scope

- No editor operation, lexer choice, theme token, plugin contract,
  persistence location, Save/Cancel decision, or application policy change.
- No GUI/EXE launch, screenshot, unit test, test asset, hardware action,
  flashing, deployment, clean-machine, or cross-machine evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent agent | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product scope | User outcome and acceptance |
| Developer 1 | Parent agent, sole writer | Domain/application/infrastructure slice |
| Developer 2 | Parent agent, sole writer | Presentation/integration/packaging slice |
| QA | Parent agent read-only validation | Deterministic checks and unrun evidence |

## Changed files and modules

- `src/quillforge/domain/models.py` — shared Qt-free style value contract.
- `src/quillforge/application/settings.py` — schema v3 validation/fallback.
- `src/quillforge/infrastructure/settings_store.py` — JSON compatibility.
- `src/quillforge/presentation/font_style.py` — shared Qt projection helper.
- `src/quillforge/presentation/theme.py` — interface QSS style projection.
- `src/quillforge/presentation/settings_dialog.py` — localized controls and
  snapshot/preview wiring.
- `src/quillforge/presentation/settings_preview.py` — pending QFont preview.
- `src/quillforge/presentation/editor_document_surface.py` and
  `editor_widget.py` — live editor application.
- `src/quillforge/presentation/i18n.py` — English/Simplified Chinese labels.

## Decisions and constraints

- Domain remains Qt-free; application owns validation; infrastructure owns
  JSON; presentation owns Qt projection and preview.
- Reuse the existing settings-save projection coordinator and editor adapter;
  no second styling system or cross-layer policy move was introduced.
- Public applicability is Python 3.12/PyQt6 only. Public CloudWeGo material is
  an engineering reference, not a private ByteDance standard or compliance
  claim. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
  applicable.
- Shared checkout writer: parent agent only, current local checkout.
- Runtime launch policy: not allowed; no QApplication/window or EXE launch.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Settings contract/migration JSON probe | PASS | schema v3, legacy defaults, invalid fallback, round-trip |
| Bilingual i18n style probe | PASS | en-US and zh-CN, style placeholder present |
| QSS style matrix | PASS | 3 themes × 4 accents × 4 styles |
| QFont projection probe | PASS | regular/semibold/bold/italic mapping |
| Source wiring probe | PASS | model, dialog, theme, editor seams present |
| `uv run python -m compileall -q src` | PASS | source compilation |
| Ruff check and format check | PASS | 10 changed source files |
| `scripts/check.ps1` | PASS | project/handoff/presentation checks |
| `scripts/verify_handoff.ps1` | PASS | indexed handoff and status contract |
| `scripts/package.ps1` | PASS | PyInstaller one-file package |
| Package identity probe | PASS | SHA/size/root-copy/manifest match |
| Release dossier invariant probe | PASS | expected no-go, 10 open gates, 3 mechanical failures |

## Unrun checks and reason

- Native Qt dialog/editor layout, painting, font fallback, metrics, DPI,
  accessibility, screenshots, and runtime startup — prohibited by the active
  no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  release-owner, and fresh artifact-bound runtime reports — require an
  authorized environment or external decision.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt font fallback, weight availability, italic metrics, and dialog
  height may vary by installed fonts and DPI; native visual review remains
  open.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S233`, `D180-AC01`.
- Evidence: ADR-0229, parent/independent review records, D180 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows review of interface/editor font fallback,
  weight/italic rendering, localized settings layout, DPI, and accessibility.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0` portable candidate.
- SHA-256 / size: `6C9B71E3A3A27F7AA3CE69998B91084B32663026BE5E4DD574381805AA5EFAF7` / `38555233` bytes.
- Source revision: `tree-sha256:b6c9bf13d312ccb8ae2df8414bd80d12a5c9e4e0e5495fcf61ffb400ecaa9670`.
- Packaging note: rebuilt PyInstaller one-file portable candidate without
  launching QuillForge.

## Disposition

`accepted-with-limits`: font-style controls, persistence, preview, projection,
and package evidence are delivered; native rendering, accessibility, and
external release evidence remain open.

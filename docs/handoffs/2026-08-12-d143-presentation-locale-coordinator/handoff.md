# Handoff: 2026-08-12-d143-presentation-locale-coordinator

| Field | Value |
|---|---|
| ID | 2026-08-12-d143-presentation-locale-coordinator |
| Delivery / slice | D143 / ARCH-125 presentation locale coordinator |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T09:30:00+08:00 |

## User outcome

Locale refresh now has one typed Qt-free orchestration boundary. Changing the
persisted language continues to update the title, command surfaces, dialogs,
workspace/editor/tab/status/search/plugin surfaces in the established order;
absent optional workspace/search surfaces remain safe no-ops.

## Scope and boundaries

### In scope

- `PresentationLocaleCoordinator` and `PresentationLocalePorts`.
- MainWindow locale callback composition and one-line delegation.
- Static/inline validation, package identity, and traceability records.

### Out of scope

- No translation catalog, locale values, font/theme/motion settings, widget
  behavior, signal, service, persistence, or application policy rewrite.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Meitner the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Linnaeus the 4th / Luna max | `NO_CONCLUSION` after two short waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/presentation_locale_coordinator.py` — new
  Qt-free locale sequence contract and coordinator.
- `src/quillforge/presentation/main_window.py` — named Ports composition and
  `_retranslate_ui` delegation.
- `docs/adr/0187-presentation-locale-coordinator.md`
- `docs/agent-team/reviews/D143-presentation-locale-parent-review.md`
- `docs/agent-team/reviews/D143-presentation-locale-independent-review.md`

## Decisions and constraints

- The coordinator owns ordering only; MainWindow retains concrete surfaces,
  dynamic-surface checks, translations, settings, and policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D143-LOCALE-SEQUENCE-PROBE=PASS`
- `D143-LOCALE-OPTIONAL-SURFACE-PROBE=PASS`
- `D143-QT-FREE-LOCALE-PROBE=PASS`
- `D143-MAINWINDOW-WIRING-PROBE=PASS`
- `D143-COMPILEALL=PASS`
- `D143-RUFF=PASS`
- `D143-FORMAT=PASS`
- `D143-CHECK=PASS`
- `D143-VERIFY-HANDOFF=PASS`
- `D143-PACKAGE-BUILD=PASS`
- `D143-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native rendering, runtime startup, event-loop timing,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes cannot prove native translation rendering, font fallback,
  accessibility, or event-loop timing on every Windows environment.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S191`, `D143-AC01`.
- Evidence: ADR-0187, parent/independent review records, D143 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application decomposition slice
  and complete authorized runtime/release gates when authority and environment
  permit.

## Artifact information

The candidate was rebuilt after the source change without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `B6C23EE8AF4D5FEE054F896A1082231528318EC007664A2939AD4AAEA6A3BD91`
- Size: `38536972` bytes
- Source revision: `tree-sha256:daf21377f0b8f1568b7f4319862bee3862166821c2356129e30b35824ba3fdf5`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: the locale refresh orchestration boundary is closed;
native rendering, runtime, release, and external evidence gates remain open.

# Handoff: 2026-08-11-d170-command-rail-visual-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d170-command-rail-visual-hierarchy` |
| Delivery / slice | `D170 / UI-82 / ARCH-157 Command-rail visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T13:30:00+08:00` |

## User outcome

The command rail now presents as a lighter, more breathable shell card. Its
buttons have a consistent modern hit height and radius, quiet actions recede,
and the context label remains easy to scan. Existing command labels, icons,
shortcuts, callbacks, and state semantics remain unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` command-rail and toolbar-context QSS.
- Surface/border/radius/spacing/hit-height/weight refinement.
- Static selector and contrast projection across supported themes and accents.

### Out of scope

- No `command_surface.py`, command registry, action callback, signal,
  shortcut, icon contract, locale, theme-token, domain, application, or
  persistence change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Raman the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Copernicus the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized command-rail visual
  hierarchy only.
- `docs/adr/0219-command-rail-visual-hierarchy.md`.
- D170 parent/independent review records and traceability files.

## Decisions and constraints

- `CommandSurface` remains the sole owner of menu/toolbar construction and
  command behavior; `theme.py` remains the sole owner of this visual rule.
- Existing foreground derivation remains authoritative for bright/dark and
  砂金 accent endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D170-COMPILEALL=PASS`.
- `D170-RUFF=PASS` and `D170-FORMAT=PASS`.
- `D170-QSS-CONTRACT-PROBE=PASS`.
- `D170-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; normal, hover,
  primary, and focus checks passed at the bounded 4.5 ratio floor.
- `D170-STATE-CONTRAST-PROBE=PASS`.
- `D170-PACKAGE-BUILD=PASS`.
- `D170-PACKAGE-IDENTITY-PROBE=PASS`.
- `D170-CHECK=PASS`, `D170-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt painting, toolbar metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different toolbar text/icon spacing than the
  static source projection; runtime visual review remains open.
- Larger user-selected fonts may change the final toolbar width; native DPI
  and font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S223`, `D170-AC01`.
- Evidence: ADR-0219, parent/independent review records, D170 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of command-rail spacing,
  focus, and large-font behavior before closing the remaining runtime gates.

## Artifact information

The D170 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `09608EA39AA7C6278AD70F98E90E29C6577EB70D2D88EE30CA73BB6DC80262EE`.
- Size: `38550855` bytes.
- Source revision: `tree-sha256:71d3c175d90225f7045d1a9af105c2395f0f9e23eeb03108b447b9cbfd937fdc`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: command-rail visual hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.

# Handoff: 2026-08-12-d310-safe-startup-recovery

| Field | Value |
|---|---|
| ID | `2026-08-12-d310-safe-startup-recovery` |
| Delivery / slice | `D310 / ARCH-280 Safe startup recovery path` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:55:00+08:00` |

## User outcome

Users who cannot reach the normal QuillForge shell can launch
`QuillForge.exe --safe-mode` to get a clean editor shell. The mode uses
product-default appearance settings, skips persisted session/recovery restore
and built-in plugin activation, and keeps explicit file paths available.

## Scope and boundaries

In scope: dispatcher flag handling, composition-root safe-mode boundary,
default settings, restore/plugin bypass, startup-path preservation, static
contract, package evidence, and documentation.

Out of scope: normal startup behavior, settings/session migration, recovery
deletion, plugin policy changes, locale catalog, widget styling, native launch,
installer, signing, updater, or release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Recoverable startup outcome |
| Developer 1 | parent | Dispatcher and composition safe-mode path |
| Developer 2 | parent | Static contract and source composition probe |
| QA | parent | Diagnostics, package, archive, and handoff verification |

## Changed files and modules

- `src/quillforge/app.py` — recognizes and strips `--safe-mode` before Qt
  argument parsing and forwards the mode to the composition root.
- `src/quillforge/composition.py` — carries the mode through `DesktopRuntime`,
  uses `DEFAULT_SETTINGS`, and skips normal plugin/restore activation only in
  safe mode.
- `scripts/audit_presentation_contracts.py` — guards the mode boundary and
  ordering.
- `README.md`, ADR, reviews, acceptance, delivery register, roadmap, plan,
  todo, release handoff, and this handoff record.

## Decisions and constraints

- The existing composition root remains the owner; no second runtime or
  settings schema field was introduced.
- Safe mode is explicit and reversible. It does not write settings, session,
  or recovery data.
- Explicit startup paths remain after the flag is removed.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup remains prohibited by `software_start_allowed=false`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Safe-mode source probe | `D310-SAFE-MODE-PROBE=PASS flag=1 strip=1 default_settings=1 skip_restore=1 skip_plugins=1 startup_paths_preserved=1` |
| Safe-mode source composition | `D310-SAFE-MODE-COMPOSITION=PASS safe_mode=true initial_document=true` |
| Source startup diagnostic | `PASS status=passed failed=0 window_shown=0 event_loop_entered=0` |
| Source regular-file-open diagnostic | `PASS status=passed startup_paths_total=1 startup_paths_open=1 window_shown=0 event_loop_entered=0` |
| Compileall | `PASS` |
| Ruff | `PASS` |
| Format | `PASS` |
| Presentation audit | `PASS` |
| Package identity | `D310-PACKAGE-IDENTITY=PASS root_dist_same=True sha=27EA28D88C4BBEC964632BEB85DDD40D1162FD04E0516F97C39ECD5C94156361 bytes=38599117 source=tree-sha256:bfbfb0b593719ac0d81e4a81ec15c783643e4de130812753adf9cc6074c243f4` |
| PE/archive | `D310-PE-ARCHIVE=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI outer_entries=169 qwindows=1 qt6_dlls=7 qsci=1 pyqt_runtime_hook=1` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after three bounded Luna/max waits |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, actual window creation, safe-mode visual interaction,
screen-reader output, DPI, clean-machine behavior, real DLL loading, signing,
installer, updater, and release-owner checks remain unrun because the active
policy prohibits native launch and external release actions. No unit tests,
mocks, fixtures, or test harnesses were created or run.

## Known risks and limits

Safe mode is source- and composition-verified but does not prove that the
Windows bootloader, Qt platform plugin, graphics driver, or unsigned artifact
can start on another machine. It also deliberately omits session/recovery
restore and built-in plugin activation, so those features must be reintroduced
by a later normal launch after the blocking state is addressed. The independent
review returned no conclusion after three bounded waits.

## Acceptance and evidence IDs

- Acceptance: `S350`.
- Architecture slice: `ARCH-280`.
- Evidence: `D310-SAFE-MODE-PROBE=PASS`,
  `D310-SAFE-MODE-COMPOSITION=PASS`, source diagnostics, static checks,
  package identity, PE/archive checks, review records, and expected release
  NO-GO.

## Next owner and next action

- Owner: Project Manager (QuillForge) / authorized QA.
- Action: on an authorized Windows x64 machine, launch the current candidate
  with `--safe-mode` once and attach the artifact-bound startup report; then
  repeat normal startup after reviewing any persisted-state blocker.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `27EA28D88C4BBEC964632BEB85DDD40D1162FD04E0516F97C39ECD5C94156361` /
  `38599117` bytes
- Source revision:
  `tree-sha256:bfbfb0b593719ac0d81e4a81ec15c783643e4de130812753adf9cc6074c243f4`

## Disposition

Accepted with limits. D310 adds a documented, source-verified recovery launch
path and rebuilds the portable candidate. Native safe-mode startup and the
remaining enterprise release gates remain open.

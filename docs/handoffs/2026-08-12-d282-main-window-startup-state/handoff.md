# Handoff: 2026-08-12-d282-main-window-startup-state

| Field | Value |
|---|---|
| ID | `2026-08-12-d282-main-window-startup-state` |
| Delivery / slice | `D282 / ARCH-252 MainWindow startup-state initialization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The desktop shell now establishes its initial busy state before constructing
coordinator ports and callback closures that can consult it. A static contract
guards that ordering so another startup regression is caught before packaging.

## Scope and boundaries

- Moved the existing `_busy = False` default to the beginning of
  `MainWindow.__init__`.
- Added a Qt-free AST/source contract for the default assignment and first
  busy callback capture.
- Preserved all runtime busy transitions and coordinator ownership.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Gibbs consultation | Startup ordering boundary and integration review |
| Developer | parent | MainWindow initialization and audit contract |
| QA | parent | Static, source-diagnostic, archive, package, and handoff verification |
| Independent reviewer | Sagan consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — early `_busy` default.
- `scripts/audit_presentation_contracts.py` — startup-state ordering contract.
- D282 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep busy-state ownership in `MainWindow`; do not introduce a second state
  source or coordinator placeholder.
- Audit only construction-time default ordering; allow later operation
  transitions to set `_busy` normally.
- The package was rebuilt from the current source and is bound below.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, and package evidence was authorized.

## Public-source applicability

Python 3.12 `ast` documentation is the applicable first-party reference for
the static contract. No manufacturer requirement applies; this delivery has no
embedded C/C++, MCU, BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware
change. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup-state contract | `D282-STARTUP-STATE-CONTRACT=PASS default_before_callbacks=1 runtime_transitions_preserved=1` |
| Presentation audit | `D282-PRESENTATION-AUDIT=PASS` |
| Source startup diagnostic | `D282-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed settings_preflight=passed` |
| Compileall | `D282-COMPILEALL=PASS` |
| Ruff/formatting | `D282-RUFF=PASS`; `D282-FORMAT=PASS` |
| Project checks | `D282-CHECK=PASS` |
| PE header | `D282-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI` |
| Frozen archive | `D282-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=7 embedded_startup_module=True` |
| Root/dist identity | `D282-MANIFEST-COPY=PASS bytes=38584830 sha=7C3978E997ED511B1D96D34771E78F31350FF69B365E83F37618A0B29CF883BF` |
| Source revision | `tree-sha256:ccd4d37649d8127dd585eca34d0e6ef3e966cd1f704bfe740d173a10382a16a3` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded wait and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, clean-machine behavior, signing, installer/update,
registry, cross-machine repeatability, and release-owner acceptance remain
unrun under the active no-launch/non-destructive policy. Release verification
is expected to remain `NO-GO` with artifact-bound mechanical failures and
remaining enterprise gates open.

## Known risks and limits

The change closes one construction-order risk but does not prove that every
Qt constructor or callback remains non-reentrant. Native startup and runtime
event ordering still require authorized execution on the target environment.

## Acceptance and evidence IDs

- Acceptance: `S322`.
- Architecture slice: `ARCH-252`.
- Evidence: `D282-STARTUP-STATE-CONTRACT=PASS`, `D282-CHECK=PASS`,
  `D282-MANIFEST-IDENTITY=PASS`,
  `D282-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D282-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D282-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the candidate still shows no window, run the documented
  no-window preflight and attach its JSON report plus startup-error log.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size:
  `7C3978E997ED511B1D96D34771E78F31350FF69B365E83F37618A0B29CF883BF` /
  `38,584,830` bytes.
- Source revision:
  `tree-sha256:ccd4d37649d8127dd585eca34d0e6ef3e966cd1f704bfe740d173a10382a16a3`.

## Disposition

`accepted-with-limits`: the construction-time busy-state window is closed and
guarded statically; native startup and enterprise release gates remain open.

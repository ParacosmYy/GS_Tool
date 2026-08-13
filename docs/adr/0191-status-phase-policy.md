# ADR-0191: status-phase policy

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D145 / ARCH-129

## Context

`MainWindow._sync_status_surface()` mixed application facts with the shell
phase priority: retained worker work had to win over dirty-document attention,
which had to win over clean readiness. The Qt status surface should render the
phase, not own or duplicate that policy.

## Decision

Add a pure `StatusPhase` contract and a frozen/slotted `StatusPhaseInput`, then
project those facts through the Qt-free `StatusPhaseCoordinator`. The
coordinator preserves the existing priority exactly:

1. `busy` or `pending_work` -> `working`;
2. otherwise `active_document_dirty` -> `attention`;
3. otherwise -> `ready`.

`MainWindow` still reads its concrete tracker/runner/tab state and passes it to
the coordinator. `StatusSurface` and `StatusRail` still own Qt rendering,
locale, accessibility description, and style refresh. The direct `error`
projection in `_show_error()`, notification levels, close policy, task
ownership, and recovery behavior remain unchanged.

## Alternatives rejected

- Keeping the conditional in `MainWindow` would leave deterministic policy
  coupled to the Qt composition root.
- Moving tracker or tab queries into the coordinator would leak application
  objects and weaken the boundary.
- A generic state machine or event bus would add complexity to a three-branch
  policy with no current need.

## Review and evidence

Archimedes the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Harvey the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because the typed input plus one deterministic coordinator method is the
smallest explicit policy boundary.

Authorized non-destructive evidence:

- `D145-STATUS-PHASE-PROBE=PASS`
- `D145-QT-FREE-PROBE=PASS`
- `D145-COMPILEALL=PASS`
- `D145-RUFF=PASS`
- `D145-FORMAT=PASS`
- `D145-CHECK=PASS`
- `D145-VERIFY-HANDOFF=PASS`
- `D145-PACKAGE-BUILD=PASS`
- `D145-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `1ED2AEE6E31F46C15E4793AECC275EB9F1044FC479945A5543FCAF3BDCC085E3`
- bytes: `38538238`
- source revision: `tree-sha256:0daa277e72dbce922b4017a603fe3718aed5dd2d96ae777a1749232c93ed43a6`

Public-source applicability is Python 3.12/PyQt6 presentation composition;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Qt-free/static probes do not prove native status-bar rendering, queued event
timing, accessibility, font/DPI metrics, runtime startup, clean-machine or
cross-machine behavior, signing, installer/update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.


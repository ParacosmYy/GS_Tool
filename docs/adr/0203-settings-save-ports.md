# ADR-0203: settings-save Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D154 / ARCH-141

## Context

`SettingsSaveCoordinator` already classified asynchronous settings-save
results, but its constructor accepted three independent result callbacks.
The stale/invalid/valid/failure behavior was stable while positional wiring
made the settings projection boundary harder to review and extend.

## Decision

Introduce the frozen/slotted Qt-free `SettingsSavePorts` contract with named
callbacks for valid `SettingsSnapshot` application, invalid-result feedback,
and matching failure projection. Preserve the existing classification:

- stale result or failure: no projection;
- invalid result: invalid-result feedback once;
- valid `SettingsSnapshot`: application once;
- matching failure: failure projection once.

MainWindow retains settings validation and persistence, theme/locale/font/editor
refresh, transition, notification, close, worker, and concrete Qt ownership.
No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable result-path wiring risk.
- Moving validation, persistence, or visual refresh into the Ports contract
  would broaden a presentation composition boundary into application policy.
- Introducing a generic event bus or settings state machine would add a second
  lifecycle abstraction without changing behavior.

## Review and evidence

Bernoulli the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Meitner the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes positional coupling
without adding behavior or policy.

Authorized non-destructive evidence:

- `D154-SETTINGS-BRANCH-PROBE=PASS`
- `D154-STALE-INVALID-VALID-FAILURE-PROBE=PASS`
- `D154-PORTS-IMMUTABILITY-PROBE=PASS`
- `D154-SOURCE-WIRING-PROBE=PASS`
- `D154-QT-FREE-CONTRACT-PROBE=PASS`
- `D154-PRESENTATION-AUDIT=PASS`
- `D154-COMPILEALL=PASS`
- `D154-RUFF=PASS`
- `D154-FORMAT=PASS`
- `D154-PACKAGE-BUILD=PASS`
- `D154-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `C0AD8F26EBE75BAAC577322F65C09EA69F342D6B2D171ED6C279ED984CE90797`
- bytes: `38542726`
- source revision: `tree-sha256:42adbabf07c46b276fd9aaf804f7c86f7222610bfdc60e48edb36c8e0b886354`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native Qt worker/timer
timing, settings filesystem durability, runtime startup, clean-machine or
cross-machine behavior, signing, installer, update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.

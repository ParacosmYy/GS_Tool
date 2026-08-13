# D51 parent review — settings-save callback boundary

## Scope and decision

- **Delivery:** D51 / ARCH-41
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`SettingsSaveTracker` replaces MainWindow's bare settings-save boolean with a
Qt-free positive operation identity and callback classifier. MainWindow keeps
the SettingsSurface dialog, SettingsService, TaskRunner, theme/locale/font/
editor application, transition animation, notifications, and close policy.

The required architecture consultation was attempted with Hooke the 2nd /
Luna max. Two bounded windows returned no conclusion; no architecture PASS is
claimed. The independent review was attempted with Tesla the 2nd / Luna max.
Two bounded windows returned no conclusion and the agent was closed; no
independent PASS is claimed.

## Source findings

- `begin()` prevents a second bound operation and validates positive IDs.
- `complete()` classifies stale callbacks before clearing current state and
  distinguishes invalid `SettingsSnapshot` results from valid ones.
- `fail()` clears only the matching operation, so stale failure callbacks do
  not unlock or hide a newer save.
- MainWindow retains the existing invalid-result error, exception error,
  persisted snapshot application, theme/locale/editor refresh, transition, and
  success notification.
- `closeEvent()` still blocks while a settings save is in flight, and the
  TaskRunner remains the only worker dispatch boundary.

## Simplification assessment

The tracker is the smallest complete extraction: one operation ID, three
read-only lifecycle properties/methods, and no duplicate settings snapshot or
presentation policy. It makes stale-callback behavior explicit without adding
a queue, signal, service, coordinator, or dependency. The MainWindow result
type check remains because it owns user-facing error policy and type narrowing.

## Authorized non-destructive validation

- D51 settings identity/stale and invalid/failure behavior probe — **PASS**.
- D51 source/integration probe — **PASS**.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `scripts\package.ps1` — **PASS**; root/dist portable candidates match at
  38,427,064 bytes with SHA-256
  `808F2EEE1AF0AB35B9B8C128D0E7F903472F9684776FE05141F0D1E2725A8E7C`.
- Handoff/check/release verification is recorded after documentation sync.
- Independent Luna review window — **NO_CONCLUSION** after two bounded waits;
  no independent PASS is claimed.
- No unit tests, mocks, fixtures, test-only assets, QApplication, Qt/EXE
  startup, screenshots, deployment, or hardware operation were created or
  run.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and vendor
requirements are not applicable. Public CloudWeGo sources remain engineering
references only; no private ByteDance standard, certification, or compliance
claim is made.

## Limits and disposition

Static evidence cannot prove native callback timing, actual settings-file I/O,
runtime theme/font rendering, or external release approval. The bounded
callback state is accepted with those limits and remains subject to the open
runtime/release gates.

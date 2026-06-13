# Specs - PRD_105 Serial Station Main Entry

## Goal

Make the existing Serial Station UART workbench visible from the main application navigation, without adding protocol or serial I/O behavior.

## Scope

- Modify `src/core/panels/PanelManager*` only for app-level assembly.
- Reuse existing `src/apps/serial_station/SerialStationWindow`.
- Keep UART logic inside `src/apps/serial_station/`.

## Non-goals

- No new protocol.
- No real COM or virtual serial validation in this iteration.
- No replacement of old `SerialConfigPanel`.

## Architecture Answers

1. Module: `core/panels` assembly of existing `apps/serial_station` app widget.
2. Interface exposed: `SerialStationWindow*` getter and navigation descriptor only.
3. Dependency: `core` may depend on lower app/widget modules for assembly; lower layers do not depend on `core`.
4. Assembly owner: `PanelManager`.
5. Verification: build target, launch probe, and navigation descriptor evidence.
6. Status: user entry improves; device status does not improve.

## Validation

```powershell
cmake --build build --target EmbedDebug --parallel 4
ctest --test-dir build -R "SerialStationWorkbench|SerialStationController|SerialProtocolPanel" --output-on-failure
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```


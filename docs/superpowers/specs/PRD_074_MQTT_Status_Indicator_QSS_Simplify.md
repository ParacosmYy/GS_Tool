# PRD-074 MQTT Status Indicator QSS Simplify Specs

## Goal

Move MQTT connection status indicator colors out of C++ inline `setStyleSheet()` calls and into the three theme QSS files.

## Non-Goals

- Do not change MQTT connection, publish, subscribe, reconnect, or statistics behavior.
- Do not add new classes, CMake entries, resources, or build directories.
- Do not redesign the MQTT panel layout.

## Constraints

- Follow `CLAUDE.md`, `01-project-overview.md`, `02-workflow.md`, `03-architecture.md`, `04-coding-standard.md`, `05-ui-standard.md`, and `07-directory-structure.md`.
- Keep `mqttStatusIndicator` as the stable QSS object name.
- Use a dynamic `state` property with existing state names: `connected`, `connecting`, `disconnected`, `error`.
- Keep theme-specific colors in `resources/themes/*.qss`.

## Acceptance Criteria

- `src/connection/mqtt/MqttWidgetSubs.cpp` and `src/connection/mqtt/MqttWidgetSlots.cpp` no longer call `setStyleSheet()`.
- All three themes define `QLabel#mqttStatusIndicator` styles for the four MQTT states.
- MQTT status text and reconnect/statistics behavior remain unchanged.
- `doctor.ps1 -RunBuild -RunLaunch` succeeds, or the failure reason is recorded.
- `simplify-scan.ps1` report is refreshed.

## Validation Commands

```powershell
rg -n "setStyleSheet" .\src\connection\mqtt\MqttWidgetSubs.cpp .\src\connection\mqtt\MqttWidgetSlots.cpp
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunBuild -RunLaunch
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\simplify-scan.ps1 -MaxFiles 5 -OutFile .\docs\reviews\simplify\latest.md
```

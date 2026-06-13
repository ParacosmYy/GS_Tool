# EmbedDebug

> A modular Qt desktop workbench for embedded debugging: Serial Station, protocol inspection, terminal workflows, waveform analysis, OTA tooling, recording, replay, automation, and engineering diagnostics in one application.

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?style=flat-square)
![CMake](https://img.shields.io/badge/build-CMake%20%2B%20Ninja-064F8C?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2B-0078D4?style=flat-square)
![Status](https://img.shields.io/badge/status-active%20engineering%20build-f59e0b?style=flat-square)

![EmbedDebug interface preview](docs/assets/readme/interface-preview.svg)

## Why It Exists

Embedded debugging is usually fragmented across a serial assistant, a waveform viewer, a protocol decoder, a log recorder, an OTA tool, and one-off scripts. EmbedDebug is being shaped into a single repeatable workstation where firmware engineers can configure links, send commands, inspect frames, capture logs, replay sessions, and move between diagnostics without constantly switching tools.

This repository is not presented as a finished commercial release. It is an active engineering build with strict PRD/spec gates, layered architecture rules, QTest coverage for key paths, and a verified Windows launch path.

## Product Snapshot

| Area | Current state |
|------|---------------|
| Primary app | `EmbedDebug` |
| Primary branch | `feat/embed-debug` |
| User entry point | `EmbedDebug.bat` from the repository root |
| Fast station entry | `EmbedDebug.bat --station serial` |
| Build system | CMake + Ninja, single `build/` directory only |
| UI stack | Qt Widgets, QSS themes, SVG icon pipeline |
| Engineering posture | Constraint-driven development with PRD, Specs, tests, launch verification, and score tracking |

## Capability Matrix

| Capability | Evidence in repo | Engineering | User path | Device validation |
|------------|------------------|-------------|-----------|-------------------|
| Serial Station workbench | `src/apps/serial_station/`, QTest targets, README entry | E5 | U4 for profile/log/command workflow | D1, pure automated tests |
| UART setup workflow | Port discovery, manual COM input, UART summary, connect/disconnect UI | E4 | U3 | D1, real hardware not verified in this repo state |
| Protocol send/parse | `ascii_text`, `modbus_rtu`, `custom_md`, registry tests | E4 | U3 through station workflow | D1 |
| Command history and profiles | Recent commands, `.edserialprofile` save/load, service tests | E5 | U4 | D1 |
| Logs, export, replay preview | Structured log service, export service, replay service, UI flow tests | E4/E5 | U3 | D1 |
| Terminal and data views | terminal, chart, FFT, heatmap, histogram/scatter, dashboard modules | E3/E4 by module | U2/U3 by module | D0-D1 |
| OTA and file workflows | X/Y/ZMODEM modules, HEX/BIN handling, export foundations | E3/E4 by module | U2 | D0-D1 |
| BLE/CAN/MQTT/USB/RTT | Framework code and staged integrations | E2-E4 by module | U1-U2 | D0-D1 unless separately verified |

Status language follows the project three-axis policy:

```text
Engineering: E0 not started -> E5 maintainable closure
User path:   U0 invisible -> U4 complete workflow
Device:      D0 unverified -> D4 real device verified
```

External device capabilities are intentionally not overstated. If a workflow has not been verified with real hardware, the README says so.

## Serial Station

Serial Station is the most active workstation track and lives under `src/apps/serial_station/`.

Current user-facing path:

1. Launch directly with `.\EmbedDebug.bat --station serial`.
2. Select or type a COM port.
3. Configure baud rate, frame format, flow control, DTR, and RTS.
4. Choose a protocol: `ascii_text`, `modbus_rtu`, or `custom_md`.
5. Send ASCII, HEX, or protocol commands.
6. Review TX/RX/system logs and status counters.
7. Export logs or generate replay previews.
8. Save or load reusable UART/protocol/command profiles.

Internal boundary:

```text
ui/ -> SerialStationController -> core/ + protocols/ + services/
workers/ -> core/
core/ -> ISerialProtocol + SerialProtocolRegistry
protocols/<name>/ -> protocol interface + shared/utils only
services/ -> JSON, logs, export, replay, profiles
```

UI panels do not parse bytes or write profile files directly. Protocols do not touch widgets. `MainWindow` and `PanelManager` stay as assembly and navigation layers.

## Architecture

EmbedDebug uses a layered dependency model. New code must follow the constraint documents before implementation.

```text
L6   src/core/                 application coordination, main window, navigation, theme runtime
L5   src/ota/ automation/      scenario workflows: OTA, automation, dashboard, plugin
L4   src/terminal/ chart/ rtt/ data presentation and live inspection
L3   src/connection/ protocol/ serial/ external access and legacy protocol modules
L5A  src/apps/serial_station/  standalone serial workstation with internal boundaries
L2   src/utils/                reusable CRC, HEX, export, logging, cache, and algorithms
L1   src/shared/               shared constants, enums, and lightweight types
L0   src/interfaces/           pure contracts
```

Core rules:

| Rule | Meaning |
|------|---------|
| No feature without PRD | New behavior requires PRD/Specs before code |
| One build directory | Only `build/` is supported |
| No dead C++ files | New `.h/.cpp` files must be registered in CMake |
| No business logic in shell layers | `MainWindow` and `PanelManager` assemble and navigate only |
| Reuse first | CRC, HEX, settings, logs, export, and buffers are shared capabilities |
| Evidence before claims | Build, tests, launch probes, and status records back every state change |

## Quick Start

First run on a configured Windows workstation:

```powershell
.\tools\bootstrap_env.bat
.\EmbedDebug.bat
```

Open directly into Serial Station:

```powershell
.\EmbedDebug.bat --station serial
```

Manual configure and build:

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat
```

The repository only supports `build/`. Do not create `build2/`, `build-debug/`, `build-release/`, or IDE-generated parallel build folders.

## Tooling Shortcuts

```powershell
uv run start-embeddebug
uv run package-embeddebug --skip-build --clean
uv run package-embeddebug --skip-build --clean --zip
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

The `uv` launch helper delegates to `EmbedDebug.bat`; the batch file remains the minimum supported user entry point. Packaging reuses `build/EmbedDebug.exe`, runs Qt deployment, and writes ignored release artifacts under `dist/`.

## Verification

Core local verification:

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial
```

Serial Station focused verification:

```powershell
cmake --build build --target test_serial_command_panel test_serial_port_panel test_serial_station_workbench --parallel 4
.\build\tests\test_serial_command_panel.exe
.\build\tests\test_serial_port_panel.exe
.\build\tests\test_serial_station_workbench.exe
```

Protocol verification:

```powershell
cmake --build build --target test_ascii_text_protocol test_modbus_rtu_protocol test_custom_md_protocol test_serial_protocol_registry --parallel 4
ctest --test-dir build -R "AsciiTextProtocol|ModbusRtuProtocol|CustomMdProtocol|SerialProtocolRegistry" --output-on-failure
```

## Repository Map

```text
GS_Tool/
|-- src/
|   |-- apps/serial_station/   # Serial Station workstation
|   |-- core/                  # application coordination and shared UI runtime
|   |-- connection/            # connection implementations and factories
|   |-- protocol/              # protocol engines, bridges, schemas
|   |-- terminal/              # terminal model, filtering, rendering
|   |-- chart/                 # waveform, FFT, overlays, heatmap, zoom
|   |-- ota/                   # firmware update workflows
|   |-- automation/            # trigger/rule automation
|   |-- dashboard/             # gauge, LED, numeric dashboard widgets
|   |-- rtt/                   # SEGGER RTT integration layer
|   |-- utils/                 # reusable utility layer
|   |-- shared/                # shared constants and lightweight types
|   `-- interfaces/            # pure contracts
|-- tests/                     # QTest targets
|-- resources/                 # themes, icons, translations, app resources
|-- docs/                      # constraints, PRDs, specs, reviews, tracking
|-- tools/                     # bootstrap, packaging, audit and launch helpers
|-- cmake/EmbedDebugSources.cmake
|-- CMakeLists.txt
|-- EmbedDebug.bat
`-- README.md
```

## Engineering Workflow

This repository is intentionally constraint-driven.

| Stage | Purpose |
|-------|---------|
| PRD | Define the user problem, boundaries, non-goals, and state targets |
| Specs | Convert PRD into implementation constraints and verification commands |
| TDD | Add or update QTest coverage before production behavior changes |
| GO loop | Execute, check, fix, and verify without widening the scope blindly |
| Commit | Commit each closed iteration with status, verification evidence, and score |

Primary entry documents:

- [CLAUDE.md](CLAUDE.md)
- [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md)
- [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md)
- [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md)
- [docs/serial_station_architecture.md](docs/serial_station_architecture.md)

## Roadmap

Near-term work is focused on turning the current engineering body into a coherent workstation:

| Priority | Direction | Target outcome |
|----------|-----------|----------------|
| P0 | Serial Station profile list and one-click apply/connect | Configuration profiles become a daily workflow, not just file import/export |
| P0 | UART verification with fake or virtual serial pair | Lift Serial Station device evidence beyond pure unit tests |
| P1 | QSS token generation and UI consistency | Reduce hand-maintained theme drift |
| P1 | Dialog and error feedback unification | Replace ad hoc message flows with consistent app dialogs |
| P2 | BLE/CAN/MQTT/USB/RTT hardening | Move staged integrations from framework code toward verified workflows |
| P2 | Release packaging polish | Make `dist/` verification and launch docs suitable for handoff |

## Contributing

Before changing code, read the constraint documents for the affected area. New features need PRD/Specs first. New `.h/.cpp` files must be registered in the CMake source list. Any change affecting build, launch, dependencies, resources, or startup must verify `EmbedDebug.bat` before commit.

## License

MIT License.

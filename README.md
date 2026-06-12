# EmbedDebug

> Industrial desktop workbench for embedded debugging: serial station, protocol decoding, waveform analysis, OTA workflows, recording, automation, and engineering diagnostics in one Qt application.

![EmbedDebug interface preview](docs/assets/readme/interface-preview.svg)

## Project Status

| Item | Status |
|------|--------|
| Main application | `EmbedDebug` |
| Repository | `GS_Tool` |
| Primary branch | `feat/embed-debug` |
| Platform | Windows 10+ |
| Stack | C++17, Qt 6, CMake, Ninja, MinGW |
| Current maturity | Active engineering build; core desktop workflows are implemented, some device integrations remain staged |
| User entry point | `EmbedDebug.bat` from the repository root |

EmbedDebug is built for firmware and embedded-tool engineers who need one quiet, repeatable desktop workflow for serial communication, binary protocols, live signals, OTA operations, recorded sessions, and automation rules. It is not positioned as a simple serial assistant; the repository is being shaped into a modular workbench with strict architecture, staged PRDs, tests, and launch verification.

## What It Solves

Embedded debugging often spreads one session across a serial terminal, a waveform viewer, a protocol decoder, a log recorder, an OTA tool, and hand-written scripts. EmbedDebug brings those loops into one application so engineers can inspect bytes, protocol frames, charts, commands, and device workflows without constantly switching tools.

## Highlights

| Area | Current capability |
|------|--------------------|
| Serial Station | Independent `src/apps/serial_station/` workbench with UART port discovery, manual COM input, 115200 8N1-style configuration summary, command panel, structured logs, log export, replay preview, status bar, send/receive loop, dispatcher, codec, and protocol registry |
| Protocols | `ascii_text`, `modbus_rtu`, and `custom_md` are implemented under the new Serial Station protocol boundary with QTest coverage |
| Encoding | ASCII, HEX, and protocol command send paths are centralized through `SerialCodec` |
| Data views | Terminal, waveform preview, FFT, multi-axis charts, histogram/scatter, heatmap, dashboard widgets, and performance panels are present in the application modules |
| OTA and files | XMODEM/YMODEM/ZMODEM-oriented OTA modules, HEX/BIN workflows, export, recording, and replay foundations exist in the source tree |
| Automation | Trigger engine, script recording/playback, command history, and rule-management modules are present |
| Engineering loop | `tools/doctor.ps1`, `tools/verify_embeddebug_launch.ps1`, and `tools/agent-loop/` support Doctor/GO style verification |
| UI system | Qt Widgets application with QSS themes, Lucide SVG assets, icon manager, command palette, empty/loading states, and panel wrappers |

Some integrations are intentionally marked as staged: BLE, CAN, MQTT, USB, and SEGGER RTT have framework code or stubs, but not every external device path is production-complete yet.

## Interface Preview

The preview above is a maintained repository asset, not a generated build artifact. It summarizes the current workbench layout:

- left navigation for connection, protocol, terminal, chart, OTA, dashboard, and automation areas;
- central Serial Station workbench for UART port setup, manual COM entry, protocol mode, command send, structured logs, export/replay preview, and status;
- right-side analysis surfaces for waveforms, decoded frames, recordings, and diagnostics.

## Architecture

The project uses a layered dependency model. New work should follow the constraint documents before code is changed.

```text
L6  src/core/                 application coordination, main window, navigation, theme runtime
L5  src/ota/ automation/      scenario workflows: OTA, automation, dashboard, plugin
L4  src/terminal/ chart/ rtt/ data presentation and live inspection
L3  src/connection/ protocol/ serial/ external access and legacy protocol modules
L5A src/apps/serial_station/  new serial workbench with internal ui/controller/core/protocol split
L2  src/utils/                reusable CRC, HEX, export, logging, cache, and timestamp tools
L1  src/shared/               shared constants, enums, and lightweight types
L0  src/interfaces/           pure contracts
```

Serial Station has an additional internal rule:

```text
ui/ -> SerialStationController -> core/ + protocols/ + services/
workers/ -> core/
core/ -> ISerialProtocol + SerialProtocolRegistry
protocols/<name>/ -> protocol interface + shared/utils only
```

UI panels do not parse bytes. Protocols do not touch widgets. `MainWindow` and `PanelManager` stay as assembly/navigation layers.

## Quick Start

Recommended first run on this repository:

```powershell
.\tools\bootstrap_env.bat
.\EmbedDebug.bat
```

`local_env.bat` is generated for the local machine and must not be committed.

UV start shortcut:

```powershell
uv run start-embeddebug
```

This delegates to `EmbedDebug.bat`; the batch file remains the minimum supported user entry point.

Manual configure/build path:

```powershell
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat
```

The only supported build directory is `build/`. Parallel build directories are not part of the documented workflow.

UV packaging shortcut:

```powershell
uv run package-embeddebug --skip-build --clean
uv run package-embeddebug --skip-build --clean --zip
uv run test-embeddebug-tools
```

This is the supported packaging shortcut for the C++/Qt application. PyInstaller is for Python application bodies; EmbedDebug is packaged by reusing `build/EmbedDebug.exe`, running Qt `windeployqt`, and writing a distributable folder under ignored `dist/`.

## Verification

Core local checks:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

Serial Station focused checks:

```powershell
cmake --build build --target test_ascii_text_protocol test_modbus_rtu_protocol test_custom_md_protocol test_serial_protocol_registry --parallel 4
ctest --test-dir build -R "AsciiTextProtocol|ModbusRtuProtocol|CustomMdProtocol|SerialProtocolRegistry" --output-on-failure
```

GO loop, when Go is available on PATH:

```powershell
Push-Location .\tools\agent-loop
go test ./...
go run . -config .\sample.embeddebug.json -dry-run
Pop-Location
```

If `go.exe` is not installed, `tools\doctor.ps1` reports a warning. The equivalent manual path is the doctor command plus `tools\verify_embeddebug_launch.ps1`.

## Repository Map

```text
GS_Tool/
├── src/
│   ├── apps/serial_station/   # new serial workbench
│   ├── core/                  # application coordination and shared UI runtime
│   ├── connection/            # connection implementations and factories
│   ├── protocol/              # legacy/general protocol engines and bridges
│   ├── terminal/              # terminal model, filtering, rendering
│   ├── chart/                 # waveform, FFT, overlays, heatmap, zoom
│   ├── ota/                   # firmware update workflows
│   ├── automation/            # trigger/rule automation
│   ├── dashboard/             # gauge, LED, numeric dashboard widgets
│   ├── rtt/                   # SEGGER RTT integration layer
│   ├── utils/                 # reusable utility layer
│   ├── shared/                # shared constants and lightweight types
│   └── interfaces/            # pure contracts
├── tests/                     # QTest targets
├── resources/                 # themes, icons, backgrounds
├── docs/                      # constraints, PRDs, specs, reviews, tracking
├── tools/                     # bootstrap, doctor, launch, GO loop, audit tools
├── CMakeLists.txt
├── EmbedDebug.bat
└── README.md
```

## Engineering Workflow

This repository is intentionally constraint-driven.

| Step | Meaning |
|------|---------|
| Specs | Define objective, non-goals, constraints, acceptance checks, and failure conditions before implementation |
| GO | Run execute -> check -> fix loops with a maximum of 20 rounds or 30 minutes |
| BATCH | Split large work into 5-30 reviewed tasks before any 3-agent or 6-agent parallel execution |
| LOOP | Route failures through Doctor, Debug, or Simplify instead of widening the change blindly |
| Commit | Commit by phase: PRD/Specs, tools/build, production code, tests, cleanup/report |

Primary entry documents:

- [CLAUDE.md](CLAUDE.md)
- [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md)
- [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md)
- [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md)
- [docs/serial_station_architecture.md](docs/serial_station_architecture.md)

## Roadmap

Near-term work is focused on making the product body more coherent, not just expanding build files:

- enrich Serial Station UI for protocol selection and parameterized protocol commands;
- connect logging/export/replay services into the Serial Station UI without crossing controller boundaries;
- continue replacing ad hoc UI styling with maintainable QSS and generated theme tokens;
- fill staged BLE/CAN/MQTT/USB/RTT integrations with real backend behavior and tests;
- add release packaging after the `EmbedDebug.bat` launch path remains stable.

## Contributing

Before changing code, read the constraint documents for the affected area. New features need PRD/Specs first. New `.h/.cpp` files must be registered in CMake. Any change affecting build, launch, dependencies, resources, or startup must verify `EmbedDebug.bat` before commit.

## License

MIT License.

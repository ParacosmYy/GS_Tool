<div align="center">

# EmbedDebug

**Enterprise-Grade Embedded Debugging Suite**

*Serial Terminal · SEGGER RTT Viewer · OTA Firmware Upgrade*

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-0078D6.svg)]()
[![Qt](https://img.shields.io/badge/Qt-6.8-41CD52.svg)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)]()
[![Build](https://img.shields.io/badge/build-CMake%2BNinja-064F8C.svg)]()

[English](#features) · [中文文档](#功能概览)

</div>

---

## Features

### Unified Connection Interface

| Connection Type | Status | Description |
|:--------------- |:------:|:----------- |
| Serial Port | ✅ | Multi-port simultaneous, full config (baud/data/stop/parity/flow) |
| TCP Client | 📋 Planned | Connect to remote devices via TCP |
| TCP Server | 📋 Planned | Listen and accept incoming connections |
| UDP | 📋 Planned | UDP broadcast and unicast |
| SEGGER RTT | 📋 Planned | J-Link RTT via SDK dynamic loading |

### Serial Terminal

- **Custom Render Engine** — QPainter-based terminal for high-throughput data display
- **Display Modes** — Text / HEX / Mixed, switchable on the fly
- **Timestamps** — Millisecond-precision RX/TX timestamps
- **Quick Commands** — Configurable button bar for one-click command sending
- **Timed Sender** — Cyclic transmission with configurable interval and queue
- **Send History** — Recent 50 commands, one-click resend
- **Data Filter** — Keyword, HEX pattern, and regex search
- **Multi-Port** — Connect multiple serial ports simultaneously, tab switching

### Protocol Parser

- **Visual Frame Editor** — Drag-and-drop field configuration (header/length/data/checksum/tail)
- **Live Parsing** — State machine parser with real-time field extraction
- **Multi-Protocol** — Switch between frame definitions on the fly
- **Error Detection** — CRC mismatch and frame length anomaly highlighting
- **Import/Export** — JSON-based frame definitions for team sharing

### Real-Time Waveform

- **Qt Charts Integration** — Smooth real-time data visualization
- **Auto-Mapping** — Numeric fields from protocol parser automatically available as channels
- **Manual Formula** — Custom data extraction rules from raw bytes
- **Multi-Channel** — Simultaneous display of multiple data streams

### OTA Firmware Upgrade

| Protocol | Block Size | Features |
|:-------- |:----------:|:-------- |
| XMODEM-Checksum | 128 B | Arithmetic checksum |
| XMODEM-CRC | 128 B | CRC16 validation |
| XMODEM-1K | 1024 B | CRC16, larger blocks |
| YMODEM | Variable | Filename + size in Block 0, batch transfer |
| YMODEM-g | Stream | No-ack streaming for fast links |
| ZMODEM | Variable | Crash recovery, compression, 32-bit CRC |

- **Auto HEX→BIN** — Drop an Intel HEX file, auto-convert and send
- **Progress Tracking** — Percentage, speed, ETA during transfer
- **OTA History** — Timestamped log of all upgrades with pass/fail status

### Data Persistence

- **Configuration** — Serial settings, panel layout, window geometry auto-saved
- **Session Management** — Save/restore complete workspace
- **Log Recording** — Auto or manual, timestamp-aligned multi-stream logging
- **Log Playback** — Replay recorded sessions at original or accelerated pace
- **Data Export** — TXT / CSV / BIN / XLSX, with time range selection

### UI & Theming

- **IDE-Style Layout** — Left navigation tree + right content panel
- **Multiple Themes** — Dark Terminal / Modern Dark / Light, extensible via QSS
- **Bilingual** — Chinese / English, Qt Linguist based i18n

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    MainWindow                        │
│  ┌──────────┐  ┌──────────────────────────────────┐ │
│  │ Nav Tree  │  │         Content Panel            │ │
│  │          │  │  ┌────────────────────────────┐  │ │
│  │ ▶ Serial │  │  │      TerminalWidget        │  │ │
│  │ ▶ TCP    │  │  │      (QPainter Engine)     │  │ │
│  │ ▶ RTT    │  │  └────────────────────────────┘  │ │
│  │ ▶ OTA    │  │  ┌────────────────────────────┐  │ │
│  │          │  │  │    QuickCommandBar          │  │ │
│  │          │  │  ├────────────────────────────┤  │ │
│  │          │  │  │    Send Bar / Config        │  │ │
│  └──────────┘  │  └────────────────────────────┘  │ │
│                └──────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│  ConnectionManager ── IConnection (Abstract)         │
│    ├── SerialConnection  (QSerialPort)               │
│    ├── TcpConnection     (QTcpSocket)                │
│    ├── UdpConnection     (QUdpSocket)                │
│    └── RttConnection     (J-Link SDK / QLibrary)     │
├─────────────────────────────────────────────────────┤
│  Data Flow: IConnection ──▶ TerminalModel            │
│              ├── TerminalWidget (display)             │
│              ├── FrameParser   (protocol decode)      │
│              ├── ChartWidget   (waveform)             │
│              ├── DataLogger    (record/replay)        │
│              └── OtaManager    (firmware upgrade)     │
└─────────────────────────────────────────────────────┘
```

**Key Design Decisions**:
- **IConnection abstraction** — All features are connection-type agnostic
- **Signal/Slot data bus** — `dataReceived` signal fans out to all consumers
- **State machine parsers** — Frame parser and OTA protocols use clean FSM pattern
- **QPainter rendering** — Terminal bypasses QTextEdit for zero-copy display at high baud rates

---

## Tech Stack

| Component | Technology |
|:--------- |:---------- |
| Language | C++17 |
| UI Framework | Qt 6.8 (Widgets) |
| Charts | Qt Charts |
| Serial | Qt SerialPort |
| Network | Qt Network |
| Build | CMake 4.0 + Ninja |
| Compiler | MinGW GCC 14.2 |
| Debugger | GDB 16.2 |
| IDE | VS Code + CMake Tools |

---

## Quick Start

### Prerequisites

- Windows 10/11
- [MinGW GCC 14.2+](https://github.com/niXman/mingw-builds-binaries/releases)
- [CMake 3.20+](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- Qt 6.8 (installed via [aqtinstall](https://github.com/miurahr/aqtinstall))

### Install Qt (if not already)

```bash
pip install aqtinstall
aqt install-qt windows desktop 6.8.3 win64_mingw ^
    --outputdir E:\Tool\DevEnv\Qt ^
    --modules qtserialport qtcharts
```

### Build

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout feat/embed-debug

# Configure
cmake -G Ninja -B build ^
    -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64

# Build
cmake --build build

# Deploy Qt DLLs
E:\Tool\DevEnv\Qt\6.8.3\mingw_64\bin\windeployqt.exe build\EmbedDebug.exe
```

### Run

**Double-click `EmbedDebug.bat`** in the project root — that's it.

Or manually:
```bash
cd build && EmbedDebug.exe
```

---

## Project Structure

```
src/
├── core/               # Application shell
│   ├── MainWindow      # Main window with IDE layout
│   ├── ConnectionManager  # Unified connection lifecycle
│   └── ThemeManager    # Multi-theme QSS loader
├── connection/         # Abstract connection layer
│   ├── IConnection     # Interface: open/close/write/signals
│   └── SerialConnection  # QSerialPort implementation
├── terminal/           # High-performance terminal
│   ├── TerminalWidget  # QPainter-based custom widget
│   └── TerminalModel   # Thread-safe line buffer
├── serial/             # Serial-specific features
│   ├── SerialConfigPanel  # Port/baud/data/stop/parity UI
│   ├── QuickCommandBar # One-click command buttons
│   └── TimedSender     # Cyclic transmission engine
├── protocol/           # Frame parser (Phase 3)
├── chart/              # Real-time waveform (Phase 3)
├── ota/                # X/Y/ZMODEM upgrade (Phase 5)
├── rtt/                # SEGGER RTT via J-Link (Phase 6)
└── utils/              # Shared utilities
    ├── CRC             # CRC8/CRC16-CCITT/CRC16-Modbus/CRC32
    ├── HexConverter    # HEX encode/decode
    ├── RingBuffer      # Thread-safe circular buffer
    └── SettingsManager # JSON config persistence
```

---

## Roadmap

| Phase | Feature | Status |
|:-----:|:------- |:------:|
| 1 | Serial basic TX/RX + HEX + terminal | ✅ Done |
| 2 | Quick commands, timed send, filter, export, multi-port | 🔄 Next |
| 3 | Protocol parser + waveform display | 📋 Planned |
| 4 | TCP/UDP Client + Server | 📋 Planned |
| 5 | OTA upgrade (XMODEM/YMODEM/ZMODEM) | 📋 Planned |
| 6 | SEGGER RTT Viewer (J-Link SDK) | 📋 Planned |
| 7 | Integration, i18n, performance, packaging | 📋 Planned |

---

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feat/your-feature`)
3. Commit with descriptive messages
4. Push to the branch (`git push origin feat/your-feature`)
5. Open a Pull Request

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

<div align="center">

*Built with Qt 6 · C++17 · CMake*

*Designed for embedded engineers, by embedded engineers.*

</div>

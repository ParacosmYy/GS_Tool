# PRD-135 Python/PyQt Migration Strategy

## Background

The project is opening a Python/PyQt migration track for Serial Station and the broader EmbedDebug desktop app. The intended workflow is:

- `uv run ...` for local startup, tests, verification, and packaging commands.
- PyQt for the Python GUI stack.
- PyInstaller for the future Python executable package.

This PRD records the decision boundary only. It does not switch the current C++/Qt application to Python, and it does not claim feature parity.

Current state:

- The production baseline remains `EmbedDebug.bat -> build/EmbedDebug.exe`.
- `pyproject.toml` currently manages local Python tools only: `start-embeddebug`, `package-embeddebug`, `test-embeddebug-tools`, `verify-package-embeddebug`.
- PRD-134 remains the VOFA+ capability target for Serial Station.
- Python/PyQt runtime code, dependencies, tests, and packaging specs are not present yet.

## Decision

Create a parallel Python/PyQt migration lane. C++/Qt remains the baseline until a later cutover PRD proves Python parity with launch, tests, package verification, and user workflow evidence.

The first implementation after this PRD must be governance/tooling only: canonical Python paths, uv command names, license decision, and test boundaries. No feature code may be added before those are documented.

## Hard Non-Goals

- Do not modify `EmbedDebug.bat` default behavior in this PRD.
- Do not route `uv run start-embeddebug` to Python yet.
- Do not repurpose the existing `uv run package-embeddebug` C++ packaging lane for PyInstaller.
- Do not delete, freeze, or rewrite the current C++ Serial Station code.
- Do not create Python production source before `docs/constraints/07-directory-structure.md` approves a canonical Python path.
- Do not emit or commit PyInstaller artifacts.
- Do not create `build2/`, `build-debug/`, `build-release/`, `cmake-build-*`, or any parallel build directory.
- Do not describe the Python migration as complete below the E/U/D gates in this PRD.

## License Gate

PyQt6 is acceptable as the requested technical direction only after a written license decision.

Required decision before adding `PyQt6` to `pyproject.toml`:

1. GPL route: the distributed Python/PyQt application and corresponding source obligations are compatible with GPLv3.
2. Commercial route: Riverbank PyQt commercial licensing, and any required Qt commercial licensing, are budgeted and recorded.

If the application must remain MIT-only or closed-source without commercial PyQt licensing, PySide6/LGPL must be evaluated in a separate decision. This PRD does not silently switch to PySide6 because the requested stack is PyQt.

Packaging and release gates:

- Maintain a Qt/PyQt module allowlist.
- Avoid GPL/commercial-only Qt modules unless the license route covers them.
- Ship `LICENSE`, third-party notices, dependency versions, and an SBOM before any public Python GUI package.
- Keep `uv.lock` committed once Python runtime dependencies are introduced.
- Run a dependency vulnerability check before release.

Reference points used for this decision:

- PyQt6 is distributed under Riverbank commercial license or GPL v3.
- PyQt6 exposes `PyQt6.QtSerialPort` with `QSerialPort` and `QSerialPortInfo`, so a PyQt serial implementation is technically viable.

### Current B2 License Decision

For the B2 Python/PyQt skeleton, the migration lane uses PyQt6 under a GPLv3-compatible development route.

This means:

- `PyQt6` may be added to `pyproject.toml` for local development and automated smoke tests.
- The Python/PyQt app must not be publicly distributed as an MIT-only binary.
- Public packaging remains blocked until `LICENSE`, third-party notices, dependency versions, and SBOM are added.
- If a proprietary or MIT-only binary is required later, the project must switch to a documented Riverbank commercial PyQt/Qt route or open a separate PySide6/LGPL decision.
- This decision does not change the current C++ baseline license or default startup path.

## Command Model

The uv model is the right management direction, but the C++ and Python lanes must stay distinct during coexistence.

Current C++ lane remains:

```powershell
uv run start-embeddebug
uv run package-embeddebug
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

Reserved Python lane:

```powershell
uv run start-embeddebug-py
uv run test-embeddebug-py
uv run package-embeddebug-py
uv run verify-package-embeddebug-py
```

Implementation note: user-facing packaging should be `uv run package-embeddebug-py`, not a raw `uv run pyinstall` command. The script can call PyInstaller internally and enforce project-specific paths, license checks, and artifact hygiene.

## Packaging Model

Current C++ packaging remains CMake/Qt deployment tooling. PRD-099 explicitly scoped Python packaging tools to wrapper/tooling, not the current C++/Qt app body.

Future Python packaging lane:

- Use PyInstaller `onedir` first.
- Package name during coexistence: `EmbedDebugPy`, not `EmbedDebug`.
- Output example: `dist/EmbedDebugPy-<version>-windows-x64/`.
- `dist/` remains untracked.
- PyInstaller work path must not use repository `build/`, because `build/` is reserved as the only C++ build directory.
- Recommended future shape:

```powershell
uv run package-embeddebug-py
```

The implementation behind that command must set a controlled `--workpath`, for example under `%TEMP%`, and a controlled `--distpath`.

## Architecture Mapping

Python must preserve the existing Serial Station boundaries:

| Layer | Python responsibility |
|------|------------------------|
| `app/` | QApplication, process entry, startup checks |
| `ui/` | PyQt widgets and views only |
| `controllers/` | UI intent orchestration, no protocol parsing |
| `core/` | session state, dispatcher, event bus, transport coordination |
| `protocols/` | command building, frame definitions, streaming parsers |
| `services/` | logs, export, replay, profile persistence, settings |
| `workers/` | serial/background work, no QWidget ownership |
| `drivers/` | QSerialPort or adapter implementations behind an interface |

The Python protocol API should mirror the current Serial Station contract rather than the older bridge API:

```python
class SerialProtocol:
    name: str

    def build_command(self, command: str, params: Mapping[str, Any]) -> bytes: ...
    def feed(self, data: bytes) -> list[ProtocolEvent]: ...
    def reset(self) -> None: ...
```

`ProtocolEvent` must carry at least:

- `type`
- `protocol_name`
- `payload`
- exact `raw: bytes`

## VOFA+ Capability Target

The Python migration is not merely a UI rewrite. It must preserve and eventually improve the VOFA+ track from PRD-134:

- RawData, FireWater, and JustFloat as first-class protocol engines.
- Data, command, and parameter binding as first-screen concepts.
- Multi-channel realtime plotting.
- Channel stats, measurement, zoom, cursor, FFT, histogram, XY, and image channel phases.
- Logging, export, replay, and profile workflows.
- Future plugin/extension path after the core workflow is stable.

First Python UI target: a four-zone workbench shell:

1. Top session bar: profile, endpoint, connect/disconnect, protocol, RX/TX/error counters.
2. Left setup and binding lane: serial config, protocol selector, channel list, symbol/parameter binding.
3. Center realtime area: multi-channel plot preview, command composer, TX/RX log.
4. Right analysis inspector: channels, bindings, stats, FFT, histogram, XY, image placeholders.

U1 should not promise drag/drop canvas, detachable tabs, plugin SDK, image channel, or full analysis interactions.

## Protocol Scope

Python protocol parity must start in pure core tests before UI work.

Minimum protocol set:

- `raw_data`: preserve bytes exactly and expose best-effort text.
- `fire_water`: newline-delimited numeric frames, optional prefix/header handling, channel names, invalid numeric values as explicit NaN representation.
- `just_float`: little-endian float32 payload terminated by `00 00 80 7F`, matching current Serial Station behavior.

Shared channel model should use ordered frames:

```text
ChannelFrame:
  protocol: str
  frame_index: int
  raw: bytes
  channels: list[ChannelSample]

ChannelSample:
  index: int
  name: str
  value: float
  unit: str | None
```

Compatibility fixtures should live under a future approved tests fixture path and compare Python output against C++ golden vectors byte-for-byte where applicable.

## Performance Gates

The PyQt plotting path must not copy the current lightweight C++ preview string path into Python.

Future implementation gates:

- Use typed batches such as `ChannelBatch(values: np.ndarray, t0_ns, dt_ns, channel_names, seq)`.
- No per-sample Qt signals.
- No unbounded Python lists, `QVariant`, strings, or DataFrame growth in the hot path.
- Parser emits batches every `2-5ms` or `4-16KB`.
- GUI consumes batches on a `QTimer`.
- Ring buffer is preallocated NumPy, `float32` by default.
- pyqtgraph updates with `PlotDataItem.setData()` once per visible channel per render tick.
- Render timer defaults to `30Hz`; `60Hz` is optional after measurement.
- Start target: `8 channels @ 10 kSamples/s/channel` for 60 seconds with bounded queue growth.
- Stretch target: `8 channels @ 25 kSamples/s/channel`.
- Byte/batch receipt to ring-write p95 latency `<= 10ms`.
- Receipt to visible plot update p95 `<= 50ms`, p99 `<= 100ms`.
- Default visible/history memory `<= 64MB`, hard cap `<= 256MB`.

The first visualization implementation should include a benchmark harness before claiming realtime plotting parity.

## Verification Strategy

Minimum Python test gates by stage:

| Gate | Evidence |
|------|----------|
| `pytest -m golden` | protocol parse/build parity with C++ fixtures |
| `pytest -m unit` | pure protocol, model, service, config logic |
| `pytest -m fake_serial` | open/write/read/error/reconnect without hardware |
| `pytest -m ui` | pytest-qt smoke and visible state checks |
| `pytest -m virtual_com` | Windows virtual COM pair, D3 evidence |
| `pytest -m hardware` | real MCU, D4 evidence |

Promotion rules:

- Do not call a Python-migrated feature complete below `E4`.
- Do not call it user-usable below `U3`.
- Do not claim hardware readiness below `D4`.
- Minimum PR gate after code starts: `unit + golden + fake_serial + ui smoke`.
- Minimum release gate: PR gate plus virtual COM.
- Hardware claim gate: release gate plus real MCU.

## Phased Migration

| Batch | Goal | Acceptance | Do not touch |
|------|------|------------|--------------|
| B0 | PRD-135 decision and baseline awareness | this PRD and Specs exist, no runtime change | source, CMake, pyproject, launcher |
| B1 | governance and canonical paths | `07-directory-structure.md` updated, C++ baseline still passes | C++ source, CMake, `EmbedDebug.bat` |
| B2 | Python/PyQt tooling skeleton | `uv run start-embeddebug-py` smoke, pytest-qt empty window | C++ behavior |
| B3 | pure domain parity | RawData/FireWater/JustFloat golden tests | PyQt widgets, hardware |
| B4 | services parity | logs/export/replay/profile fixtures pass | live serial ports, UI redesign |
| B5 | transport layer | fake transport and QSerialPort adapter tests | default launcher |
| B6 | PyQt Serial Station MVP | fake connect -> send -> receive -> log workflow | chart/dashboard/OTA/plugin scope |
| B7 | user workflow parity | export, replay, profiles, history matrix | C++ deletion |
| B8 | visualization/VOFA parity | ring buffer and pyqtgraph benchmark gates | unmeasured realtime claims |
| B9 | side-by-side packaging | Python package starts, C++ package still starts | `EmbedDebug.bat` routing |
| B10 | cutover decision | separate explicit approval and archived C++ baseline | C++ removal before parity |

## E/U/D Matrix

This PRD itself is:

| Axis | State | Reason |
|------|-------|--------|
| Engineering | E1 | strategy and boundaries documented |
| User | U0 | no user-visible runtime change |
| Device | D0 | no serial, virtual COM, or hardware verification |

Future minimum claims:

- Runnable Python shell: at most `E2-E3/U1/D0`.
- Fake serial workflow: at most `E4/U2/D2`.
- Virtual COM workflow: at most `E4/U3/D3`.
- Real MCU workflow: at most `E4-E5/U3-U4/D4`.
- Default entry cutover: only after separate PRD and at least `E5/U4/D3`, preferably `D4` for hardware claims.

## Risks

| Risk | Mitigation |
|------|------------|
| PyQt license incompatible with intended distribution | require GPL/commercial decision before adding dependency |
| C++ baseline lost during rewrite | keep `EmbedDebug.bat` and `uv run start-embeddebug` on C++ until cutover PRD |
| Python product code mixed into `tools/` | update directory constraints before creating runtime package |
| PyInstaller uses repository `build/` | wrapper command must force external work path |
| VOFA+ capability regresses to a simple serial terminal | PRD-134 remains capability target and golden tests gate protocol parity |
| Realtime plot stalls | require NumPy ring buffer, batched signals, pyqtgraph benchmark harness |
| Dirty worktree hides regressions | do not revert user changes; freeze baseline before code batches |

## Acceptance For This PRD

- `docs/prd/PRD_135_Python_PyQt_Migration.md` exists.
- `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md` exists.
- The PRD documents no immediate C++ cutover.
- The PRD documents uv startup, test, and PyInstaller package lanes.
- The PRD documents the PyQt license gate.
- The PRD records E1/U0/D0 for this decision batch.
- No source, CMake, launcher, README, or `pyproject.toml` changes are made in this batch.

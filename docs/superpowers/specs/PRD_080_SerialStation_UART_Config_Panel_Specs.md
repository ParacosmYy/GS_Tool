# Specs - PRD-080 Serial Station UART Config Panel

## 1. 目标

新增 Serial Station 的 UART 配置面板，使用户能在新工站中配置端口、波特率、数据位、校验、停止位、流控和 DTR/RTS，并通过 controller 发起连接或断开。

## 2. 非目标

- 不打开真实串口做自动化测试。
- 不新增协议配置面板。
- 不迁移旧 `src/serial/config/`。
- 不修改旧主窗口或 PanelManager。

## 3. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/05-ui-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`
- `docs/prd/PRD_080_SerialStation_UART_Config_Panel.md`

## 4. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| UI | `src/apps/serial_station/ui/SerialPortPanel.h/.cpp` | 新增 |
| Controller | `src/apps/serial_station/SerialStationController.h/.cpp` | 增加串口配置 slot/signal |
| Window | `src/apps/serial_station/SerialStationWindow.h/.cpp` | 装配面板 |
| CMake | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册源码和测试 |
| Test | `tests/serial_station/test_serial_port_panel.cpp` | 新增 |

## 5. 验收标准

- [ ] `SerialPortPanel` 不 include `core/SerialManager.h`。
- [ ] `SerialStationWindow` 只连接 signal/slot，不处理业务逻辑。
- [ ] `SerialStationController` 负责调用 `SerialManager::configure/open/close`。
- [ ] 面板默认配置为 115200 8N1 无流控。
- [ ] 点击连接会发出 `connectRequested(SerialPortConfig)`。
- [ ] 点击断开会发出 `disconnectRequested()`。
- [ ] `ctest --test-dir build -R "SerialPortPanel|AsciiTextProtocol|SerialProtocolRegistry|SerialSession"` 通过。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `EmbedDebug.bat` 启动通过。

## 6. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 原因：本轮跨 UI/controller/window/CMake 合流点，必须串行。

## 7. LOOP 路由

- Doctor：构建和启动失败。
- Debug：测试失败或 signal/slot 连接错误。
- Simplify：如果 UI 面板职责膨胀，拆分配置模型或 helper。

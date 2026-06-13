# PRD-120 - Serial Station Startup Profile Specs

## 1. 目标

- 为 Serial Station 增加启动即加载配置档案的 CLI 路径。
- 保持启动参数、主窗口装配和配置档案服务分层。
- 本轮不改变真实串口连接行为，不自动连接设备。

## 2. 参数契约

| 参数 | 行为 |
|------|------|
| `--profile <path>` | 保存 `path` 为启动档案路径，并默认打开 `serial.station` |
| `--profile=<path>` | 同上 |
| `--serial-profile <path>` | 同上，语义更明确 |
| `--serial-profile=<path>` | 同上 |
| `--station serial --profile <path>` | 打开 `serial.station` 并加载档案 |
| `--panel serial.station --profile <path>` | 打开指定面板并加载档案 |

## 3. 调用链

```text
QCoreApplication::arguments()
  -> StartupOptions::fromArguments()
  -> MainWindow::applyStartupOptions()
  -> MainWindow::openPanelById("serial.station")
  -> PanelManager::serialStationWindow()
  -> SerialStationWindow::loadProfileFromFile(path)
  -> SerialStationController::loadProfileFromFile(path)
  -> SerialProfileService::loadFromFile(path)
```

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| PRD | `docs/prd/PRD_120_SerialStation_Startup_Profile.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_120_SerialStation_Startup_Profile_Specs.md` | 新增 |
| startup | `src/core/mainwindow/StartupOptions.*` | 增加 profile 参数解析 |
| main window | `src/core/mainwindow/MainWindow.*`, `src/main.cpp` | 转发启动选项 |
| tests | `tests/test_startup_options.cpp`, `tests/serial_station/test_serial_station_workbench.cpp` | 红绿回归 |
| docs | `README.md`, `docs/tracking/SCORE_TRACKING.md` | 同步入口和评分 |

## 5. 验收清单

- [x] 红灯测试先失败，失败原因是 `StartupOptions` 无 profile API / `SerialStationWindow` 无启动档案应用入口。
- [x] `test_startup_options` 构建和运行通过。
- [x] `test_serial_station_workbench` 构建和运行通过。
- [x] `EmbedDebug` 主目标构建通过。
- [x] `EmbedDebug.bat --station serial --profile <file>` 启动探针通过。
- [x] Serial Station 边界 include 检查通过。
- [x] README 已同步启动档案入口，并按用户要求更新为中文企业级首页。
- [x] 本轮变更纳入单一 commit，并在收口阶段 push。

## 6. 失败条件

- `StartupOptions` 依赖 QWidget 或 Serial Station 具体实现。
- `MainWindow` 解析 profile JSON 或直接访问 `SerialProfileService`。
- profile 加载失败导致应用退出。
- 自动连接串口。
- README 暗示真实设备已验证。

## 7. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | E5：启动参数解析、工作台启动档案加载、失败不污染状态均有 QTest 覆盖 |
| 用户状态 | U4：可用 `EmbedDebug.bat --station serial --profile <file>` 直接进入档案化串口工站 |
| 设备状态 | 不提升 |
| 验证命令 | `cmake --build build --target test_startup_options test_serial_station_workbench --parallel 4`; `.\build\tests\test_startup_options.exe`; `.\build\tests\test_serial_station_workbench.exe`; `cmake --build build --target EmbedDebug --parallel 4`; `.\EmbedDebug.bat --station serial --profile <temp.edserialprofile>` |
| commit | 本轮收口提交 |

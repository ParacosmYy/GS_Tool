# PRD-119 - Serial Station Profile UI Flow Specs

## 1. 目标

- 将 PRD-118 的配置档案服务接入 Serial Station 工作台。
- 用户可从工作台保存/加载 UART、协议、发送模式和常用命令档案。
- 本轮三轴目标：工程 `E4 -> E5`，用户 `U3 -> U4`，设备不提升。

## 2. 非目标

- 不做档案列表、最近档案、删除或重命名。
- 不自动连接串口。
- 不修改协议实现。
- 不做真实串口验证。

## 3. 调用链

保存：

```text
SerialStationWindow 保存入口
  -> collectProfileFromUi()
  -> SerialStationController::saveProfileToFile(profile, path)
  -> SerialProfileService::saveToFile()
  -> SerialLogPanel 追加结果
```

加载：

```text
SerialStationWindow 加载入口
  -> SerialStationController::loadProfileFromFile(path)
  -> SerialProfileService::loadFromFile()
  -> SerialPortPanel::applyConfig()
  -> SerialProtocolPanel::setActiveProtocol() + controller::setActiveProtocol()
  -> SerialCommandPanel::applyProfileCommands()
  -> SerialLogPanel 追加结果
```

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| PRD | `docs/prd/PRD_119_SerialStation_Profile_UI_Flow.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_119_SerialStation_Profile_UI_Flow_Specs.md` | 新增 |
| controller | `src/apps/serial_station/SerialStationController.*` | 复用 profile service |
| window | `src/apps/serial_station/SerialStationWindow.*` | 保存/加载入口和 UI 协调 |
| ui | `SerialPortPanel.*`, `SerialCommandPanel.*` | 应用档案值对象 |
| tests | `tests/serial_station/test_serial_station_workbench.cpp` | 工作台闭环回归 |
| README/tracking | `README.md`, `docs/tracking/SCORE_TRACKING.md` | 同步真实状态 |

## 5. 验收标准

- [x] 红灯测试先失败，失败原因是按钮/方法不存在。
- [x] `test_serial_station_workbench` 构建通过。
- [x] `test_serial_station_workbench` QTest 通过。
- [x] `EmbedDebug` 主目标构建通过。
- [x] `EmbedDebug.bat --station serial` 启动探针通过。
- [x] 根目录只存在 `build/`。
- [x] 本轮 commit。

## 6. 失败条件

- UI 直接解析 JSON 或直接写文件。
- UI include `core/SerialManager` 或具体协议目录。
- controller 写入 QWidget 逻辑。
- 加载失败污染当前 UI 状态。
- README 把设备验证写成真实硬件可用。

## 7. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E4 -> E5`，profile service 经 controller 接入工作台，QTest 覆盖保存/加载闭环 |
| 用户状态 | `U3 -> U4`，用户可从工作台保存/加载配置档案 |
| 设备状态 | 不提升 |
| 验证命令 | `test_serial_command_panel` 25/25；`test_serial_port_panel` 27/27；`test_serial_station_workbench` 22/22；`cmake --build build --target EmbedDebug --parallel 4`；边界 `rg` 检查；`EmbedDebug.bat --station serial` |
| commit | 本轮随实现提交 |

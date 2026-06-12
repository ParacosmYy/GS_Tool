# PRD-087 - Serial Station UART 配置校验与摘要

## 1. 标题

`PRD-087 - Serial Station UART 配置校验与摘要`

## 2. 目标

- 让 `SerialPortConfig` 提供标准化端口名、配置错误原因和 UART 摘要。
- 让 Controller 在连接前输出配置摘要，并在非法配置时输出明确错误。
- 主要交付物是 `SerialStationConfig`、`SerialStationController` 和 QTest 回归。

## 3. 非目标

- 不做 UI 布局/QSS 调整。
- 不改旧 `src/serial/`。
- 不做真实硬件串口依赖测试。
- 不新增协议。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `src/apps/serial_station/SerialStationConfig.h/.cpp` | 修改 |
| 主要文件 | `src/apps/serial_station/SerialStationController.cpp` | 修改 |
| 测试 | `tests/serial_station/test_serial_station_config.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_station_controller.cpp` | 修改 |
| 构建 | `tests/CMakeLists.txt` | 必要注册 |
| 禁止修改 | `src/serial/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 6. 验收标准

- [ ] `SerialPortConfig` 的摘要覆盖端口、波特率、数据位、校验、停止位、流控、DTR、RTS。
- [ ] 配置错误原因覆盖空端口、非正波特率和非法枚举值。
- [ ] Controller 非法配置路径使用配置错误原因。
- [ ] Controller 合法配置尝试连接时输出配置摘要日志。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialStationConfig|SerialStationController|SerialPortPanel|SerialStationWorkbench" --output-on-failure` 通过。
- [ ] `tools/doctor.ps1` 通过。
- [ ] `EmbedDebug.bat` 可启动。

## 7. 失败条件

- 出现真实串口依赖导致测试不稳定。
- 配置摘要在 UI 和 Controller 中重复实现。
- 修改范围扩展到旧串口模块或启动脚本。
- 新增测试未注册 CMake。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build build --target EmbedDebug --parallel 4",
    "ctest --test-dir build -R \"SerialStationConfig|SerialStationController|SerialPortPanel|SerialStationWorkbench\" --output-on-failure"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "git diff --cached --numstat -- '*.cpp' '*.h' '*.cc' '*.cxx' '*.c' '*.hpp'"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：不适用。
- 并行度上限：1。
- 人工审查状态：不适用，本轮改动范围小且触碰共享配置接口。

## 10. LOOP 路由

- Doctor：构建、测试、bat 启动失败时先跑 `tools/doctor.ps1`。
- Debug：若 Controller 日志或错误信号断言失败，先缩小到单测复现。
- Simplify：若配置摘要逻辑开始在多个类重复，回收进 `SerialPortConfig`。

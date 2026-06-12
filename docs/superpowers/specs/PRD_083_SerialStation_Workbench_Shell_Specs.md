# PRD-083 - Serial Station Workbench Shell Specs

## 目标

- 把 Serial Station 从单个 UART 配置表单推进为可见的串口工作台壳层。
- 用户可观察到配置区、命令区、日志区、状态栏四个区域。
- 主要交付物为 `src/apps/serial_station/ui/` 下的新 UI 面板、`SerialStationWindow` 装配、三套 QSS 样式和 QTest。

## 非目标

- 不实现真实串口发送、协议帧构建、日志文件导出或回放。
- 不修改旧 `src/serial/`。
- 不修改启动脚本、构建目录和输出路径。
- 不启用并行子 Agent，本轮为单 Agent 串行实现。

## 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/05-ui-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`

## 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| PRD | `docs/prd/PRD_083_SerialStation_Workbench_Shell.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_083_SerialStation_Workbench_Shell_Specs.md` | 新增 |
| UI 面板 | `src/apps/serial_station/ui/SerialCommandPanel.*` | 新增 |
| UI 面板 | `src/apps/serial_station/ui/SerialLogPanel.*` | 新增 |
| UI 面板 | `src/apps/serial_station/ui/SerialStatusBar.*` | 新增 |
| 装配 | `src/apps/serial_station/SerialStationWindow.*` | 修改 |
| 构建 | `CMakeLists.txt` | 仅注册新增源码/测试 |
| 测试 | `tests/serial_station/test_serial_station_workbench.cpp` | 新增 |
| 主题 | `resources/themes/*.qss` | 仅补新增 objectName 样式 |
| 禁止修改 | `src/apps/serial_station/core/`、`src/apps/serial_station/protocols/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` | 禁止 |

## 验收标准

- [ ] 新增 UI 文件不 include `SerialManager.h` 或具体协议头。
- [ ] `SerialStationWindow` 包含命令区、日志区和状态栏。
- [ ] 所有新增 QWidget 有 `objectName`。
- [ ] 用户可见文字使用 `tr()`。
- [ ] 新增源码和测试加入 `CMakeLists.txt`。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions" --output-on-failure` 通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1` 通过或仅保留已知 `go.exe` warning。
- [ ] `cmd /c call .\EmbedDebug.bat` 启动验证通过。

## 失败条件

- 需要修改 core/protocols 才能完成 UI 壳层。
- UI 直接拼接串口帧或写日志文件。
- 新文件未注册到 CMake。
- 构建或启动验证失败。
- 同一阶段混入 CMake 瘦身、旧 serial 迁移或无关重构。

## GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build build --target EmbedDebug --parallel 4"
  ],
  "check": [
    "ctest --test-dir build -R \"SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions\" --output-on-failure",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "cmd /c call .\\EmbedDebug.bat",
    "git status --short"
  ],
  "fix": []
}
```

## BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：3 个 UI 面板 + 1 个装配 + 1 个测试，文件互相关联，串行更稳。
- 并行度上限：1。
- 人工审查状态：不需要并行审查。

## LOOP 路由

- Doctor：构建环境、bat 启动和工具链问题。
- Debug：QTest 失败、控件状态错误、启动崩溃。
- Simplify：新增 UI 文件过大、重复样式或职责越界。

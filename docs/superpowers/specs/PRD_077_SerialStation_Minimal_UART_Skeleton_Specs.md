# Specs - PRD-077 Serial Station Minimal UART Skeleton

## 1. 标题

`PRD-077 - Serial Station Minimal UART Skeleton`

## 2. 目标

- 建立 `src/apps/serial_station/` 的 PascalCase C++/Qt 最小骨架。
- 让 protocols/core/controller/app 文件进入 CMake，不再只是未引用死代码。
- 增加不依赖真实 COM 口的 QTest，覆盖 ASCII 协议、协议注册表和会话状态。

## 3. 非目标

- 不删除旧小写 `app.h/app.cpp/window.h/config.h/constants.h/models.h`。
- 不接入 `MainWindow` 或 `PanelManager`。
- 不实现 UI 面板。
- 不实现 Modbus RTU/custom_md。
- 不改启动脚本。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/05-ui-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`
- `docs/prd/PRD_076_Source_Tree_Slimming_And_SerialStation_Minimal_Uart.md`
- `docs/prd/PRD_077_SerialStation_Minimal_UART_Skeleton.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要源码 | `src/apps/serial_station/` | 新增 PascalCase 骨架文件 |
| 测试 | `tests/serial_station/` | 新增 QTest |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册新增源码和测试 |
| 文档 | `docs/prd/`, `docs/superpowers/specs/`, `docs/superpowers/plans/` | 新增计划 |
| 只读参考 | `src/serial/`, `src/connection/serial_port/`, `src/core/connect/` | 只读 |
| 禁止修改 | `EmbedDebug.bat`, `tools/launch_embeddebug.ps1`, `src/core/mainwindow/`, `src/core/panels/` | 禁止 |

## 6. 验收标准

- [ ] 新增生产 `.h/.cpp` 已加入 `CMakeLists.txt`。
- [ ] 新增测试已加入 `tests/CMakeLists.txt`。
- [ ] 协议层不 include UI/QWidget。
- [ ] core 层不 include 具体协议目录。
- [ ] `cmake --build build` 通过。
- [ ] `cmake -S . -B build -DBUILD_TESTS=ON` 后新增 Serial Station 测试通过。
- [ ] `tools/source-tree-audit.ps1 -OutFile docs/reviews/simplify/source-tree-latest.md` 通过。
- [ ] 未创建第二构建目录。

## 7. 失败条件

- 新增文件未进入 CMake。
- UI 直接调用 `SerialManager` 或 core include 具体协议实现。
- 测试依赖真实 COM 口。
- 为了接入新工站修改 `MainWindow` 或 `PanelManager`。
- 构建失败且无法在本轮修复。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build .\\build --parallel 4",
    "cmake -S . -B build -DBUILD_TESTS=ON",
    "cmake --build .\\build --target test_ascii_text_protocol test_serial_protocol_registry test_serial_session --parallel 4"
  ],
  "check": [
    "ctest --test-dir .\\build -R \"AsciiTextProtocol|SerialProtocolRegistry|SerialSession\" --output-on-failure",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\source-tree-audit.ps1 -OutFile .\\docs\\reviews\\simplify\\source-tree-latest.md",
    "git status --short"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：是，作为后续并行开发方案。
- 子任务数量：8。
- 并行度上限：3。
- 人工审查状态：未审查。
- 本轮执行策略：共享接口、CMake 和最小骨架由主 Agent 串行处理。

## 10. LOOP 路由

- Doctor：CMake、Qt、构建、启动入口异常。
- Debug：测试失败、协议事件错误、CMake 注册遗漏。
- Simplify：旧小写骨架、重复目录、未引用文件清理。

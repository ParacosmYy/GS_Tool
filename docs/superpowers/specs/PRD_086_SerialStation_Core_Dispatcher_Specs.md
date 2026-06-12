# PRD-086 - Serial Station Core Dispatcher Specs

## 目标

- 新增 `core/SerialDispatcher`，承接原始 bytes 到 `ISerialProtocol::feed()` 的流式分发。
- 减少 `SerialStationController` 对接收协议状态的直接持有，让 controller 专注事件转译和 UI 信号。
- 保持当前发送闭环、接收闭环和 UI 表现不变。

## 非目标

- 不实现协议选择 UI。
- 不新增协议实现。
- 不新增 worker、队列、RingBuffer、日志 service 或文件导出。
- 不修改启动脚本、构建目录和输出路径。
- 不启动并行子 Agent；本轮新增 core 类并触碰 controller/CMake，串行合流更稳。

## 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`

## 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| PRD | `docs/prd/PRD_086_SerialStation_Core_Dispatcher.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_086_SerialStation_Core_Dispatcher_Specs.md` | 新增 |
| Core | `src/apps/serial_station/core/SerialDispatcher.h/.cpp` | 新增 |
| Controller | `src/apps/serial_station/SerialStationController.*` | 接入 dispatcher，移除直接接收协议持有 |
| 构建 | `CMakeLists.txt` | 仅注册新增 dispatcher 源码 |
| 测试 | `tests/serial_station/test_serial_dispatcher.cpp` | 新增 |
| 测试构建 | `tests/CMakeLists.txt` | 仅注册新增 dispatcher 测试 |
| 既有测试 | `tests/serial_station/test_serial_station_controller.cpp` | 保持/补充接收行为回归 |
| 禁止修改 | `src/apps/serial_station/ui/` | 本轮不改 UI |
| 禁止修改 | `src/apps/serial_station/protocols/ascii_text/` | 除非发现明确 feed bug |
| 禁止修改 | `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` | 禁止 |

## 行为规格

1. `SerialDispatcher::setProtocol(std::unique_ptr<ISerialProtocol>)` 替换当前协议实例。
2. `SerialDispatcher::feed(bytes)` 返回 `QVector<SerialProtocolEvent>`。
3. 空 bytes 不调用协议，返回空事件，并把最近状态标记为 0 bytes / no event。
4. 未设置协议时返回空事件，并标记协议不可用。
5. 非空 bytes 且协议返回空事件时，dispatcher 不自行生成 UI 文案，只记录最近 bytes 数和 no event。
6. `SerialDispatcher::reset()` 调用当前协议 `reset()`，不销毁协议实例。
7. Controller 根据 dispatcher 最近 feed 状态决定是否发“接收缓存”系统日志。
8. Controller 的公开信号和 UI 行为保持不变。

## 验收标准

- [ ] Dispatcher 不 include 具体协议目录、UI、QWidget 或 controller。
- [ ] UI 文件不 include `SerialDispatcher.h`。
- [ ] Controller 不直接持有 `std::unique_ptr<ISerialProtocol>` 接收成员。
- [ ] 新增源码已加入主 `CMakeLists.txt`。
- [ ] 新测试已加入 `tests/CMakeLists.txt`。
- [ ] Dispatcher QTest 覆盖未设置协议、空 bytes、完整行、半包补齐、reset。
- [ ] 既有 controller/workbench QTest 继续通过。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialDispatcher|SerialStationController|SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions" --output-on-failure` 通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1` 通过或仅保留已知 `go.exe` warning。
- [ ] `cmd /c call .\EmbedDebug.bat` 启动验证通过。

## 失败条件

- Dispatcher 直接依赖具体协议实现。
- Dispatcher 发 UI 文案或直接更新 UI。
- Controller 接收行为相对 PRD-085 回退。
- 新文件未注册到 CMake。
- 同一阶段混入旧 serial 迁移、协议新增或无关 UI 改动。

## GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build build --target EmbedDebug --parallel 4"
  ],
  "check": [
    "ctest --test-dir build -R \"SerialDispatcher|SerialStationController|SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions\" --output-on-failure",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "cmd /c call .\\EmbedDebug.bat",
    "git status --short"
  ],
  "fix": []
}
```

## BATCH 判定

- 是否需要 BATCH：否。
- 原因：新增 dispatcher、controller 接入、CMake 注册和测试目标互相关联，接口未稳定到可并行写代码。
- 并行度上限：1。
- 后续可拆分方向：协议选择 UI、Modbus/custom 协议、日志 service、worker 队列可在 dispatcher 稳定后拆分。

## LOOP 路由

- Doctor：Qt/MinGW、CMake、bat 启动路径和工具链问题。
- Debug：dispatcher feed 行为、reset 行为、controller 回归测试失败。
- Simplify：dispatcher 职责膨胀、controller 仍持有协议状态、重复解析逻辑。

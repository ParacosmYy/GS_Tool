# PRD-085 - Serial Station Controller Receive Loop Specs

## 目标

- 把 Serial Station 接收方向从底层 bytes 信号推进到 controller 协调闭环。
- Controller 使用默认协议处理流式接收事件，UI 只显示 RX 日志和状态计数。
- 第一阶段只复用现有 `ascii_text` 协议，不新增协议族、worker 或持久化服务。

## 非目标

- 不实现 Modbus/custom 协议。
- 不实现接收线程、接收队列、回放、文件日志或导出。
- 不迁移旧 `src/serial/` 和旧 `src/protocol/`。
- 不修改 `EmbedDebug.bat`、启动器、构建目录或可执行输出路径。
- 不启动并行子 Agent；本轮触碰共享 controller/window 接口，串行实现。

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
| PRD | `docs/prd/PRD_085_SerialStation_Controller_Receive_Loop.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_085_SerialStation_Controller_Receive_Loop_Specs.md` | 新增 |
| Controller | `src/apps/serial_station/SerialStationController.*` | 增加接收入口、协议 feed、RX/错误信号 |
| 装配 | `src/apps/serial_station/SerialStationWindow.*` | 连接 controller 接收结果到日志和状态栏 |
| 测试 | `tests/serial_station/test_serial_station_controller.cpp` | 增加接收路径单测 |
| 测试 | `tests/serial_station/test_serial_station_workbench.cpp` | 增加 UI 回写断言 |
| 禁止修改 | `src/apps/serial_station/core/` | 除非发现 bytes 信号不可达 |
| 禁止修改 | `src/apps/serial_station/protocols/ascii_text/` | 除非现有 feed 行为有明确 bug |
| 禁止修改 | `CMakeLists.txt`、`tests/CMakeLists.txt` | 本轮不新增测试目标时不改 |
| 禁止修改 | `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` | 禁止 |

## 行为规格

1. `SerialStationController::handleBytesReceived(bytes)` 是本轮唯一接收协调入口。
2. 空 `QByteArray` 直接忽略，不发日志、不计数。
3. Controller 通过默认协议实例调用 `feed(bytes)`。
4. 协议返回 `frame` 事件时：
   - 优先读取 payload 中的 `text` 字段作为 RX 日志。
   - 如果 text 为空，则以 raw bytes 的十六进制摘要展示。
   - 每个 frame 事件增加一次 RX 计数。
5. 协议返回 `log` 事件时，发系统日志，不增加 RX 计数。
6. 协议返回未知事件类型时，发系统日志并增加错误计数。
7. 协议暂未返回事件但 bytes 非空时，发系统日志说明已接收缓存字节，不增加 RX 计数。
8. 串口关闭或重新连接时，接收协议实例 reset，避免上一会话半包污染下一会话。

## 验收标准

- [ ] `SerialStationController` 连接 `SerialManager::bytesReceived`。
- [ ] Controller 不 include 具体协议目录。
- [ ] UI 文件不 include `SerialManager.h`、协议接口或具体协议头。
- [ ] Controller 单测覆盖空 bytes、完整 ASCII 行、半包粘包、未知事件防御入口。
- [ ] Workbench QTest 覆盖 RX 日志和 RX 计数显示。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialStationController|SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions" --output-on-failure` 通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1` 通过或仅保留已知 `go.exe` warning。
- [ ] `cmd /c call .\EmbedDebug.bat` 启动验证通过。

## 失败条件

- UI 直接解析 `QByteArray`。
- Core 直接依赖具体协议目录。
- Controller 复制协议解析逻辑而不是调用 `ISerialProtocol::feed`。
- 接收 bytes 被静默吞掉，UI 没有任何可观察反馈。
- 同一阶段混入 CMake 瘦身、旧 serial 迁移或无关 UI 大改。

## GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build build --target EmbedDebug --parallel 4"
  ],
  "check": [
    "ctest --test-dir build -R \"SerialStationController|SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions\" --output-on-failure",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "cmd /c call .\\EmbedDebug.bat",
    "git status --short"
  ],
  "fix": []
}
```

## BATCH 判定

- 是否需要 BATCH：否。
- 原因：本轮集中触碰 `SerialStationController` 的接收状态和 `SerialStationWindow` 的 UI 回写，接口仍在稳定期。
- 并行度上限：1。
- 后续可拆分方向：`SerialDispatcher`、协议选择器、日志 service、接收 worker 可在接口稳定后按 3 个子 Agent 拆分。

## LOOP 路由

- Doctor：构建环境、Qt/MinGW、bat 启动路径和工具链问题。
- Debug：QTest 失败、协议 feed 事件未触发、日志/计数未回写。
- Simplify：controller 变厚、UI 越界、重复协议解析逻辑。

# PRD-084 - Serial Station Controller Send Loop Specs

## 目标

- 把 Serial Station 命令发送从 UI 本地预览推进到 controller 协调闭环。
- UI 只表达发送意图；controller 统一校验、构帧、发送和日志/状态信号。
- 第一阶段只复用现有协议和 core 能力，不新增协议族和复杂发送队列。

## 非目标

- 不实现 Modbus/custom 协议。
- 不实现定时发送、发送队列、重试、超时等待、文件日志或回放。
- 不修改旧 `src/serial/`。
- 不修改 `EmbedDebug.bat`、启动器、构建目录或可执行输出路径。
- 不启动并行子 Agent；本轮触碰共享 controller/window 接口，串行更稳。

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
| PRD | `docs/prd/PRD_084_SerialStation_Controller_Send_Loop.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_084_SerialStation_Controller_Send_Loop_Specs.md` | 新增 |
| Controller | `src/apps/serial_station/SerialStationController.*` | 增加发送入口、发送结果信号、校验逻辑 |
| 装配 | `src/apps/serial_station/SerialStationWindow.*` | 把命令区信号接入 controller，展示 controller 结果 |
| UI 面板 | `src/apps/serial_station/ui/SerialCommandPanel.*` | 仅允许补必要状态接口，不直接发送 |
| UI 面板 | `src/apps/serial_station/ui/SerialLogPanel.*` | 仅允许补日志展示辅助接口，不直接写文件 |
| UI 面板 | `src/apps/serial_station/ui/SerialStatusBar.*` | 仅允许补计数/错误状态接口 |
| 测试 | `tests/serial_station/test_serial_station_controller.cpp` | 新增或更新 |
| 测试 | `tests/serial_station/test_serial_station_workbench.cpp` | 更新窗口连接断言 |
| 构建 | `tests/CMakeLists.txt` | 仅注册新增测试 |
| 禁止修改 | `src/apps/serial_station/core/` | 除非发现现有公开接口无法发送 |
| 禁止修改 | `src/apps/serial_station/protocols/` | 除非发现 registry 公开接口无法选择协议 |
| 禁止修改 | `CMakeLists.txt` | 不新增生产文件时不改 |
| 禁止修改 | `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` | 禁止 |

## 行为规格

1. `SerialStationController::sendCommand(command, mode)` 是本轮唯一发送入口。
2. `command.trimmed().isEmpty()` 时立即失败，不调用 core。
3. 串口未打开时立即失败，不调用 core。
4. 模式当前限定为 `ascii` 和 `protocol`：
   - `ascii`：复用默认文本协议构建帧。
   - `protocol`：同样经默认协议构建帧，后续协议选择另开 PRD。
   - 其他模式：立即失败并提示模式暂不支持。
5. 发送成功时至少发出发送日志和发送计数事件。
6. 发送失败时至少发出错误日志和错误计数事件。
7. `SerialStationWindow` 只连接信号，不做命令校验和构帧。

## 验收标准

- [ ] `SerialStationWindow` 中不再直接把发送按钮转成本地 TX 预览。
- [ ] UI 文件不 include `SerialManager.h`、具体协议头或 core 发送类。
- [ ] Controller 不 include 具体协议目录。
- [ ] Controller 对空命令、未打开串口、未知模式都有 QTest。
- [ ] Workbench QTest 覆盖发送按钮到日志/错误状态的连接。
- [ ] 新测试加入 `tests/CMakeLists.txt`。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialStationController|SerialStationWorkbench|SerialPortPanel|Qt6CompatRegressions" --output-on-failure` 通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1` 通过或仅保留已知 `go.exe` warning。
- [ ] `cmd /c call .\EmbedDebug.bat` 启动验证通过。

## 失败条件

- UI 直接拼接协议帧、调用 core 或写文件。
- Controller 为本轮发送闭环引入第二套协议 registry。
- 发送错误被静默吞掉，UI 没有可观察反馈。
- 新增测试没有注册到 CMake。
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
- 原因：本轮集中触碰 `SerialStationController` 和 `SerialStationWindow` 两个共享接口，拆成多个写代码子 Agent 会增加合流冲突。
- 并行度上限：1。
- 后续可拆分方向：协议选择器、发送队列、日志服务、回放服务可在接口稳定后按 3 个子 Agent 拆分。

## LOOP 路由

- Doctor：构建环境、Qt/MinGW、bat 启动路径和工具链问题。
- Debug：QTest 失败、信号未触发、发送状态错误。
- Simplify：controller 职责膨胀、UI 越界、重复协议构建逻辑。

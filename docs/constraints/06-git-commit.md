# 06 - Git规范与Commit规则

> 本文档是 EmbedDebug 约束体系的第6模块。每次提交前都应先对照检查。

---

## 一、Git基本规范

- 分支: `feat/embed-debug`
- 远程: `https://github.com/ParacosmYy/GS_Tool.git`
- commit message 用中文
- 不提交 `build/` 目录
- 不创建、不引用、不兼容第二构建目录；项目唯一构建目录是 `build/`
- `.vscode/settings.json` 和 `c_cpp_properties.json` 需要提交
- 项目目标分数是 `1000`，每次 commit 只允许 +1 分

---

## 二、Commit规则

### 默认要求

1. 每次提交前先确认改动范围，尽量保持单一主题
2. 提交前必须保证编译通过，不能带编译错误提交
3. `EmbedDebug.bat` 双击能启动是最低验收线，影响构建/启动/资源/依赖/路径的改动在提交前必须验证
4. 每次 commit 只允许记 **1 分**：`评分: <当前分> + 1 = <新分>`
5. 包含代码文件的提交（`.cpp/.h/.cc/.cxx/.c/.hpp`）必须满足：
   - 代码增量（`added + removed`）**≥ 500 行**
   - 增量定义来自 `git diff --cached --numstat`（仅统计以上代码扩展名）
   - 计算命令（PowerShell）：
   ```powershell
   $codeDiff = git diff --cached --numstat -- '*.cpp' '*.h' '*.cc' '*.cxx' '*.c' '*.hpp'
   $total = 0
   foreach ($line in $codeDiff) {
       $fields = $line -split "`t"
      if ($fields.Length -ge 2) { $total += [int]$fields[0] + [int]$fields[1] }
   }
   "code_line_delta=$total"
   ```
6. **任何代码提交都需先在 `docs/tracking/SCORE_TRACKING.md` 补充对应加分记录或里程碑说明**，提交前由变更人确认。
7. `local_env.bat` 为本地环境文件，必须在 `.gitignore` 中排除且不纳入提交。
8. 超过单点修复的任务必须分阶段 commit：约束/PRD/Specs、构建与工具、生产代码、测试、清理与审计报告应尽量拆开提交。
9. 每个阶段完成后都要及时 commit，不把多个阶段长期堆在工作区；确因验证依赖无法立即提交时，必须先记录原因并在下一可验证点提交。
10. 每次提交前必须检查 `git status --short` 和 `git diff --cached --stat`，确认没有混入 `local_env.bat`、build 产物、无关用户改动或平行构建目录。
11. 涉及产品能力、UI、Serial Station、协议、构建或启动方式变化的提交，必须检查 README 是否仍符合企业级宣传入口标准；若不更新 README，提交说明或收口说明需要写明原因。
12. 每轮迭代完成后必须提交一次；未提交的工作只能视为“工作区改动”，不能视为已收口。
13. commit message 或收口说明必须写清三轴状态变化；没有状态提升时写“状态不提升，仅文档/结构/测试收口”。
14. 如果工作区已有用户改动，提交必须只包含本轮明确修改的文件；无法隔离时禁止提交，并说明阻塞原因。
15. 子 Agent 不直接 commit；并行任务由主 Agent 合流验证后统一提交。

### 硬性禁止

- 禁止提交编译不过的代码
- 禁止提交后 `EmbedDebug.bat` 无法双击启动；这是最低忍耐度，不允许用“代码能编译”替代启动验证
- 禁止提交任何 `build2/`、`build-debug/`、`build-release/`、`cmake-build-*` 等平行构建目录引用
- 禁止提交构建系统(CMakeLists.txt)未注册的源码文件（`.cpp/.h`）
- 禁止提交 build 产物作为代码（`.o/.obj/.exe/.dll/.so/.a/CMakeCache.txt/CMakeFiles/` 等）
- 禁止提交越界的 Serial Station 改动（UI 直接调用 core/protocol/service 边界外代码）
- 禁止提交与本轮任务无关的已暂存用户改动
- 禁止在没有验证证据时把功能状态提升为用户完成或设备完成

### commit message格式

```
<模块名>: <简述改了什么>

<详细说明为什么这样改，解决了什么问题>

状态: 工程 <Ex->Ey 或不提升>, 用户 <Ux->Uy 或不提升>, 设备 <Dx->Dy 或不提升>
验证: <构建/测试/启动/文档检查命令和结论；不能验证时写具体原因>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

### Serial Station 提交前检查

涉及 `src/apps/serial_station/` 时，commit 前必须确认：

1. 新增 `.h/.cpp` 已加入 `CMakeLists.txt`。
2. 新增协议已加入 `SerialProtocolRegistry` 或对应注册入口。
3. 新增协议已有 `tests/serial_station/test_<protocol>_protocol.cpp`。
4. `core/` 未直接 include 具体协议目录。
5. `ui/` 未直接 include `core/SerialManager.h` 或具体协议头文件。
6. `EmbedDebug.bat` 启动验证仍然通过。
7. 如果声明 UART/协议/日志/导出/回放用户可用，必须有工作台入口和用户操作路径证据。
8. 如果声明真实串口或设备可用，必须写明 COM 口、设备、虚拟串口或替身验证方式；纯单测只能标为 `D1`。

---

## 三、关联文档

评分追踪请看: [docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md)

本文件只定义提交规则与提交格式，不再重复记录阶段评分、里程碑摘要或增长口径。

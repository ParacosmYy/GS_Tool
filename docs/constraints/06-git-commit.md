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

### 硬性禁止

- 禁止提交编译不过的代码
- 禁止提交后 `EmbedDebug.bat` 无法双击启动；这是最低忍耐度，不允许用“代码能编译”替代启动验证
- 禁止提交任何 `build2/`、`build-debug/`、`build-release/`、`cmake-build-*` 等平行构建目录引用
- 禁止提交构建系统(CMakeLists.txt)未注册的源码文件（`.cpp/.h`）
- 禁止提交 build 产物作为代码（`.o/.obj/.exe/.dll/.so/.a/CMakeCache.txt/CMakeFiles/` 等）
- 禁止提交越界的 Serial Station 改动（UI 直接调用 core/protocol/service 边界外代码）

### commit message格式

```
<模块名>: <简述改了什么>

<详细说明为什么这样改，解决了什么问题>

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

---

## 三、关联文档

评分追踪请看: [docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md)

本文件只定义提交规则与提交格式，不再重复记录阶段评分、里程碑摘要或增长口径。

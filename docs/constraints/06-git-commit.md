# 06 - Git规范与Commit规则

> 本文档是 EmbedDebug 约束体系的第6模块。每次提交前都应先对照检查。

---

## 一、Git基本规范

- 分支: `feat/embed-debug`
- 远程: `https://github.com/ParacosmYy/GS_Tool.git`
- commit message 用中文
- 不提交 `build/` 目录
- `.vscode/settings.json` 和 `c_cpp_properties.json` 需要提交

---

## 二、Commit规则

### 默认要求

1. 提交前先确认改动范围清楚，尽量让单次 commit 只覆盖一个主题
2. 提交前必须保证编译通过，不能带编译错误提交
3. 提交后必须验证 `EmbedDebug.bat` 能正常启动
4. `git diff --stat` 只作为参考，不再作为绝对门槛

### 变更规模建议

- 默认建议：单次 commit 尽量达到 **300 行左右** 的有效变更，便于形成有意义的检查点
- 允许例外：以下类型的改动可以低于 300 行
  - 紧急 bugfix
  - 纯文档修订
  - 构建/脚本修复
  - 小范围重构或拆分
  - 生成文件或自动化产物的独立更新

### 例外说明

当 commit 低于默认建议规模时，需要在以下位置之一说明原因:
- commit message
- PR 描述
- 变更记录

说明内容至少包括:
- 为什么这次不适合继续合并
- 为什么此时提交比继续等待更合适
- 有没有后续要接上的关联改动

### commit message格式

```
<模块名>: <简述改了什么>

<详细说明为什么这样改，解决了什么问题>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

### 禁止的行为

- 禁止提交编译不过的代码
- 禁止重复造轮子（公共组件只写一次）
- 禁止不经 PRD 直接写代码
- 禁止不经架构审查直接加新类
- 禁止把明显不完整的改动硬拆成多个无说明的 commit
- 禁止提交后 `EmbedDebug.bat` 无法启动
- **禁止提交构建系统(CMakeLists.txt)中不存在的源文件** — 所有 .h/.cpp 文件必须在 CMakeLists.txt 中注册后才能提交。禁止创建"查无产生"的死代码文件来刷分。违反此条的 commit 一经发现全部回退
- **禁止提交build产物作为代码** — 严禁将编译中间文件(.o/.obj)、Qt生成文件(moc_*/ui_*/qrc_*)、构建目录(build/)、二进制产物(.exe/.dll/.a/.so)、CMake缓存(CMakeCache.txt/CMakeFiles/)等作为新特性代码commit。允许提交的文件类型: .h/.cpp/.qss/.qrc/.ui/.cmake/CMakeLists.txt/.md/.json/.py/.bat/.sh 等源码和配置文件。build产物充数一律回退

---

## 三、关联文档

评分追踪请看: [docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md)

本文件只定义提交规则与提交格式，不再重复记录阶段评分、里程碑摘要或增长口径。

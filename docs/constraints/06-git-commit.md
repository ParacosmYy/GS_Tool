# 06 - Git规范与Commit规则

> 本文档是 EmbedDebug 约束体系的第6模块。每次提交时必须遵守。

---

## 一、Git基本规范

- 分支: `feat/embed-debug`
- 远程: `https://github.com/ParacosmYy/GS_Tool.git`
- commit message 用中文
- 不提交 build/ 目录
- `.vscode/settings.json` 和 `c_cpp_properties.json` 需要提交

---

## 二、Commit规则（铁律）

### 强制要求

1. **每次commit必须 ≥ 300行代码变更**（不含空行和注释）
2. **每次commit = +1分**，不足300行不允许commit
3. **零编译错误才能commit** — 编译不过必须先修
4. **每次commit后必须验证 EmbedDebug.bat 能正常启动**
5. **commit前自检**:
   - `cmake --build build` 确认零错误
   - `git diff --stat` 确认 ≥ 300行变更

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
- 禁止不经PRD直接写代码
- 禁止不经架构审查直接加新类
- 禁止不足300行变更就提交commit
- 禁止提交后bat无法启动应用

---

## 三、评分追踪

评分追踪独立文档: [docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md)

起点: 1分 | 目标: 1000分 | 每次commit = +1分

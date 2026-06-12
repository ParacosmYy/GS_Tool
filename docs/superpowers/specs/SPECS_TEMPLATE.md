# Specs Template

> 用途：任何超过单点修复的 Agent 迭代，先写 Specs，再执行。Specs 是执行边界，不是事后总结。

## 1. 标题

`<PRD编号或任务编号> - <短标题>`

## 2. 目标

- 本轮要达成什么。
- 用户可观察到什么变化。
- 哪些文件或模块是主要交付物。

## 3. 非目标

- 本轮明确不做什么。
- 哪些已有问题只记录，不顺手扩大。
- 哪些模块禁止触碰。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- 按任务类型补充 `03-architecture.md`、`04-coding-standard.md`、`05-ui-standard.md`、`07-directory-structure.md` 或 `docs/serial_station_architecture.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `<path>` | 新增 / 修改 / 删除 |
| 只读参考 | `<path>` | 只读 |
| 禁止修改 | `<path>` | 禁止 |

## 6. 验收标准

- [ ] 功能或文档结果满足 PRD。
- [ ] 构建或测试命令通过。
- [ ] `EmbedDebug.bat` 启动链路在受影响时已验证。
- [ ] 没有新增第二构建目录。
- [ ] 用户已有改动没有被回退。

## 7. 失败条件

- 出现无法解释的构建失败。
- 需要修改禁止文件。
- 子任务之间发生同文件冲突。
- 验证命令不能覆盖本轮核心行为。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build .\\build --config Release --parallel 4"
  ],
  "check": [
    "git status --short"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：是 / 否。
- 若需要，子任务数量：5-30。
- 并行度上限：1 / 3 / 6。
- 人工审查状态：未审查 / 已审查。

## 10. LOOP 路由

- Doctor：系统体检、环境、构建、启动。
- Debug：可复现 Bug、崩溃、逻辑错误。
- Simplify：清理重复、缩小复杂度、拆分过大文件。

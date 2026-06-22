# LOOP_STATE - 闭环状态追踪

> 本文件是 [09-closed-loop.md](../constraints/09-closed-loop.md) 的可追踪状态落点。
> 每轮迭代收口后追加一行，把"我在闭环里"变成可审计的状态。
> 与 `docs/superpowers/LOOP_PROTOCOL.md` 配套：LOOP_PROTOCOL 定义规则，本文件记录执行。

---

## 一、闭环批次记录

> 列：日期 | Batch/任务 | 视角（5 项打勾） | 门禁（6 项打勾） | 三轴变化 | 评分变化 | 闭环结果 | 下一步

| 日期 | Batch / 任务 | 架构 | 实现 | 测试 | 产品 | 用户 | test | smoke | bat | lint | check | 行数 | E/U/D 变化 | 评分 | 结果 | 下一步 |
|------|-------------|------|------|------|------|------|------|-------|-----|------|-------|------|-----------|------|------|------|
| 2026-06-22 | 约束文档闭环化改造（本次） | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | ✓ | ✓ | 无（纯文档） | 787→788 预期 | 通过 | dogfooding 首条；后续 AI 按本表追加 |

---

## 二、已知阻塞 / pre-existing 项

> 不在当前闭环范围、但需记录避免误判的已知问题。

| 项目 | 状态 | 发现日期 | 说明 | 跟踪 |
|------|------|----------|------|------|
| `tests/python/unit/test_stagger_coordinator.py` collection error | pre-existing | 2026-06-22 | `uv run pytest --collect-only -q` 报 `1 error during collection`，但该文件单独 collect 正常（10 tests collected），疑似与其他文件 fixture 冲突 | 待 Debug 路由定位 |
| CLAUDE.md 历史 ROADMAP 表（UI-01~EX-05）| 历史归档 | 2026-06-22 | 含 native 时代遗留状态标记，不再作为完成依据；三轴口径以 09-closed-loop §四 为准 | 后续清理批次 |

---

## 三、使用说明

### 3.1 谁来写

当前会话的 AI 在每轮收口时追加一行，不要求用户维护。

### 3.2 何时写

- 每个通过门禁的 commit 后
- 每次 LOOP 触发（Doctor/Debug/Simplify）后，无论是否解决
- 每次安全刹车（20 轮 / 30 分钟）后

### 3.3 怎么读

- 评审上一轮 AI 工作时，看本表最后一行
- 排查回归时，按日期倒查"门禁"列的 ✗
- 规划下一轮时，看"下一步"列

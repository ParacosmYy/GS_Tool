# 00 - 单一真相源（SSOT）

> 适用范围：任何需要引用项目动态事实（分数、测试数、文件数、版本、分支）的文档或 commit message。
> 本文件**不存储任何动态值**，只声明"真相在哪里"。
> AI 写文档或 commit 前先读本文件，从 canonical 处取值，禁止凭记忆硬编码。

---

## 一、为什么需要 SSOT

历史教训（2026-06-22 审计）：同一个"当前评分"曾同时出现 4 个值——
CLAUDE.md=787、README=772、01-overview=618、01-overview §2-a.1=500、SCORE_TRACKING=787。
AI 每次想 +1 分都不知该写哪，必然漂移。

本文件用"指针表 + 机械守护脚本"彻底消除这类漂移：
- 动态值只在 canonical 处维护一处
- 其他文档只能引用，不能复制
- `tools/check_constraints.py`（即 `uv run check-constraints`）每次 commit 前机械检测，drift 即 fail

---

## 二、动态事实 → canonical 落点

| 事实 | canonical 落点 | 查询方式 |
|------|----------------|----------|
| 当前评分 | `docs/tracking/SCORE_TRACKING.md` 首行 `当前: NNN分` | 读文件首行 |
| 目标评分 | `docs/tracking/SCORE_TRACKING.md` 首行 `目标: 1000分` | 同上 |
| 距目标差值 | `docs/tracking/SCORE_TRACKING.md` 首行 `距离目标还差 NNN分` | 同上 |
| 评分历史 | `docs/tracking/SCORE_TRACKING.md` 正文表格 | 读文件 |
| Git 分支 | `git branch --show-current` | 实时采集 |
| Git 远程 | `git remote -v` | 实时采集 |
| 版本号 | `pyproject.toml` → `[project].version` | 读文件 |
| .py 源码文件数 | `git ls-files "python/embeddebug/*.py" \| wc -l` | 实时采集 |
| 测试文件数 | `git ls-files "tests/python/*.py" \| wc -l` | 实时采集 |
| 测试用例数 | `uv run pytest --collect-only -q 2>&1 \| tail -3` | 实时采集（禁止硬编码 passed 数） |
| lint 状态 | `uv run lint-embeddebug-py` | 实时采集 |
| 三轴 E/U/D | `docs/tracking/SCORE_TRACKING.md` 或 PRD 收口段 | 引用，不复制 |

---

## 三、引用规则（强制）

### 3.1 允许的引用形式

```markdown
评分：见 docs/tracking/SCORE_TRACKING.md 首行
测试通过数：运行 `uv run pytest --collect-only -q | tail -3`
分支：运行 `git branch --show-current`
```

### 3.2 禁止的硬编码形式

```markdown
评分：787 / 1000                    ← 禁止（会漂移）
测试通过：2133 passed, 2 skipped    ← 禁止（每次 commit 都变）
测试文件数：172                     ← 禁止
当前分支：feat/embed-debug          ← 禁止
```

例外：`SCORE_TRACKING.md` 首行本身是 canonical，允许硬编码。其他文档一律引用。

---

## 四、机械守护

`tools/check_constraints.py`（入口 `uv run check-constraints`）会在以下情况返回非 0：

1. CLAUDE.md / README.md / 01-project-overview.md / 06-git-commit.md 出现硬编码 `NNN/1000` 或 `NNN分` 字样且与 SCORE_TRACKING 不一致
2. CLAUDE.md / README.md 出现硬编码 `NNNN passed` / `NNN 个测试文件` 字样
3. 仓库出现冻结/禁止目录：`src/`、`build2/`、`build-debug/`、`build-release/`、`native-build-*`
4. `python/embeddebug/**/*.py` 单文件 > 300 行，或 `tests/python/**/*.py` 单文件 > 250 行

该脚本是 [09-closed-loop.md](09-closed-loop.md) 门禁 5 的实现，每次 commit 前必须通过。

---

## 五、与加载顺序的关系

`AGENTS.md` 的强制加载顺序已把本文件列为"最先读"——
任何 Agent 在写文档或 commit message 前，必须先读本文件确认真相落点，再取值。

详见 [AGENTS.md §一 强制加载顺序](../../AGENTS.md)。

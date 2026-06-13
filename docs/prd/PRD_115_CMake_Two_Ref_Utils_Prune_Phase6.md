# PRD-115 - CMake Two Ref Utils Prune Phase 6

## 背景

PRD-110 到 PRD-114 已连续收口多批低风险 `utils` CMake 清单瘦身，源码保留，构建与启动验证通过。最新审计显示真实命名 `src/utils/` 目录仍有 2-ref 候选。

本轮继续小步处理下一批 30 个真实命名 2-ref 目录，不处理 generated numbered utils。

## 目标

从 `cmake/EmbedDebugSources.cmake` 中移除以下目录条目：`catboost`, `cauchy`, `cepstrum`, `cgls`, `changept`, `chebyshev`, `chisq`, `cholupdate`, `cic`, `circadian`, `circbuf`, `circqueue`, `circulant`, `classifier`, `clenshaw_curtis`, `cluster`, `collocation`, `combination`, `complement`, `conjugate`, `convhull`, `convolution`, `correlator`, `cosinedist`, `countmin`, `covariance`, `crc64b`, `crosscorr`, `crossval`, `cuckoo_hash`。

## 非目标

1. 不修改生产 C++。
2. 不删除、移动或重命名源码。
3. 不修改测试 CMake。
4. 不处理 generated numbered utils。
5. 不改变用户可见能力。

## 安全依据

- 上述目录均只有 2 条 active CMake 引用。
- 路径级扫描未发现 `utils/<dir>/` 外部引用。
- 本轮只改 CMake 清单，失败可单点回退。

## 验收标准

1. `python tools/project-audit/project_audit.py --limit 5` 通过，缺失文件和重复条目均为 0。
2. 30 个目标目录 active CMake refs 归零。
3. CMake 配置和 `EmbedDebug` 主目标构建通过。
4. 刷新 `docs/reviews/simplify/source-tree-latest.md`。
5. `.\EmbedDebug.bat` 启动探针通过。
6. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以审计、配置、构建和启动探针为证据。
- 用户状态：不提升，产品行为无变化。
- 设备状态：不提升，真实硬件未验证。

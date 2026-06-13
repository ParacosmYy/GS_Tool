# PRD-113 - CMake Single Ref Utils Prune Phase 4

## 背景

PRD-110 到 PRD-112 已分三批从主 GUI 目标移除低风险 `utils` 清单条目，源码保留，构建与启动验证通过。最新审计显示仍有一批真实命名 `src/utils/` 目录只剩 1 条 active CMake 引用。

本轮继续小步瘦身，只修改主 GUI CMake 清单，不删除源码，不处理 generated numbered utils。

## 目标

1. 从 `cmake/EmbedDebugSources.cmake` 中移除以下每目录 1 条 CMake 引用：
   - `avltree`
   - `bfgs`
   - `capture`
   - `crc_verifier`
   - `cycle`
   - `decomposer`
   - `gaussquad`
   - `golay`
   - `groebner`
   - `heap`
   - `hessenberg`
   - `hmm`
   - `inv_perm`
   - `lfu`
   - `lifting`
   - `lru`
   - `moment`
   - `movingmax`
   - `qrupdate`
   - `redblack`
   - `scaler`
   - `segment`
   - `skiplist`
   - `sliding`
   - `smoother`
   - `state`
   - `threshold`
   - `trie`
   - `trigger`
   - `twoway`
   - `warp`
2. 不删除源码文件。
3. 不处理 `src/utils/<name><number>/` generated numbered 目录。
4. 保持构建、审计和启动链路通过。

## 非目标

1. 不修改生产 C++。
2. 不删除、移动或重命名文件。
3. 不修改测试 CMake。
4. 不改变用户可见能力。
5. 不提升设备验证状态。

## 安全依据

- 上述目录均只有 1 条 active CMake 引用。
- 路径级扫描未发现 `utils/<dir>/` 外部引用。
- 本轮只改 CMake 清单，失败可单点回退。

## 验收标准

1. `python tools/project-audit/project_audit.py --limit 5` 通过，缺失文件和重复条目均为 0。
2. `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 配置通过。
3. `cmake --build build --target EmbedDebug --parallel 4` 构建通过。
4. `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md` 通过。
5. `.\EmbedDebug.bat` 启动探针通过。
6. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以审计、配置、构建和启动探针为证据。
- 用户状态：不提升，产品行为无变化。
- 设备状态：不提升，真实硬件未验证。

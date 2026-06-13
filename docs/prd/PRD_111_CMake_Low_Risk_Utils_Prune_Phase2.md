# PRD-111 - CMake Low Risk Utils Prune Phase 2

## 背景

PRD-110 已从主 GUI 目标移除第一批无外部引用证据的 `utils` 条目，主目标配置、构建和启动探针均通过。刷新后的 `source-tree-latest.md` 继续列出第二批低风险 active utils 候选。

本轮继续小步瘦身，处理第二批候选目录。它们在审计报告中 `External include refs = 0`，直接搜索也未发现测试或主源码 include。

## 目标

1. 从 `cmake/EmbedDebugSources.cmake` 中移除以下 `src/utils/` 目录条目：
   - `aggregator`
   - `bitmask`
   - `compare`
   - `gmm`
   - `hex_diff`
   - `hex_editor`
   - `packet_lib`
   - `pattern`
   - `rate`
   - `ringhash`
   - `spectrum`
   - `waveform`
2. 不删除源码文件。
3. 保持 `scripting` 不动，继续留给测试目标引用。
4. 保持构建、审计和启动链路通过。

## 非目标

1. 不修改生产 C++。
2. 不删除、移动或重命名文件。
3. 不修改测试 CMake。
4. 不处理 generated numbered utils。
5. 不改变用户可见能力。

## 安全依据

- `source-tree-latest.md` 中上述目录均为低风险 active utils 候选。
- 直接搜索未发现主源码或测试源码 include 这些 `src/utils/<dir>/` 路径。
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

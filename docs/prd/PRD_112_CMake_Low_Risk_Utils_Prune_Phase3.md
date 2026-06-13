# PRD-112 - CMake Low Risk Utils Prune Phase 3

## 背景

PRD-110 与 PRD-111 已分两批从主 GUI 目标移除无外部引用证据的 `utils` 条目，构建与启动验证均通过。刷新后的审计报告仍列出第三批低风险候选，主要是每组 3 个 CMake refs 的独立工具目录。

本轮继续小步瘦身，只修改主 GUI CMake 清单，不删除源码。

## 目标

1. 从 `cmake/EmbedDebugSources.cmake` 中移除以下 `src/utils/` 目录条目：
   - `align`
   - `annotation`
   - `compress`
   - `data_inspector`
   - `decoder`
   - `encoder`
   - `frequency`
   - `fuzzer`
   - `loss`
   - `protocol_timer`
   - `recorder`
   - `signal_gen`
   - `splitter`
   - `template_lib`
   - `validator`
2. 不删除源码文件。
3. 不处理 `src/utils/scripting/`。
4. 保持构建、审计和启动链路通过。

## 非目标

1. 不修改生产 C++。
2. 不删除、移动或重命名文件。
3. 不修改测试 CMake。
4. 不处理 generated numbered utils。
5. 不改变用户可见能力。

## 安全依据

- 上述目录在 `source-tree-latest.md` 中均为低风险 active utils 候选。
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

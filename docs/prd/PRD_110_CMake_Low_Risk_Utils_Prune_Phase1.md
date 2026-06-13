# PRD-110 - CMake Low Risk Utils Prune Phase 1

## 背景

`docs/reviews/simplify/source-tree-latest.md` 的 `Low-Risk Active Utils Split Candidates` 显示，若干 `src/utils/` 子目录仍在主 GUI 目标中 active，但没有外部 include 证据。它们多为独立工具或实验性 widget，当前并未从主应用入口引用。

本轮只做第一批低风险 CMake 瘦身：从主 GUI 目标移除 6 个无外部引用证据的 `utils` 目录条目，源码保留在工作区，后续如需要可独立 target 或重新评审接入。

## 目标

1. 从 `cmake/EmbedDebugSources.cmake` 中移除以下目录的 `.cpp/.h` 条目：
   - `src/utils/filter_design/`
   - `src/utils/firmware/`
   - `src/utils/gps/`
   - `src/utils/network/`
   - `src/utils/statemachine/`
   - `src/utils/wavegen/`
2. 不删除源码文件。
3. 不处理 `src/utils/scripting/`，因为测试目标仍引用 `ScriptEngine`。
4. 保持主目标配置、构建和 `EmbedDebug.bat` 启动探针通过。

## 非目标

1. 不修改生产 C++ 源码。
2. 不删除、移动或重命名文件。
3. 不修改测试 CMake。
4. 不处理 generated numbered utils。
5. 不改变用户可见功能状态。

## 安全依据

- `source-tree-latest.md` 中上述目录 `External include refs = 0`。
- 直接搜索未发现主源码或测试源码 include 这些目录。
- 本轮只改 CMake 清单，失败可单点回退。
- `src/utils/scripting/` 虽也在候选表中，但测试目标引用它，本轮明确排除。

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

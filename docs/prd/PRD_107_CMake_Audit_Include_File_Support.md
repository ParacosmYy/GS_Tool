# PRD-107 - CMake Audit Include File Support

## 背景

PRD-106 将根 `CMakeLists.txt` 中的 `SOURCES` 与 `HEADERS` 清单拆入 `cmake/EmbedDebugSources.cmake`。构建已经通过，但现有审计工具仍按旧结构解析 CMake：

- `tools/project-audit/project_audit.py` 只读取根 `CMakeLists.txt`。
- `tools/source-tree-audit.ps1` 只收集 `CMakeLists.txt` 文件，不读取 `cmake/*.cmake` 片段。

这会导致源码清单统计误判。例如 `project_audit.py` 的 `listed code entries` 变成 0，后续源码树瘦身和 CMake 引用审计会失去依据。

## 目标

1. 让 Python 项目审计工具读取根 `CMakeLists.txt`、所有非 build 目录下的 `CMakeLists.txt`，以及仓库 `cmake/*.cmake` 片段。
2. 让 PowerShell 源码树审计工具读取 `cmake/*.cmake` 片段，使 active CMake 引用恢复真实口径。
3. 为 Python 工具补单元测试，覆盖 `cmake/EmbedDebugSources.cmake` 中的源码引用。
4. 刷新 `docs/reviews/simplify/source-tree-latest.md`。

## 非目标

1. 不修改生产 C++ 源码。
2. 不修改根 `CMakeLists.txt`。
3. 不改变 CMake 构建结果。
4. 不删除、移动源码。
5. 不提升用户状态或设备验证状态。

## 约束

- 审计工具只读源码树，除指定报告路径外不得写入其他位置。
- 构建目录只能是 `build/`。
- 报告输出只能写入 `docs/reviews/simplify/`。
- 用户已有改动不得回退。

## 验收标准

1. `python tools/project-audit/project_audit.py --limit 5` 的 `listed code entries` 大于 0。
2. `uv run test-embeddebug-tools` 通过，或在无 uv 时使用 `python tools/test_embeddebug_tools.py` 通过。
3. `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md` 通过。
4. 报告 Summary 中 `Active CMake source references` 大于 0，且 `CMake refs under src/apps/serial_station` 大于 0。
5. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以工具测试和审计报告为证据。
- 用户状态：不提升，用户侧产品行为无变化。
- 设备状态：不提升，真实硬件未验证。

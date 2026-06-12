# PRD-081 - Source Tree Active CMake Audit

## 背景

PRD-076 的 `source-tree-audit.ps1` 已经能量化源码规模、CMake 文本引用和 Serial Station 缺口。但当前报告里的 `Direct CMake source references` 仍然基于原始 CMake 文本扫描，存在两个误导点：

1. 已注释的 `# removed: duplicate class` 条目仍可能被计入。
2. `CMakeLists.txt` 中 `list(FILTER SOURCES|HEADERS EXCLUDE REGEX ...)` 已过滤的数字 utils 草稿目录，仍会出现在原始引用计数里。

这会让后续判断“编译为什么慢、哪些文件还在主 GUI 目标里”时不够精确。

## 目标

1. 审计报告同时输出三类 CMake 指标：
   - 原始文本引用数量。
   - 去注释后的声明引用数量。
   - 应用已知过滤规则后的活跃引用数量。
2. 把 `src/utils/<目录名含数字>/` 过滤命中数量单独列出。
3. Serial Station 的 `In CMake` 判断改用活跃引用集合。
4. 保持脚本只读源码和 CMake；除指定报告外不写其他文件。

## 非目标

1. 不删除源码。
2. 不修改 `CMakeLists.txt`。
3. 不改变构建目标。
4. 不启动 BATCH 并行。
5. 不把 PowerShell 审计脚本接入产品运行时。

## 验收标准

1. `tools/source-tree-audit.ps1` 能继续输出到 `docs/reviews/simplify/source-tree-latest.md`。
2. 报告 Summary 至少包含：
   - Raw CMake source references
   - Declared CMake source references
   - Active CMake source references
   - Filtered generated utils references
3. `Serial Station Minimal UART Gap` 的 `In CMake` 使用 active refs 判断。
4. 报告能说明数字 utils 目录被过滤后不再代表主 GUI 目标编译面。
5. `tools/doctor.ps1` 继续通过。

## 失败条件

1. 审计脚本误写 `src/`、`CMakeLists.txt` 或构建目录。
2. 报告路径可逃逸出 `docs/reviews/simplify/`。
3. 活跃引用口径与当前 CMake 过滤规则不一致。
4. 输出报告不再包含旧 UART 证据或 Serial Station 缺口表。

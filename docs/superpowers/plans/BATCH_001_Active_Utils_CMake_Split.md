# BATCH Plan - Active Utils CMake Split

## 总目标

基于 `docs/reviews/simplify/source-tree-latest.md` 的 active utils 依赖审计，把“没有外部 include、非 canonical 基础能力、仍在主 GUI target 中”的 utils 目录拆成可审查的 CMake 收缩子任务。

目标是降低 `EmbedDebug` 主目标的编译面，缓解“上位机文件太多、编译很慢”的问题。

## 本轮不做

1. 不删除任何 `src/` 文件。
2. 不移动目录。
3. 不修改生产 C++ 行为。
4. 不改 `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` 或构建输出路径。
5. 不启动多个子 Agent 写代码，除非本方案经人工审查通过。
6. 不处理 canonical utils 目录：`checksum`、`converter`、`crypto`、`data`、`export`、`log`、`packet`、`perf`、`pipeline`、`settings`、`timestamp`。

## 共享约定

- 接口：不新增公共接口。
- 命名：所有过滤项以 `^src/utils/<dir>/` 形式写入 CMake regex 或明确注释。
- CMake：只允许修改根 `CMakeLists.txt` 中现有 `list(FILTER SOURCES HEADERS EXCLUDE REGEX ...)` 区域。
- 验证命令：
  - `cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64`
  - `cmake --build build --target EmbedDebug --parallel 4`
  - `ctest --test-dir build -R "Qt6CompatRegressions|SerialPortPanel|AsciiTextProtocol|SerialProtocolRegistry|SerialSession" --output-on-failure`
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1`
  - `EmbedDebug.bat` 启动验证
- 报告刷新：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md`

## 子任务

| # | 任务 | 可改文件 | 禁止文件 | 验收方式 | 并行组 |
|---|------|----------|----------|----------|--------|
| 1 | 过滤控制/调参类候选：`pid`、`simulator` | `CMakeLists.txt`、`docs/reviews/simplify/source-tree-latest.md`、`docs/tracking/SCORE_TRACKING.md` | `src/`、启动脚本 | 配置、主目标构建、相关 ctest、doctor、bat | A |
| 2 | 过滤设计器类候选：`filter_design`、`statemachine`、`wavegen` | 同上 | 同上 | 同上 | B |
| 3 | 过滤固件/设备工具候选：`firmware`、`gps`、`network` | 同上 | 同上 | 同上 | C |
| 4 | 过滤脚本/比较工具候选：`scripting`、`compare`、`hex_editor` | 同上 | 同上 | 同上 | A |
| 5 | 过滤速率/聚合候选：`rate`、`aggregator`、`bitmask` | 同上 | 同上 | 同上 | B |
| 6 | 过滤数据可视辅助候选：`gmm`、`hex_diff`、`packet_lib` | 同上 | 同上 | 同上 | C |
| 7 | 过滤模式/频谱候选：`pattern`、`spectrum`、`waveform` | 同上 | 同上 | 同上 | A |
| 8 | 过滤小型无外部 include 候选：`align`、`annotation`、`compress`、`data_inspector` | 同上 | 同上 | 同上 | B |
| 9 | 过滤编解码候选：`decoder`、`encoder`、`frequency`、`fuzzer` | 同上 | 同上 | 同上 | C |
| 10 | 过滤低层实验候选：`loss`、`protocol_timer`、`recorder`、`signal_gen` | 同上 | 同上 | 同上 | A |
| 11 | 过滤模板/校验候选：`splitter`、`template_lib`、`validator` | 同上 | 同上 | 同上 | B |
| 12 | 刷新审计和最终合流检查 | `docs/reviews/simplify/source-tree-latest.md`、`docs/tracking/SCORE_TRACKING.md` | `src/`、`CMakeLists.txt` | active refs 下降、doctor、bat | 串行 |

## 合流顺序

1. 先串行执行 1 个子任务作为探针，确认过滤规则不会误伤构建。
2. 如果探针通过，再按人工审查结果决定是否启用 3 个子 Agent 并行。
3. 并行组只能修改同一 CMake 过滤区域中的各自目录项，不允许改其它 CMake 逻辑。
4. 所有子任务完成后由主 Agent 串行刷新报告、运行构建、测试、doctor 和 `EmbedDebug.bat`。

## 冲突处理

- 同文件冲突：停止并行，主 Agent 串行合并 CMake 过滤项。
- 构建失败：进入 LOOP Debug，先移除最近一组过滤项并复测。
- 启动失败：停止瘦身，优先恢复 `EmbedDebug.bat` 启动底线。
- 报告异常：进入 LOOP Simplify，修正 `source-tree-audit.ps1` 口径后再继续。

## 人工审查点

本方案尚未授权并行开发。进入 3 个或 6 个子 Agent 前，需要人工确认：

1. 是否接受第一批候选目录作为 CMake 主目标过滤对象。
2. 是否允许每个子任务只改 CMake 过滤项和审计报告。
3. 是否接受“先过滤出主目标，不删除源码”的策略。

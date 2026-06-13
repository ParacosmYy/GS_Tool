# PRD-108 - CMake Source List Dedup

## 背景

PRD-107 恢复 CMake 清单审计后，`tools/project-audit/project_audit.py --limit 5` 显示当前源码清单仍有 14 个重复路径，集中在：

- `src/chart/eye/*`
- `src/chart/math/*`

这些重复来自早期 chart 段和后续 F75/F76 功能段的重复枚举。虽然当前构建可以通过，但重复清单会增加审计噪声、合流风险和后续 CMake 修改成本。

## 目标

1. 删除 `cmake/EmbedDebugSources.cmake` 中重复的 chart eye/math 清单项。
2. 保留每个源文件和头文件至少一个 CMake 引用。
3. 不修改任何生产 `.h/.cpp`。
4. 让 `project_audit.py` 的 `duplicate entries` 降为 0。

## 非目标

1. 不重排整个 CMake 清单。
2. 不改变源文件内容。
3. 不移动、删除源码。
4. 不处理 generated utils 目录。
5. 不提升用户状态或设备验证状态。

## 验收标准

1. `python tools/project-audit/project_audit.py --limit 5` 显示 `duplicate entries: 0`。
2. `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 配置通过。
3. `cmake --build build --target EmbedDebug --parallel 4` 构建通过。
4. `docs/reviews/simplify/source-tree-latest.md` 刷新后仍显示 active CMake 引用和 Serial Station 引用。
5. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以审计、配置和构建通过为证据。
- 用户状态：不提升，产品行为无变化。
- 设备状态：不提升，真实硬件未验证。

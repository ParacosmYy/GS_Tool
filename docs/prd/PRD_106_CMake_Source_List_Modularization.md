# PRD-106 - CMake Source List Modularization

## 背景

`CMakeLists.txt` 已承担项目配置、Qt 依赖、主目标源码清单、资源、翻译和测试入口等多种职责。当前文件超过 6000 行，其中绝大多数是 `SOURCES` 与 `HEADERS` 的显式枚举，导致构建规则审查、冲突合流和后续 CMake 修改成本过高。

PRD-078 已把数字后缀 `src/utils/<name><number>/` 草稿目录从主 GUI 目标过滤出去，但根 CMake 文件仍然过长。本轮继续做可维护性瘦身，不改变主目标包含的源码集合。

## 目标

1. 新增 `cmake/EmbedDebugSources.cmake`，集中承接现有 `SOURCES` 与 `HEADERS` 清单。
2. 根 `CMakeLists.txt` 通过 `include(cmake/EmbedDebugSources.cmake)` 加载源码清单。
3. 保留现有数字 utils 目录过滤规则；过滤规则随源码清单一起进入 `cmake/EmbedDebugSources.cmake`。
4. 根 `CMakeLists.txt` 保留资源、翻译、测试入口和 target 配置。
5. 降低根 CMake 合流风险，让后续构建线修改集中在更小文件内完成。

## 非目标

1. 不删除或移动 `src/` 下源码。
2. 不改变主 GUI 目标的源码集合。
3. 不重写测试 CMake。
4. 不改变 UI、Serial Station 业务行为或启动脚本。
5. 不提升用户状态或设备验证状态。

## 约束

- 构建目录仍然只能是 `build/`。
- 新增 CMake 文件只放构建清单，不承载业务逻辑。
- `EmbedDebug.bat` 启动链路必须保持 `build/EmbedDebug.exe`。
- 用户已有改动不得回退。

## 验收标准

1. `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64` 配置通过。
2. `cmake --build build --target EmbedDebug --parallel 4` 构建通过，或失败时能证明失败不是本轮 CMake 拆分引入。
3. 根 `CMakeLists.txt` 不再直接承载数千行 `SOURCES/HEADERS` 清单。
4. 不创建第二构建目录。
5. 三轴状态：工程状态目标 `E3 -> E4`，用户状态不提升，设备状态不提升。

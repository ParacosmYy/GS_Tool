# PRD-106 - CMake Source List Modularization

## 1. 标题

`PRD-106 - CMake Source List Modularization`

## 2. 目标

- 将根 `CMakeLists.txt` 中的 `SOURCES` 与 `HEADERS` 显式清单拆到 `cmake/EmbedDebugSources.cmake`。
- 根 CMake 继续负责项目配置、Qt 依赖、目标、资源、翻译和测试入口；源码清单模块负责 `SOURCES/HEADERS` 和数字 utils 目录过滤规则。
- 用户可观察变化：无产品行为变化，仅工程结构更易维护。
- 本轮三轴目标：工程 `E3 -> E4`，用户 `U0 -> U0`，设备 `D0 -> D0`。
- 证明命令：CMake 配置和主目标构建。

## 3. 非目标

- 不移动、删除或重命名 `src/` 源码。
- 不改变 Serial Station、UI、协议、连接、启动脚本行为。
- 不修改 `tests/CMakeLists.txt`。
- 不提升用户状态或设备状态，真实硬件仍未验证。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`docs/constraints/02-workflow.md`、`docs/constraints/03-architecture.md`、`docs/constraints/04-coding-standard.md`、`docs/constraints/07-directory-structure.md`。
- 构建目录只能是 `build/`。
- 用户已有改动不得回退。
- 本轮不新增 `.h/.cpp`。

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `CMakeLists.txt` | 修改 |
| 主要文件 | `cmake/EmbedDebugSources.cmake` | 新增 |
| 文档 | `docs/prd/PRD_106_CMake_Source_List_Modularization.md` | 新增 |
| 文档 | `docs/superpowers/specs/PRD_106_CMake_Source_List_Modularization_Specs.md` | 新增 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 6. 验收标准

- [ ] 根 `CMakeLists.txt` 通过 `include(cmake/EmbedDebugSources.cmake)` 加载源码清单。
- [ ] `cmake/EmbedDebugSources.cmake` 定义 `SOURCES` 与 `HEADERS`。
- [ ] CMake 配置通过：`cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64`。
- [ ] 主目标构建通过：`cmake --build build --target EmbedDebug --parallel 4`。
- [ ] 没有新增第二构建目录。
- [ ] 本轮结束有 commit；无法 commit 时写明具体阻塞。

## 7. 失败条件

- CMake 无法找到拆出的源码清单。
- 拆分后源码集合丢失导致新增编译错误。
- 需要修改 `src/` 或启动脚本才能完成本轮目标。
- 验证不能区分 CMake 拆分错误和既有构建错误。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64",
    "cmake --build build --target EmbedDebug --parallel 4"
  ],
  "check": [
    "git status --short",
    "Select-String -Path CMakeLists.txt -Pattern \"EmbedDebugSources.cmake\""
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 共享文件锁：`CMakeLists.txt`。
- 子任务是否文件互不重叠：否，本轮串行。

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|----------|------|----------|
| Qt/CMake/Ninja 环境异常 | Doctor | 命令输出摘要 |
| 拆分后 CMake 配置失败 | Debug | 失败行号和修复补丁 |
| 文件仍过大或清单重复 | Simplify | 下一轮瘦身建议 |

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E3 -> E4`，CMake 配置和主目标构建通过 |
| 用户状态 | 不提升，产品行为无变化 |
| 设备状态 | 不提升，真实硬件未验证 |
| 验证命令 | `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64`；`cmake --build build --target EmbedDebug --parallel 4`；`.\EmbedDebug.bat` 启动探针 |
| commit | 随本轮 `CMake: 拆分源码清单` 提交记录 |

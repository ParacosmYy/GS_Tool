# PRD-117 - Workstation Quick Launch

## 1. 标题

`PRD-117 - Workstation Quick Launch`

## 2. 目标

- 提供企业级工站快速打开入口。
- 支持 `--panel serial.station` 和 `--station serial` 启动后直达串口工站。
- 保持 `EmbedDebug.bat` 双击默认行为不变，同时透传命令行参数。
- 本轮三轴目标：工程 `E3 -> E4`，用户 `U2 -> U3`，设备不提升。

## 3. 非目标

- 不新增串口协议或设备 IO 能力。
- 不修改 Serial Station 内部 UI/core/protocol/service 行为。
- 不重做导航树或命令面板视觉。
- 不处理构建清单瘦身。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`02-workflow.md`、`03-architecture.md`、`04-coding-standard.md`、`05-ui-standard.md`、`07-directory-structure.md`。
- 构建目录只能是 `build/`。
- `MainWindow` 只转发到 `NavigationController`。
- 启动脚本只能透传参数，不改变构建/启动目录。

## 5. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| PRD | `docs/prd/PRD_117_Workstation_Quick_Launch.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_117_Workstation_Quick_Launch_Specs.md` | 新增 |
| 启动入口 | `src/main.cpp` | 解析 `--panel` / `--station` |
| 主窗口 | `src/core/mainwindow/MainWindow.h/.cpp分片` | 暴露按 panel id 打开方法 |
| 导航 | `src/core/navigation/NavigationController.h/.cpp` | 增加按 id 恢复/切换能力 |
| 启动脚本 | `EmbedDebug.bat` 或 `tools/launch_embeddebug.ps1` | 参数透传 |
| 测试 | `tests/test_navigation_controller_category.cpp`, `tests/test_startup_options.cpp`, `tests/CMakeLists.txt` | 增加 QTest 覆盖 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 更新 |

## 6. 设计

启动参数解析规则：

- `--panel <panelId>`：直接使用稳定 panel id，例如 `serial.station`。
- `--station serial`：用户友好别名，映射到 `serial.station`。
- 未提供参数：保持现有会话恢复行为。
- 参数无效：不崩溃，不阻断启动，保持现有默认/会话恢复面板。

导航复用规则：

- `NavigationController::restorePanelById()` 负责按 `NavPanelMapping::id` 查找并显示面板。
- `MainWindow::openPanelById()` 只调用导航控制器，不复制映射表。
- 后续命令面板、快捷方式、脚本启动都复用同一个 panel id 入口。

## 7. 验收标准

- [x] 新增 QTest 先红后绿，覆盖按 id 恢复和未知 id 失败。
- [x] `cmake --build build --target test_navigation_controller_category --parallel 4` 通过。
- [x] `cmake --build build --target test_startup_options --parallel 4` 通过。
- [x] `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 通过。
- [x] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [x] `.\EmbedDebug.bat --station serial` 启动探针通过。
- [x] `.\EmbedDebug.bat` 默认启动探针通过。
- [ ] 根目录只存在 `build/`。
- [ ] 本轮 commit。

## 8. 失败条件

- 双击 `EmbedDebug.bat` 默认启动行为被破坏。
- 参数导致启动崩溃或阻断主窗口显示。
- `MainWindow` 写入面板查找业务逻辑或复制 panel id 映射。
- 新增第二构建目录。

## 9. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E4`，导航恢复和启动参数解析 QTest 通过，主目标构建通过 |
| 用户状态 | `U3`，用户可通过 `EmbedDebug.bat --station serial` 直达串口工站 |
| 设备状态 | 不提升，真实硬件未验证 |
| 验证命令 | `test_navigation_controller_category` 11/11 passed；`test_startup_options` 9/9 passed；`EmbedDebug` 构建 passed；带参和无参 bat 探针 exit=0 |
| commit | 待处理 |

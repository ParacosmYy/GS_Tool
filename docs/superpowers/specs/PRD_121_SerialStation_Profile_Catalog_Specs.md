# PRD-121 - Serial Station Profile Catalog Specs

## 1. 目标

- 增加 Serial Station 最近档案/上次档案索引能力。
- 让用户在工站工具栏中快速恢复最近配置。
- 不改变档案 JSON schema，不自动连接串口。

## 2. 服务契约

| API | 行为 |
|-----|------|
| `recordProfilePath(path)` | 成功保存/加载/启动应用档案后记录路径，去重后置顶 |
| `recentProfilePaths()` | 返回 MRU 顺序路径列表，最多 8 条 |
| `lastProfilePath()` | 返回最近一次成功记录的路径 |
| `clear()` | 清理 catalog，用于测试和用户未来设置入口 |

## 3. 调用链

```text
SerialStationWindow::saveCurrentProfileToFile()
  -> SerialStationController::saveProfileToFile()
  -> SerialProfileService::saveToFile()
  -> SerialProfileCatalogService::recordProfilePath()
  -> SettingsManager

SerialStationWindow::loadProfileFromFile()
  -> SerialStationController::loadProfileFromFile()
  -> SerialProfileService::loadFromFile()
  -> SerialProfileCatalogService::recordProfilePath()
  -> refreshProfileCatalogUi()

SerialStationWindow::reloadLastProfile()
  -> SerialProfileCatalogService::lastProfilePath()
  -> SerialStationWindow::loadProfileFromFile(path)
```

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| PRD | `docs/prd/PRD_121_SerialStation_Profile_Catalog.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_121_SerialStation_Profile_Catalog_Specs.md` | 新增 |
| service | `src/apps/serial_station/services/SerialProfileCatalogService.*` | 新增 |
| UI/app | `src/apps/serial_station/SerialStationWindow.*` | 工具栏和调用链 |
| CMake | `cmake/EmbedDebugSources.cmake`, `tests/CMakeLists.txt` | 注册源码和测试 |
| tests | `tests/serial_station/test_serial_profile_catalog_service.cpp`, `tests/serial_station/test_serial_station_workbench.cpp` | 红绿回归 |
| docs | `README.md`, `docs/tracking/SCORE_TRACKING.md` | 同步入口和评分 |

## 5. 验收清单

- [x] 红灯测试先失败，失败原因是 catalog service / window 最近档案 API 尚不存在。
- [x] `test_serial_profile_catalog_service` 构建和运行通过。
- [x] `test_serial_station_workbench` 构建和运行通过。
- [x] `EmbedDebug` 主目标构建通过。
- [x] `EmbedDebug.bat --station serial` 启动探针通过。
- [x] Serial Station include 边界检查通过。
- [x] README 已同步最近/上次档案入口。
- [x] 本轮变更纳入单一 commit，并在收口阶段 push。

## 6. 失败条件

- UI 直接读写 `SettingsManager`。
- catalog service 解析或写入 `.edserialprofile` JSON。
- 加载失败仍写入最近档案。
- 自动连接串口。
- README 暗示真实设备已验证。

## 7. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | E5：catalog service 与 workbench 最近档案路径均有 QTest 覆盖 |
| 用户状态 | U4：工作台可通过最近档案下拉和“重载上次”恢复配置 |
| 设备状态 | 不提升 |
| 验证命令 | `cmake --build build --target test_serial_profile_catalog_service test_serial_station_workbench --parallel 4`; `.\build\tests\test_serial_profile_catalog_service.exe`; `.\build\tests\test_serial_station_workbench.exe`; `cmake --build build --target EmbedDebug --parallel 4`; `.\EmbedDebug.bat --station serial`; `cmake --build build --target test_serial_station_replay_flow --parallel 4`; `.\build\tests\test_serial_station_replay_flow.exe` |
| commit | 本轮收口提交 |

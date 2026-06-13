# PRD-122 Serial Station 最近档案目录管理规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| services | `SerialProfileCatalogService.*` | 增加路径查询和单项移除 |
| app/ui 装配 | `SerialStationWindow.*` | 增加清空最近档案入口和状态刷新 |
| tests | `test_serial_profile_catalog_service.cpp` | 覆盖查询、移除、持久化和幂等 |
| tests | `test_serial_station_workbench.cpp` | 覆盖按钮、日志、跨窗口持久化 |
| docs | README、评分追踪 | 同步企业级中文说明与分数 |

## 服务层行为

1. `containsProfilePath(path)` 对输入路径做 trim 和 `QDir::cleanPath`，空路径返回 false。
2. `removeProfilePath(path)` 只移除最近档案索引中的匹配路径，不删除文件。
3. 移除最后使用路径后，若还有最近档案，则 last 回退到最新一条；否则清空 last。
4. `clear()` 继续清空整个 `serial_station/profiles` 分组。

## UI 行为

1. 新增按钮 objectName：`serialProfileClearRecentButton`。
2. 用户可见文字使用 `tr("清空最近")`。
3. 无最近档案时，下拉、重载、清空按钮均禁用。
4. 清空成功后写入系统日志：“最近配置档案已清空”。
5. 失败或空列表清空不弹窗，不阻塞工作流。

## 调用链

```text
QPushButton::clicked
  -> SerialStationWindow::clearRecentProfiles()
  -> SerialProfileCatalogService::clear()
  -> SettingsManager
  -> refreshProfileCatalogUi()
  -> SerialLogPanel::appendSystem()
```

## 验证命令

```powershell
cmake --build build --target test_serial_profile_catalog_service test_serial_station_workbench --parallel 4
.\build\tests\test_serial_profile_catalog_service.exe
.\build\tests\test_serial_station_workbench.exe
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial
```

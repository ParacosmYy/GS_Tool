# PRD-124 Serial Station 失效档案自动清理规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| services | `SerialProfileCatalogService.*` | 增加失效路径清理和上次档案回退 |
| app/ui 装配 | `SerialStationWindow.*` | 增加清理入口、系统日志和启动/重载回退 |
| tests | `test_serial_profile_catalog_service.cpp` | 覆盖清理数量、顺序、持久化、last 回退 |
| tests | `test_serial_station_workbench.cpp` | 覆盖 UI 控件、点击清理、启动/重载回退 |
| docs | README、评分追踪 | 同步企业版中文说明和分数 |

## 行为规则

1. `pruneMissingProfilePaths()` 只检查最近档案索引中的路径是否存在。
2. 不存在的路径从最近档案索引移除，存在的路径保持原顺序。
3. 当前 last 不存在、为空或不在剩余最近列表内时，回退到剩余列表第一项。
4. 剩余列表为空时清空 last。
5. `SerialStationWindow::pruneMissingProfiles()` 刷新 UI，并记录“已清理失效配置档案: N”或“没有失效配置档案”。
6. `loadStartupLastProfile()` 和 `reloadLastProfile()` 在读取 last 前先执行清理，支持自动回退。

## 验证命令

```powershell
cmake --build build --target test_serial_profile_catalog_service test_serial_station_workbench --parallel 4
.\build\tests\test_serial_profile_catalog_service.exe
.\build\tests\test_serial_station_workbench.exe
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial --last-profile
```

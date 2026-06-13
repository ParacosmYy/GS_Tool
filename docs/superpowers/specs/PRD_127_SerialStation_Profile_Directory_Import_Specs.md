# PRD-127 Serial Station 默认档案目录导入规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| services | `SerialProfileCatalogService.*` | 增加目录扫描和批量导入路径索引 API |
| app/ui 装配 | `SerialStationWindow.h/.cpp` | 增加导入目录按钮、公开测试入口和日志反馈 |
| tests | `test_serial_profile_catalog_service.cpp` | 覆盖扫描、排序、过滤、批量导入、去重和容量上限 |
| tests | `test_serial_station_workbench.cpp` | 覆盖按钮入口、导入数量、UI 刷新、日志和状态不变 |
| docs | README、评分追踪 | 同步中文企业级能力和分数 |

## 服务 API

```cpp
QStringList discoverProfilePaths(const QString& directoryPath) const;
int importProfileDirectory(const QString& directoryPath);
```

### `discoverProfilePaths`

1. 输入路径 trim 后用 `QDir::cleanPath()` 标准化。
2. 空路径、目录不存在或不是目录时返回空列表。
3. 仅扫描当前目录，不递归。
4. 匹配扩展名：`*.edserialprofile`、`*.json`。
5. 返回绝对路径，按文件名升序排序。
6. 不读取文件内容，不修改最近档案索引。

### `importProfileDirectory`

1. 调用 `discoverProfilePaths()` 获取候选路径。
2. 按发现顺序逐个 `recordProfilePath()`。
3. 返回本次实际新纳入最近索引的数量；已存在路径不重复计数。
4. 保持最近档案容量上限和最近优先排序。
5. 不修改默认档案目录。

## 窗口语义

1. 顶部工具栏新增按钮文案 `导入目录`，objectName 为 `serialProfileImportDirectoryButton`。
2. `SerialStationWindow::importProfilesFromDefaultDirectory()` 调用 catalog 导入当前 `defaultProfileDirectory()`。
3. 返回导入数量，并刷新最近档案 UI。
4. 导入数量为 0 时写系统日志 `未发现可导入配置档案`。
5. 导入数量大于 0 时写系统日志 `已导入配置档案: N`。
6. 导入不调用 `loadProfileFromFile()`，不改变当前面板配置。

## 验证命令

```powershell
cmake --build build --target test_serial_profile_catalog_service test_serial_station_workbench EmbedDebug --parallel 4
.\build\tests\test_serial_profile_catalog_service.exe
.\build\tests\test_serial_station_workbench.exe
.\EmbedDebug.bat --station serial --profile-dir .\profiles
```

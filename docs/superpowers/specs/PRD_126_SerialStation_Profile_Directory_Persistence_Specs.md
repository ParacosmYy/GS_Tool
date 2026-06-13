# PRD-126 Serial Station 默认档案目录持久化规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| services | `SerialProfileCatalogService.*` | 在最近/上次档案索引旁新增默认目录持久化键 |
| app/ui 装配 | `SerialStationWindow.cpp` | 构造时恢复默认目录 |
| app/ui 档案流 | `SerialStationWindowProfiles.cpp` | 设置、保存、加载时同步默认目录 |
| tests | `test_serial_profile_catalog_service.cpp` | 覆盖服务层目录持久化、清理和互不干扰 |
| tests | `test_serial_station_workbench.cpp` | 覆盖窗口恢复、保存/加载更新和清除持久化 |
| docs | README、评分追踪 | 同步中文企业级说明和分数 |

## 服务语义

1. 配置组继续使用 `serial_station/profiles`。
2. 新增键 `defaultDirectory`。
3. `defaultProfileDirectory()` 返回标准化后的存储目录；无值时返回空字符串。
4. `setDefaultProfileDirectory(<nonblank>)` trim 后用 `QDir::cleanPath()` 标准化，写入配置并 `sync()`，返回 true。
5. `setDefaultProfileDirectory(<blank>)` 返回 false，不改变已有配置。
6. `clearDefaultProfileDirectory()` 只删除 `defaultDirectory` 并 `sync()`。
7. `clear()` 继续删除整个组，因此会同时移除最近档案、上次档案和默认目录。

## 窗口语义

1. `SerialStationWindow` 创建 `SerialProfileCatalogService` 后，读取 `defaultProfileDirectory()` 到 `m_defaultProfileDirectory`。
2. `SerialStationWindow::setDefaultProfileDirectory(<nonblank>)` 写入服务并刷新成员状态。
3. `SerialStationWindow::setDefaultProfileDirectory(<blank>)` 调用 `clearDefaultProfileDirectory()`，成员置空，`defaultProfileDirectory()` 回退到系统文档目录或 home。
4. `saveCurrentProfileToFile()` 成功后以实际写入文件所在目录更新默认目录。
5. `loadProfileFromFile()` 成功后以加载文件所在目录更新默认目录。
6. 最近档案 UI 刷新仍只由最近档案索引驱动；默认目录变更不应清空最近档案。

## 调用链

```text
启动参数/保存/加载/用户设置
  -> SerialStationWindow::setDefaultProfileDirectory()
  -> SerialProfileCatalogService::setDefaultProfileDirectory()
  -> SettingsManager serial_station/profiles/defaultDirectory
```

```text
SerialStationWindow()
  -> SerialProfileCatalogService::defaultProfileDirectory()
  -> m_defaultProfileDirectory
  -> save/load QFileDialog 默认目录
```

## 验证命令

```powershell
cmake --build build --target test_serial_profile_catalog_service test_serial_station_workbench test_startup_options EmbedDebug --parallel 4
.\build\tests\test_serial_profile_catalog_service.exe
.\build\tests\test_serial_station_workbench.exe
.\build\tests\test_startup_options.exe
.\EmbedDebug.bat --station serial --profile-dir .\profiles
```

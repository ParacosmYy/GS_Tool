# PRD-123 Serial Station 上次档案快捷启动规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| startup | `StartupOptions.*` | 增加 `loadLastProfile()` 解析结果 |
| app | `MainWindowLifecycle.cpp` | 启动路由无显式 profile 时加载上次档案 |
| app/ui 装配 | `SerialStationWindow.*` | 增加 `loadStartupLastProfile()` |
| tests | `test_startup_options.cpp` | 覆盖 CLI 解析、优先级、布尔形式 |
| tests | `test_serial_station_workbench.cpp` | 覆盖工作台从上次档案启动加载 |
| docs | README、评分追踪 | 同步启动说明和分数 |

## 启动参数语义

1. `--last-profile`：启用上次 Serial Station 档案加载。
2. `--serial-last-profile`：同义参数，便于脚本可读。
3. `--last-profile=true|1|yes|on`：启用。
4. `--last-profile=false|0|no|off`：不启用。
5. `--profile` / `--serial-profile` 显式路径优先于上次档案。
6. 如果没有显式 panel，但启用了上次档案，则 `panelId` 默认为 `serial.station`。

## 调用链

```text
QCoreApplication::arguments()
  -> StartupOptions::fromArguments()
  -> MainWindow::applyStartupOptions()
  -> PanelManager::serialStationWindow()
  -> SerialStationWindow::loadStartupLastProfile()
  -> SerialProfileCatalogService::lastProfilePath()
  -> SerialStationWindow::loadStartupProfile(path)
```

## 验证命令

```powershell
cmake --build build --target test_startup_options test_serial_station_workbench --parallel 4
.\build\tests\test_startup_options.exe
.\build\tests\test_serial_station_workbench.exe
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial --last-profile
```

# PRD-125 Serial Station 档案目录快捷启动规格

## 修改范围

| 区域 | 文件 | 说明 |
|------|------|------|
| startup | `StartupOptions.*` | 增加 `profileDirectoryPath()` |
| app | `MainWindowLifecycle.cpp` | 把启动目录转发给 Serial Station |
| app/ui 装配 | `SerialStationWindow.*`、`SerialStationWindowProfiles.cpp` | 增加默认档案目录状态并拆分档案相关方法 |
| build | `cmake/EmbedDebugSources.cmake`、`tests/CMakeLists.txt` | 注册新增编译单元 |
| tests | `test_startup_options.cpp` | 覆盖目录参数解析和优先级 |
| tests | `test_serial_station_workbench.cpp` | 覆盖窗口目录设置和默认目录回退 |
| docs | README、评分追踪 | 同步启动说明和分数 |

## 启动参数语义

1. `--profile-dir <dir>`：设置 Serial Station 档案对话框默认目录。
2. `--serial-profile-dir <dir>`：同义参数。
3. `--profile-dir=<dir>` / `--serial-profile-dir=<dir>`：等号形式。
4. 空目录值忽略；flag 后遇到另一个 `--` 参数视为缺失。
5. 目录参数出现且未指定面板时，默认路由到 `serial.station`。
6. 显式 `--panel` 仍可覆盖面板路由，但目录只会在 Serial Station 面板存在时生效。

## 调用链

```text
QCoreApplication::arguments()
  -> StartupOptions::fromArguments()
  -> MainWindow::applyStartupOptions()
  -> PanelManager::serialStationWindow()
  -> SerialStationWindow::setDefaultProfileDirectory()
  -> SerialStationWindow::saveProfileWithDialog/loadProfileWithDialog
```

## 验证命令

```powershell
cmake --build build --target test_startup_options test_serial_station_workbench --parallel 4
.\build\tests\test_startup_options.exe
.\build\tests\test_serial_station_workbench.exe
cmake --build build --target EmbedDebug --parallel 4
.\EmbedDebug.bat --station serial --profile-dir .\profiles
```

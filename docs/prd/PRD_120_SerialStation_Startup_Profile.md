# PRD-120 - Serial Station Startup Profile

## 背景

PRD-117 已支持 `EmbedDebug.bat --station serial` 直达串口工站，PRD-119 已支持在工站内保存和加载 `.edserialprofile` 档案。真实工位仍缺少一个更快捷的入口：从桌面脚本或快捷方式直接打开指定设备档案，让 UART、协议和命令配置随启动应用。

## 目标

1. 启动参数支持 `--profile <path>` 和 `--profile=<path>`。
2. 启动参数支持更明确的 `--serial-profile <path>` 和 `--serial-profile=<path>`。
3. 提供 profile 参数时，即使没有显式 `--station serial`，也默认打开 Serial Station。
4. MainWindow 只做启动选项转发和面板装配，不解析 JSON、不写业务文件。
5. 加载结果进入 Serial Station 既有日志路径；失败不崩溃，仍打开工站供用户修正。

## 非目标

1. 不自动连接串口。
2. 不新增最近档案列表。
3. 不改变 `.edserialprofile` JSON schema。
4. 不提升真实硬件验证状态。

## 架构边界

- `StartupOptions` 只解析参数和值对象，不依赖 QWidget。
- `main.cpp` 只创建 `StartupOptions` 并交给 `MainWindow`。
- `MainWindow` 只打开面板并调用 `SerialStationWindow::loadProfileFromFile()`。
- `SerialStationWindow` 继续通过 Controller 和 `SerialProfileService` 完成档案读写。
- UI/core/protocols/services 边界不变。

## 验收标准

1. `StartupOptions` QTest 覆盖 `--profile` 和 `--serial-profile` 两种写法。
2. `StartupOptions` QTest 覆盖 profile 参数会默认路由到 `serial.station`。
3. `StartupOptions` QTest 覆盖缺失 profile 值不会错误路由。
4. `SerialStationWorkbenchTest` 覆盖启动档案加载 API 能应用档案并记录日志。
5. 主目标构建通过。
6. `EmbedDebug.bat --station serial --profile <临时/示例档案>` 启动探针通过。

## 三轴状态目标

- 工程状态：`E5` 保持，启动路由和工作台加载有自动化测试。
- 用户状态：Serial Station 配置档案启动路径 `U4`，用户可用快捷方式直接进入档案化工站。
- 设备状态：不提升，真实硬件未验证。

# PRD-117 - Workstation Quick Launch

## 背景

用户要求项目按企业级方式提升，重点是打开方便快捷、代码复用程度高、工站配置齐全。当前 Serial Station 已接入主窗口导航，但用户仍需要启动应用后手动展开导航并查找“串口工站”，缺少稳定的直达入口。

本轮先解决“打开方便快捷”：提供命令行和 bat 参数直达工站能力，复用现有导航映射，不新增第二套路由表。

## 目标

1. 支持 `EmbedDebug.exe --panel serial.station` 启动后直接打开串口工站。
2. 支持 `EmbedDebug.exe --station serial` 作为面向用户的短别名，映射到 `serial.station`。
3. `EmbedDebug.bat` 透传用户参数，例如 `EmbedDebug.bat --station serial`。
4. 启动目标面板不存在时不崩溃，保持会话恢复或默认面板逻辑。
5. 导航定位复用 `NavigationController` 的稳定 panel id，不复制面板查找逻辑。

## 非目标

1. 不新增 Serial Station 协议、串口 IO 或设备验证能力。
2. 不重做导航树 UI。
3. 不改变现有双击 `EmbedDebug.bat` 默认启动行为。
4. 不处理 generated utils 或 CMake 瘦身。

## 架构边界

- `main.cpp` 只解析启动参数并调用 `MainWindow` 公开的启动面板入口。
- `MainWindow` 只做装配转发，不写业务逻辑。
- `NavigationController` 作为唯一稳定 panel id 查找入口。
- `EmbedDebug.bat` 只透传参数，不引入第二构建目录或第二启动路径。

## 验收标准

1. `NavigationController` QTest 覆盖按 panel id 恢复目标面板、未知 id 返回失败。
2. `cmake --build build --target test_navigation_controller_category --parallel 4` 通过。
3. `cmake --build build --target test_startup_options --parallel 4` 通过。
4. `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 配置通过。
5. `cmake --build build --target EmbedDebug --parallel 4` 构建通过。
6. `.\EmbedDebug.bat --station serial` 启动探针通过。
7. `.\EmbedDebug.bat` 默认启动探针通过。
8. 不创建第二构建目录。

## 三轴状态

- 工程状态：`E3 -> E4`，以 QTest、配置、构建和启动探针为证据。
- 用户状态：`U2 -> U3`，用户可从启动入口直接进入串口工站。
- 设备状态：不提升，真实硬件未验证。

## 收口记录

- `NavigationControllerCategoryTest`: 11 passed, 0 failed。
- `StartupOptionsTest`: 9 passed, 0 failed。
- `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64`: passed。
- `cmake --build build --target EmbedDebug --parallel 4`: passed。
- `.\EmbedDebug.bat --station serial`: exit=0，启动进程后探针关闭。
- `.\EmbedDebug.bat`: exit=0，默认启动路径保持可用。

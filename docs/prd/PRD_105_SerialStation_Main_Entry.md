# PRD_105 - Serial Station 主入口收敛

## 背景

用户反馈当前看不到 UART 开发成果。仓库已经存在 `src/apps/serial_station/`，包含 UART 配置、发送、接收、日志、导出、回放和协议选择能力，但主窗口导航仍主要暴露旧 `SerialConfigPanel`，导致新 Serial Station 对用户不可达或不明显。

## 目标

1. 把 `SerialStationWindow` 接入主窗口导航，作为明确的串口工站入口。
2. 用户从主界面进入后能看到 UART 配置、协议选择、命令发送、日志和状态栏。
3. `PanelManager` 只负责创建、包装、注册和导航映射，不写 UART 业务逻辑。

## 非目标

1. 不新增协议。
2. 不改真实串口收发底层行为。
3. 不迁移或删除旧 `src/serial/` 面板。
4. 不声明真实设备已验证。

## 三轴目标

- 工程状态：维持或达到 `E4`，以主目标构建和已有 Serial Station QTest 作为证据。
- 用户状态：从 `U2局部可用` 提升到 `U3主流程可用` 的入口前置条件，即主程序可达。
- 设备验证：保持 `D1纯单测`，真实设备未验证。

## 验收

1. 导航映射包含稳定 ID `serial.station`。
2. `serial.station` 面板指向 `SerialStationWindow` 的 BasePanel 包装器。
3. `SerialStationWindow` objectName 为 `serialStationWindow`。
4. `cmake --build build --target EmbedDebug --parallel 4` 通过。
5. `tools/verify_embeddebug_launch.ps1` 通过。


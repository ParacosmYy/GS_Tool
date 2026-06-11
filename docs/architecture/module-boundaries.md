# 模块边界

## 读法

- `shared/` 只放稳定事实。
- `interfaces/` 只放契约。
- `core/` 只做协调和基础 UI。
- 业务能力落在各自领域目录，不向上回流到 `core/`。

## 常见归属

- 连接创建、参数预设、连接工厂: `src/connection/` + `src/core/connect/`
- 协议解析、协议桥、协议编辑: `src/protocol/`
- 新串口上位机工站: `src/apps/serial_station/`
- 终端显示、搜索、过滤: `src/terminal/`
- 波形、缩放、FFT、统计图: `src/chart/`
- OTA、触发器、插件、仪表盘: 各自领域目录

## 判定原则

1. 能否独立理解和测试。
2. 是否会被多个模块重复引用。
3. 是否属于运行时协调，还是纯数据/契约。
4. 是否应该先进入 `src/features/` 作为迁移骨架。

## Serial Station 边界

串口上位机后续重构不继续扩大旧 `src/serial/` + `src/protocol/` + `MainWindow` 的耦合点，优先落到 `src/apps/serial_station/`。

- UI 面板只发 Qt signal，不直接操作串口或具体协议。
- `SerialStationController` 是 UI 与业务之间的协调入口。
- `core/` 只管 `QByteArray` 收发、会话、dispatcher 和 codec。
- `protocols/` 只管命令构建和流式解析。
- `services/` 只管日志、导出、回放和设备档案。
- 完整规则见 `docs/serial_station_architecture.md`。

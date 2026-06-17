# 模块边界

## 读法

- `python/embeddebug/shared/` 只放稳定事实、值对象和轻量工具。
- `python/embeddebug/interfaces/` 只放契约。
- `python/embeddebug/app/` 只做应用装配、导航、主题和生命周期协调。
- 业务能力落在各自领域目录，不向上回流到 `app/`。

## 常见归属

- 应用入口、窗口装配、全局主题: `python/embeddebug/app/`
- 串口上位机工站: `python/embeddebug/serial_station/`
- 连接创建、参数预设、连接工厂: `python/embeddebug/serial_station/drivers/`
- 协议解析、协议桥、协议编辑: `python/embeddebug/serial_station/protocols/`
- 终端显示、搜索、过滤: `python/embeddebug/serial_station/ui/terminal/`
- 波形、缩放、FFT、统计图: `python/embeddebug/serial_station/ui/plotting/`
- 日志、导出、回放、设备档案: `python/embeddebug/serial_station/services/`
- 启动、打包、体检脚本: `tools/`、`EmbedDebug.bat`、`pyproject.toml`

## 判定原则

1. 能否独立理解和测试。
2. 是否会被多个模块重复引用。
3. 是否属于运行时协调，还是纯数据/契约。
4. 是否应该先进入 `python/embeddebug/features/` 作为迁移骨架。

## Serial Station 边界

串口上位机后续重构不恢复旧 C++/CMake 工程树，优先落到 `python/embeddebug/serial_station/`。

- UI 面板只发 PyQt signal 或 controller intent，不直接操作串口或具体协议。
- `SerialStationController` 是 UI 与业务之间的协调入口。
- `core/` 只管 bytes 收发、会话、dispatcher 和 codec。
- `protocols/` 只管命令构建和流式解析。
- `services/` 只管日志、导出、回放和设备档案。
- 完整规则见 `docs/serial_station_architecture.md`。

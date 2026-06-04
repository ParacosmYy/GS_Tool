# EmbedDebug 评分追踪

> 分支: `feat/embed-debug` | 远程: `https://github.com/ParacosmYy/GS_Tool.git`
> 起点: 1分 | 目标: 1000分 | 每次 commit 默认记 1 分

---

## 一、评分历史

> 这里只保留可读的历史摘要和关键节点。完整逐 commit 记录以 Git 历史为准。

| 阶段 | 代表内容 | 分数 |
|------|----------|------|
| 0 | 项目初始状态 | 1 |
| 1-20 | 工厂模式、协议解析、OTA、QSS 迁移、架构重构 | 22 |
| 21-40 | 主窗口拆分、控制器提取、面板重构、串口深度能力 | 42 |
| 41-60 | Bug 修复冲刺、波形/统计增强、驱动检测、主题完善 | 60 |
| 61-80 | Doxygen/tr() 收尾、文件拆分、动画与面板交互优化 | 80 |
| 81-100 | 终端/OTA/主题/协议持续重构与质量修复 | 100 |
| 101-120 | F1-F11 路线图能力落地、UI 现代化基础设施完善 | 120 |
| 121-140 | 统计增强、接口层扩展、弹窗统一、MQTT 修复 | 140 |
| 141-160 | 主题生成器、RTT/USB/CAN/Dashboard 扩展、Doxygen 收尾 | 160 |
| 161-170 | 文件体积约束修复(.h≤200/.cpp≤500)、PluginLoader编译修复、PanelManager拆分 | 170 |
| 161-170 | 文件体积持续拆分(DataLogger/ChartModel/PacketBuilder/CursorOverlay/FftWidget/MqttConnection) | 170 |
| 171-172 | SettingsManager/ScatterWidget/ProtocolView拆分+linter常量迁移修复 | 172 |
| 173-176 | BasePanelStates/WebSocketFrame/TerminalWidgetSlots/OtaManagerProgress拆分 | 176 |
| 177-190 | Tier3核心功能实现(BLE GATT/MQTT TopicModel/CAN DBC/USB) + Doxygen全量达标 + 持续文件拆分 | 190 |
| 191-200 | ProtocolEngine CRC验证+Dashboard持久化+BLE Scanner增强+ShortcutManager键盘系统+Lucide图标扩展 | 200 |
| 201 | 终端搜索增强(大小写/全词/历史)+OTA CRC32校验+串口芯片识别21厂商+协议桥自动检测+Protobuf嵌套解码 | 201 |

---

## 二、里程碑

| 分数 | 里程碑 | 说明 |
|------|--------|------|
| 16 | 复用与重构起步 | 基础设施、协议层和 OTA 体系逐步成型 |
| 50 | 方法长度与结构整顿 | 大量方法拆分，文件体积控制开始稳定 |
| 80 | UI 体验强化 | 动画、提示、面板和主题体验连续完善 |
| 100 | 核心能力阶段性收口 | 录制回放、协议、导出、仪表盘等进入稳定阶段 |
| 120 | 现代化 UI 和扩展框架 | 快捷键、响应式布局、插件/扩展骨架推进 |
| 140 | 工程化与可维护性增强 | 统计、接口层、弹窗和主题链路持续补强 |
| 160 | 扩展能力和文档收尾 | 主题生成、设备协议、RTT/USB/CAN 相关骨架推进 |
| 170 | 全量文件体积约束达标 | .h≤200/.cpp≤500全部清零，PluginLoader接口对齐 |
| 172 | 大文件预防性拆分 | SettingsManager/ScatterWidget/ProtocolView拆分至安全范围 |
| 176 | 第二轮大文件拆分完成 | BasePanel/WebSocket/TerminalWidget/OtaManager全量拆分 |
| 190 | Tier3核心功能实现 | BLE GATT/MQTT TopicModel/CAN DBC/USB全部从空方法变为功能实现 |
| 200 | 功能增强里程碑 | CRC验证/仪表盘持久化/快捷键系统/图标扩展等第二优先级完成 |
| 201 | 搜索/OTA/串口/协议深度增强 | 终端多规则过滤+OTA校验和+21厂商芯片库+协议桥自动检测+嵌套Protobuf |
| 202 | MqttTopicModelIndex/TerminalFilterApply/CanBusMonitorFrame拆分 | .cpp文件体积持续控制在400行以内 |
| 203 | SerialDetectorChip/TerminalSearchBuilders+WebSocket/Histogram/Scatter/Chart linter增强 | 芯片数据库和搜索构建器独立文件+帧统计/延迟追踪 |
| 204 | HistogramWidgetStats/ChartWidgetExport拆分+BLE/TCP/Recording增强 | 统计计算独立文件+TCP Nagle/KeepAlive+BLE通知缓存 |
| 205 | ModbusScanWidgetStats/ProtobufDecoderEncoding拆分+BLE/Recording/Settings统计增强 | Modbus扫描统计独立+Protobuf编解码独立+BLE连接/扫描周期+录制停止/标记+设置变更追踪 |
| 206 | ClipboardManager/DataAggregator/ConnectionPool/TcpConnection/UdpConnection/PortWatcher统计增强 | 剪贴板推送/写入/固定计数+聚合器输入/窗口/滚动计数+连接池借还/驱逐+TCP DNS/KeepAlive+UDP广播/错误+端口扫描错误 |
| 207 | DashboardModel/Widget/Serializer+ShortcutManager+BasePanel+PanelManager+ChartWidget/FftWidget/CursorOverlay/ScatterWidget/ChartExporter统计增强 | 布局变更/组件增删/序列化/面板生命周期/快捷键注册重绑定冲突/波形采样/FFT峰值/游标拖拽/散点图/图表导出 |
| 208 | ChartWidgetRender/ProtocolBridgeAutoDetect拆分+FFT/CursorOverlay/Scatter/Dashboard/PanelManager linter增强 | 波形渲染独立文件+协议桥自动检测独立+各模块统计增强 |
| 209 | ChartExporterFormats/BleGattModelQuery拆分+PanelManager统计增强 | 图表导出格式独立+BLE GATT查询独立+面板管理器统计 |
| 210 | YModemTransferProtocol/PortWatcherStats拆分 | YModem协议独立文件+端口监听统计独立 |
| 211 | MainWindowLifecycle/ResponsiveLayoutQuery拆分+TrafficMonitor/SignalLine/PerformanceOverlay/TimedSender/DataStatistics linter增强 | 主窗口生命周期独立+响应式查询独立+流量/信号/性能/定时/数据统计增强 |
| 212 | TimedSender统计增强+TrafficMonitor头文件修正 | 定时发送/字节/错误统计+流量监控getter修正 |

---

## 三、重要状态

- 当前文档已整理到 `#160` 附近的阶段性记录
- 后续新增分数时，优先补“阶段摘要”和“里程碑”，不再恢复长篇流水账
- 如果需要精确到单次提交，请直接查 `git log` 或对应 commit message
- 本文件只负责评分追踪，不承载工作流、提交规则或架构约束正文

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
| 202-266 | 持续文件拆分+统计增强+Doxygen补全+WidgetFactory mutable修复 | 266 |

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
| 213 | TriggerEngine+ThemeManager+SendController+SessionManager统计增强 | 触发器评估/执行/错误+主题缓存命中/未命中+发送宏执行+会话创建/删除 |
| 214 | ZModemTransferProtocol/DataStatisticsDisplay拆分 | ZModem协议独立+数据统计显示独立 |
| 215 | ModbusMaster+ModbusSlave+CanConnection+ProtocolSchema统计增强 | Modbus主站请求/响应/超时/异常+从站处理/响应/错误+CAN帧错误+模式加载/验证/错误 |
| 216 | ThemeManagerColor/ProtocolSchema拆分+linter增强 | 主题管理颜色查询独立文件+模式定义拆分 |
| 217 | ChecksumPanel+PacketBuilderPanel+TimestampPanel统计增强 | 校验面板计算/算法/剪贴板+包构建器构建/发送/字段+时间戳分析/格式 |
| 218 | ChecksumPanelAlgo拆分+ConverterPanel/DragDropHelper/TimestampPanel统计增强 | 校验算法独立+转换器格式切换+拖放拒绝计数+时间戳解析统计 |
| 219 | MainWindowInit拆分+TriggerRule linter增强 | 主窗口构造函数拆分为MainWindowInit.cpp(250行)+触发器规则linter修正 |
| 220 | QssThemeGeneratorWidgets/UsbDeviceDetectorStats拆分+Widget统计增强 | QSS主题生成器组件独立+USB检测统计独立+ScopeWidget/AudioSpectrum/FrequencyCounter/CommandPalette2/ScriptRecorder2/ToastWidget2/DataDiffWidget2/EdDialog2统计 |
| 221 | Plugin/Scope/Audio/Freq/CommandPalette/Recorder/Toast/Diff统计增强+文件拆分 | PluginApi面板注册/通道/订阅+PluginManager扫描/发现+MainWindowPanelConnect+PanelManagerQuery拆分 |
| 222 | QssThemeGeneratorWidgets/UsbDeviceDetectorStats拆分+Widget统计增强 | QSS主题生成器组件独立+USB检测统计独立+DataAggregator压缩 |
| 223 | NotificationManager/WorkspaceManager/ConnectionMonitor统计增强+文件拆分 | 通知创建/确认/消除+工作区保存/加载/删除/切换+OtaWidgetDisplay+QuickCommandBarActions拆分 |
| 224 | QssThemeGeneratorWidgets/UsbDeviceDetectorStats拆分+PluginLoader2/SpiI2cBridge统计增强 | PluginLoader2加载/卸载/扫描+SpiI2cBridge模式切换/队列排空 |
| 225 | PluginLoader2/SpiI2cBridgeManager统计增强+IntelHex/OtaManager拆分修复 | IntelHex命名空间修复+OtaManagerVerify拆分 |
| 226 | TcpConnectionHelpers/ProtocolEngineParsing拆分+多模块统计增强+头文件压缩 | TCP辅助函数独立+协议引擎解析独立+AnimationUtility2/WidgetFactory等统计 |
| 227 | ProtocolBridgeManager.h压缩+DataPipeline/PluginLoader统计增强 | .h从271→136行+管线处理/字节/错误+插件加载统计 |
| 228 | WebSocketConnection/PortWatcher头文件压缩+DashboardWidgetSlots拆分 | .h分别从247→103/230→102行+仪表盘槽函数独立 |
| 229-231 | Linter自动拆分+多模块统计增强 | OtaHistoryModelIndex/ZoomControllerQuery/UsbConnectionDescriptors等拆分 |
| 232 | BLE/CAN/MQTT/UDP/Terminal/Recording/Playback统计增强+文件拆分 | 连接模块帧错误/退订/队列+终端布局/搜索/过滤统计 |
| 233 | ModbusSlaveResponse/FrameParserStats拆分+连接模块统计增强 | Modbus从站响应独立+帧解析器统计独立 |
| 234-235 | BackgroundWidgetPaint/FrameVisualEditorFields拆分+PacketBuilder/Timestamp统计增强 | 背景绘制独立+帧编辑器字段独立+包构建/时间戳统计 |
| 236-237 | CursorOverlayInteraction/MqttConnectionPublish/TerminalSearchBarHistory拆分+10个头文件压缩 | 游标交互独立+MQTT发布独立+搜索历史独立 |
| 238 | BleConnection/SerialDetector头文件压缩 | BLE连接/串口检测器Doxygen压缩 |
| 239 | CanConnectionFrame/DbcParserSignal/UsbLibraryLoaderQuery拆分+头文件压缩 | CAN帧独立+DBC信号独立+USB查询独立 |
| 240 | DashboardSerializerExport/ModbusMasterStats/JLinkSdkLoaderQuery拆分+多模块压缩 | Dashboard导出独立+Modbus统计独立+JLink查询独立+23个头文件压缩至<180行 |
| 241 | IconManager/DashboardSerializer/DataLogger/ChartExporter统计增强 | 图标缓存命中+序列化计数+日志写入计数+图表导出计数 |
| 242 | ConnectionControllerQuery/ProtocolViewDisplay/ScatterWidgetCompute拆分+Doxygen增强 | 连接控制器查询独立+协议视图显示独立+散点图计算独立 |
| 243 | BleGattBrowserDisplay/PluginManagerScan拆分+Doxygen全量达标 | BLE浏览器显示独立+插件管理器扫描独立 |
| 244-245 | 全模块tr()国际化合规审计+SerialConnectionStats/SendControllerStats/EdDialogTabs拆分 | TCP/UDP/Serial/CAN/MQTT/BLE/Modbus/Protobuf/Terminal/Widget等tr()合规+统计计数器独立文件 |
| 246 | resetStats()审计修复+XModemTransferProtocol/FftEngineWindow拆分 | CRC/PluginManager/PluginApi/FrequencyCounter缺失重置方法补全 |
| 247-250 | Stats A/B批量增强+Linter大规模文件拆分 | TCP/UDP/WebSocket/TCP Server/TerminalHighlighter/FrameVisualEditor/DataStatistics/ThemeManager统计增强+ChannelConfigSetMethods/ProtocolBridgeManagerConfig/I2cConnectionTransfer/ZoomControllerViewport等20+文件拆分 |
| 251-256 | 持续文件拆分(PacketBuilderPanelFields/HistogramWidgetUI/TcpConnectionHandlers/ThemeManagerApply/ShortcutManagerQuery/MqttWidgetSubs/DataAggregatorCompute/DashboardModelItems/FlatBuffersDecoderFields/ConnectionControllerLifecycle/TerminalWidgetEvents等) | 所有.cpp控制在300行以内+统计计数器独立+事件处理独立 |
| 257 | SerialConfigPanelConfig/TriggerEngineStats/DataExporterStats拆分+多模块统计增强+头文件压缩 | 配置读取/统计getter独立文件+SignalLineMonitor/TrafficMonitor/PerformanceMonitor/CanConnectionProtocol统计增强+14个.h压缩至<180行 |
| 258 | TerminalWidgetSearch/DashboardWidgetPersistence/WebSocketFrameStats拆分 | 搜索功能独立+持久化方法独立+WebSocket统计getter独立 |
| 259 | SendControllerSend/PluginConfigPanelUI拆分 | 发送逻辑独立(283→124行)+插件面板列表/详情独立(283→238行) |
| 260 | ChecksumCalculatorStats拆分 | 算法元数据/统计getter独立(281→193行) |
| 261 | UdpConnectionStats拆分 | 多播组管理/统计getter/resetStats独立(275→219行) |
| 262 | TlsConnectionStats/SpiConnectionStats/CanBusMonitorStats/NavigationControllerStatus拆分 | TLS统计独立(278→259行)+SPI统计独立(279→260行)+CAN监控统计独立(281→207行)+导航状态动画独立(285→160行) |
| 263 | DataStatisticsRefresh/ResponsiveLayoutTransition/DbcParserDecode/UsbDeviceDetectorDescriptor拆分 | 刷新定时器独立(279→208行)+布局过渡动画独立(276→190行)+DBC位域解码独立(272→144行)+USB描述符获取独立(272→145行) |
| 264 | TerminalLayoutManagerStats/ShortcutManagerPersist/UsbConnectionInterface/CursorOverlayEvent拆分 | 终端统计getter独立(271→197行)+快捷键持久化独立(269→201行)+USB接口管理独立(272→207行)+游标事件过滤独立(271→167行) |
| 265 | PerformanceMonitorLatency/PortWatcherPoll/YModemTransferStats/SpiI2cConfigPanelPersist拆分 | 性能延迟查询独立(268→194行)+串口轮询防抖独立(268→112行)+YModem统计/文件独立(268→135行)+SPI/I2C持久化独立(271→222行) |
| 266 | DataConverterCodec/ProtocolViewData/FftWidgetSlots/BleGattBrowserSlots拆分 + WidgetFactory mutable修复 | 编解码独立(DataConverter 175→80行)+协议帧数据独立(ProtocolView 158→63行)+FFT槽函数独立(FftWidget 175→80行)+BLE GATT操作槽独立(BleGattBrowser 147→47行) |
| 267 | SendHistoryStats/MqttConnectionStats/TerminalSelectionManagerStats/SchemaViewerStats拆分 | 发送历史频率分析/统计独立(SendHistory 259→134行)+MQTT统计独立(MqttConnection 254→209行)+终端选区统计独立(TerminalSelection 253→197行)+Schema解码统计独立(SchemaViewer 251→149行) |
| 268 | OtaHistoryModelStats/SerialDetectorQuery/DataDiffWidgetDiff/RecordingMarkerQuery拆分 | OTA统计摘要独立(OtaHistoryModel 260→143行)+串口多维度查询独立(SerialDetector 261→176行)+LCS差异算法独立(DataDiffWidget 263→137行)+标记查询导航独立(RecordingMarker 260→66行) |
| 269 | OtaManagerSignals/UdpMulticastGroup/TerminalModelStats/TlsConnectionSetup拆分 | OTA信号连接/传输控制独立(OtaManager 257→63行)+UDP组播组管理独立(UdpMulticast 255→168行)+终端统计独立(TerminalModel 247→222行)+TLS证书/SSL回调独立(TlsConnection 259→194行) |
| 270 | RegisterEditorStats/SpiConnectionConfig/DragDropHelperEvents/SpiI2cBridgeManagerQueue拆分 | 寄存器日志/统计独立(RegisterEditor 265→207行)+SPI配置/帧组装独立(SpiConnection 260→122行)+拖放事件处理独立(DragDropHelper 259→103行)+桥接事务队列独立(SpiI2cBridgeManager 240→78行) |
| 271 | ChecksumPanelSlots/TimestampPanelConvert/ModbusMasterProtocol/QuickCommandBarDialog拆分 | 校验槽函数/拖放独立(ChecksumPanel 268→137行)+时间戳转换独立(TimestampPanel 264→108行)+Modbus帧协议独立(ModbusMaster 253→114行)+快捷指令编辑对话框独立(QuickCommandBar 238→55行) |
| 272 | EventBusStats/FireWaterBridgeStats/JustFloatBridgeStats/SettingsControllerPersistence拆分 | 事件总线统计独立(EventBus 238→170行)+FireWater统计独立(FireWaterBridge 251→210行)+JustFloat统计独立(JustFloatBridge 230→189行)+设置持久化独立(SettingsController 249→147行) |
| 273 | GaugeWidgetPaint/JLinkRttConnectionStats/CommandPaletteFilter/MainWindowSignalConnectUI拆分 | 量表绘制独立(GaugeWidget 241→100行)+RTT配置统计独立(JLinkRttConnection 247→189行)+命令面板过滤独立(CommandPalette 247→161行)+信号连接UI独立(MainWindowSignalConnect 265→131行) |
| 274 | WebSocketConnectionHandshake/IntelHexParserStats/NavigationControllerQuery/SerialConnectionUtility拆分 | WebSocket握手独立(WebSocketConnection 238→93行)+HEX解析统计独立(IntelHexParser 237→145行)+导航查询独立(NavigationController 227→173行)+串口辅助方法独立(SerialConnectionError 223→127行) |
| 275 | FrameDefinitionSerialization/ConverterPanelSlots/BasePanelConfig/YAxisManagerStats拆分 | 帧定义序列化独立(FrameDefinition 254→160行)+转换面板槽函数独立(ConverterPanel 246→145行)+基础面板配置独立(BasePanel 277→149行)+Y轴统计独立(YAxisManager 253→193行) |
| 276 | TrafficMonitorStats/TerminalModelQuery/JLinkSdkLoaderRtt/ProtocolSchemaConfig拆分 | 流量统计getter独立(TrafficMonitor 221→169行)+终端行查询独立(TerminalModel 222→157行)+RTT操作独立(JLinkSdkLoader 230→157行)+协议setter独立(ProtocolSchema 229→205行) |
| 277 | ToolbarControllerQuery/TerminalControllerDisplay/SignalLineMonitorStats/TimedSenderStats拆分 | 主题语言查询独立(ToolbarController 219→155行)+终端显示搜索导出独立(TerminalController 210→107行)+信号线统计独立(SignalLineMonitor 177→113行)+定时发送统计独立(TimedSender 185→138行) |
| 278 | NumericDisplayWidgetPaint/HeatmapWidgetPaint/BookmarkWidgetStats/PerformanceOverlayStats拆分 | 数值绘制独立+热力图渲染独立+书签统计独立+性能叠加层统计独立 |
| 279 | SignalLineWidgetStats/TriggerManagerStats/ProtocolTemplateLibraryBuiltins/UsbDescriptorViewerTree拆分 | 信号线控件统计独立+触发器管理器统计独立+协议模板内置构造独立+USB描述符树构建独立+SerialDetector静态成员修复 |
| 278 | NumericDisplayWidgetPaint/HeatmapWidgetPaint/BookmarkWidgetStats/PerformanceOverlayStats拆分 | 数值绘制独立(NumericDisplayWidget 161→64行)+热力图渲染独立(HeatmapWidget 201→139行)+书签统计独立(BookmarkWidget 193→176行)+性能叠加层统计独立(PerformanceOverlay 183→119行) |
| 279 | WebSocket/BLE/SerialConfig/SerialDetector/OtaManager/OtaWidget/XModem/YModem/ZModem统计增强+TriggerManagerStats/ChartWidgetSlots/UsbDescriptorViewerTree/ModbusSlaveStats/FlatBuffersDecoderRead拆分 | WebSocket握手统计+BLE resetStats修复+串口波特率/流控统计+芯片识别静态化+OTA CRC/浏览/取消+X/Y/ZModem超时/取消+编译修复 |
| 280-284 | 持续统计增强+文件拆分+Doxygen审计+编译修复 | 多模块统计增强(ChecksumCalculator/ConverterPanel/RttConfigPanel/JLinkRttConnection/IconManager/PlaybackWidget/DataExporter/NavigationController等)+文件拆分(CursorOverlayPaintMain/ZModemTransferDispatch/TerminalSearchBarAnimation等) |
| 285 | ClipboardManager/DataPipeline/ConnectionManager/NotificationManager/ShortcutManager/DataAggregator/SettingsManager/ResponsiveLayout统计增强+6文件拆分+Doxygen审计 | 8模块统计增强(hex/base64/restores/bytesCopied+pipeline stage/output+connection serial/network+notification critical/high+shortcut unregister/category+aggregator reset/timeWindow+settings sync/group)+FrameVisualEditorPreview/BleScannerFilter/ProtobufDecoderField/ProtocolEngineCore/TerminalHighlighterHighlight/PanelManagerFactorySystem拆分+11文件Doxygen补全 |
| 286-291 | LedIndicatorWidget~ResponsiveLayout统计增强+6拆分+23文件Doxygen | 多模块统计增强+PlaybackWidgetSlots/RegisterEditorUI/FftEngineCompute/TerminalFilterCompat/DashboardWidgetLayout拆分+chart/dashboard/core/widgets Doxygen审计 |
| 292 | TCP/TLS/UDP/WebSocket/USB/SPI/I2C/SerialDetector/PortWatcher/RecordingMarker/ChartExporter/DataAggregator统计增强+头文件压缩 | 9连接模块+5基础设施模块统计增强(22+20个新计数器)+TerminalWidget 244→194/DataStatistics 239→200/SerialConfigPanel 209→161/DashboardSerializer 202→182 |
| 293 | ProtocolBridgeManager/FireWater/JustFloat/NavigationController/BasePanel/ScopeWidget/CursorOverlay/HistogramWidget统计增强+7文件Doxygen | 8模块31个新统计计数器+ToolbarController/TriggerEngine/TriggerManager/ToastWidget/ChecksumCalculator/DataConverter/TimestampPanel Doxygen审计(~55方法修复) |
| 294-340 | Wave 3-5功能冲刺 | DataAnnotation/FrequencyCounter/ConfigProfileManager/MemoryHexEditor拆分/DataChecksumVerifier(17算法)/PacketReassembler/DataMaskEditor/DataHistoryBuffer/ProtocolSimulator/DataStreamFilter/ScrollChart/WaveformPattern/ConnectionProfile/ScopeChannelManager/SerialPortProfiler/NotificationHistory/DataStreamRecorder/ScriptableProtocolEngine(Qml)/ProtocolFieldEditor/LedMatrixSimulator/CircularBufferWidget/WaveformGenerator/SerialTimingAnalyzer |
| 341-355 | Wave 5-6功能冲刺 | DataChecksumVerifier拆分(Algo+Stats)+SerialDataEncoder(8种编解码)+BytePatternAnalyzer(通配符搜索)+DataStreamSplitter(4种分割)+DataRateCalculator(滑动窗口速率)+Qt6::Qml链接修复 |
| 356 | Bug修复Batch 7 — 25+安全/崩溃/逻辑bug修复 | NavigationController静态变量→实例成员+LoadingSpinner showEvent/hideEvent+YModem除零保护+ZModem无限重试限制/定时器兜底+TlsConnection CA证书泄漏+onSslErrors空指针+UdpConnection析构泄漏+XModem offset溢出+TrafficMonitor非单调时间戳+DbcParserDecode int溢出+RingBuffer线程安全+FlatBuffers类型大小+SerialPortProfiler截断+ZModem除零+ProtocolSimulator负值包装+ModbusSlave功能码范围+WaterfallWidget鼠标+OtaManager flush+TerminalSearchRenderer提前终止+TerminalContextMenuManager空事件 |
| 357 | Wave 7功能冲刺 — 5大新工具+25+bug修复+头文件压缩 | DataComparator(LCS差异算法/相似度/Hex并排对比)+PacketLossDetector(序列号追踪/间隙检测/rollover)+ByteFrequencyAnalyzer(Shannon熵/累积频率/模式检测)+SerialProtocolFuzzer(6种fuzz策略/确定性PRNG)+FrameTemplateLibrary(帧模板CRUD/校验组装/JSON导入导出)+linter修复(RingBuffer/TrafficMonitor/ChecksumPanel/ConverterPanel/TimestampPanel/NavIndicator/TerminalSearchRenderer/AnimatedProgressBar/NavigationController) |

---

## 三、重要状态

- 当前文档已整理到 `#160` 附近的阶段性记录
- 后续新增分数时，优先补“阶段摘要”和“里程碑”，不再恢复长篇流水账
- 如果需要精确到单次提交，请直接查 `git log` 或对应 commit message
- 本文件只负责评分追踪，不承载工作流、提交规则或架构约束正文

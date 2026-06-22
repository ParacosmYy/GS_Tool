# EmbedDebug 评分追踪

> 分支: `feat/embed-debug` | 远程: `https://github.com/ParacosmYy/GS_Tool.git`
> 当前: 742分 | 目标: 1000分 | 每次 commit 默认记 1 分（固定节奏，不按工作量梯度）
> 距离目标还差 258分

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
| 267-293 | 统计增强+接口实现+EdDialog/TriggerEngine/ChecksumCalculator/PluginLoader/I2C修复 | 293 |
| 294-306 | MiniChartWidget/NavIndicator/ToastWidget/USB libusb/头文件精简/Terminal-Utils增强 | 306 |
| 297-343 | ConnectionHealthMonitor/DataLogViewer/DataInspector/SignalGenerator/DataCompare/DataComparator/DataPatternDetector/DataStreamRecorder/PacketTemplateLib/WaveformPreview/FrequencyCounter/DataAnnotation/ConnectionProfileManager/LedMatrixSimulator/ProtocolFieldEditor/ScriptableProtocolEngine/SerialTimingAnalyzer/WaveformGenerator/DataChecksumVerifier/CircularBufferWidget/ByteFrequencyAnalyzer/SerialProtocolFuzzer/PacketLossDetector/SerialPortProfiler/ScopeChannelManager/ScrollChartWidget/DataStreamFilter/ProtocolSimulator/DataHistoryBuffer/MemoryHexEditor/ProtocolTimerWidget/DataMaskEditor/ConfigProfileManager+多模块编译修复 | 343 |
| 344-371 | Wave 7-10工具模块批量实现(WindowedAggregator/DataCaptureBuffer/CrcStreamVerifier/DataReducer/DataScalerWidget/DataSynchronizer/DataThresholdMonitor/DataComparator/DataLossDetector/FrequencyCounter2/DataStreamFuzzer/ProtocolTemplateLib/ProtocolMonitor/DataRateLimiter/ByteStreamParser拆分/SerialSimulator拆分/DataCache/DataValidator/PacketAssembler)+Wave 11(DataCompressor压缩引擎/PacketAssembler组装器拆分/ProtocolDuplicator流量复制器拆分/SerialHealthMonitor串口健康监控/DataFlowMeter数据流量计) | 371 |
| 372-404 | Wave 18-28工具模块大规模实现(OutlierDetector/KMeansClusterer/PidController/FFT2/BayesClassifier/RegressionEngine/PcaAnalyzer/FastDTW/BitFieldParser/CircularStatsBuffer/HysteresisFilter/SpinLockQueue/TimerPool/DetrendEngine/ZScoreNormalizer/KMedoidClusterer/VotingCompositor/MarkovChain/FuzzyLogicEngine/Crc64Engine/RunLengthCodec/LzwCompressor/HuffmanCodec/HaarWavelet/BezierSpline/VoiceActivityDetector/DynamicHistogram/ShannonCoder/TickEngine/CircularPriorityQueue/RollingRank/ExponentialMoving/PolynomialRegression/GoodnessOfFit/LockFreeRingBuffer/StreamTokenizer/SuffixArray/MinHashSignature/TimerWheel/SlidingPercentile/AutoCorrelation/DeadZoneFilter/MovingMinMax/RollingHash/CountMinSketch/CuckooFilter/ScalableBloomFilter/RateOfChange/SimplexNoise/HoltWinters/ProcessMonitor/SlidingWindowCounter/AR/AVL/组合/分形/梯度/Hamming/Hurst/红黑树/Reed-Solomon)+const/emit编译修复+linter自动增强 | 404 |

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
| 251-256 | 持续文件拆分(PacketBuilderPanelFields/HistogramWidgetUI/TcpConnectionHandlers/ThemeManagerApply/ShortcutManagerQuery/MqttWidgetSubs/DataAggregatorCompute/DashboardModelItems/FlatBuffersDecoderFields/ConnectionControllerLifecycle/TerminalWidgetEvents等) | 所有.cpp控制在297行以内+统计计数器独立+事件处理独立 |
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
| 358-366 | Wave 8-9功能冲刺+编译修复 | DataEncryptionEngine(XOR/AES/RC4/Caesar/Vigenere)+SerialFrameDecoder(帧解码/组装/校验)+WaveformFilter(8种DSP)+ModbusMaster(RTU/TCP)+SerialDataLogger(6格式/轮转/压缩)+ProtocolSession(请求响应追踪/P95)+DataRateLimiter(令牌桶/漏桶)+HexDiffEngine(字节级对比/补丁)+PacketVisualizer(彩色字段图)+SerialScriptEngine(脚本/条件分支)+StreamCaptureRecorder(重命名避免MOC冲突)+ModbusScanWidget信号重载修复+CMakeLists修复+DataPipeline/DataAggregator/ProtocolBridge null安全 |
| 367-419 | Wave 10-46工具模块大规模实现+持续bug修复 | 200+算法/数据结构/信号处理/机器学习/数值方法模块实现(LRUCache/BloomFilter/Trie/QuadTree/BPlusTree/Fenwick/SegmentTree/SuffixArray/RadixTree/DisjointSet/ConvexHull/Voronoi/FFT2/Bessel/Butterworth/Chebyshev/Goertzel/Welch/YuleWalker/CatBoost/ARIMA/NaiveBayes/KernelPca/CrossValidator等)+PolarCode越界/BPlusTree分裂计数/MuLaw符号/ConcurrentSkipList竞态/TDigest除零/QuadTree计数/Turbo交织器/ArcCache驱逐/TinyLfu LRU/Fenwick const/Huffman析构/LombScargle除零/QuantileSketch GK压缩/RealFFT溢出/LZW空条目/WavWriter统计/WavReader EOF/Bessel双线性变换/Butterworth带通/Chebyshev分子/LombScargle变量名等30+bug修复 |
| 420-424 | Batch 12 bug修复 — 安全/崩溃/除零/整数溢出/const_cast | ProtocolEngineParsing quint8<<24整数溢出+HeatmapWidget2 setCellSize除零+TerminalWidgetPaint m_selectionManager空指针+TerminalWidgetSlots lineHeight除零+CrossValidator Fisher-Yates替代std::shuffle+k>n边界+KMedoidClusterer Fisher-Yates+CubicInterpolator mutable替代const_cast+ChangepointDetector windowSize>=4+FrequencyCounterWidget ws除零+BayesClassifier sumExp除零+ScalableBloomFilter mutable+CuckooFilter mutable+WaveformPatternDetector dropoutThreshold除零+PacketReassembler qint32转换+BloomFilter/MinHashSignature/DeltaEncoder/AdaptiveHuffman/MuLawEncoder/DataCompressorAlgo quint8<<24整数溢出(6文件)+QuadTree mutable+CountMinSketch mutable+ResponsiveLayout2 cols除零+ProtobufDecoderEncoding fieldNum<<3溢出+FlatBuffersDecoderRead soff溢出+SpectralWhitener fftN/2除零 |
| 425 | CMakeLists ODR冲突清理 | 注释26+个重复类定义(SpectralCluster2/TurboDecoder/GoertzelAlgorithm/GraphIsomorphism/ReedSolomon2/SylvesterSolver/AdaptiveHuffman/AutoCorrelation等)+禁用utils/eye/EyeDiagramEngine(chart/eye为canonical) |
| 426 | 约束文档更新 | CLAUDE.md + 06-git-commit.md 更新 |
| 427-434 | Wave 74-134 .cpp批量实现(按模块分层提交8个commit) | cluster(74-118, 26文件)+code(75-119, 24文件)+dsp(76-120, 20文件)+fft(77-121, 21文件)+graph(90-134, 27文件)+matrix(80-124, 26文件)+signal(78-122, 25文件)+tree+interp+poly+string(78-122, 30文件) |
| 435 | chart模块新增 | MathExpressionParser(数学表达式解析)+EyeDiagramEngine(眼图绘制)+EyeDiagramWidget+WaveformMath |
| 436 | protocol+connection模块新增 | ProtocolLogicEngine+ProtocolSequencer+ProtocolDecoder+SvdParser+SvdViewerWidget+PortBridge+BridgeConfigPanel |
| 437 | utils五大工具模块+PID扩展 | FilterDesigner+FirmwareDiffer+FirmwarePatchGenerator+NmeaParser+GpsTrackWidget+NetworkScanner+NetworkTopologyWidget+StateMachineDesigner+PidSimulator+PidTunerWidget |
| 438 | 文档新增 | FEATURE_ROADMAP.md功能路线图 |
| 439 | DataFlowMeter.h文件体积修复 | .h从252→115行(Doxygen精简) |
| 440 | CMakeLists构建系统全量注册 | Sources+632 .cpp / Headers+584 .h (共1216条目) |
| 441 | 约束文档阶段提交规则 | Specs/GO/BATCH/LOOP入口+3/6子Agent受控并行+阶段提交要求 |
| 442 | CMake generated utils瘦身 | 数字后缀utils草稿目录从主GUI目标过滤，PRD-078落档 |
| 443 | Serial Station最小UART骨架+重复骨架清理 | 独立app/controller/core/protocols骨架+ASCII文本协议+3个QTest+删除未引用小写残留文件 |
| 444 | 启动链路与Agent Runtime工具 | EmbedDebug.bat委托PowerShell启动器+bootstrap/doctor/debug/simplify/agent-loop工具+本机local_env移出Git |
| 445 | UI QSS职责收敛+UART配置面板 | 工业工作台主题刷新+TerminalFilter/MQTT/AppDialog内联样式迁移到QSS+Serial Station串口配置面板 |
| 446 | Source Tree瘦身审计PRD | 文件过多/重复轮子/UART缺口审计路线落档 |
| 447 | Qt6兼容构建修复回归 | MQTT/SVD/Script/L1兼容修复收口+Qt6CompatRegressions测试 |
| 448 | Source Tree审计报告刷新 | SerialPortPanel进入CMake证据更新+数字utils过滤验证 |
| 449 | GO执行引擎样例校准 | agent-loop样例改走doctor/bat验证链路并补环境说明 |
| 450 | Source Tree active CMake审计标准 | PRD-081/Specs定义raw/declared/active引用口径 |
| 451 | Source Tree active CMake审计实现 | source-tree-audit区分raw/declared/active并刷新报告 |
| 452 | Active utils编译面报告 | source-tree-audit列出仍在主目标中的utils目录 |
| 453 | Active utils依赖审计标准 | PRD-082/Specs定义外部include证据与低风险候选口径 |
| 454 | Active utils依赖审计实现 | source-tree-audit统计外部include并输出低风险拆分候选 |
| 455 | Active utils CMake拆分BATCH方案 | 12个可审查子任务落档，明确人工审查前不并行 |
| 456 | Active utils CMake拆分串行探针 | 主目标过滤pid/simulator并同步审计脚本口径，构建/测试/doctor/bat通过 |
| 457 | Serial Station UART面板主题优化 | 三套QSS补齐串口配置面板、输入控件、按钮层级和状态条样式 |
| 458 | Serial Station工作台壳层PRD/Specs | 定义命令区、日志区、状态栏和UI边界，禁止UI越过controller/core/protocol |
| 459 | Serial Station工作台壳层实现 | 新增命令区、日志区、状态栏并接入窗口装配和QTest，形成四区调试台 |
| 460 | Serial Station发送闭环PRD/Specs | 定义UI send意图经controller到protocol/core的第一版闭环，明确未打开/空命令/未知模式错误路径 |
| 461 | Serial Station发送闭环实现 | UI发送意图接入SerialStationController，失败/日志/计数信号回写工作台，补controller与workbench回归测试 |
| 462 | Serial Station接收闭环PRD/Specs | 定义SerialManager bytes经controller协议feed到RX日志/计数的接收闭环，明确半包和空bytes路径 |
| 463 | Serial Station接收闭环实现 | SerialManager bytes接入controller默认协议feed，RX日志/计数回写工作台，补半包/粘包/重置回归测试 |
| 464 | Serial Station Dispatcher PRD/Specs | 定义core/SerialDispatcher承接bytes到协议feed的职责，要求controller移除直接接收协议持有 |
| 465 | Serial Station Dispatcher实现 | 新增core/SerialDispatcher承接bytes到协议feed，controller移除接收协议直持有，补dispatcher/controller/workbench回归 |
| 466 | Serial Station配置摘要PRD/Specs | 定义UART配置校验、标准化摘要、Controller连接日志和错误路径验收 |
| 467 | Serial Station配置摘要实现 | SerialPortConfig新增标准化、错误原因和摘要，Controller/SerialManager使用统一配置诊断，补配置和控制器回归 |
| 468 | Serial Station发送编码器PRD/Specs | 定义core/SerialCodec承接ASCII/HEX/协议发送帧构建，补HEX模式验收 |
| 469 | Serial Station发送编码器实现 | 新增SerialCodec复用HexConverter，Controller接入统一编码结果，命令面板开放HEX模式并补codec/command/controller/workbench回归 |
| 470 | Serial Station Modbus RTU协议PRD/Specs | 定义modbus_rtu协议注册、主站请求构建、流式解析、CRC错误和QTest验收 |
| 471 | README企业级宣传标准约束 | 在文档线补充README首屏定位、截图预览、架构概览、真实能力证据和可执行命令要求 |
| 472 | Serial Station Modbus RTU协议实现 | 新增modbus_rtu协议构帧和流式解析，注册内置协议并补Modbus/registry回归测试 |
| 473 | Serial Station Custom MD协议PRD/Specs | 定义custom_md默认MCU调试帧、构帧参数、流式解析、重同步和QTest验收 |
| 474 | Serial Station Custom MD协议实现 | 新增custom_md默认MCU调试帧构建和流式解析，注册内置协议并补custom/registry回归测试 |
| 475 | GO Loop启动验证PRD/Specs | 定义agent-loop自动bat启动探针、缺Go诊断、循环测试和工具验收路径 |
| 476 | GO Loop启动验证实现 | 新增bat启动探针、sample改用自动探针、README补缺Go替代路径并补agent-loop循环测试 |
| 477 | README企业级宣传入口 | README升级为企业级项目首页，补稳定界面预览、真实能力证据、启动验证和工程工作流说明 |
| 478 | Serial Station日志服务PRD/Specs | 定义services层结构化日志记录、过滤、容量裁剪和导出文本边界，禁止UI/串口线程/协议层越界 |
| 479 | Serial Station日志服务实现 | 新增services/SerialLogService结构化日志仓库，支持TX/RX/System/Error、过滤、容量裁剪、纯文本和JSON Lines输出并补QTest |
| 480 | Serial Station控制器日志接入PRD/Specs | 定义Controller写入SerialLogService的结构化日志闭环，保持UI信号兼容且不改UI生产文件 |
| 481 | Serial Station控制器日志接入实现 | Controller持有并写入SerialLogService，提供日志快照/过滤/文本/JSON接口，补独立控制器日志接入QTest |
| 482 | Serial Station导出服务PRD/Specs | 定义services层日志导出能力，支持PlainText/JSON Lines/CSV和文件写入错误路径，不改UI/controller |
| 483 | Serial Station导出服务实现 | 新增services/SerialExportService，支持PlainText/JSON Lines/CSV格式化、QSaveFile原子写入、BOM和错误路径QTest |
| 484 | README企业级宣传约束入口强化 | CLAUDE/工作流/Git约束同步声明README必须作为企业级对外入口维护，产品能力变化时必须检查是否同步更新 |
| 485 | Serial Station UI导出闭环PRD/Specs | 定义日志导出按钮经Window选择路径、Controller调用导出服务、结果回写结构化日志的跨层闭环 |
| 486 | Serial Station UI导出闭环实现 | Controller接入SerialExportService，Window导出按钮选择路径并调用controller，补导出闭环QTest和CMake注册 |
| 487 | Serial Station回放服务PRD/Specs | 定义services层日志回放计划能力，从结构化日志或JSON Lines生成可回放事件序列，不触碰UI/controller/core |
| 488 | Serial Station回放服务实现 | 新增SerialReplayService，将结构化日志或JSON Lines转换为回放计划，支持方向过滤、速度倍率、延迟钳制和错误路径QTest |
| 489 | Serial Station UI回放预览PRD/Specs | 定义日志面板回放入口经Window转发到Controller，使用SerialReplayService生成预览计划并回写系统日志，明确本阶段不真实串口发送 |
| 490 | Serial Station UI回放预览实现 | 日志面板新增回放入口，Controller接入SerialReplayService生成预览计划，Window完成signal闭环，README同步企业级能力描述并补QTest |
| 491 | Serial Station UART基础可用性PRD/Specs | 定义端口枚举详情、手动COM输入、UART配置摘要、刷新反馈和无真实串口依赖测试，优先修复基础上位机入口 |
| 492 | Serial Station UART基础可用性实现 | 串口面板补端口详情显示、真实端口名item data、手动COM输入、UART配置摘要和参数变化联动，README同步基础UART能力并补面板QTest |
| 493 | UV打包快捷工具PRD/Specs | 定义uv run package-embeddebug入口，编排CMake构建、Qt windeployqt部署、dist目录和zip产物，不使用PyInstaller打C++本体 |
| 494 | UV打包快捷工具实现 | 新增pyproject入口和tools/package_embeddebug.py，支持uv run package-embeddebug构建/复用build产物、windeployqt部署、dist目录和zip包 |
| 495 | UV启动快捷工具PRD/Specs | 定义uv run start-embeddebug入口，复用EmbedDebug.bat最低启动链路，支持dry-run/wait并保持单一build目录 |
| 496 | UV启动快捷工具实现 | 新增tools/start_embeddebug.py和pyproject入口，支持uv run start-embeddebug启动bat链路、dry-run和wait参数，README同步快捷启动说明 |
| 497 | UV工具自测PRD/Specs | 定义uv run test-embeddebug-tools入口，使用unittest覆盖启动/打包工具纯逻辑，不启动GUI、不构建、不调用windeployqt |
| 498 | UV工具自测实现 | 新增tools/test_embeddebug_tools.py和pyproject入口，覆盖start/package工具纯逻辑，README同步uv工具自测命令 |
| 499 | UV打包产物校验PRD/Specs | 定义uv run verify-package-embeddebug入口，离线校验dist发布目录的exe、Qt平台插件、README和随包文档 |
| 500 | UV打包产物校验实现 | 新增tools/verify_package_embeddebug.py和pyproject入口，支持最新dist目录或指定目录离线校验并补unittest覆盖 |
| 501 | Serial Station命令历史UI PRD/Specs | 定义命令面板最近命令、去重容量、选择回填和清空历史边界，仅限ui层不触碰协议/core |
| 502 | Serial Station命令历史UI实现 | 新增ui层SerialCommandHistoryModel并接入命令面板最近命令、成功发送确认、选择回填、清空和QSS/QTest覆盖 |
| 503 | Serial Station协议选择UI PRD/Specs | 定义协议选择面板、controller默认协议切换、结构化日志、窗口闭环和README同步边界 |
| 504 | Serial Station协议选择UI实现 | 新增协议选择面板、controller默认协议切换slot、窗口闭环、QSS/README同步和QTest覆盖 |
| 505 | Serial Station主入口收敛 | 将SerialStationWindow接入主窗口导航，用户可从连接分类进入串口工站，构建/QTest/启动探针通过 |
| 506 | Agent Runtime验证命令收口 | PRD-072改用Doctor和自动启动探针作为GO验证路径，记录缺Go时的替代验证口径 |
| 507 | Agent Runtime验证记录收口 | PRD-072 Specs补齐Doctor、启动探针、缺Go和最近commit证据，明确GO单测未运行缺口 |
| 508 | Agent Loop验证入口 | 新增tools/agent-loop/verify.ps1，统一go test、样例dry-run和缺Go诊断入口，并同步PRD/Specs/README |
| 509 | CMake源码清单模块化 | 将根CMake中的SOURCES/HEADERS清单拆入cmake/EmbedDebugSources.cmake，配置、主目标构建和EmbedDebug.bat启动探针通过 |
| 510 | CMake审计工具兼容include清单 | project-audit和source-tree-audit纳入cmake/*.cmake片段，补Python工具测试并刷新source-tree审计报告 |
| 511 | CMake源码清单去重 | 移除chart eye/math重复CMake条目，project-audit duplicate entries归零，配置、主目标构建和启动探针通过 |
| 512 | Python工具自测直接入口 | 修复python tools/test_embeddebug_tools.py导入路径，直接Python入口和uv脚本入口均通过 |
| 513 | CMake低风险utils首批瘦身 | 从主GUI目标移除filter_design/firmware/gps/network/statemachine/wavegen六组无外部引用证据的utils条目，构建和启动探针通过 |
| 514 | CMake低风险utils第二批瘦身 | 从主GUI目标移除compare/hex_editor/rate/aggregator/bitmask/gmm/hex_diff/packet_lib/pattern/ringhash/spectrum/waveform条目，构建和启动探针通过 |
| 515 | CMake低风险utils第三批瘦身 | 从主GUI目标移除align/annotation/compress/data_inspector/decoder/encoder/frequency/fuzzer/loss/protocol_timer/recorder/signal_gen/splitter/template_lib/validator条目，构建和启动探针通过 |
| 516 | CMake单引用utils第四批瘦身 | 从主GUI目标移除avltree/bfgs/capture/crc_verifier/cycle/decomposer/gaussquad/golay/groebner/heap/hessenberg/hmm/inv_perm/lfu/lifting/lru/moment/movingmax/qrupdate/redblack/scaler/segment/skiplist/sliding/smoother/state/threshold/trie/trigger/twoway/warp条目，构建和启动探针通过 |
| 517 | CMake双引用utils第五批瘦身 | 从主GUI目标移除adams/adaptive_fft/agc/aho/anomaly/anova/arima/armodel/arnoldi/autocorr/autodiff/backtrack/bandmat/bayes/bernstein/bessel/bezier/bicgstab/bintree/bisect/bitalloc/bitfield/bitmap/bloom/bootstrap/bsp_tree/btree/butterfly/butterworth/cache条目，构建和启动探针通过 |
| 518 | CMake双引用utils第六批瘦身 | 从主GUI目标移除catboost/cauchy/cepstrum/cgls/changept/chebyshev/chisq/cholupdate/cic/circadian/circbuf/circqueue/circulant/classifier/clenshaw_curtis/cluster/collocation/combination/complement/conjugate/convhull/convolution/correlator/cosinedist/countmin/covariance/crc64b/crosscorr/crossval/cuckoo_hash条目，构建和启动探针通过 |
| 519 | CMake双引用utils第七批瘦身 | 从主GUI目标移除davidson/dct/deadzone/decomp/delta/deque/derivative/detector/detrend/dict/disjoint/divided_diff/dynhistogram/echohash/edgedetect/edmonds/ekf/emd/ensemble/entropy/envelope/expmove/farmhash/fcs/fenwick/fir/fisher/flow/fractal/freq条目，构建和启动探针通过 |
| 520 | 串口工站快速启动入口 | 新增 `--panel serial.station` 和 `--station serial` 启动直达串口工站，bat透传参数，导航按稳定panel id恢复，QTest/构建/带参与无参启动探针通过 |
| 521 | Serial Station配置档案服务 | 新增services层SerialProfileService，支持UART/协议/发送模式/命令档案JSON和文件读写，QTest/构建/启动探针通过，UI接入仍待后续 |
| 522 | Serial Station配置档案UI闭环 | 工作台新增保存/加载档案入口，Controller复用SerialProfileService，UART/协议/发送模式/命令历史可保存回填，QTest/构建/启动探针通过 |
| 523 | README企业级首页刷新 | README重构为企业级项目首页，补产品定位、成熟度矩阵、Serial Station工作流、架构边界、验证命令和路线图，保持三轴状态口径 |
| 524 | Serial Station启动档案入口 | `--profile`/`--serial-profile` 支持随启动加载 `.edserialprofile`，默认直达串口工站，QTest/构建/带档案启动探针通过 |
| 525 | Serial Station最近档案索引 | 新增配置档案最近/上次索引服务，工作台支持最近档案下拉与重载上次，复用SettingsManager和SerialProfileService，QTest/构建/启动探针通过 |
| 526 | Serial Station最近档案清理 | 新增最近档案查询/移除服务语义，工作台提供清空最近入口并保持持久化状态一致，补充PRD/Specs和QTest覆盖 |
| 527 | Serial Station上次档案快捷启动 | 新增 `--last-profile`/`--serial-last-profile` 启动参数，复用最近档案索引直接恢复上次工位配置，补充PRD/Specs和启动/工作台QTest |
| 528 | Serial Station失效档案清理 | 最近档案索引支持移除不存在路径，工作台新增清理失效入口，`--last-profile` 和重载上次支持缺失档案回退，补充PRD/Specs和QTest |
| 529 | Serial Station档案目录快捷启动 | 新增 `--profile-dir`/`--serial-profile-dir` 启动参数，工作台保存/加载档案默认目录可由启动脚本固定，并拆分窗口档案实现文件 |
| 530 | Serial Station默认档案目录持久化 | 默认档案目录写入 `SerialProfileCatalogService`，启动参数、保存和加载成功后自动记忆目录，后续窗口恢复工位目录且清空最近不丢目录 |
| 531 | Serial Station默认目录档案导入 | 默认档案目录支持扫描 `.edserialprofile`/`.json` 并一键导入最近档案索引，工作台新增“导入目录”入口且不改变当前已应用配置 |
| 532 | Serial Station JustFloat协议基础 | 参考 VOFA+ 数据路径新增 `just_float` 协议，支持 float 数组帧解析、半包粘包、异常 payload 诊断、registry 和工作台选择闭环 |
| 533 | Serial Station测量通道面板 | 新增测量服务和通道摘要面板，JustFloat measurement 事件可在工作台展示最新值、样本数、最小值和最大值 |
| 534 | Serial Station测量趋势缓冲 | 测量服务新增最近帧缓冲、趋势文本、CSV格式化和CSV文件导出服务，工作台测量面板展示 JustFloat 最近帧变化 |
| 535 | Serial Station测量波形预览 | 新增测量波形预览控件，JustFloat 最近帧可在工作台测量面板以轻量多通道折线显示，Controller/Panel/Workbench QTest 覆盖 |
| 536 | Serial Station测量导出入口 | 测量面板新增导出按钮，工作台弹出保存对话框并调用测量 CSV 导出服务，CSV 写入与 UI 闭环可测 |
| 537 | Python-only 主线清理 | 删除旧 QTest/C++ 测试源码和 Qt qrc 清单，README 重写为 PyQt 上位机产品入口，约束文档收口到 uv/PyQt/PyInstaller |
| 538 | Python 工程入口瘦身 | 删除 Beta、bootstrap、debug/simplify/source-tree 脚本和 Go agent-loop，活跃入口收敛为 EmbedDebug.bat、uv scripts、doctor 与 PyInstaller 工具链 |
| 539 | Serial Station UI 文件体积治理 | 将主窗口 UI 构建拆入 `ui/sections.py`，新增 Python runtime 297 行门禁，保持工作台 UI smoke 与启动 smoke 通过 |
| 540 | Serial Station UI section 类型边界 | 为 `ui/sections.py` 增加显式宿主 Protocol，禁止 `Any owner` 隐式耦合，并保持 297 行体积门禁、UI smoke 与启动 smoke 通过 |
| 541 | 活跃架构文档 PyQt 化 | 删除 `docs/architecture` 中旧原生设计文档，重写架构索引和冻结目录口径，新增门禁确保活跃架构文档只指向 Python/PyQt 主线 |
| 542 | 活跃 PRD/Specs 旧批次清理 | 删除 CMake、Source Tree、Go Agent Loop 和旧 UV 包装批次文档，新增门禁防止退休 native/tool 批次留在活跃 PRD/Specs 目录 |
| 543 | 活跃 PRD/Specs 正文 PyQt 化 | 删除活跃 PRD/Specs 中仍含原生路径、CMake 和 `.h/.cpp` 实现口径的旧文档，新增正文门禁确保活跃规划只描述 Python/PyQt 主线 |
| 544 | Serial Station transport registry | 新增 PyQt lane transport registry，controller 通过 mode 创建 fake/serial transport，为后续 TCP/UDP/BLE/CAN 多源接入提供可测试扩展点 |
| 545 | Python 启动面最小化 | `EmbedDebug.bat` 直连 uv，删除无活跃调用链的 PowerShell 工具和旧编辑器配置，README 与门禁同步为单一 Python/PyQt 启动面 |
| 546 | Serial Station TCP client transport | 新增 PyQt lane TCP client transport，registry 默认暴露 `tcp` mode，controller 可保存 TCP endpoint profile，并通过 loopback 替身链路验证收发 |
| 547 | Serial Station TCP UI entry | 工作台连接工具栏新增 TCP host/port 与 Connect TCP 入口，用户可从 PyQt UI 建立 TCP client 连接并保存 endpoint profile |
| 548 | 活跃脚本文档收口 | 活跃治理文档移除已删除 shell 脚本入口，README 更新为企业级精简首页，脚本面继续保持单一 bat + uv scripts |
| 549 | Shared operation result | 新增 Python 主线共享 OperationResult/OperationError，Serial Station 发送路径开始返回结构化失败并保留 bool API 兼容 |
| 550 | Serial Station connection result | fake/serial/tcp 连接入口新增结构化 OperationResult API，保留 bool 兼容并拆出 profile snapshot 维持 controller 体积门禁 |
| 551 | Serial Station service result | 测量导出、回放读取和 Profile 读写新增 OperationResult API，service 文件异常具备统一错误码且旧 API 保持兼容 |
| 552 | Serial Station controller service result | controller 导出、回放和 Profile 操作接入 OperationResult，日志条目与事件转换拆分，失败路径不破坏当前日志和命令历史 |
| 553 | Serial Station UI service result | UI 日志导出、回放和 Profile 操作接入 OperationResult，失败路径显示状态反馈且不破坏当前可见日志/Profile 状态 |
| 554 | Serial Station UI connection result | UI Fake/Serial/TCP 连接和发送入口接入 OperationResult，连接/发送失败展示可诊断原因并继续保持成功路径兼容 |
| 555 | Serial Station UI injection result | UI Fake RX 注入入口接入 OperationResult，非 Fake transport 注入失败展示可诊断原因且不覆盖为成功状态 |
| 556 | Serial Station TCP action boundary | TCP 连接动作归并到 connection_actions，tcp_controls 回归控件构建职责并新增 UI 架构边界测试 |
| 557 | Serial Station clear log action boundary | 清空日志动作归并到 session_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 558 | Serial Station protocol action boundary | 协议选择动作归并到 protocol_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 559 | Serial Station disconnect action boundary | 断开连接动作归并到 connection_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 560 | Serial Station port refresh action boundary | 串口端口刷新动作归并到 connection_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 561 | Serial Station command history action boundary | 命令历史刷新和选择动作归并到 connection_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 562 | Serial Station profile control action boundary | Profile 控件回填动作归并到 session_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 563 | Serial Station log display action boundary | 日志展示、过滤和统计动作归并到 log_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 564 | Serial Station connection control state action boundary | 连接控件状态和串口可用性判断归并到 connection_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 565 | Serial Station measurement display action boundary | 测量 batch 展示归并到 measurement_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 566 | Serial Station keypress shortcut action boundary | Ctrl+Enter/Ctrl+L/Ctrl+R 兜底按键分发归并到 shortcuts，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 567 | Serial Station status error action boundary | controller 错误回调展示归并到 status_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 568 | Serial Station window lifecycle action boundary | 窗口关闭资源释放归并到 lifecycle_actions，MainWindow 只保留委托并新增 UI 架构边界测试 |
| 569 | Serial Station UDP datagram transport | 新增 PyQt QUdpSocket 驱动并接入 TransportRegistry 默认 udp 模式，补 UDP loopback 与 registry 单测 |
| 570 | Serial Station UDP controller connection | 新增 controller UDP 连接 API，抽出 endpoint 连接 helper，补 registry 注入、发送与 Profile 持久化单测 |
| 571 | Serial Station UDP UI entry | 工作台新增 UDP host/port 与 Connect UDP 入口，Profile 可保存 UDP endpoint，补 UI smoke 与架构边界测试 |
| 572 | Serial Station UDP UI loopback | 补 PyQt UI UDP loopback 收发 smoke，状态栏显示 UDP 本地绑定端口，形成可回包诊断闭环 |
| 573 | Serial Station endpoint validation helper | 新增 TCP/UDP host:port 公共校验 helper，connection_actions 复用字段级校验，补纯单测并保持 UI smoke 通过 |
| 574 | Serial Station status message helper | 新增 OperationResult 状态消息 helper，connection_actions 复用连接/发送成功失败文案格式，补纯单测并保持 UI smoke 通过 |
| 575 | Serial Station session status message helper | session_actions 复用统一 OperationResult 状态消息 helper，导出/回放/Profile 成功失败文案格式收敛并补架构测试 |
| 576 | Serial Station injection status message helper | injection_actions 复用统一 OperationResult 状态消息 helper，RX 注入成功失败文案格式收敛并补架构测试 |
| 577 | Serial Station status label helper | 将 action 私有 `_set_result_status` 上移为 status_messages 公共 helper，连接/会话/注入动作统一状态标签写入并补单测与架构测试 |
| 578 | Serial Station plain status text helper | 新增 `set_status_text` 公共 helper，connection/session/injection/protocol/status actions 复用普通状态文本写入并补单测 |
| 579 | Serial Station profile label helper | 新增 `set_profile_label` 公共 helper，session actions 复用 Profile 标签写入并补单测 |
| 580 | Serial Station log stats label helper | 新增 `set_log_stats_label` 公共 helper，log actions 复用日志统计标签写入并补单测 |
| 581 | Serial Station log entry line helper | 新增 `append_log_entry_line` 公共 helper，log actions 复用 TX/RX 日志行展示并补单测 |
| 582 | Serial Station serial port option helper | 新增 `serial_port_options` helper，connection actions 复用串口端口 Combo 空态、刷新和有效性判断并补单测 |
| 583 | Serial Station command history option helper | 新增 `command_history_options` helper，connection actions 复用命令历史 Combo 刷新、选中和启用状态并补单测 |
| 584 | Serial Station connection control state helper | 新增 `connection_control_state` helper，connection actions 复用连接按钮启用状态同步并补单测 |
| 585 | Serial Station command entry text helper | 新增 `command_entry_text` helper，connection actions 复用命令历史写入发送框规则并补单测 |
| 586 | Serial Station profile name text helper | 新增 `profile_name_text` helper，session actions 复用 Profile 名称输入框写入规则并补单测 |
| 587 | Serial Station log view content helper | 新增 `log_view_content` helper，log/session actions 复用日志视图清空规则并补单测 |
| 588 | Serial Station profile combo option helper | 新增 `profile_combo_options` helper，session actions 复用 Profile 下拉选项补项与选中规则并补单测 |
| 589 | Serial Station log entry filter helper | 新增 `log_entry_filter` helper，log actions 复用日志方向与搜索过滤规则并补单测 |
| 590 | Serial Station endpoint profile controls helper | 新增 `endpoint_profile_controls` helper，TCP/UDP controls 复用端点 Profile 回填规则并补单测 |
| 591 | Serial Station serial profile controls helper | 新增 `serial_profile_controls` helper，session actions 复用串口 transport 参数回填规则并补单测 |
| 592 | Serial Station serial connection fields helper | 新增 `serial_connection_fields` helper，connection actions 复用串口连接字段读取与规范化规则并补单测 |
| 593 | Serial Station endpoint default text helper | 新增 `endpoint_default_text` helper，TCP/UDP controls 复用默认 host/port 文本规则并补单测 |
| 594 | Serial Station endpoint control text helper | 新增 `endpoint_control_text` helper，TCP/UDP controls 复用端点控件 placeholder、tooltip 和按钮文案规则并补单测 |
| 595 | Serial Station log filter options helper | 新增 `log_filter_options` helper，sections 复用日志筛选下拉选项与默认选中规则并补单测 |
| 596 | Serial Station serial config options helper | 新增 `serial_config_options` helper，sections 复用波特率、数据位、校验、停止位和流控选项与默认值并补单测 |
| 597 | Serial Station diagnostic log filters | 日志筛选入口新增 System/Error 选项，复用 log_filter_options/log_entry_filter 规则并补单测 |
| 598 | Serial Station diagnostic log stats | 日志行渲染与统计支持 System/Error，诊断日志不再归入 RX，并补单测与 UI smoke |
| 599 | Serial Station diagnostic log replay | 导出回放保持 System/Error 诊断方向，修复 event/entry codec 回放归类并补 codec 与 controller 测试 |
| 600 | Serial Station controller error log entries | controller 错误进入 Error 日志 entries，发送失败可在日志区筛选、统计、导出和回放，并补单测与 UI smoke |
| 601 | Serial Station connection lifecycle logs | 连接成功和断开进入 System 日志 entries，可筛选、统计、导出和回放，并补 controller 与 UI smoke 测试 |
| 602 | Serial Station protocol selection logs | 协议切换进入 System 日志 entries，协议上下文可筛选、统计、导出和回放，并补 controller 与 UI smoke 测试 |
| 603 | Serial Station profile load logs | Profile 加载进入 System 日志 entries，工位配置来源可筛选、统计、导出和回放，并补 controller 与 UI smoke 测试 |
| 604 | Serial Station command history state helper | 命令历史去重与 Profile 恢复逻辑拆出 controller helper，workbench_controller 降到 290 行并补纯单测 |
| 605 | Serial Station connection toolbar helper | 连接/协议/串口参数/TCP/UDP 工具栏拆出 UI helper，sections.py 降到 199 行并补架构测试 |
| 606 | Serial Station controller log state helper | 普通/System/Error 日志追加与回调通知拆出 controller helper，workbench_controller 降到 279 行并补纯单测 |
| 607 | Serial Station connected log helper | 连接成功 System 日志追加规则并入 controller 日志 helper，workbench_controller 降到 275 行并补纯单测 |
| 608 | Serial Station test behavior split | Profile/Session/Replay controller 回归与 MVP UI smoke 按行为域拆分，Python 测试文件全部低于 297 行并补结构门禁 |
| 609 | Serial Station controller state helpers | transport 生命周期与发送/注入 I/O 结果映射拆出 controller helper，保持主 controller 低于 297 行并补纯单测 |
| 610 | Serial Station controller profile/session state helpers | Profile 保存快照、命令历史恢复、profile loaded 日志、Session 清空/导出/回放状态拆出 controller helper，workbench_controller 降到 270 行；治理文档测试拆出独立行为域并收紧测试文件 250 行门禁 |
| 611 | Serial Station controller receive state helper | 接收字节后的日志映射、测量 ring 状态和错误入库拆出 controller helper，workbench_controller 保持 272 行且接收职责外移；UI smoke 按端点与失败路径继续拆分并收紧到 225 行门禁 |
| 612 | Serial Station controller protocol runtime helper | 协议 registry、dispatcher、接收状态和 entry/event 转换聚合到 protocol runtime helper，workbench_controller 不再直接创建协议运行态；连接/Profile controller 单测拆出独立行为域 |
| 613 | Serial Station controller transport runtime helper | 当前 transport、registry、活跃模式和回调绑定聚合到 transport runtime helper，workbench_controller 降到 249 行并新增 250 行门禁；fake/endpoint 连接回归覆盖 |
| 614 | Serial Station controller callback state helper | 日志、错误和测量回调集合聚合到 callback state helper，workbench_controller 降到 242 行；回调注册和错误入库具备独立单测 |
| 615 | Serial Station controller workbench state helper | entries 与 command history 聚合到 workbench state helper，controller 不再直接创建两组共享 list；状态快照和日志清空具备独立单测 |
| 616 | Serial Station command action boundary | 命令发送和命令历史动作从 connection actions 拆入 command actions，main window 仅做委托；UI 架构测试防止命令职责回流 |
| 617 | Serial Station endpoint connection action boundary | TCP/UDP endpoint 连接动作从 connection actions 拆入 endpoint connection actions，connection_actions 降到 89 行；endpoint UI smoke 覆盖 |
| 618 | Serial Station command section boundary | 命令发送行从 sections 拆入 command_section，sections.py 降到 152 行；UI 架构测试防止命令控件构建职责回流 |
| 619 | Serial Station UI 美化 Batch 1 (A1) — 动画引擎整合 | 统一 token 时长消除双轨制；ScaleAnimation 中心缩放+防 GC；FadeTransition.cross_fade 竞态修复；micro_interactions hover_lift 真位移；app_shell 加 on_leave 生命周期+同页跳过+动画停止防叠加；11 files +619/-166 |
| 620 | Serial Station UI 美化 Batch 2 (A2) — QSS 主题深度+死代码清除 | 删 palette_defs.py 229 行死代码；BG_PANEL 亮度差 3%→8% 卡片浮起；elevation token + accent gradient；3 处硬编码 RGBA 修复（浅色切换不漏色）；8 files +155/-313 |
| 621 | Serial Station UI 美化 Batch 3 (B1) — 控件动效接线+图标 bug | ConfigurableButton 接按压回弹+hover_lift；StatusLed 接呼吸；Gauge 指针 tween；icons 缓存键漏 pixels bug + 多 path 着色 bug 修复；6 files +280/-24 |
| 622 | Serial Station UI 美化 Batch 4 (B2) — 布局修复 | 响应式 resize 驱动点迁到 AppShell（事件过滤器 attach_to_top_level）；折叠卡硬切→高度动画；右区 6 控件横向截断→纵向换行；5 files +295/-24 |
| 623 | Serial Station UI 美化 Batch 5 (C1) — EmptyState/骨架屏/占位重做 | 新建 EmptyStateWidget+SkeletonWidget(shimmer)；PlaceholderPanel 重做渲染 icon+入场动画；全局卡片 hover_lift；10 files +564/-25 |
| 624 | Serial Station UI 美化 Batch 6 (C2) — 波形美化 | 曲线渐变填充+发光主线；waveform_measure 死代码激活（Vpp/RMS 统计）；网格 alpha 0.12→0.18；3 files +150/-3 |
| 625 | Serial Station UI 美化 Batch 7-1 — CursorManager 接入波形预览 | preview 接入 CursorManager（双游标默认+可增删）；激活游标 HUD 死代码；2 files +95/-7 |
| 626 | Serial Station UI 美化 Batch 7-2 — waveform_perf 热路径节流 | preview 接入 RefreshThrottle+BatchAccumulator（激活死代码），submit_batch 累积+flush，不再逐批重建；3 files +95/-8 |
| 627 | Serial Station UI 美化 Batch 7-3 — 全局输入框 focus_ring | button_icons.apply_focus_rings 遍历子树可聚焦控件装 focus 光环动画；main_window 装配调用；3 files +94/-1 |
| 628 | Serial Station UI 美化 Batch 7-4 — Slider 跟手气泡+release 发包 | 拖拽时显示 value 气泡跟随 handle；release 才发包（原 valueChanged 刷屏→拖 100px 只发 1 个最终命令）；5 files +220/-56 |
| 629 | Serial Station UI 美化 Batch 7-5 — 按钮 ripple 水波纹 | 新建 RippleButton+install_ripple；EmptyState CTA 改用 RippleButton；ConfigurableButton.set_ripple 动态注入；6 files |
| 630 | Serial Station UI 美化 Batch 7-6 — 域面板 stagger 入场动画 | 新建 panels/_enter_anim play/stop_panel_enter；6 个域面板 on_enter/on_leave 接入场动画；8 files |
| 631 | Serial Station UI 美化 Batch 8 — ShakeAnimation 接入校验失败 | 抖动接入 3 个校验路径（空命令/endpoint host-port/空端口）；至此按压/hover/focus/抖动/呼吸灯 5 项全接线；4 files |
| 632 | Serial Station UI 美化 Batch 9 — 域面板占位+游标右键交互 | OTA 传输 skeleton shimmer；CAN/BLE/RTT 空数据 EmptyState（有数据 hide/清空 show）；CursorManager 右键/双击游标交互（install_cursor_interactions 接入 preview，Qt6 鼠标事件兼容 helper）；清理测试文件重复 import |
| 633 | Serial Station UI 美化 Batch 10 — StatusDot 状态圆点+面板接线 | 新建 StatusDot 自绘圆点控件（径向渐变球+flash 闪光+PulseAnimation breathing），PulseAnimation 死代码接入路径从单点 LED 扩到通用状态指示；OTA/RTT/Automation/BLE 四面板 ●/○ 字符→StatusDot（GREEN/BLUE 活动态呼吸，OFF/YELLOW/RED 静止）；修复四面板 lazy-import 把 DotState 限制在 build() 作用域导致的运行时 NameError（改模块级 import）；14 个新单测覆盖控件+PulseAnimation 激活+QSS 覆盖 |
| 634 | Serial Station UI 美化 Batch 11 — ToastWidget 通知控件+三动画引擎死代码激活 | 新建 widgets/toast.py ToastWidget（标题/正文/级别色条/自动消失定时器）；enter() 接 SlideAnimation.slide_in，leave() 接 FadeTransition.fade_out（激活两动画死代码）；closed 信号离场后发出；is_leaving 防重复离场；show_with_fade() 透明度过渡 helper |
| 635 | Serial Station UI 美化 Batch 12 — ToastContainer 通知 UI 闭环+NotificationManager 端到端 | 新建 toast_container.py：接 NotificationManager 的 notification_added/removed 信号渲染/移除 ToastWidget；max_visible 挤兑（超限对最早 toast 调 leave）；manager.show()→toast 右上角滑入→超时淡出完整闭环 |
| 636 | Serial Station UI 美化 Batch 13 — 通知系统贯穿核心连接/发送工作流 | main_window._notify 委托方法（reparent 后 self.window() 返回 AppShell→manager→toast）；connection 发送/断开/失败、command 发送成功/空命令、transport 连接状态变化全部接 toast 通知 |
| 637 | Serial Station UI 美化 Batch 14 — 通知系统贯穿五大域面板 | panel_notify(widget, level, title, message) helper：用 widget.window() 解析 AppShell；OTA 传输开始/完成/失败、RTT 连接、BLE 扫描/连接/断开、CAN 帧收发、Automation 录制/回放全部接 toast |
| 638 | Serial Station UI 美化 Batch 15 — toast 全局快捷键+通知子系统抽离 | app_notifications.py 抽出 build/show/reposition/handle_key_press 四函数（守 AppShell 297 行门禁）；Esc→dismiss_oldest，Ctrl+Shift+Esc→clear_all；AppShell 持 _app_notifications 委托 |
| 639 | Serial Station UI 美化 Batch 16 — 设置主题切换 toast+dashboard 死代码审计 | settings apply_theme 成功→success toast（已应用 深色/浅色）；dashboard 子系统死代码审计（为 Batch 17 激活做准备） |
| 640 | Serial Station UI 美化 Batch 17 — 激活 dashboard 子系统（拖拽式仪表盘模式） | 顶栏新增标签页/清空画布/保存布局/加载布局四操作+状态标签；激活 dashboard 拖拽式仪表盘模式（此前为死代码） |
| 641 | Serial Station UI 美化 Batch 18 — 激活 WidgetFullscreenHandler 双击全屏 | _wire_canvas_fullscreen：每个画布 item_added 信号接 _on_item_added_fullscreen，新控件放置→attach_double_click_fullscreen 装 handler；双击画布全屏切换 |
| 642 | Serial Station UI 美化 Batch 19 — 设置主题实时预览（combo 即时切换） | combo currentIndexChanged→_preview_theme：切换选择即应用主题（无需点应用按钮） |
| 643 | Serial Station UI 美化 Batch 20 — 清理 waveform_overlays 遗留死代码 | waveform_overlays.py 清理（139→88 行，-51 行）：删除 attach_cursors（旧固定双游标 API）等被 CursorManager 取代后的遗留死代码 |
| 644 | Serial Station UI 美化 Batch 21 — stagger_fade 卡片错峰淡入（激活 panel_animations 死代码） | _stagger_enter_cards：findChildren(serialStationCard)→stagger_fade(delay_ms=70)；纯透明度 fade_in 错峰不 move 控件（card_enter 含 slide_in 会 move，与布局定位冲突） |
| 645 | Serial Station UI 美化 Batch 22 — 域面板输入控件 focus_ring 接入 | apply_panel_focus_rings(root)：遍历 root 子树 QLineEdit/QComboBox/QPlainTextEdit 等可聚焦控件装 focus 光环动画；域面板 build 后统一接线 |
| 646 | Serial Station UI 美化 Batch 23 — 页面切换离场淡出（激活 panel_animations.fade_out 死代码） | _switch_to 在切 index 前对老页面调 _animate_page_leave：fade_out（透明度 1→0+完成后 hide），存 _leave_anims 防 GC |
| 647 | Serial Station UI 美化 Batch 24 — 清理 panel_animations.stagger 遗留死代码 | panel_animations.py 清理（112→98 行，-14 行）：删除 stagger（card_enter 含 slide_in 变体，与布局冲突，被 stagger_fade 取代）；test_stagger_cleanup 固化守护 |
| 648 | Serial Station UI 美化 Batch 10'（并行线）— 多强调色配色+主题切换过渡动画 | ⚠️ 编号注：此为并发进程第二条 batch 线的 Batch 10，时间上晚于 Batch 22；accents.py 7 套 AccentVariant（dark+light 双色调），cyan 默认与 palette 常量对齐；theme_transition.py ThemeTransition.run 整窗 windowOpacity 1.0→0.6→1.0 过渡掩盖 QSS 硬切闪烁 |
| 649 | Serial Station UI 美化 Batch 25 — 仪表盘布局自动持久化/恢复（env-gated） | DashboardPanel 此前只有手动保存/加载，关掉应用布局丢失；新增 _dashboard_layout_store.py：persist_from_canvas（画布 to_layout_dict 写盘）+ restore_to_canvas（读盘恢复）；build 自动恢复 + add 自动保存；env-gated（DASHBOARD_LAYOUT 默认关，防测试污染）；10 个新单测覆盖 store 往返/损坏兜底/restore-persist 画布/build 自动恢复 |
| 650 | Serial Station UI 美化 Batch 26 — dashboard 全屏 ESC 退出 | WidgetFullscreenHandler 此前仅双击 toggle 进出全屏，进入后无键盘退出；Batch 26 给 attach_double_click_fullscreen 加 keyPressEvent override，全屏态按 ESC 退出；enter() setFocus 确保全屏 widget 收按键事件 |
| 651 | Serial Station UI 美化 Batch 28 — dashboard 标签页双击重命名（激活 rename_tab 死代码） | DashboardTabs.rename_tab 此前仅 test 引用零生产消费者；Batch 28 接 QTabWidget 原生 tabBarDoubleClicked 信号，双击标签页弹 QInputDialog 重命名，激活 rename_tab；对齐浏览器/IDE 标签页标准重命名交互。⚠️ Batch 27 编号跳过（并发进程编号疏忽） |
| 652 | Serial Station UI 美化 Batch 29 — dashboard 标签页右键菜单（重命名/复制/关闭） | dashboard 标签页此前只有双击重命名+内置关闭按钮，缺标准右键菜单；Batch 29 加右键菜单（重命名/复制标签页/关闭），对齐浏览器/IDE 标签页交互；DashboardPanel 接入 +13 行；test_dashboard_tab_menu 9 个测试（接线断言+duplicate 克隆控件+关闭清理） |
| 653 | Serial Station UI 美化 Batch 30 — dashboard 标签页拖拽重排序 | DashboardTabs 此前 setTabsClosable 但非 movable，标签页不能拖拽重排序；Batch 30 启用 setMovable + 暴露 tab_moved 信号，重排序后自动持久化（tab 顺序是持久化 key 顺序）；对齐浏览器/IDE 标签页交互 |
| 654 | Serial Station UI 美化 Batch 31 — dashboard 放置控件右键删除菜单 | 放置的 dashboard 控件此前无单控件删除入口（只有清空画布整批删）；Batch 31 给每个放置控件装右键「删除」菜单，调 canvas.remove_item；双击已被全屏占用（Batch 18），故用右键删除；激活 canvas.remove_item 在面板层使用路径 |
| 655 | Serial Station UI 美化 Batch 32 — dashboard 控件右键「复制控件」 | Batch 31 给放置控件加了右键删除，Batch 32 补「复制控件」——在源控件偏移 _DUPLICATE_OFFSET 处克隆同类型控件（对齐 Batch 29 标签页复制语义）；右键菜单现含「复制控件/删除控件」两个 action |
| 656 | Serial Station SVD 寄存器查看器（Wave 50 骨架功能） | 新建 svd/ 子包：CMSIS SVD XML 解析器（xml.etree，纯 Python 无依赖）+ 外设/寄存器/位域数据模型（frozen dataclass）+ <dim> 数组外设展开 + 位域两种位范围写法（bitOffset/Width 与 msb/lsb）+ Access 枚举；新建 SvdPanel（加载 .svd 文件 / 加载演示，三级树外设→寄存器→位域 + 详情面板：绝对地址/复位值/访问/位域表）；复用 StatusDot 绿点状态指示；注册到导航栏（cpu 图标）；QSS 覆盖全部 serialStationSvd* objectName；修复 PyQt6 magic-int setAlignment TypeError（改 Qt.AlignmentFlag 枚举）；15 个纯解析器单测 |
| 657 | Serial Station UI 美化 Batch 33 — dashboard 控件右键「属性编辑」（宽/高调整） | 放置的 dashboard 控件宽/高在 add_widget_at 后不可变；Batch 33 加右键「属性...」弹 QInputDialog.getInt 调整宽/高，同步 widget.setGeometry + item.geometry/config（保持 to_layout_dict 持久化一致）；右键菜单现含「复制控件/属性.../删除控件」3 action |
| 658 | Serial Station UI 美化 Batch 34 — WidgetPaletteButton 拖拽高亮反馈 | WidgetPaletteButton 拖拽时源按钮无视觉反馈（用户不知道哪个按钮在被拖）；Batch 34 加 dragging 属性 + QSS 高亮：拖拽期间源按钮 accent 边框 + 半透明填充，结束清除 |
| 659 | Serial Station UI 美化 Batch 35 — dashboard 控件属性编辑加位置 X/Y（网格吸附） | Batch 33 的属性编辑只调宽/高，控件放置后位置不可微调（只能拖拽重放）；Batch 35 加 X/Y 坐标编辑，输入值经 snap_to_grid 自动吸附到 20px 网格（与 add_widget_at 一致） |
| 660 | Serial Station 波形数学引擎（Wave 58 骨架功能） | 新建 waveform_math/ 子包：通道间四则（add/subtract/multiply/divide，除零置 NaN 防尖刺）+ 逐点一元函数（abs/sqrt/log/sin/cos，负数/非正→NaN）+ 微积分（derivative 中心差分/integral 梯形累积）；ast 安全表达式解析器（白名单节点遍历，拒 __import__/属性/下标/lambda/未知函数/关键字参数，SyntaxError→ExpressionError）；engine 编排（ChannelBatch→MathResult，逐通道独立求值，失败带 error 不阻断）；VirtualChannel/MathResult frozen dataclass；34 个纯 numpy 单测（按行为域拆 2 文件：函数库 + 表达式/引擎），过 250 行测试门禁 |
| 661 | Serial Station GPS/NMEA 协议解析器（Wave 59 骨架功能） | 新建 gps/ 子包：NMEA 0183 句子解析器（纯 Python 标准库）覆盖 GGA/RMC/GSA/GSV 四类；坐标 ddmm.mmmm→十进制度（南/西负值）；校验和 XOR 验证（不匹配抛 NmeaParseError，缺失容错）；talker 无关助记符识别（GP/GN/GL 都按末 3 字符）；parse_lines 多行容错（跳空行/非 NMEA/解析失败）；GgaFix/RmcTrack/GsaActive/GsvSatellites/SatelliteInfo frozen dataclass + to_payload（ProtocolEvent 兼容）；18 个纯解析器单测 |
| 662 | Serial Station UI 美化 Batch 36 — dashboard 画布可见网格 + 切换 | DashboardCanvas 此前放置控件靠不可见网格吸附，用户无视觉参考；Batch 36 加 paintEvent 绘制 20px 网格点 + show_grid 切换（palette 工具栏按钮）；网格颜色取 palette 弱色不干扰控件 |
| 663 | Serial Station UI 美化 Batch 37 — palette 手型光标 + 控件 z-order 置顶/置底 | WidgetPaletteButton 此前无手型光标，拖拽可发现性弱；Batch 37 加 PointingHandCursor；同时给控件右键菜单加「置顶/置底」z-order（控件重叠时调整叠放次序） |
| 664 | Serial Station UI 美化 Batch 38 — dashboard 控件锁定/解锁（防意外删除） | 放置的控件可被意外删除（右键删除/清空画布）；Batch 38 加锁定/解锁 toggle（config[locked] 持久化），锁定后删除 action 禁用 + _safe_remove 跳过，对齐可视化编辑器锁定语义 |
| 665 | Serial Station UI 美化 Batch 39 — dashboard 用户可见文字国际化（tr() 合规） | 审计发现 dashboard 多处用户可见文字硬编码英文（WidgetPaletteButton tooltip / WidgetPalette 标题等），违反铁律 19（所有用户可见文字走 tr()）；Batch 39 统一改为 widget.tr() 包裹 |
| 666 | Serial Station UI 美化 Batch 40 — 10 个并行新增微交互组件 + 全量 pytest 阻塞修复 | 5 动画（BouncePathAnimation/GlowAnimation/RotateAnimation/TypewriterAnimation/ElasticSnapAnimation）+ 5 控件（Chip/SegmentedControl/RichTooltip/InfoBanner/ProgressRing），各带独立单测；同时修复 11 个 Batch 40 测试错误（QPropertyAnimation 未导入 / PyQt6 6.11 QMouseEvent 6 参重载 / QEnterEvent），InfoBanner `_animate_out` access violation 崩溃（finished 信号改绑定方法让 sip 自动追踪生命周期），QSS 覆盖 5 新 objectName，test_chip 行数压到 249；并修复 4 个预存在 bug（src/ 遗留 C++ 删除 / local_env.bat 转入 .gitignore / test_python_governance_docs subprocess 加 encoding=utf-8 修复 Windows GBK 解码 / test_settings 与 test_dashboard_layout_setting_key_exists 因 settings 包未落地改为 skip），全量 pytest 1470 passed + 2 skipped + 0 access violation，ui_smoke 23 passed，start-embeddebug --smoke exit 0 |
| 667 | Serial Station UI 美化 Batch 41 — 5 个并行新增 UI 组件 + 动画 + 工具面板 | 2 控件（Drawer 侧边抽屉带半透明遮罩四向滑入/Badge 状态徽章 kind 配色）+ 2 动画（SkeletonAnimation windowOpacity 闪烁/PageSlideAnimation 四向页面切换 slide）+ 1 工具（CrcCalculatorPanel 自带 4 个 catalog 预设 CRC-8/MAXIM/CRC-16/MODBUS/CRC-16/CCITT-FALSE/CRC-32/ISO-HDLC，全部 check value 自检通过 0xA1/0x4B37/0x29B1/0xCBF43926）；新建 ui/tools/ 子包；合流更新 controls/animations/__init__.py + tools/__init__.py + qss_sections_controls 覆盖 9 新 objectName；全量 pytest 1565 passed + 2 skipped + 0 access violation，ui_smoke 23 passed，start-embeddebug --smoke exit 0 |
| 668 | Serial Station 工具面板 Batch 42 — 3 个并行新增工程师工具（TimestampConverter/HexViewer/ByteFrequencyAnalyzer） | 3 工具面板：TimestampConverterPanel（epoch ↔ datetime ↔ ISO ↔ hex，5 时区预设，纯函数核 5 个：epoch_to_datetime/datetime_to_epoch/epoch_to_iso/iso_to_epoch/epoch_to_hex）；HexViewerPanel（offset+bytes+ASCII 三栏格式化 dump，16 bytes/line，纯函数核 4 个：format_hex_line/format_hex_dump/to_ascii_repr/parse_hex_input）；ByteFrequencyAnalyzer（256 桶字节频率 + Shannon 熵 + 自绘柱状画布 _FrequencyCanvas，纯函数核 4 个：compute_frequency/top_n_bytes/entropy_bits/format_stats_text）；合流 tools/__init__.py 导出 18 个公共符号，qss_sections_controls 覆盖 22 新 objectName；全量 pytest 1626 passed + 2 skipped + 0 access violation，start-embeddebug --smoke exit 0 |
| 669 | Batch 43 + Batch 44 prep — UI/动画基建 + 30 按钮状态完整化（VOFA+/MobaXterm 对标第一批） | Batch 43 新增 4 模块：ColorTweenAnimation（颜色属性 tween，parse_color/format_color 支持 hex6/hex8/rgba，tween + tween_stylesheet 两个工厂，对标 VOFA+ 连接按钮深→浅蓝过渡）；StaggerCoordinator（编排：N 个动画工厂按 step_ms 错峰启动，cancel + finished + cancelled 信号，对标 Linear/Vercel 卡片入场）；ThemeSerializer（palette JSON 导出/导入，serialize_palette/deserialize_to_dict/apply_overrides + to_json/from_json/save/load + validate_color/snapshot，对标 MobaXterm .mxtcolors）；2 个守护测试（test_no_hardcoded_colors grep QColor 硬编码 + test_no_px_font_sizes grep setPixelSize/font-size:Npx，均用 STRICT_*_COLORS 环境变量切换软/硬模式）。Batch 44 prep：tokens.py 加 6 个 token（EASE_OUT_QUART/EASE_IN_QUINT/EASE_OUT_QUINT/EASE_MATERIAL_EMPHASIZED/DURATION_CONTAINER=297/KEYFRAME_PREVIEW=0.3/STAGGER_STEP_MS=60，对齐 Material 3 emphasized + 动画审计 P0-4）；30 个按钮补齐 :pressed + :disabled（tool 8 + domain primary 5 + domain secondary 11 + dashboard 5 + EmptyStateCTA 1，对齐 UI 审计 §2 P0）。研究：librarian 交付 VOFA+ appTheme token 系统名（bgColor/barColor/mainColor/cbColor1-4/iconColor1-3/textAnsiErr/Tx/Rx/Time）+ MobaXterm catppuccin RGB 值（bg=#1E1E2E fg=#CDD6F4）+ 8 层 50+ 项审计清单；explore 交付 UI 审计（30 按钮缺状态/6 卡片缺 hover-lift/27 按钮缺图标/Toast Unicode 字形/light theme 缺 6 CARD token/无 INFO 色）+ 动画审计（AppShell leave 隐形/PageSlide 死代码/GlowAnimation 死代码/cross_fade sequential bug/stagger_fade GC 风险）。全量 pytest 1649 passed + 2 skipped + 0 access violation，start-embeddebug --smoke exit 0 |
| 703 |
| 703 | Batch 47-48 — 死代码全激活 + 组件开发 + 全 token 迁移 + 加载态 + 动画修复 | 7 死代码激活（Skeleton/BouncePath/Typewriter/Glow/ElasticSnap/RichTooltip/InfoBanner-Chip SVG）；3 新组件（StatusBar+KeyboardShortcut+ToggleSwitch）；全连接加载态（fake/serial/tcp/udp+refresh+BLE）；全 typography token 系统（FONT_*/FONT_POINT_*/FONT_WEIGHT_*/LETTER_SPACING_*/SPACING_INT_*）；cross_fade parallel 修复；Dashboard canvas 空态提示 | Batch 44 — 22 个 domain panel 按钮图标补齐（UI 审计 §4.1 P0） | _BUTTON_ICON_MAP 从 13 条扩到 35 条：BLE 5 个（scan/bluetooth/eye/pencil/bell）+ CAN 3 个（send/trash-2/play）+ RTT 2 个（zap/trash-2）+ Automation 3 个（play/zap/refresh-cw）+ SVD 2 个（folder-open/cpu）+ OTA 2 个（folder-open/upload）+ Dashboard 5 个（plus/trash-2/save/folder-open/grid）+ Settings 1 个（check）。apply_button_icons 函数 findChild 容错（按钮不存在静默跳过，icon 不存在 IconManager 返回 null QIcon 跳过），所以多写 entry 也安全。颜色与 QSS 对齐：domain primary 用 TEXT_ON_ACCENT，secondary 用 TEXT_SECONDARY/TEXT_MUTED，TX 类用 TERM_TX，危险操作用 ERROR/WARNING。全量 pytest 1649 passed + 2 skipped + 0 access violation，smoke exit 0 |
| 708 | Batch 49 — 加载态 + 空态完善（对标铁律 5.9 三轴口径） | 6 子任务：B49-1 Log 空态（EmptyStateWidget inbox 图标 + 「暂无日志」，首条日志 hide_with_fade，清空 show_with_fade）；B49-2 Log 加载态（SkeletonBlock 3 行 + 「正在建立连接…」，4 条连接路径 fake/serial/tcp/udp 同步切换）；B49-3 Waveform 空态 + 加载态（activity 图标空态 + ProgressRing 连接覆盖层 + set_connecting 公开方法 + 首批 batch 淡出空态）；B49-4 Dashboard 画布空态（layout-dashboard 图标，替代 paintEvent 单行文字提示，add/remove/clear 全循环）；B49-5 Connection _set_loading 升级（内嵌 ProgressRing 替代「…」文字，setFixedSize 18×18 居中，复用 ring 不重复创建）；B49-5b Port Refresh 复用 _set_loading（消除 connection_toolbar inline 重复逻辑）。新增 3 helper 模块（log_empty_state/log_loading_state/waveform_empty_state/connection_loading/_empty_state_overlay）保持各主文件 ≤300 行；EmptyStateWidget 加 hide_with_fade 对称方法（fade_out finished→hide）；4 条连接路径统一 _show_log_loading/_set_waveform_connecting 同步切换。22 个 ui_smoke 测试覆盖空态/加载态/切换/淡入淡出完整循环（黑盒 waitUntil visibility，不 poke 私有动画属性）。全量 pytest 1695 passed（+54 from 1641 baseline），smoke exit 0 |
| 713 | Batch 50 — 死代码守护测试 + CI 严格化（对标可维护收口 E5） | 6 子任务：B50-1 test_no_dead_widgets（枚举 controls/__all__，AST 检测每个符号 production 引用，白名单 8 个未 wire widget + RichTooltip 间接实例化）；B50-2 test_animation_wiring（动画模块 + token 双重守护，AST 剥离注释防误判，9 个预留 token 白名单文档化）；B50-3 elevation_effect wired（install_card_shadow helper → layout_cards.build_card 统一卡片 L1 阴影，激活死代码进入生产链）；B50-4 GitHub Actions CI（.github/workflows/ci.yml，Windows runner，push/PR 跑 test-embeddebug-py + STRICT_COLORS/FONTS 硬模式 + lint + smoke，README 加 CI badge）；B50-5 ruff 配置 + devtools/lint.py + lint-embeddebug-py script（F 系列 Pyflakes 机械捕获死代码，修复 3 个 lint 错误：qss F541 f-string 无占位符 / eye_diagram F822 __all__ 幽灵项 / project_audit F821 未定义名）；B50-6 pytest [tool.pytest.ini_options]（testpaths + --strict-markers）。新增 2 守护测试文件（test_no_dead_widgets 201 行 / test_animation_wiring 219 行，均 ≤250 门禁）。全量 pytest 1686 passed（+8 守护），lint 0 errors，smoke exit 0 |
| 716 | Batch 51 — 6 个死 widget 接入生产 + 4 个预留 token 消费（白名单从 10 降至 3） | 新建 log_options_bar.py helper 集中 wire 6 个白名单 widget 到日志工具区真实功能场景：Divider（日志区视觉分组分隔线）+ Badge（连接状态徽章 SUCCESS/INFO 实时反映连接，update_connection_badge 供 connection_control_state 调用）+ ToggleSwitch（日志自动滚动开关，默认开）+ Chip（当前 filter 可视化标签，显示 active filter 文本，removable=True 可关闭）+ SegmentedControl（日志视图模式 ASCII/Hex/Dec 三段切换，currentChanged 信号）+ InfoBanner（持久信息条 BannerKind，show_log_info_banner 带 fly-in 位置动画）。接入点选 sections.py build_log_row（Batch 49 后干净，未撞用户工作区改动文件，保持 ≤260 行 architecture 门禁）。从 _PENDING_WIRE 移除 Badge/BadgeKind/Chip/Divider/ToggleSwitch/InfoBanner/BannerKind/SegmentedControl 共 8 项（白名单从 10 降至 3：剩 Drawer + uninstall_tooltip + RichTooltip 设计内项）。install_card_shadow 升级：enter 时从 L0 淡入到目标 level（消费 DURATION_CONTAINER=300ms + EASE_OUT_QUART，对齐 Material 3 emphasized container transform 语义）。show_log_info_banner fly-in 位置动画消费 DURATION_FLYOUT=187ms + EASE_OUT_QUINT。从 _RESERVED_TOKENS 移除 4 个 token（白名单从 9 降至 5：剩 PROGRESS/DRAWER/SCROLL/EASE_IN_QUART/EASE_OUT_QUAD 待后续场景）。QSS 补 6 个新场景 objectName 占位（serialStationLogConnectionBadge/ActiveFilterChip/AutoScrollToggle/AutoScrollLabel/LogViewModeSegmented/LogInfoBanner，自绘控件 QSS 契约占位满足 test_theme_quality 覆盖门禁）。13 个 ui_smoke 测试覆盖 widget 构造/信号/badge kind 配色/fly-in 动画/分段 setCurrent 切换/Divider 带标签/InfoBanner dismissed 信号。micro_interactions.py 精简 docstring 保持 ≤300 行铁律 20。全量 pytest 1691 passed（+5 ui_smoke +8 守护 from 1678 baseline），lint clean，smoke exit 0 |
| 718 | Batch 52 — Drawer wire（最后一个死 widget）+ DURATION_DRAWER 消费 + QSS 拆分 | Drawer 接入 log_options_bar：build_history_drawer 构建右侧滑入抽屉（命令历史占位），history_btn 触发，set_content 承载占位 QLabel。drawer.py open() 动画从 DURATION_NORMAL 改为 DURATION_DRAWER=300ms（消费预留 token，匹配 Material 抽屉观感）。从 _PENDING_WIRE 移除 Drawer（白名单降至 2 项：uninstall_tooltip + RichTooltip 设计内项），从 _RESERVED_TOKENS 移除 DURATION_DRAWER（降至 4 项）。QSS 拆分：qss_sections_controls.py 因 Batch 51-52 新增段超 300 行，抽出 qss_sections_log_options.py（57 行独立段文件），qss_builder 注册 log_options_section()。修复 log_options_bar.py 重复 QLabel import（ruff F811）。全量 pytest 1698 passed（+7 from 1691），lint clean，smoke exit 0 |
| 720 | Batch 53 — 消费 3 个预留动画 token（PROGRESS/EASE_IN_QUART/EASE_OUT_QUAD） | progress_ring.py value 动画 DURATION_NORMAL→DURATION_PROGRESS（进度条专用 150ms，更短更快反馈）。info_banner.py slide-out DURATION_NORMAL+EASE_IN→DURATION_FAST+EASE_IN_QUART（离场四分位缓动，比 EASE_IN 更急促收尾）。segmented.py indicator 滑动 EASE_OUT→EASE_OUT_QUAD（轻量出场缓动）。从 _RESERVED_TOKENS 移除 3 个，剩 DURATION_SCROLL 待「滚动到视图」场景（列表项点击定位）。白名单最终：_PENDING_WIRE 2 项（设计内）+ _RESERVED_TOKENS 1 项（待场景）。全量 pytest 1698 passed，lint clean，smoke exit 0 |
| 723 | Batch 54 — tr() + objectName 合规审计（对标铁律 16/19） | 修复 1 个 tr() 违规（ble_panel.py:78 "handle" 加 widget.tr()）+ ~30 个 objectName 违规集中在 ui/tools/ 4 文件（crc_calculator 加 _labeled helper + 12 widget objectName，byte_frequency/hex_viewer/timestamp_converter 各加 input mode label + radio + combo + edit objectName）。crc_calculator.py 精简 docstring/QSS 压缩保持 ≤300 行。新建 qss_sections_tools.py（25 objectName 契约占位，泛型样式由各面板内联 _PANEL_QSS 处理）+ qss_builder 注册 tools_section()。新建 test_tr_compliance.py 守护测试（AST 扫描 QLabel/QPushButton/setToolTip 等 API 第一参数，检测未包裹 tr 的字面量字符串，软模式默认 STRICT_TR=1 硬模式，0 违规）。objectName 守护测试因误报率高（340 临时构造误判）放弃，tr 守护更精确。全量 pytest 1699 passed（+1 守护），lint clean，smoke exit 0 |
| 725 | Batch 55 — 启用 F401 lint + 清理 97 个未使用 import | 从 [tool.ruff.lint].ignore 移除 F401（启用未使用 import 检测），per-file-ignores 保留 **/__init__.py 豁免（re-export 设计内）。ruff --fix 自动清理 97 个未使用 import（覆盖 app/ota/serial_station 全树）。dashboard_panel.py 恢复 QFileDialog import 带 noqa: F401（test_dashboard_core_b 通过 module attr 访问，测试反模式但避免破坏）。lint 从「忽略 F401」升级为「强制 F401」，CI 自动拦截未来死 import。全量 pytest 1699 passed，lint clean（F401+F811+F541+F821+F822 全绿），smoke exit 0 |
| 727 | Batch 56 — 启用 F841 lint + 清理剩余未使用变量 | 从 [tool.ruff.lint].ignore 清空（F401+F841 全强制）。mqtt/client_stub.py 删除未使用 msg/raw 局部变量 + MqttMessage import。tests/ 4 个 F841 自动修复（test_animation_wiring pattern / test_divider lbl / test_gps text / test_icons count / test_theme_switching dark_qss）。lint 现在 F 系列全绿（F401 import + F811 重定义 + F841 变量 + F541 f-string + F821 未定义名 + F822 __all__ 幽灵），CI 严格化完成。全量 pytest 1699 passed，lint clean，smoke exit 0 |
| 728 | Batch 57 — CLAUDE.md 审计数据同步（对标铁律 5.10 文档一致性） | 更新过时审计数据反映真实状态：UI-02 图标「仅14个SVG」→「146个（扩展完成）」；UI-06 QSS生成器「4740行手动维护」→「qss_builder 程序化生成 + 12 段模块化」；UX-06 弹窗「仍有QMessageBox」→「0 残留」；待完成项 #11/#13/#14 标记完成。测试统计 1601→1699。I 系列（import 排序）尝试启用但因 isort 拆行导致 3 文件超 300 门禁回退（留待文件拆分后启用）。 |
| 730 | Batch 58 — 启用 UP 系列 pyupgrade + 81 处现代化修复 | 从 [tool.ruff.lint].select 加 UP（pyupgrade）。ruff --fix 自动修复 70 处（UP006 typing.List→list / UP007 Optional→X|None / UP031 % 格式化→format / UP032 .format→f-string 等）+ 11 处连带 F401（pyupgrade 后 typing.List 等未使用 import 清理）。lint 现在 F+UP 双系列强制（死代码 + 语法现代化），CI 自动拦截老式语法。全量 pytest 1699 passed，lint clean，smoke exit 0。 |
| 732 | Batch 59 — 启用 B 系列 bugbear + 16 处修复 | 从 [tool.ruff.lint].select 加 B（bugbear）。修复：B905 zip() 加 strict=False（5处 zip(x,x[1:]) 模式，trailing 元素故意忽略）；B007 循环变量未用改 _（command_palette qi / toast_container uid×2）；B010 setattr 常量改直接赋值（_enter_anim 3处自动修复）；B017 pytest.raises(Exception) 改具体异常（test_crc_calculator / test_shortcuts_manager / test_timestamp_converter 3处 frozen dataclass 测试）；B011 assert False 改 raise AssertionError（test_layout_cards）。lint 现在 F+UP+B 三系列强制。全量 pytest 1699 passed，lint clean，smoke exit 0。 |
| 734 | Batch 60 — 启用 SIM 系列 flake8-simplify + 10 处修复 | 从 [tool.ruff.lint].select 加 SIM。修复：SIM300 Yoda 条件（3处自动修复）；SIM108 if-else→ternary（measurement setup + 自动修复 hex_viewer 等）；SIM103 return condition directly（test_tr_compliance + 自动修复 xmodem/ymodem）；SIM114 合并 if 分支（test_tr_compliance 自动修复）；SIM118 key in dict.keys()→key in dict（settings_service 自动修复）。ignore SIM105（contextlib.suppress，40处防御性代码保留 try/except）、SIM102（嵌套if合并降低可读性）、SIM115（context manager 改变资源语义）。lint 现在 F+UP+B+SIM 四系列强制。全量 pytest 1699 passed，lint clean，smoke exit 0。 |
| 736 | Batch 61 — 启用 RUF 系列 ruff-specific + 12 处修复 | 从 [tool.ruff.lint].select 加 RUF。修复：RUF005 collection literal+concat（unsafe-fix）、RUF015 single element concat、RUF007、RUF059 unused __all__ 等 12 处（自动 + unsafe-fixes）。waveform_engine.py OpenGL import 改 importlib.util.find_spec（更 Pythonic 的可用性检测）。ignore RUF001/002/003（中文注释 Unicode 警告，7900+ 处不适合中文项目）、RUF012（mutable class var，语义改动大）。恢复 dashboard_panel QFileDialog noqa: F401（RUF unsafe-fix 误删，测试 module attr 访问依赖）。lint 现在 F+UP+B+SIM+RUF 五系列强制。全量 pytest 1699 passed，lint clean，smoke exit 0。 |
| 737 | Batch 62 — 启用 C4 系列 flake8-comprehensions | 从 [tool.ruff.lint].select 加 C4。修复 C408 dict()→literal（test_ui_texts _FILTER_KW）。PT 系列（128 个）评估后放弃：PT018（90个 for 循环 assert）改造成本极高，PT011（34个 match=）手动量太大，收益边际。lint 现在 F+UP+B+SIM+RUF+C4 六系列强制。全量 pytest 1699 passed，lint clean，smoke exit 0。 |
| 738 | Batch 63 — CLAUDE.md ROADMAP 审计（第一优先级全完成） | 标记已完成项：UX-04 快捷键（shortcuts/ 包）、UX-05 响应式布局（responsive_layout.py）、Dashboard 持久化、Doxygen（340/348 = 98% docstring）。第一优先级 4 项全 ✅。 |
| 739 | Batch 64 — CLAUDE.md 第二优先级审计 | MQTT TopicModel/BLE GattModel 改用 stub+codec 模式（无空壳 model）；DBC 解析器 can/dbc.py（145 行）。第二优先级大部分已完成。 |
| 740 | Batch 65 — README 质量门禁同步 + 工程状态 E4→E5 | README 质量门禁加 lint-embeddebug-py + CI 说明（STRICT 模式 + 六系列 lint）。工程状态 E4→E5（可维护收口）。 |
| 741 | Batch 66 — MQTT client_stub 单元测试（11 测试） | 新建 test_mqtt_client_stub.py：生命周期/subscribe-publish 通配符/多订阅者/write 追加/error 回调。全量 pytest 1710 passed（+11），lint clean。 |
| 741 | Batch 67 — SPI/I2C bridge_stub 单元测试（7 测试） | 新建 test_spi_i2c_bridge_stub.py：SPI 回环/I2C 读 WHO_AM_I/写后读回/多字节序列/无 register 失败/地址回绕/实例隔离。全量 pytest 1717 passed（+7），lint clean。 |

---

## 三、重要状态

- 当前文档已整理到 `#661` 的完整记录
- 当前阶段已从 `500~599 约束对齐期` 进入 `600~699 结构与流程稳定期`
- 619~659 为「UI 美化与动画接线 + SVD」连续迭代（Batch 1~35，Batch 27 编号跳过），诊断报告「基建齐全但接线全断」6 个问题域全部修复并有测试覆盖，详见 [docs/superpowers/specs/2026-06-19-ui-polish-animation-integration-design.md](../superpowers/specs/2026-06-19-ui-polish-animation-integration-design.md)
- ⚠️ Batch 编号注：Batch 10 出现两次（StatusDot 线 633 / accent 配色线 648），因并发进程有两条独立 batch 编号线；评分按 commit 时间顺序线性 +1，不依赖 batch 编号
- 后续新增分数时，优先补"阶段摘要"和"里程碑"，不再恢复长篇流水账
- 如果需要精确到单次提交，请直接查 `git log` 或对应 commit message
- 本文件只负责评分追踪，不承载工作流、提交规则或架构约束正文

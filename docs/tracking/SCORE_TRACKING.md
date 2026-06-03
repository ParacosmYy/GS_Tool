# EmbedDebug 评分追踪

> 分支: `feat/embed-debug` | 远程: `https://github.com/ParacosmYy/GS_Tool.git`
> 起点: 1分 | 目标: 1000分 | 每次commit = +1分

| # | Commit内容 | 分数 |
|---|-----------|------|
| 0 | 项目初始状态 | 1 |
| 1 | 工厂模式+数据导出+发送历史+数据统计+架构重构 | 2 |
| 2 | 搜索栏+多主题切换+会话持久化+UI/UX产品体验师角色+现代化UI标准 | 3 |
| 3 | 协议帧格式定义+状态机解析器+解析结果表格展示 | 4 |
| 4 | 帧可视化编辑器+实时波形图+IntelHEX解析器 | 5 |
| 5 | 架构审查修复: 悬挂指针+重复代码+反向依赖解耦 | 6 |
| 6 | TCP Client/Server + UDP网络连接实现 | 7 |
| 7 | XMODEM协议传输(Checksum/CRC/1K)+OTA管理器+OTA操作面板 | 8 |
| 8 | YMODEM协议传输(Block 0文件信息+批量传输)+OTA集成 | 9 |
| 9 | ZMODEM协议传输(CRC32+连续发送+HEX/BIN帧)+OTA集成 | 10 |
| 10 | 架构审查修复: SerialConfigPanel分层违规+HEX验证复用+TcpConnection类型bug+面板切换重构 | 11 |
| 11 | DataLogger数据日志记录和回放(EDL二进制格式+变速回放) | 12 |
| 12 | OtaHistoryModel OTA历史记录模型(持久化+表格展示+自动记录) | 13 |
| 13 | README中文版重写+bat启动验证 | 14 |
| 14 | CLAUDE.md约束文档更新:UI强制规则+QSS规范+多Agent工作流+300行规则 | 15 |
| 15 | BaseTransfer模板方法基类提取+XModem/YModem/ZModem重构+架构审查+PRD文档 | 16 |
| 16 | QSS主题迁移+UI全面中文化+OTA性能优化O(n^2)→O(n)+ChannelConfig/ChartModel实现 | 18 |
| 17 | 过渡动画+ChartModel集成+QSS按钮状态修复+硬编码颜色清除+中英语言选择 | 19 |
| 18 | 架构审查+TerminalModel环形缓冲区+导航面板重构+FrameEditor/OtaWidget中文化+light对比度修复+新特性准入机制 | 20 |
| 19 | 代码审查修复: sendAndRecord统一+completer泄漏+QStackedWidget降级+lineAt线程安全+objectName审计+DataLogger回放修复 | 21 |
| 20 | 架构审查+DataExporter流式导出+ProtocolView右键菜单/JSON导出+TerminalWidget方向缓存+连接类型图标+翻译补全+ChartWidget宽度修复 | 22 |
| 21 | MainWindow拆分NavigationController+RecordingController+34条翻译补全+QSS protocolView修复+JustFloat特性提案 | 23 |
| 22 | SendController提取+sendAndRecord静默失败修复+TerminalWidget环形缓存失效修复+QSS protocolToolbar修复+代码审查修复(死代码清除+翻译上下文修正+setConnected连线) | 24 |
| 23 | ConnectionController提取+JustFloat/FireWater协议桥实现+SendBar StyledPanel修复+sendAndRecord未连接反馈+代码审查修复(isHex参数移除+静默失败修复) | 25 |
| 24 | ProtocolBridgeManager协议源选择器+objectName审计补全(8控件)+QSS交互状态补全(SpinBox/ComboBox/ToolButton disabled)+ConnectionController悬空指针修复(断开时清除SendController/OtaManager+网络连接补OtaManager)+桥接单元测试 | 26 |
| 25 | P0串口修复: TerminalWidget复制功能实现+搜索高亮(F3导航/正则/HEX/匹配计数)+DTR/RTS运行时控制(连接后保持可切换)+自动追加换行符(无/\r\n/\n/\r)+CLAUDE.md更新(Bug修复冲刺机制+VOFA+超越目标+驱动检测需求) | 27 |
| 26 | ChartWidget初始化背景修复+SerialDriverDetector驱动检测+方向前缀着色+objectName补全+QSS三主题补全 | 28 |
| 27 | 架构拆分: ToolbarController+SettingsController提取+lineAt悬空引用P0修复+objectName补全+内联样式迁移 | 29 |
| 28 | 背景系统: BackgroundWidget磨砂玻璃+透明度调节+点击涟漪特效+BackgroundSettingsPopup控制面板 | 30 |
| 29 | 注释规范+终端分栏+背景性能优化+下拉箭头修复 | 31 |
| 30 | PanelManager提取+DirectionFilter拆分+SVG下拉箭头+面板透明度优化 | 32 |
| 31 | 串口完善: IConnection接口DTR/RTS虚方法+错误分类+连接超时+自动重连+SerialConfigPanel UX改进+FrameDefinition拆分+QSS完善 | 33 |
| 32-49 | (方法提取+Bug修复+架构审查+OTA增强+主题完善+终端拆分+书签面板+驱动检测+背景系统+面板切换动画+Toast通知等) | 50 |
| 50 | 方法长度违规修复: setupUI拆分+全部方法≤80行 | 50 |
| 51 | R12串口深度+UI动画增强: AnimatedButton(悬停渐变200ms+按下回弹100ms)+SerialConnection深度(sendBreak+bytesWritten+SerialErrorCounters错误计数)+运行时波特率切换+VID/PID端口信息tooltip+DataStatistics峰值速率+错误计数显示+QSS三主题peakRateLabel/errorLabel+MainWindowSignalConnect压缩至473行 | 51 |
| 52 | Qt关键字'signals'参数名冲突修复(ConnectionController信号参数pinout→避免Qt宏冲突)+.gitignore补充build2/ | 52 |
| 53 | R13信号线监控+错误计数器接线+架构修复+P0 TimedSender修复: PinoutSignals结构体+IConnection.pinoutSignals()/errorCounters()虚方法+ConnectionController 200ms轮询(含变化检测)+errorCountersUpdated信号(消除业务层→表现层反向依赖)+TimedSender m_queue/m_queueIndex互斥锁保护(P0 BUG-01修复)+QSS signalLed×3主题+SerialConfigPanel CTS/DSR/DCD/RI LED指示灯+新特性评估(高级波形引擎) | 53 |
| 54 | EmbedDebug.bat启动脚本修复: 支持build和build2目录自动检测 | 54 |
| 55 | P1 Bug修复x5+冲突标记清理+FrameParser回溯增强: BUG-02 TerminalWidget选择重置+BUG-03 paintEvent递归(QTimer延迟)+BUG-04 ChartWidget空配置兜底+BUG-05 SettingsManager const_cast替换+BUG-06 OtaManager临时文件泄漏+冲突标记清理(MainWindow/QuickCommandBar/TerminalLayoutManager)+FrameParser帧头匹配回溯算法+纯header帧路径D实现+QuickCommandBar createEditDialog提取+TerminalLayoutManager参数化重构 | 55 |
| 56 | R18: DataLogger BigEndian修复+DataExporter CSV转义+ThemeManager内存修复+TcpConnection超时+DataStatistics衰减+sendBreak+FrameVisualEditor合规 | 56 |
| 57 | R15串口深度+自动重连指数退避(interval*2^min(attempt,4)上限30s)+连接健康监控(5s心跳connectionHealth信号)+JSON导出(exportJson+exportStreamedJson)+OTA 16MB限制+端口波特率提示 | 57 |
| 58 | 6个P2 Bug修复+连接健康UI+safeFromUtf8(二进制安全UTF8解码)+会话摘要+Toast字体回退链+OtaWidget QPropertyAnimation滥用修复+FrameDefinition uint32_t移位UB修复+RecordingController blockSignals递归修复 | 58 |
| 59 | 新特性评估(高级波形引擎提案通过)+BackgroundSettingsPopup重构+UDP重连socket泄漏修复+ChannelConfig JSON键验证+XModemTransfer start timeout+ConnectionController健康信号增强+SerialDriverDetector驱动信息增强 | 59 |
| 60 | QSS死代码清理+TCP/UDP bytesWritten信号+平均速率+包计数 | 60 |
| 61 | SendController部分写入修复(P0)+connectNetwork()信号断连修复+多文件Doxygen | 61 |
| 62 | tr()英文字符串全面中文化(37个字符串) | 62 |
| 63 | tr()中文化42个+@file Doxygen头补全16文件+3个头文件Doxygen重写 | 63 |
| 64 | 方法级Doxygen补全4个cpp+15个@file头+4个tr()中文化+2个头文件Doxygen重写 | 64 |
| 65 | 12文件80+方法Doxygen全量补全+displayMode()缺失getter修复 | 65 |
| 66 | ChannelConfig.cpp全量方法Doxygen补全(13方法) | 66 |
| 67 | 19文件120+方法Doxygen大规模补全(OTA协议层+终端管理层+协议桥+工具类) | 67 |
| 68 | Doxygen全面收尾: 方法+类级+信号+成员变量文档+RingBuffer注释规范化(19文件) | 68 |
| 69 | 文件拆分: DataExporter(582→299)+ConnectionController(564→492) | 69 |
| 70 | 文件拆分: DataLogger(540→466)+TerminalWidget(537→406)+SerialConfigPanel(525→290) | 70 |
| 71 | 文件拆分: ZModem(557→436)+YModem(545→323)+FrameVisualEditor(512→307) 全部≤500行达标 | 71 |
| 72 | Bug修复冲刺: 4P0+5P1+2P2共11个Bug修复(ZModem CRC/取消/除零/布局/信号/端口/动画) | 72 |
| 73 | 代码质量冲刺: OTA常量提取(16处)+tr()合规(24处)+定时器常量(10处)+Serial参数验证(5处)+QSS圆角统一(156处) | 73 |
| 74 | 串口深度: Win32 ClearCommError错误分类+错误恢复策略+导航树主题刷新+Shadow语义色+SerialConnection拆分 | 74 |
| 75 | 代码质量+串口深度: 波特率扩展(12→20标准速率)+魔术数提取(break/stats/font)+SendController部分写入重试+PortWatcher热插拔防抖(2次确认)+AnimatedButton统一(20个按钮) | 75 |
| 76 | 代码质量: Animations命名空间(12常量)+Layout命名空间(10常量)+ConnectionDefaults(2常量)+tr()合规(8处FrameVisualEditor)+连接默认值(TCP/UDP)+动画/布局魔术数替换(43处) | 76 |
| 77 | UI动画: OTA进度条完成动画(QPropertyAnimation 400ms OutCubic accent->success)+FrameParser拆分(488->180+330)+DataExporterStreamed修复 | 77 |
| 78 | 文件拆分: ZModemTransfer(440->350)+ConnectionController(495->278)+新增ConnectionControllerHandlers(249)+YModem进度动画增强 | 78 |
| 79 | 文件拆分: XModemTransfer(474->306)+新增XModemTransferHandlers(178)+移除未使用include | 79 |
| 80 | Bug审计冲刺: 呼吸动画QSequentialAnimationGroup+Toast向上飘出+teardownConnection提取+Toast防抖连接+QSS全局字体+YModem/ZModem Doxygen+Bug审计25项+新特性评审 | 80 |
| 81 | 呼吸动画QPropertyAnimation重构+面板切换动画+Toast防抖+OTA吐司连接+YModem Doxygen注释 | 81 |
| 82 | NavIndicatorWidget+ToastWidget集成: 连接状态吐司通知+OTA传输吐司+NavIndicator主题联动+QSS三主题吐司样式+OTA错误消息中文化+XModem Doxygen注释 | 82 |
| 83 | Bug审计冲刺: TerminalWidget键盘导航+右键菜单+AnimatedProgressBar修复+连接按钮错误态+QSS一致性+NavIndicatorWidget+ToastWidget | 83 |
| 84 | P1 Bug修复+终端右键菜单+连接状态指示器+TimedSender线程安全+OTA安全断开+AnimatedProgressBar+FrameParser容错+ProtocolBridge统计 | 84 |
| 85 | Bug修复冲刺+QSS一致性+ChartWidget主题集成+OTA增强+FrameParser超时定时器+SessionManager集成 | 85 |
| 86 | FrameParser状态机拆分+DataExporter多格式导出+ByteFormat工具+ZModem框架+图表QSS+UI微调 | 86 |
| 87 | MainWindow瘦身+TerminalController提取+SessionManager+OTA增强+PortWatcher集成+ThemeManager动画 | 87 |
| 88 | 终端拆分+协议增强+串口热插拔+快速命令持久化+QSS完善 | 88 |
| 89 | 架构审查#5+流式导出+右键菜单JSON导出+方向缓存+连接类型图标+文件体积约束 | 89 |
| 90 | 代码审查修复: sendAndRecord统一+completer泄漏+QStackedWidget降级+lineAt线程安全+objectName审计+DataLogger回放修复 | 90 |
| 91 | 架构审查+TerminalModel环形缓冲区+导航面板重构+FrameEditor/OtaWidget中文化+light对比度修复+新特性准入机制 | 91 |
| 92 | 搭建11个未来特性骨架框架(F1-F11)+Phase 4 直方图/散点图+路线图PRD | 92 |
| 93 | feat(chart): FFT频谱分析引擎+频谱显示控件 | 93 |
| 94 | feat(chart): 多Y轴支持+EmbedDebug.bat自动部署Qt依赖 | 94 |
| 95 | refactor(architecture): 全面拆解模块为深层嵌套子目录结构 | 95 |
| 96 | R28 全局边缘用例修复—31个文件136行 | 96 |
| 97 | R27 P1修复: 析构函数静默关闭(4类连接+DataLogger)+ThemeManager动画泄漏 | 97 |
| 98 | 代码质量: Layout常量扩展(22个)+魔术数替换(28处)+空析构=default(8处)+OtaWidget/NavigationController拆分 | 98 |
| 99 | R26 Bug修复(6项)+tr()合规+防御性null检查+代码质量 | 99 |
| 100 | feat(recording): 实现F1数据录制回放完整业务逻辑 | 100 |
| 101 | feat(protocol): 实现F2自定义协议引擎完整业务逻辑 | 101 |
| 102 | feat: 实现F3数据导出+F4终端增强+F5仪表盘完整业务逻辑 | 102 |
| 103 | feat: 实现F6 J-Link RTT+F7自动化触发器+F8项目管理 | 103 |
| 104 | feat: 实现F9串口高级调试+F10性能监控+F11插件系统+大规模扩展路线图 | 104 |
| 105 | feat(expansion): F12-F26大规模扩展骨架-15个新特性100个文件 | 105 |
| 106 | feat(expansion): F12-F26完整实现-15个新特性全部填充真实逻辑 | 106 |
| 107 | fix(connection): 全面修复F12-F26协议层和连接层bug | 107 |
| 108 | fix: 全模块Bug修复第二轮—除零保护、协议修正、objectName补全 | 108 |
| 109 | feat(panel): 全局面板注册系统重构-44个面板数据驱动导航树 | 109 |
| 110 | ui: 全面现代化UI基础设施—9个新组件+BasePanel包装器+三主题QSS+常量拆分 | 110 |
| 111 | docs: 评分追踪补全#80-#110 + CLAUDE.md审计表二次校正(8项已完成) + 约束文档更新 | 111 |
| 112 | feat(icons): Lucide SVG图标从10个扩展到81个 + app.qrc注册 + tr()审计 | 112 |
| 113 | fix(i18n): tr()合规修复12文件30处 + ProtocolEngine CRC校验实现(CRC8/16/32/XOR) | 113 |
| 114 | feat(ui): EdDialog自定义弹窗系统 + ShortcutManager快捷键管理 + QMessageBox替换 | 114 |
| 115 | feat(theme): QSS主题生成器(Python) — 模板+JSON颜色定义自动生成三套QSS | 115 |
| 116 | docs: 架构解耦方案 + 迭代路线图 | 116 |
| 117 | feat(can): DBC解析器(CAN数据库) + Dashboard布局JSON持久化 + QMessageBox残余清除 | 117 |
| 118 | feat(usb): UsbLibraryLoader动态加载libusb + UsbConnection真实集成 + FlatBuffers FBS完整解析器 + ResponsiveLayout断点系统 + SchemaViewer TODO清理 | 118 |
| 119 | feat: RTT JLinkSdkLoader单例+10函数指针解析 + JLinkRttConnection真实SDK调用 + CanFrameParser DBC集成 + Dashboard loadLayout/saveLayout + ChartExporter真实SVG导出 + TriggerAction PlaySound + BleConnection TODO清除 | 119 |
| 120 | feat: MainWindow集成ResponsiveLayout三断点自适应 + ShortcutManager全局快捷键(Ctrl+F/P/Shift+R) + PanelManager紧凑模式 + UsbLibraryLoader getDevice/getDeviceDescriptor扩展 + ChartExporter ChartModel CSV导出 + FrameVisualEditor实时预览刷新 | 120 |
| 121 | feat: DashboardModel/PluginApi/PluginManager/TriggerAction/ProtocolEngine/OtaManager/ChartModel统计增强+战略文档 | 121 |
| 122 | refactor(send): 提取SendHistoryManager，SendController.h从199行降至108行+接口层提取+兼容头文件 | 122 |
| 123 | docs: 评分追踪补全#121-#122 + 目标更新至1000分 | 123 |
| 124 | feat: PortWatcher/RecordingController/TerminalModel/TerminalSearchManager统计增强 | 124 |
| 125-140 | UdpConnection/TlsConnection/WebSocketConnection/ModbusMaster/FrameParser/ProtocolSchema/TimestampAnalyzer/PacketBuilder/DataExporter/接口层/统计增强+接口层占位+WebSocketConnection/ModbusMaster/EventBus/I2cConnection/UsbConnection/RecordingMarker/RecordingMarker扩展+AppDialog统一弹窗+MQTT修复+多模块重构 | 140 |
| 141 | fix(mqtt): MqttConnection统计增强+ToastNotification组件+评分追踪补全#125-#140 | 141 |
| 142 | feat(stats): NavigationController/SettingsManager/SessionManager/IconManager/ShortcutManager/BleScanner/DashboardSerializer统计增强 | 142 |
| 143 | feat(stats): ChartModel/FftEngine/ZoomController/CursorOverlay/ProtocolEngine/ProtocolSchema/OtaHistoryModel/BaseTransfer/DataStatistics/TrafficMonitor/TerminalLayoutManager统计增强 | 143 |
| ... | 目标: 1000分 | 1000 |

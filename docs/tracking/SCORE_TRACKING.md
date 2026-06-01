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
| ... | 目标: 1000分 | 1000 |

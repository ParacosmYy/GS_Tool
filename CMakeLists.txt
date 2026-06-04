cmake_minimum_required(VERSION 3.20)

project(EmbedDebug VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认使用Release构建（若未指定）— 确保性能优化和正确的NDEBUG断言行为
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt安装路径 - 根据实际安装位置修改
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "E:/Tool/DevEnv/Qt/6.8.3/mingw_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    SerialPort
    Charts
    Network
)

# 源文件
set(SOURCES
    src/main.cpp
    src/core/mainwindow/MainWindow.cpp
    src/core/mainwindow/MainWindowInit.cpp
    src/core/mainwindow/MainWindowConnectionEntry.cpp
    src/core/mainwindow/MainWindowSetupUI.cpp
    src/core/mainwindow/MainWindowSignalConnect.cpp
    src/core/mainwindow/MainWindowSignalConnectUI.cpp
    src/core/mainwindow/MainWindowPanelConnect.cpp
    src/core/mainwindow/MainWindowLifecycle.cpp
    src/core/connect/ConnectionController.cpp
    src/core/connect/ConnectionControllerLifecycle.cpp
    src/core/connect/ConnectionControllerQuery.cpp
    src/core/connect/ConnectionControllerReconnect.cpp
    src/core/connect/ConnectionControllerHandlers.cpp
    src/core/connect/ConnectionPresetBuilder.cpp
    src/core/recording/RecordingController.cpp
    src/core/navigation/NavigationController.cpp
    src/core/navigation/NavigationControllerQuery.cpp
    src/core/navigation/NavigationControllerAnimations.cpp
    src/core/navigation/NavigationControllerStatus.cpp
    src/core/navigation/IconNavBar.cpp
    src/core/send/SendController.cpp
    src/core/send/SendControllerSend.cpp
    src/core/send/SendControllerStats.cpp
    src/core/send/SendHistoryManager.cpp
    src/core/event/EventBus.cpp
    src/core/event/EventBusStats.cpp
    src/core/widgets/AppDialog.cpp
    src/core/widgets/AppDialogContent.cpp
    src/core/widgets/DragDropHelper.cpp
    src/core/widgets/DragDropHelperEvents.cpp
    src/core/widgets/ConnectionQuickDialog.cpp
    src/plugin/loader/PluginLoader.cpp
    src/core/workspace/WorkspaceManager.cpp
    src/core/factory/WidgetFactory.cpp
    src/serial/detector/SerialDetector.cpp
    src/serial/detector/SerialDetectorChip.cpp
    src/serial/detector/SerialDetectorQuery.cpp
    src/chart/waterfall/WaterfallWidget.cpp
    src/chart/heatmap/HeatmapWidget.cpp
    src/core/settings/SettingsController.cpp
    src/core/settings/SettingsControllerPersistence.cpp
    src/core/toolbar/ToolbarController.cpp
    src/core/toolbar/ToolbarControllerQuery.cpp
    src/core/terminal/TerminalController.cpp
    src/core/terminal/TerminalControllerDisplay.cpp
    src/core/panels/PanelManager.cpp
    src/core/panels/PanelManagerCreation.cpp
    src/core/panels/PanelManagerFactory.cpp
    src/core/panels/PanelManagerGetters.cpp
    src/core/panels/PanelManagerQuery.cpp
    src/core/background/BackgroundWidget.cpp
    src/core/background/BackgroundWidgetPaint.cpp
    src/core/background/BackgroundSettingsPopup.cpp
    src/core/widgets/BasePanel.cpp
    src/core/widgets/BasePanelConfig.cpp
    src/core/widgets/BasePanelStates.cpp
    src/core/widgets/AnimatedButton.cpp
    src/core/widgets/EmptyStateWidget.cpp
    src/core/widgets/LoadingSpinner.cpp
    src/core/widgets/SkeletonWidget.cpp
    src/core/widgets/SmartAutoComplete.cpp
    src/core/widgets/CommandPalette.cpp
    src/core/widgets/CommandPaletteFilter.cpp
    src/core/widgets/ScriptRecorder.cpp
    src/core/widgets/DataDiffWidget.cpp
    src/core/widgets/DataDiffWidgetDiff.cpp
    src/core/widgets/EdDialog.cpp
    src/core/widgets/EdDialogTabs.cpp
    src/core/connect/ConnectionManager.cpp
    src/core/connect/ConnectionFactory.cpp
    src/core/theme/ThemeManager.cpp
    src/core/theme/ThemeManagerApply.cpp
    src/core/theme/ThemeManagerColor.cpp
    src/core/theme/QssThemeGenerator.cpp
    src/core/theme/QssThemeGeneratorWidgets.cpp
    src/core/theme/QssThemeGeneratorPanels.cpp
    src/core/theme/QssThemeGeneratorColors.cpp
    src/core/animation/AnimationUtility.cpp
    src/core/theme/IconManager.cpp
    src/connection/serial_port/SerialConnection.cpp
    src/connection/serial_port/SerialConnectionError.cpp
    src/connection/serial_port/SerialConnectionUtility.cpp
    src/connection/serial_port/SerialConnectionStats.cpp
    src/connection/network/TcpConnection.cpp
    src/connection/network/TcpConnectionStats.cpp
    src/connection/network/TcpConnectionHelpers.cpp
    src/connection/network/TcpConnectionHandlers.cpp
    src/connection/network/UdpConnection.cpp
    src/connection/network/UdpConnectionStats.cpp
    src/terminal/widget/TerminalWidget.cpp
    src/terminal/widget/TerminalWidgetEvents.cpp
    src/terminal/widget/TerminalWidgetSlots.cpp
    src/terminal/widget/TerminalWidgetSearch.cpp
    src/terminal/widget/TerminalWidgetPaint.cpp
    src/terminal/model/TerminalModel.cpp
    src/terminal/model/TerminalModelQuery.cpp
    src/terminal/model/TerminalModelStats.cpp
    src/terminal/search/TerminalSearchBar.cpp
    src/terminal/search/TerminalSearchBarHistory.cpp
    src/terminal/types/DirectionFilter.cpp
    src/terminal/layout/TerminalLayoutManager.cpp
    src/terminal/layout/TerminalLayoutManagerStats.cpp
    src/terminal/layout/TerminalLayoutSplit.cpp
    src/terminal/selection/TerminalSelectionManager.cpp
    src/terminal/selection/TerminalSelectionManagerStats.cpp
    src/terminal/search/TerminalSearchManager.cpp
    src/terminal/search/TerminalSearchBuilders.cpp
    src/terminal/search/TerminalSearchRenderer.cpp
    src/terminal/menu/TerminalContextMenuManager.cpp
    src/serial/config/SerialConfigPanel.cpp
    src/serial/config/SerialConfigPanelUI.cpp
    src/serial/config/SerialConfigPanelConfig.cpp
    src/serial/commands/QuickCommandBar.cpp
    src/serial/commands/QuickCommandBarActions.cpp
    src/serial/commands/QuickCommandBarDialog.cpp
    src/serial/data/BookmarkWidget.cpp
    src/serial/commands/TimedSender.cpp
    src/serial/commands/TimedSenderStats.cpp
    src/serial/commands/SendHistory.cpp
    src/serial/commands/SendHistoryStats.cpp
    src/serial/data/DataStatistics.cpp
    src/serial/data/DataStatisticsRefresh.cpp
    src/serial/data/DataStatisticsDisplay.cpp
    src/serial/data/DataStatisticsRolling.cpp
    src/serial/port/SerialDriverDetector.cpp
    src/serial/port/PortWatcher.cpp
    src/serial/port/PortWatcherPoll.cpp
    src/serial/port/PortWatcherStats.cpp
    src/protocol/parser/FrameParser.cpp
    src/protocol/parser/FrameParserHelpers.cpp
    src/protocol/parser/FrameParserStateHandlers.cpp
    src/protocol/parser/FrameParserStats.cpp
    src/protocol/view/ProtocolView.cpp
    src/protocol/view/ProtocolViewData.cpp
    src/protocol/view/ProtocolViewDisplay.cpp
    src/protocol/view/ProtocolViewMenu.cpp
    src/protocol/editor/FrameVisualEditor.cpp
    src/protocol/editor/FrameVisualEditorFields.cpp
    src/protocol/editor/FrameVisualEditorUI.cpp
    src/protocol/hex/IntelHexParser.cpp
    src/protocol/hex/IntelHexParserStats.cpp
    src/protocol/hex/IntelHexParserRecord.cpp
    src/protocol/bridge/JustFloatBridge.cpp
    src/protocol/bridge/JustFloatBridgeStats.cpp
    src/protocol/bridge/FireWaterBridge.cpp
    src/protocol/bridge/FireWaterBridgeStats.cpp
    src/protocol/bridge/ProtocolBridgeManager.cpp
    src/protocol/bridge/ProtocolBridgeManagerConfig.cpp
    src/protocol/bridge/ProtocolBridgeManagerHelpers.cpp
    src/protocol/bridge/ProtocolBridgeManagerStats.cpp
    src/protocol/bridge/ProtocolBridgeManagerAutoDetect.cpp
    src/protocol/bridge/ProtocolBridgeManagerConfig.cpp
    src/chart/widget/ChartWidget.cpp
    src/chart/widget/ChartWidgetTheme.cpp
    src/chart/widget/ChartWidgetExport.cpp
    src/chart/widget/ChartWidgetRender.cpp
    src/chart/model/ChannelConfig.cpp
    src/chart/model/ChannelConfigSetMethods.cpp
    src/chart/model/ChannelConfigStyle.cpp
    src/chart/model/ChartModel.cpp
    src/chart/model/ChartModelSlots.cpp
    src/chart/model/ChartModelStats.cpp
    src/chart/scale/YAxisManager.cpp
    src/chart/scale/YAxisManagerStats.cpp
    src/chart/overlay/CursorOverlay.cpp
    src/chart/overlay/CursorOverlayEvent.cpp
    src/chart/overlay/CursorOverlayPaint.cpp
    src/chart/overlay/CursorOverlayInteraction.cpp
    src/chart/zoom/ZoomController.cpp
    src/chart/zoom/ZoomControllerQuery.cpp
    src/chart/zoom/ZoomControllerStats.cpp
    src/chart/zoom/ZoomControllerViewport.cpp
    src/chart/fft/FftEngine.cpp
    src/chart/fft/FftEngineWindow.cpp
    src/chart/fft/FftWidget.cpp
    src/chart/fft/FftWidgetSlots.cpp
    src/chart/fft/FftWidgetSetup.cpp
    src/chart/stats/HistogramWidget.cpp
    src/chart/stats/HistogramWidgetUI.cpp
    src/chart/stats/HistogramWidgetStats.cpp
    src/chart/stats/ScatterWidget.cpp
    src/chart/stats/ScatterWidgetCompute.cpp
    src/chart/stats/ScatterWidgetStats.cpp
    src/ota/manager/OtaManager.cpp
    src/ota/manager/OtaManagerSignals.cpp
    src/ota/manager/OtaManagerFileOps.cpp
    src/ota/manager/OtaManagerProgress.cpp
    src/ota/manager/OtaManagerVerify.cpp
    src/ota/widget/OtaWidget.cpp
    src/ota/widget/OtaWidgetDisplay.cpp
    src/ota/widget/OtaWidgetSlots.cpp
    src/ota/widget/OtaWidgetSlots2.cpp
    src/ota/history/OtaHistoryModel.cpp
    src/ota/history/OtaHistoryModelStats.cpp
    src/ota/history/OtaHistoryModelIndex.cpp
    src/ota/protocols/base/BaseTransfer.cpp
    src/ota/protocols/xmodem/XModemTransfer.cpp
    src/ota/protocols/xmodem/XModemTransferHandlers.cpp
    src/ota/protocols/xmodem/XModemTransferProtocol.cpp
    src/ota/protocols/ymodem/YModemTransfer.cpp
    src/ota/protocols/ymodem/YModemTransferStats.cpp
    src/ota/protocols/ymodem/YModemTransferHandlers.cpp
    src/ota/protocols/ymodem/YModemTransferProtocol.cpp
    src/ota/protocols/zmodem/ZModemTransfer.cpp
    src/ota/protocols/zmodem/ZModemTransferHandlers.cpp
    src/ota/protocols/zmodem/ZModemTransferProtocol.cpp
    src/utils/settings/SettingsManager.cpp
    src/utils/settings/SettingsManagerConvenience.cpp
    src/utils/settings/SettingsManagerGroups.cpp
    src/utils/export/DataExporter.cpp
    src/utils/export/DataExporterFormats.cpp
    src/utils/export/DataExporterStats.cpp
    src/utils/export/DataExporterStreamed.cpp
    src/utils/export/DataExporterEdl.cpp
    src/utils/aggregator/DataAggregator.cpp
    src/utils/aggregator/DataAggregatorCompute.cpp
    src/utils/log/DataLogger.cpp
    src/utils/log/DataLoggerEdl.cpp
    src/utils/log/DataLoggerPlayback.cpp
    src/utils/log/DataLoggerSession.cpp
    src/protocol/parser/FrameDefinition.cpp
    src/protocol/parser/FrameDefinitionSerialization.cpp
    src/core/settings/SessionManager.cpp
    src/core/managers/ShortcutManager.cpp
    src/core/managers/ShortcutManagerPersist.cpp
    src/core/managers/ShortcutManagerActions.cpp
    src/core/managers/ShortcutManagerQuery.cpp
    src/core/layout/ResponsiveLayout.cpp
    src/core/layout/ResponsiveLayoutTransition.cpp
    src/core/layout/ResponsiveLayoutQuery.cpp

    # ── F1: 数据录制回放 ──
    src/core/recording/RecordingTimeline.cpp
    src/core/recording/PlaybackController.cpp
    src/core/recording/PlaybackControllerSpeed.cpp
    src/core/recording/PlaybackWidget.cpp
    src/core/recording/RecordingMarker.cpp
    src/core/recording/RecordingMarkerQuery.cpp
    src/utils/log/RecordingFileFormat.cpp
    src/utils/log/RecordingFileIndex.cpp

    # ── F3: 多通道数据导出 ──
    src/utils/export/ChartExporter.cpp
    src/utils/export/ChartExporterFormats.cpp
    src/utils/export/ChartExporterImage.cpp
    src/utils/export/ExportDialog.cpp

    # ── F2: 自定义协议引擎 ──
    src/protocol/schema/ProtocolSchema.cpp
    src/protocol/schema/ProtocolSchemaConfig.cpp
    src/protocol/schema/ProtocolSchemaFields.cpp
    src/protocol/engine/ProtocolEngine.cpp
    src/protocol/engine/ProtocolEngineStats.cpp
    src/protocol/engine/ProtocolEngineParsing.cpp
    src/protocol/engine/ProtocolEngineChecksum.cpp
    src/protocol/engine/ProtocolEngineAlgorithm.cpp
    src/protocol/schema/ProtocolTemplateLibrary.cpp
    src/protocol/editor/ProtocolSchemaEditor.cpp
    src/protocol/engine/ProtocolFieldMapper.cpp

    # ── F4: 终端增强 ──
    src/terminal/layout/TerminalSplitter.cpp
    src/terminal/layout/TerminalTabManager.cpp
    src/terminal/layout/TerminalDualView.cpp

    # ── F5: 仪表盘 ──
    src/dashboard/DashboardWidget.cpp
    src/dashboard/DashboardWidgetSerialization.cpp
    src/dashboard/DashboardWidgetSlots.cpp
    src/dashboard/DashboardWidgetPersistence.cpp
    src/dashboard/GaugeWidget.cpp
    src/dashboard/GaugeWidgetPaint.cpp
    src/dashboard/ProgressBarWidget.cpp
    src/dashboard/LedIndicatorWidget.cpp
    src/dashboard/NumericDisplayWidget.cpp
    src/dashboard/DashboardModel.cpp
    src/dashboard/DashboardModelItems.cpp
    src/dashboard/DashboardSerializer.cpp
    src/dashboard/DashboardSerializerExport.cpp
    src/dashboard/DashboardSerializerProfile.cpp

    # ── F6: J-Link RTT ──
    src/rtt/JLinkRttConnection.cpp
    src/rtt/JLinkRttConnectionStats.cpp
    src/rtt/JLinkSdkLoader.cpp
    src/rtt/JLinkSdkLoaderQuery.cpp
    src/rtt/JLinkSdkLoaderRtt.cpp
    src/rtt/RttConfigPanel.cpp
    src/rtt/RttChannelManager.cpp

    # ── F7: 自动化触发 ──
    src/automation/TriggerEngine.cpp
    src/automation/TriggerEngineStats.cpp
    src/automation/TriggerAction.cpp
    src/automation/TriggerManager.cpp
    src/automation/TriggerListPanel.cpp

    # ── F8: 项目管理 ──
    src/core/settings/ProjectManager.cpp
    src/core/settings/ProjectWelcomeDialog.cpp

    # ── F9: 串口高级调试 ──
    src/serial/signals/SignalLineMonitor.cpp
    src/serial/signals/SignalLineMonitorStats.cpp
    src/serial/signals/SignalLineWidget.cpp
    src/serial/data/TrafficMonitor.cpp
    src/serial/data/TrafficMonitorStats.cpp
    src/serial/data/TrafficMonitorWidget.cpp

    # ── F10: 性能监控 ──
    src/utils/perf/PerformanceMonitor.cpp
    src/utils/perf/PerformanceMonitorLatency.cpp
    src/utils/perf/PerformanceOverlay.cpp

    # ── F11: 插件系统 ──
    src/plugin/PluginManager.cpp
    src/plugin/PluginManagerScan.cpp
    src/plugin/PluginApi.cpp
    src/plugin/PluginConfigPanel.cpp
    src/plugin/PluginConfigPanelUI.cpp

    # ── F12: Bluetooth/BLE 调试 ──
    src/connection/ble/BleScanner.cpp
    src/connection/ble/BleScannerDiscovery.cpp
    src/connection/ble/BleConnection.cpp
    src/connection/ble/BleGattModel.cpp
    src/connection/ble/BleGattModelQuery.cpp
    src/connection/ble/BleGattBrowser.cpp
    src/connection/ble/BleGattBrowserSlots.cpp
    src/connection/ble/BleGattBrowserServices.cpp
    src/connection/ble/BleGattBrowserDisplay.cpp
    src/connection/ble/BleConfigPanel.cpp
    src/connection/ble/BluetoothAtConsole.cpp

    # ── F13: CAN/CAN-FD 总线 ──
    src/connection/can/CanConnection.cpp
    src/connection/can/CanConnectionProtocol.cpp
    src/connection/can/CanConnectionFrame.cpp
    src/connection/can/CanFrameParser.cpp
    src/connection/can/CanFrameParserBuild.cpp
    src/connection/can/CanBusMonitor.cpp
    src/connection/can/CanBusMonitorFrame.cpp
    src/connection/can/CanBusMonitorStats.cpp
    src/connection/can/CanConfigPanel.cpp
    src/protocol/can/DbcParser.cpp
    src/protocol/can/DbcParserParse.cpp
    src/protocol/can/DbcParserSignal.cpp
    src/protocol/can/DbcParserDecode.cpp

    # ── F14: MQTT 客户端 ──
    src/connection/mqtt/MqttConnection.cpp
    src/connection/mqtt/MqttConnectionProtocol.cpp
    src/connection/mqtt/MqttConnectionStats.cpp
    src/connection/mqtt/MqttConnectionPublish.cpp
    src/connection/mqtt/MqttTopicModel.cpp
    src/connection/mqtt/MqttTopicModelHelpers.cpp
    src/connection/mqtt/MqttTopicModelIndex.cpp
    src/connection/mqtt/MqttSubscriptionPanel.cpp
    src/connection/mqtt/MqttConfigPanel.cpp
    src/connection/mqtt/MqttWidget.cpp
    src/connection/mqtt/MqttWidgetSubs.cpp
    src/connection/mqtt/MqttWidgetSlots.cpp

    # ── F15: TCP/UDP 增强 ──
    src/connection/tcp/TcpServerConnection.cpp
    src/connection/tcp/TcpServerConnectionClients.cpp
    src/connection/tcp/TcpMultiConnectionManager.cpp
    src/connection/tcp/UdpMulticastConnection.cpp
    src/connection/tcp/UdpMulticastGroup.cpp
    src/connection/tcp/TlsConnection.cpp
    src/connection/tcp/TlsConnectionSetup.cpp
    src/connection/tcp/TlsConnectionStats.cpp
    src/connection/tcp/MultiConnectionPanel.cpp

    # ── F16: SPI/I2C 桥接 ──
    src/connection/spi_i2c/SpiConnection.cpp
    src/connection/spi_i2c/SpiConnectionStats.cpp
    src/connection/spi_i2c/SpiConnectionConfig.cpp
    src/connection/spi_i2c/SpiConnectionTransfer.cpp
    src/connection/spi_i2c/I2cConnection.cpp
    src/connection/spi_i2c/I2cConnectionScan.cpp
    src/connection/spi_i2c/I2cConnectionTransfer.cpp
    src/connection/spi_i2c/SpiI2cBridgeManager.cpp
    src/connection/spi_i2c/SpiI2cBridgeManagerQueue.cpp
    src/connection/spi_i2c/SpiI2cConfigPanel.cpp
    src/connection/spi_i2c/SpiI2cConfigPanelPersist.cpp
    src/connection/spi_i2c/RegisterEditor.cpp
    src/connection/spi_i2c/RegisterEditorStats.cpp

    # ── F17: WebSocket 客户端 ──
    src/connection/ws/WebSocketConnection.cpp
    src/connection/ws/WebSocketConnectionHandshake.cpp
    src/connection/ws/WebSocketFrame.cpp
    src/connection/ws/WebSocketFrameStats.cpp
    src/connection/ws/WsConfigPanel.cpp

    # ── F18: Modbus 主站/从站 ──
    src/protocol/modbus/ModbusMaster.cpp
    src/protocol/modbus/ModbusMasterStats.cpp
    src/protocol/modbus/ModbusMasterProtocol.cpp
    src/protocol/modbus/ModbusSlave.cpp
    src/protocol/modbus/ModbusSlaveResponse.cpp
    src/protocol/modbus/ModbusScanWidget.cpp
    src/protocol/modbus/ModbusScanWidgetStats.cpp
    src/protocol/modbus/ModbusConfigPanel.cpp

    # ── F19: Protobuf/FlatBuffers 解码 ──
    src/protocol/protobuf/ProtobufDecoder.cpp
    src/protocol/protobuf/ProtobufDecoderEncoding.cpp
    src/protocol/protobuf/ProtobufDecoderVarint.cpp
    src/protocol/protobuf/ProtobufDecoderHelpers.cpp
    src/protocol/protobuf/FlatBuffersDecoder.cpp
    src/protocol/protobuf/FlatBuffersDecoderFields.cpp
    src/protocol/protobuf/FlatBuffersDecoderSchema.cpp
    src/protocol/protobuf/SchemaViewer.cpp
    src/protocol/protobuf/SchemaViewerStats.cpp

    # ── F20: USB 调试 ──
    src/connection/usb/UsbConnection.cpp
    src/connection/usb/UsbConnectionInterface.cpp
    src/connection/usb/UsbConnectionTransfer.cpp
    src/connection/usb/UsbConnectionDescriptors.cpp
    src/connection/usb/UsbDeviceDetector.cpp
    src/connection/usb/UsbDeviceDetectorDescriptor.cpp
    src/connection/usb/UsbDeviceDetectorWmic.cpp
    src/connection/usb/UsbDeviceDetectorStats.cpp
    src/connection/usb/UsbDescriptorViewer.cpp
    src/connection/usb/UsbConfigPanel.cpp
    src/connection/usb/UsbLibraryLoader.cpp
    src/connection/usb/UsbLibraryLoaderQuery.cpp

    # ── F21: 正则终端过滤器 ──
    src/terminal/filter/TerminalFilter.cpp
    src/terminal/filter/TerminalFilterApply.cpp
    src/terminal/filter/TerminalFilterBar.cpp
    src/terminal/filter/TerminalHighlighter.cpp

    # ── F22: 校验和计算器 ──
    src/utils/checksum/ChecksumCalculator.cpp
    src/utils/checksum/ChecksumCalculatorStats.cpp
    src/utils/checksum/ChecksumPanel.cpp
    src/utils/checksum/ChecksumPanelAlgo.cpp
    src/utils/checksum/ChecksumPanelSlots.cpp

    # ── F23: ASCII/表格转换器 ──
    src/utils/converter/DataConverter.cpp
    src/utils/converter/DataConverterCodec.cpp
    src/utils/converter/ConverterPanel.cpp
    src/utils/converter/ConverterPanelSlots.cpp

    # ── F24: 时间戳分析器 ──
    src/utils/timestamp/TimestampAnalyzer.cpp
    src/utils/timestamp/TimestampPanel.cpp
    src/utils/timestamp/TimestampPanelConvert.cpp

    # ── F25: 数据包构建器 ──
    src/utils/packet/PacketBuilder.cpp
    src/utils/packet/PacketBuilderTemplate.cpp
    src/utils/packet/PacketBuilderPanel.cpp
    src/utils/packet/PacketBuilderPanelFields.cpp

    # ── F26: 设备注册表 ──
    src/core/device/DeviceRegistry.cpp
    src/core/device/DeviceProfilePanel.cpp
)

set(HEADERS
    # 接口层 - 零出站依赖的纯虚接口
    src/interfaces/IPanelProvider.h
    src/interfaces/IDataSink.h
    src/interfaces/IProtocolParser.h
    src/interfaces/IDevice.h
    src/core/event/EventBus.h

    src/core/mainwindow/MainWindow.h
    src/core/connect/ConnectionController.h
    src/core/connect/ConnectionPresetBuilder.h

    src/core/widgets/AnimatedButton.h
    src/core/recording/RecordingController.h
    src/core/navigation/NavigationController.h
    src/core/navigation/IconNavBar.h
    src/core/send/SendController.h
    src/core/send/SendHistoryManager.h
    src/core/settings/SettingsController.h
    src/core/toolbar/ToolbarController.h
    src/core/terminal/TerminalController.h
    src/core/panels/PanelManager.h
    src/core/panels/PanelManagerPanels.h
    src/core/background/BackgroundWidget.h
    src/core/background/BackgroundSettingsPopup.h
    src/core/connect/ConnectionManager.h
    src/core/connect/ConnectionFactory.h
    src/core/theme/ThemeManager.h
    src/core/theme/QssThemeGenerator.h
    src/core/animation/AnimationUtility.h
    src/core/theme/IconManager.h
    src/core/widgets/BasePanel.h
    src/core/widgets/EmptyStateWidget.h
    src/core/widgets/LoadingSpinner.h
    src/core/widgets/SkeletonWidget.h
    src/core/widgets/SmartAutoComplete.h
    src/core/widgets/ToastWidget.h
    src/core/widgets/AppDialog.h
    src/core/widgets/ConnectionQuickDialog.h
    src/core/widgets/DragDropHelper.h
    src/plugin/loader/PluginLoader.h
    src/core/workspace/WorkspaceManager.h
    src/core/factory/WidgetFactory.h
    src/serial/detector/SerialDetector.h
    src/chart/waterfall/WaterfallWidget.h
    src/chart/heatmap/HeatmapWidget.h
    src/core/widgets/CommandPalette.h
    src/core/widgets/ScriptRecorder.h
    src/core/widgets/DataDiffWidget.h
    src/core/widgets/EdDialog.h
    src/core/navigation/NavIndicatorWidget.h
    src/core/theme/Constants.h
    src/core/theme/AppConstants.h
    src/core/theme/AnimationConstants.h
    src/core/theme/LayoutConstants.h
    src/core/theme/ConnectionConstants.h
    src/core/theme/TimerConstants.h
    src/core/settings/SessionManager.h
    src/core/managers/ShortcutManager.h
    src/core/layout/ResponsiveLayout.h
    src/connection/interface/IConnection.h
    src/connection/serial_port/SerialConnection.h
    src/connection/network/TcpConnection.h
    src/connection/network/UdpConnection.h
    src/terminal/widget/TerminalWidget.h
    src/terminal/model/TerminalModel.h
    src/terminal/search/TerminalSearchBar.h
    src/terminal/types/DirectionFilter.h
    src/terminal/layout/TerminalLayoutManager.h
    src/terminal/selection/TerminalSelectionManager.h
    src/terminal/search/TerminalSearchManager.h
    src/terminal/search/TerminalSearchRenderer.h
    src/terminal/menu/TerminalContextMenuManager.h
    src/terminal/types/TerminalTypes.h
    src/serial/config/SerialConfigPanel.h
    src/serial/commands/QuickCommandBar.h
    src/serial/data/BookmarkWidget.h
    src/serial/commands/TimedSender.h
    src/serial/commands/SendHistory.h
    src/serial/data/DataStatistics.h
    src/serial/port/SerialDriverDetector.h
    src/serial/port/PortWatcher.h
    src/protocol/parser/FrameDefinition.h
    src/protocol/parser/FrameParser.h
    src/protocol/view/ProtocolView.h
    src/protocol/editor/FrameVisualEditor.h
    src/protocol/hex/IntelHexParser.h
    src/protocol/bridge/IProtocolBridge.h
    src/protocol/bridge/JustFloatBridge.h
    src/protocol/bridge/FireWaterBridge.h
    src/protocol/bridge/ProtocolBridgeManager.h
    src/chart/widget/ChartWidget.h
    src/chart/widget/ChartColors.h
    src/chart/model/ChannelConfig.h
    src/chart/model/ChartModel.h
    src/chart/scale/YAxisManager.h
    src/chart/overlay/CursorOverlay.h
    src/chart/zoom/ZoomController.h
    src/chart/fft/FftEngine.h
    src/chart/fft/FftWidget.h
    src/chart/stats/HistogramWidget.h
    src/chart/stats/ScatterWidget.h
    src/ota/manager/OtaManager.h
    src/ota/widget/OtaWidget.h
    src/ota/history/OtaHistoryModel.h
    src/ota/widget/AnimatedProgressBar.h
    src/ota/protocols/base/BaseTransfer.h
    src/ota/protocols/xmodem/XModemTransfer.h
    src/ota/protocols/ymodem/YModemTransfer.h
    src/ota/protocols/zmodem/ZModemTransfer.h
    src/utils/settings/SettingsManager.h
    src/utils/export/DataExporter.h
    src/utils/aggregator/DataAggregator.h
    src/utils/log/DataLogger.h
    src/utils/data/DataBookmark.h
    src/utils/data/ByteFormat.h
    src/utils/crypto/CRC.h
    src/utils/crypto/HexConverter.h
    src/utils/data/RingBuffer.h

    # ── F1: 数据录制回放 ──
    src/core/recording/RecordingTimeline.h
    src/core/recording/PlaybackController.h
    src/core/recording/PlaybackWidget.h
    src/core/recording/RecordingMarker.h
    src/utils/log/RecordingFileFormat.h

    # ── F3: 多通道数据导出 ──
    src/utils/export/ChartExporter.h
    src/utils/export/ExportDialog.h

    # ── F2: 自定义协议引擎 ──
    src/protocol/schema/ProtocolSchema.h
    src/protocol/engine/ProtocolEngine.h
    src/protocol/schema/ProtocolTemplateLibrary.h
    src/protocol/editor/ProtocolSchemaEditor.h
    src/protocol/engine/ProtocolFieldMapper.h

    # ── F4: 终端增强 ──
    src/terminal/layout/TerminalSplitter.h
    src/terminal/layout/TerminalTabManager.h
    src/terminal/layout/TerminalDualView.h

    # ── F5: 仪表盘 ──
    src/dashboard/DashboardWidget.h
    src/dashboard/GaugeWidget.h
    src/dashboard/ProgressBarWidget.h
    src/dashboard/LedIndicatorWidget.h
    src/dashboard/NumericDisplayWidget.h
    src/dashboard/DashboardModel.h
    src/dashboard/DashboardSerializer.h

    # ── F6: J-Link RTT ──
    src/rtt/JLinkRttConnection.h
    src/rtt/JLinkSdkLoader.h
    src/rtt/RttConfigPanel.h
    src/rtt/RttChannelManager.h

    # ── F7: 自动化触发 ──
    src/automation/TriggerRule.h
    src/automation/TriggerEngine.h
    src/automation/TriggerAction.h
    src/automation/TriggerManager.h
    src/automation/TriggerListPanel.h

    # ── F8: 项目管理 ──
    src/core/settings/Project.h
    src/core/settings/ProjectManager.h
    src/core/settings/ProjectWelcomeDialog.h

    # ── F9: 串口高级调试 ──
    src/serial/signals/SignalLineMonitor.h
    src/serial/signals/SignalLineWidget.h
    src/serial/data/TrafficMonitor.h
    src/serial/data/TrafficMonitorWidget.h

    # ── F10: 性能监控 ──
    src/utils/perf/PerformanceMonitor.h
    src/utils/perf/PerformanceOverlay.h

    # ── F11: 插件系统 ──
    src/plugin/IEmbedDebugPlugin.h
    src/plugin/PluginManager.h
    src/plugin/PluginApi.h
    src/plugin/PluginConfigPanel.h

    # ── F12: Bluetooth/BLE 调试 ──
    src/connection/ble/BleScanner.h
    src/connection/ble/BleConnection.h
    src/connection/ble/BleGattModel.h
    src/connection/ble/BleGattBrowser.h
    src/connection/ble/BleConfigPanel.h
    src/connection/ble/BluetoothAtConsole.h

    # ── F13: CAN/CAN-FD 总线 ──
    src/connection/can/CanConnection.h
    src/connection/can/CanFrameParser.h
    src/connection/can/CanBusMonitor.h
    src/connection/can/CanConfigPanel.h
    src/protocol/can/DbcParser.h

    # ── F14: MQTT 客户端 ──
    src/connection/mqtt/MqttConnection.h
    src/connection/mqtt/MqttTopicModel.h
    src/connection/mqtt/MqttSubscriptionPanel.h
    src/connection/mqtt/MqttConfigPanel.h
    src/connection/mqtt/MqttWidget.h

    # ── F15: TCP/UDP 增强 ──
    src/connection/tcp/TcpServerConnection.h
    src/connection/tcp/TcpMultiConnectionManager.h
    src/connection/tcp/UdpMulticastConnection.h
    src/connection/tcp/TlsConnection.h
    src/connection/tcp/MultiConnectionPanel.h

    # ── F16: SPI/I2C 桥接 ──
    src/connection/spi_i2c/SpiConnection.h
    src/connection/spi_i2c/I2cConnection.h
    src/connection/spi_i2c/SpiI2cBridgeManager.h
    src/connection/spi_i2c/SpiI2cConfigPanel.h
    src/connection/spi_i2c/RegisterEditor.h

    # ── F17: WebSocket 客户端 ──
    src/connection/ws/WebSocketConnection.h
    src/connection/ws/WsConfigPanel.h

    # ── F18: Modbus 主站/从站 ──
    src/protocol/modbus/ModbusTypes.h
    src/protocol/modbus/ModbusMaster.h
    src/protocol/modbus/ModbusSlave.h
    src/protocol/modbus/ModbusScanWidget.h
    src/protocol/modbus/ModbusConfigPanel.h

    # ── F19: Protobuf/FlatBuffers 解码 ──
    src/protocol/protobuf/ProtobufDecoder.h
    src/protocol/protobuf/FlatBuffersDecoder.h
    src/protocol/protobuf/SchemaViewer.h

    # ── F20: USB 调试 ──
    src/connection/usb/UsbConnection.h
    src/connection/usb/UsbDeviceDetector.h
    src/connection/usb/UsbDescriptorViewer.h
    src/connection/usb/UsbConfigPanel.h
    src/connection/usb/UsbLibraryLoader.h

    # ── F21: 正则终端过滤器 ──
    src/terminal/filter/TerminalFilter.h
    src/terminal/filter/TerminalFilterBar.h
    src/terminal/filter/TerminalHighlighter.h

    # ── F22: 校验和计算器 ──
    src/utils/checksum/ChecksumCalculator.h
    src/utils/checksum/ChecksumPanel.h

    # ── F23: ASCII/表格转换器 ──
    src/utils/converter/DataConverter.h
    src/utils/converter/ConverterPanel.h

    # ── F24: 时间戳分析器 ──
    src/utils/timestamp/TimestampAnalyzer.h
    src/utils/timestamp/TimestampPanel.h

    # ── F25: 数据包构建器 ──
    src/utils/packet/PacketField.h
    src/utils/packet/PacketBuilder.h
    src/utils/packet/PacketBuilderPanel.h

    # ── F26: 设备注册表 ──
    src/core/device/DeviceProfile.h
    src/core/device/DeviceRegistry.h
    src/core/device/DeviceProfilePanel.h
)

set(RESOURCES
    resources/app.qrc
)

# 定义可执行目标
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::SerialPort
    Qt6::Charts
    Qt6::Network
)

# Windows平台特殊设置
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE
        _UNICODE
    )
endif()

# 翻译文件处理: .ts → .qm
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
set(TS_FILES
    ${CMAKE_SOURCE_DIR}/resources/translations/embeddebug_en.ts
)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION
    ${CMAKE_BINARY_DIR}/translations
)
qt_add_translations(${PROJECT_NAME} TS_FILES ${TS_FILES})
# 确保translations目录存在
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/translations)

# 测试
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

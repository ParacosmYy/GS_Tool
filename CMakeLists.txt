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
    Qml
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
    src/core/mainwindow/MainWindowPanelConnectOta.cpp
    src/core/mainwindow/MainWindowInitShortcuts.cpp
    src/core/mainwindow/MainWindowLifecycle.cpp
    src/core/connect/ConnectionController.cpp
    src/core/connect/ConnectionControllerLifecycle.cpp
    src/core/connect/ConnectionControllerQuery.cpp
    src/core/connect/ConnectionControllerReconnect.cpp
    src/core/connect/ConnectionControllerHandlers.cpp
    src/core/connect/ConnectionControllerTeardown.cpp
    src/core/connect/ConnectionPresetBuilder.cpp
    src/core/recording/RecordingController.cpp
    src/core/recording/RecordingControllerStats.cpp
    src/core/recording/DataStreamRecorder.cpp
    src/core/recording/DataStreamRecorderStats.cpp
    src/core/navigation/NavigationController.cpp
    src/core/navigation/NavigationControllerQuery.cpp
    src/core/navigation/NavigationControllerAnimations.cpp
    src/core/navigation/NavigationControllerStatus.cpp
    src/core/navigation/IconNavBar.cpp
    src/core/navigation/NavIndicatorWidget.cpp
    src/core/send/SendController.cpp
    src/core/send/SendControllerSend.cpp
    src/core/send/SendControllerStats.cpp
    src/core/send/SendHistoryManager.cpp
    src/core/send/SendHistoryManagerStats.cpp
    src/core/event/EventBus.cpp
    src/core/event/EventBusStats.cpp
    src/core/widgets/AppDialog.cpp
    src/core/widgets/AppDialogContent.cpp
    src/core/widgets/DragDropHelper.cpp
    src/core/widgets/DragDropHelperEvents.cpp
    src/core/widgets/ConnectionQuickDialog.cpp
    src/core/widgets/ConnectionQuickDialogParams.cpp
    src/plugin/loader/PluginLoader.cpp
    src/core/workspace/WorkspaceManager.cpp
    src/core/factory/WidgetFactory.cpp
    src/serial/detector/SerialDetector.cpp
    src/serial/detector/SerialDetectorChip.cpp
    src/serial/detector/SerialDetectorQuery.cpp
    src/chart/preview/WaveformPreviewWidget.cpp
    src/chart/preview/WaveformPreviewWidgetStats.cpp
    src/chart/waterfall/WaterfallWidget.cpp
    src/chart/heatmap/HeatmapWidget.cpp
    src/chart/heatmap/HeatmapWidgetPaint.cpp
    src/core/settings/SettingsController.cpp
    src/core/settings/SettingsControllerPersistence.cpp
    src/core/toolbar/ToolbarController.cpp
    src/core/toolbar/ToolbarControllerQuery.cpp
    src/core/terminal/TerminalController.cpp
    src/core/terminal/TerminalControllerDisplay.cpp
    src/core/panels/PanelManager.cpp
    src/core/panels/PanelManagerCreation.cpp
    src/core/panels/PanelManagerFactory.cpp
    src/core/panels/PanelManagerFactorySystem.cpp
    src/core/panels/PanelManagerGetters.cpp
    src/core/panels/PanelManagerQuery.cpp
    src/core/panels/PanelManagerWrap.cpp
    src/core/background/BackgroundWidget.cpp
    src/core/background/BackgroundWidgetPaint.cpp
    src/core/background/BackgroundSettingsPopup.cpp
    src/core/health/ConnectionHealthMonitor.cpp
    src/core/health/ConnectionHealthMonitorCalc.cpp
    src/utils/logviewer/DataLogViewer.cpp
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
    src/core/widgets/ToastWidget.cpp
    src/core/widgets/EdDialog.cpp
    src/core/widgets/EdDialogPaint.cpp
    src/core/widgets/EdDialogTabs.cpp
    src/core/connect/ConnectionManager.cpp
    src/core/connect/ConnectionFactory.cpp
    src/core/theme/ThemeManager.cpp
    src/core/theme/ThemeManagerApply.cpp
    src/core/theme/ThemeManagerStats.cpp
    src/core/theme/ThemeManagerColor.cpp
    src/core/theme/QssThemeGenerator.cpp
    src/core/theme/QssThemeGeneratorWidgets.cpp
    src/core/theme/QssThemeGeneratorNav.cpp
    src/core/theme/QssThemeGeneratorTerminal.cpp
    src/core/theme/QssThemeGeneratorPanels.cpp
    src/core/theme/QssThemeGeneratorColors.cpp
    src/core/animation/AnimationUtility.cpp
    src/core/animation/AnimationUtilityEffects.cpp
    src/core/theme/IconManager.cpp
    src/core/icons/SvgIconProvider.cpp
    src/core/icons/SvgIconProviderPrivate.cpp
    src/connection/serial_port/SerialConnection.cpp
    src/connection/serial_port/SerialConnectionError.cpp
    src/connection/serial_port/SerialConnectionUtility.cpp
    src/connection/serial_port/SerialConnectionStats.cpp
    src/connection/network/TcpConnection.cpp
    src/connection/network/TcpConnectionLifecycle.cpp
    src/connection/network/TcpConnectionStats.cpp
    src/connection/network/TcpConnectionHelpers.cpp
    src/connection/network/TcpConnectionHandlers.cpp
    src/connection/network/UdpConnection.cpp
    src/connection/network/UdpConnectionHandlers.cpp
    src/connection/network/UdpConnectionStats.cpp
    src/terminal/widget/TerminalWidget.cpp
    src/terminal/widget/TerminalWidgetConfig.cpp
    src/terminal/widget/TerminalWidgetEvents.cpp
    src/terminal/widget/TerminalWidgetSlots.cpp
    src/terminal/widget/TerminalWidgetSearch.cpp
    src/terminal/widget/TerminalWidgetPaint.cpp
    src/terminal/model/TerminalModel.cpp
    src/terminal/model/TerminalModelQuery.cpp
    src/terminal/model/TerminalModelStats.cpp
    src/terminal/search/TerminalSearchBar.cpp
    src/terminal/search/TerminalSearchBarHistory.cpp
    src/terminal/search/TerminalSearchBarAnimation.cpp
    src/terminal/types/DirectionFilter.cpp
    src/terminal/layout/TerminalLayoutManager.cpp
    src/terminal/layout/TerminalLayoutManagerApply.cpp
    src/terminal/layout/TerminalLayoutManagerStats.cpp
    src/terminal/layout/TerminalLayoutSplit.cpp
    src/terminal/selection/TerminalSelectionManager.cpp
    src/terminal/selection/TerminalSelectionManagerStats.cpp
    src/terminal/search/TerminalSearchManager.cpp
    src/terminal/search/TerminalSearchBuilders.cpp
    src/terminal/search/TerminalSearchBuild.cpp
    src/terminal/search/TerminalSearchBuild.cpp
    src/terminal/search/TerminalSearchRenderer.cpp
    src/terminal/menu/TerminalContextMenuManager.cpp
    src/serial/config/SerialConfigPanel.cpp
    src/serial/config/SerialConfigPanelInternal.cpp
    src/serial/config/SerialConfigPanelUI.cpp
    src/serial/config/SerialConfigPanelUISignals.cpp
    src/serial/config/SerialConfigPanelConfig.cpp
    src/serial/commands/QuickCommandBar.cpp
    src/serial/commands/QuickCommandBarActions.cpp
    src/serial/commands/QuickCommandBarDialog.cpp
    src/serial/data/BookmarkWidget.cpp
    src/serial/data/BookmarkWidgetStats.cpp
    src/serial/commands/TimedSender.cpp
    src/serial/commands/TimedSenderStats.cpp
    src/serial/commands/SendHistory.cpp
    src/serial/commands/SendHistoryStats.cpp
    src/serial/data/DataStatistics.cpp
    src/serial/data/DataStatisticsRefresh.cpp
    src/serial/data/DataStatisticsDisplay.cpp
    src/serial/data/DataStatisticsRolling.cpp
    src/serial/data/DataStatisticsUpdate.cpp
    src/serial/port/SerialDriverDetector.cpp
    src/serial/port/PortWatcher.cpp
    src/serial/port/PortWatcherPoll.cpp
    src/serial/port/PortWatcherStats.cpp
    src/protocol/parser/FrameParser.cpp
    src/protocol/parser/FrameParserHelpers.cpp
    src/protocol/parser/FrameParserStateHandlers.cpp
    src/protocol/parser/FrameParserPayload.cpp
    src/protocol/parser/FrameParserStats.cpp
    src/protocol/parser/FrameParserTimers.cpp
    src/protocol/view/ProtocolView.cpp
    src/protocol/view/ProtocolViewData.cpp
    src/protocol/view/ProtocolViewDisplay.cpp
    src/protocol/view/ProtocolViewMenu.cpp
    src/protocol/editor/FrameVisualEditor.cpp
    src/protocol/editor/FrameVisualEditorFields.cpp
    src/protocol/editor/FrameVisualEditorUI.cpp
    src/protocol/editor/FrameVisualEditorUIConnect.cpp
    src/protocol/editor/FrameVisualEditorPreview.cpp
    src/protocol/hex/IntelHexParser.cpp
    src/protocol/hex/IntelHexParserStats.cpp
    src/protocol/hex/IntelHexParserRecord.cpp
    src/protocol/bridge/JustFloatBridge.cpp
    src/protocol/bridge/JustFloatBridgeStats.cpp
    src/protocol/bridge/FireWaterBridge.cpp
    src/protocol/bridge/FireWaterBridgeParsing.cpp
    src/protocol/bridge/FireWaterBridgeStats.cpp
    src/protocol/bridge/ProtocolBridgeManager.cpp
    src/protocol/bridge/ProtocolBridgeManagerConfig.cpp
    src/protocol/bridge/ProtocolBridgeManagerHelpers.cpp
    src/protocol/bridge/ProtocolBridgeManagerStats.cpp
    src/protocol/bridge/ProtocolBridgeManagerAutoDetect.cpp
    src/protocol/bridge/ProtocolBridgeManagerConfig.cpp
    src/chart/widget/ChartWidget.cpp
    src/chart/widget/ChartWidgetSlots.cpp
    src/chart/widget/ChartWidgetTheme.cpp
    src/chart/widget/ChartWidgetExport.cpp
    src/chart/widget/ChartWidgetRender.cpp
    src/chart/model/ChannelConfig.cpp
    src/chart/model/ChannelConfigSet.cpp
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
    src/chart/overlay/CursorOverlayPaintMain.cpp
    src/chart/overlay/CursorOverlayInteraction.cpp
    src/chart/zoom/ZoomController.cpp
    src/chart/zoom/ZoomControllerQuery.cpp
    src/chart/zoom/ZoomControllerStats.cpp
    src/chart/zoom/ZoomControllerViewport.cpp
    src/chart/zoom/ZoomControllerHandlers.cpp
    src/chart/fft/FftEngine.cpp
    src/chart/fft/FftEngineCompute.cpp
    src/chart/fft/FftEngineWindow.cpp
    src/chart/fft/FftWidget.cpp
    src/chart/fft/FftWidgetSlots.cpp
    src/chart/fft/FftWidgetSetup.cpp
    src/chart/fft/FftWidgetTheme.cpp
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
    src/ota/widget/OtaWidgetDragDrop.cpp
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
    src/ota/protocols/ymodem/YModemTransferHandlersEarly.cpp
    src/ota/protocols/ymodem/YModemTransferHandlersFinal.cpp
    src/ota/protocols/ymodem/YModemTransferProtocol.cpp
    src/ota/protocols/zmodem/ZModemTransfer.cpp
    src/ota/protocols/zmodem/ZModemTransferHandlers.cpp
    src/ota/protocols/zmodem/ZModemTransferFrames.cpp
    src/ota/protocols/zmodem/ZModemTransferFrames.cpp
    src/ota/protocols/zmodem/ZModemTransferProtocol.cpp
    src/ota/protocols/zmodem/ZModemTransferDispatch.cpp
    src/utils/settings/SettingsManager.cpp
    src/utils/settings/SettingsManagerConvenience.cpp
    src/utils/settings/SettingsManagerGroups.cpp
    src/utils/export/DataExporter.cpp
    src/utils/export/DataExporterFormats.cpp
    src/utils/export/DataExporterUtils.cpp
    src/utils/export/DataExporterStats.cpp
    src/utils/export/DataExporterStreamed.cpp
    src/utils/export/DataExporterStreamedFormats.cpp
    src/utils/export/DataExporterEdl.cpp
    src/utils/aggregator/DataAggregator.cpp
    src/utils/aggregator/DataAggregatorCompute.cpp
    src/utils/aggregator/DataAggregatorTimeWindow.cpp
    src/utils/pipeline/DataPipeline.cpp
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
    src/core/recording/PlaybackWidgetSlots.cpp
    src/core/recording/RecordingMarker.cpp
    src/core/recording/RecordingMarkerQuery.cpp
    src/utils/log/RecordingFileFormat.cpp
    src/utils/log/RecordingFileIndex.cpp
    src/utils/log/RecordingFileIndexStats.cpp

    # ── F3: 多通道数据导出 ──
    src/utils/export/ChartExporter.cpp
    src/utils/export/ChartExporterFormats.cpp
    src/utils/export/ChartExporterJson.cpp
    src/utils/export/ChartExporterImage.cpp
    src/utils/export/ExportDialog.cpp
    src/utils/export/ExportDialogSlots.cpp

    # ── F2: 自定义协议引擎 ──
    src/protocol/schema/ProtocolSchema.cpp
    src/protocol/schema/ProtocolSchemaConfig.cpp
    src/protocol/schema/ProtocolSchemaFields.cpp
    src/protocol/schema/ProtocolSchemaStats.cpp
    src/protocol/schema/ProtocolSchemaParse.cpp
    src/protocol/engine/ProtocolEngine.cpp
    src/protocol/engine/ProtocolEngineStats.cpp
    src/protocol/engine/ProtocolEngineCore.cpp
    src/protocol/engine/ProtocolEngineParsing.cpp
    src/protocol/engine/ProtocolEngineChecksum.cpp
    src/protocol/engine/ProtocolEngineAlgorithm.cpp
    src/protocol/schema/ProtocolTemplateLibrary.cpp
    src/protocol/schema/ProtocolTemplateLibraryBuiltins.cpp
    src/protocol/editor/ProtocolSchemaEditor.cpp
    src/protocol/editor/ProtocolSchemaEditorStats.cpp
    src/protocol/engine/ProtocolFieldMapper.cpp

    # ── F4: 终端增强 ──
    src/terminal/layout/TerminalSplitter.cpp
    src/terminal/layout/TerminalTabManager.cpp
    src/terminal/layout/TerminalDualView.cpp

    # ── F5: 仪表盘 ──
    src/dashboard/DashboardWidget.cpp
    src/dashboard/DashboardWidgetLayout.cpp
    src/dashboard/DashboardWidgetSerialization.cpp
    src/dashboard/DashboardWidgetSlots.cpp
    src/dashboard/DashboardWidgetPersistence.cpp
    src/dashboard/GaugeWidget.cpp
    src/dashboard/GaugeWidgetPaint.cpp
    src/dashboard/ProgressBarWidget.cpp
    src/dashboard/LedIndicatorWidget.cpp
    src/dashboard/NumericDisplayWidget.cpp
    src/dashboard/NumericDisplayWidgetPaint.cpp
    src/dashboard/MiniChartWidget.cpp
    src/dashboard/MiniChartWidgetPaint.cpp
    src/dashboard/DashboardModel.cpp
    src/dashboard/DashboardModelItems.cpp
    src/dashboard/DashboardSerializer.cpp
    src/dashboard/DashboardSerializerExport.cpp
    src/dashboard/DashboardSerializerProfile.cpp
    src/dashboard/DashboardSerializerProfileCrud.cpp
    src/dashboard/DashboardSerializerProfileCrud.cpp
    src/dashboard/DashboardSerializerStats.cpp

    # ── F6: J-Link RTT ──
    src/rtt/JLinkRttConnection.cpp
    src/rtt/JLinkRttConnectionStats.cpp
    src/rtt/JLinkSdkLoader.cpp
    src/rtt/JLinkSdkLoaderQuery.cpp
    src/rtt/JLinkSdkLoaderRtt.cpp
    src/rtt/RttConfigPanel.cpp
    src/rtt/RttConfigPanelSettings.cpp
    src/rtt/RttChannelManager.cpp

    # ── F7: 自动化触发 ──
    src/automation/TriggerEngine.cpp
    src/automation/TriggerEngineStats.cpp
    src/automation/TriggerAction.cpp
    src/automation/TriggerManager.cpp
    src/automation/TriggerManagerStats.cpp
    src/automation/TriggerListPanel.cpp

    # ── F8: 项目管理 ──
    src/core/settings/ProjectManager.cpp
    src/core/settings/ProjectWelcomeDialog.cpp

    # ── F9: 串口高级调试 ──
    src/serial/signals/SignalLineMonitor.cpp
    src/serial/signals/SignalLineMonitorStats.cpp
    src/serial/signals/SignalLineWidget.cpp
    src/serial/signals/SignalLineWidgetStats.cpp
    src/serial/data/TrafficMonitor.cpp
    src/serial/data/TrafficMonitorStats.cpp
    src/serial/data/TrafficMonitorWidget.cpp

    # ── F10: 性能监控 ──
    src/utils/perf/PerformanceMonitor.cpp
    src/utils/perf/PerformanceMonitorLatency.cpp
    src/utils/perf/PerformanceOverlay.cpp
    src/utils/perf/PerformanceOverlayStats.cpp

    # ── F11: 插件系统 ──
    src/plugin/PluginManager.cpp
    src/plugin/PluginManagerScan.cpp
    src/plugin/PluginManagerQuery.cpp
    src/plugin/PluginApi.cpp
    src/plugin/PluginConfigPanel.cpp
    src/plugin/PluginConfigPanelSetup.cpp
    src/plugin/PluginConfigPanelUI.cpp

    # ── F12: Bluetooth/BLE 调试 ──
    src/connection/ble/BleScanner.cpp
    src/connection/ble/BleScannerStats.cpp
    src/connection/ble/BleScannerDiscovery.cpp
    src/connection/ble/BleScannerFilter.cpp
    src/connection/ble/BleConnection.cpp
    src/connection/ble/BleConnectionStats.cpp
    src/connection/ble/BleGattModel.cpp
    src/connection/ble/BleGattModelIndex.cpp
    src/connection/ble/BleGattModelQuery.cpp
    src/connection/ble/BleGattBrowser.cpp
    src/connection/ble/BleGattBrowserSlots.cpp
    src/connection/ble/BleGattBrowserServices.cpp
    src/connection/ble/BleGattBrowserDisplay.cpp
    src/connection/ble/BleConfigPanel.cpp
    src/connection/ble/BleConfigPanelSlots.cpp
    src/connection/ble/BluetoothAtConsole.cpp
    src/connection/ble/BluetoothAtConsoleStats.cpp

    # ── F13: CAN/CAN-FD 总线 ──
    src/connection/can/CanConnection.cpp
    src/connection/can/CanConnectionConfig.cpp
    src/connection/can/CanConnectionProtocol.cpp
    src/connection/can/CanConnectionFrame.cpp
    src/connection/can/CanFrameParser.cpp
    src/connection/can/CanFrameParserBuild.cpp
    src/connection/can/CanBusMonitor.cpp
    src/connection/can/CanBusMonitorFrame.cpp
    src/connection/can/CanBusMonitorColor.cpp
    src/connection/can/CanBusMonitorStats.cpp
    src/connection/can/CanConfigPanel.cpp
    src/connection/can/CanConfigPanelPersist.cpp
    src/protocol/can/DbcParser.cpp
    src/protocol/can/DbcParserParse.cpp
    src/protocol/can/DbcParserSignal.cpp
    src/protocol/can/DbcParserDecode.cpp

    # ── F14: MQTT 客户端 ──
    src/connection/mqtt/MqttConnection.cpp
    src/connection/mqtt/MqttConnectionProtocol.cpp
    src/connection/mqtt/MqttConnectionQueue.cpp
    src/connection/mqtt/MqttConnectionStats.cpp
    src/connection/mqtt/MqttConnectionPublish.cpp
    src/connection/mqtt/MqttTopicModel.cpp
    src/connection/mqtt/MqttTopicModelHelpers.cpp
    src/connection/mqtt/MqttTopicModelRoute.cpp
    src/connection/mqtt/MqttTopicModelIndex.cpp
    src/connection/mqtt/MqttSubscriptionPanel.cpp
    src/connection/mqtt/MqttConfigPanel.cpp
    src/connection/mqtt/MqttWidget.cpp
    src/connection/mqtt/MqttWidgetSubs.cpp
    src/connection/mqtt/MqttWidgetSubsSignals.cpp
    src/connection/mqtt/MqttWidgetSlots.cpp

    # ── F15: TCP/UDP 增强 ──
    src/connection/tcp/TcpServerConnection.cpp
    src/connection/tcp/TcpServerConnectionClients.cpp
    src/connection/tcp/TcpServerConnectionHandlers.cpp
    src/connection/tcp/TcpMultiConnectionManager.cpp
    src/connection/tcp/UdpMulticastConnection.cpp
    src/connection/tcp/UdpMulticastConnectionHandlers.cpp
    src/connection/tcp/UdpMulticastGroup.cpp
    src/connection/tcp/TlsConnection.cpp
    src/connection/tcp/TlsConnectionSetup.cpp
    src/connection/tcp/TlsConnectionStats.cpp
    src/connection/tcp/MultiConnectionPanel.cpp
    src/connection/tcp/MultiConnectionPanelUI.cpp

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
    src/connection/spi_i2c/SpiI2cConfigPanelUI.cpp
    src/connection/spi_i2c/SpiI2cConfigPanelPersist.cpp
    src/connection/spi_i2c/RegisterEditor.cpp
    src/connection/spi_i2c/RegisterEditorUI.cpp
    src/connection/spi_i2c/RegisterEditorStats.cpp

    # ── F17: WebSocket 客户端 ──
    src/connection/ws/WebSocketConnection.cpp
    src/connection/ws/WebSocketConnectionHandshake.cpp
    src/connection/ws/WebSocketFrame.cpp
    src/connection/ws/WebSocketFrameStats.cpp
    src/connection/ws/WsConfigPanel.cpp

    # ── F18: Modbus 主站/从站 ──
    # -- F44: Modbus Master --
    src/protocol/modbus/ModbusMaster.cpp
    src/protocol/modbus/ModbusMasterStats.cpp
    src/protocol/modbus/ModbusMasterProtocol.cpp
    src/protocol/modbus/ModbusSlave.cpp
    src/protocol/modbus/ModbusSlaveResponse.cpp
    src/protocol/modbus/ModbusSlaveStats.cpp
    src/protocol/modbus/ModbusScanWidget.cpp
    src/protocol/modbus/ModbusScanWidgetScan.cpp
    src/protocol/modbus/ModbusScanWidgetStats.cpp
    src/protocol/modbus/ModbusConfigPanel.cpp

    # ── F19: Protobuf/FlatBuffers 解码 ──
    src/protocol/protobuf/ProtobufDecoder.cpp
    src/protocol/protobuf/ProtobufDecoderEncoding.cpp
    src/protocol/protobuf/ProtobufDecoderVarint.cpp
    src/protocol/protobuf/ProtobufDecoderField.cpp
    src/protocol/protobuf/ProtobufDecoderHelpers.cpp
    src/protocol/protobuf/FlatBuffersDecoder.cpp
    src/protocol/protobuf/FlatBuffersDecoderFields.cpp
    src/protocol/protobuf/FlatBuffersDecoderRead.cpp
    src/protocol/protobuf/FlatBuffersDecoderSchema.cpp
    src/protocol/protobuf/FlatBuffersDecoderParse.cpp
    src/protocol/protobuf/SchemaViewer.cpp
    src/protocol/protobuf/SchemaViewerStats.cpp

    # ── F20: USB 调试 ──
    src/connection/usb/UsbConnection.cpp
    src/connection/usb/UsbConnectionLifecycle.cpp
    src/connection/usb/UsbConnectionInterface.cpp
    src/connection/usb/UsbConnectionTransfer.cpp
    src/connection/usb/UsbConnectionDescriptors.cpp
    src/connection/usb/UsbDeviceDetector.cpp
    src/connection/usb/UsbDeviceDetectorDescriptor.cpp
    src/connection/usb/UsbDeviceDetectorWmic.cpp
    src/connection/usb/UsbDeviceDetectorStats.cpp
    src/connection/usb/UsbDescriptorViewer.cpp
    src/connection/usb/UsbDescriptorViewerTree.cpp
    src/connection/usb/UsbConfigPanel.cpp
    src/connection/usb/UsbLibraryLoader.cpp
    src/connection/usb/UsbLibraryLoaderQuery.cpp

    # ── F21: 正则终端过滤器 ──
    src/terminal/filter/TerminalFilter.cpp
    src/terminal/filter/TerminalFilterCompat.cpp
    src/terminal/filter/TerminalFilterApply.cpp
    src/terminal/filter/TerminalFilterStats.cpp
    src/terminal/filter/TerminalFilterBar.cpp
    src/terminal/filter/TerminalHighlighter.cpp
    src/terminal/filter/TerminalHighlighterHighlight.cpp

    # ── F22: 校验和计算器 ──
    src/utils/checksum/ChecksumCalculator.cpp
    src/utils/checksum/ChecksumCalculatorStats.cpp
    src/utils/checksum/ChecksumPanel.cpp
    src/utils/checksum/ChecksumPanelAlgo.cpp
    src/utils/checksum/ChecksumPanelSlots.cpp
    src/utils/checksum/DataChecksumVerifier.cpp
    src/utils/checksum/DataChecksumVerifierAlgo.cpp
    src/utils/checksum/DataChecksumVerifierStats.cpp

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
    src/utils/packet/PacketBuilderBuild.cpp
    src/utils/packet/PacketBuilderTemplate.cpp
    src/utils/packet/PacketBuilderPanel.cpp
    src/utils/packet/PacketBuilderPanelFields.cpp

    # ── 数据检查器 ──
    src/utils/data_inspector/DataInspectorWidget.cpp
    src/utils/data_inspector/DataInspectorWidgetStats.cpp

    # ── F26: 设备注册表 ──
    src/core/device/DeviceRegistry.cpp
    src/core/device/DeviceProfilePanel.cpp
    src/core/device/DeviceProfilePanelActions.cpp

    # ── 信号发生器 ──
    src/utils/signal_gen/SignalGeneratorWidget.cpp
    src/utils/signal_gen/SignalGeneratorWidgetStats.cpp

    # ── F27: 数据字节级对比 ──
    src/utils/compare/DataCompareWidget.cpp
    src/utils/compare/DataCompareWidgetStats.cpp

    # ── F37: 数据比较器 ──
    src/utils/compare/DataComparator.cpp
    src/utils/compare/DataComparatorStats.cpp

    # ── 频率计数器 ──
    src/utils/frequency/FrequencyCounterWidget.cpp
    src/utils/frequency/FrequencyCounterWidgetStats.cpp

    # ── F28: 数据流标注管理 ──
    src/core/profiles/ConnectionProfileManager.cpp
    src/utils/annotation/DataAnnotationWidget.cpp
    src/utils/annotation/DataAnnotationWidgetStats.cpp

    # ── F29: 报文模板库 ──
    src/utils/packet_lib/PacketTemplateLib.cpp
    src/utils/packet_lib/PacketTemplateLibBuiltins.cpp
    src/utils/packet_lib/PacketTemplateLibStats.cpp

    # ── F30: 数据掩码编辑器 ──
    src/utils/bitmask/DataMaskEditor.cpp
    src/utils/bitmask/DataMaskEditorIo.cpp
    src/utils/bitmask/DataMaskEditorStats.cpp

    # ── F31: 数据包重组引擎 ──
    src/protocol/reassembly/PacketReassembler.cpp
    src/protocol/reassembly/PacketReassemblerStats.cpp

    # ── F31: 字节模式搜索器 ──
    src/utils/pattern/BytePatternAnalyzer.cpp
    src/utils/pattern/BytePatternAnalyzerStats.cpp
    src/utils/pattern/DataPatternDetector.cpp
    src/utils/scaler/DataScalerWidget.cpp
    src/utils/normalize/DataNormalizer.cpp
    src/utils/threshold/DataThresholdMonitor.cpp
    src/utils/capture/DataCaptureBuffer.cpp
    src/utils/crc_verifier/CrcStreamVerifier.cpp
    src/utils/segment/DataSegmentAnalyzer.cpp
    src/utils/outlier/OutlierDetector.cpp
    src/utils/quality/DataQualityScorer.cpp
    src/utils/sampler/DataSampler.cpp
    src/utils/interpolator/DataInterpolator.cpp
    src/utils/correlator/DataCorrelator.cpp
    src/utils/anomaly/AnomalyDetector.cpp
    src/utils/filter2/DigitalFilter.cpp
    src/utils/predictor/TrendPredictor.cpp
    src/utils/histogram/HistogramBuilder.cpp
    src/utils/spectrum/SpectrumAnalyzer.cpp
    src/utils/statdist/StatDistribution.cpp
    src/utils/entropy/EntropyCalculator.cpp
    src/utils/crosscorr/CrossCorrelator.cpp
    src/utils/derivative/NumericalDerivative.cpp
    src/utils/window/DataWindowManager.cpp
    src/utils/decomposer/SignalDecomposer.cpp
    src/utils/sliding/SlidingWindowStats.cpp
    src/utils/trigger/DataTrigger.cpp
    src/utils/state/StateTracker.cpp
    src/utils/cycle/CycleDetector.cpp
    src/utils/smoother/DataSmoother.cpp
    src/utils/peaks/PeakDetector.cpp
    src/utils/warp/TimeWarp.cpp
    src/utils/resample/DataResampler.cpp
    src/utils/classifier/DataClassifier.cpp
    src/utils/quantize/DataQuantizer.cpp
    src/utils/autocorr/AutoCorrelator.cpp
    src/utils/peakdetect/RidgeDetector.cpp
    src/utils/freq/FrequencyCounter2.cpp
    src/utils/integrator/NumericalIntegrator.cpp
    src/utils/convolution/ConvolutionEngine.cpp
    src/utils/modulation/Demodulator.cpp
    src/utils/pid/PidController.cpp
    src/utils/fft2/FftPipeline.cpp
    src/utils/bayes/BayesClassifier.cpp
    src/utils/regress/RegressionEngine.cpp
    src/utils/pca/PcaAnalyzer.cpp
    src/utils/dtw2/FastDtw.cpp
    src/utils/bitfield/BitFieldParser.cpp
    src/utils/circbuf/CircularStatsBuffer.cpp
    src/utils/hysteresis/HysteresisFilter.cpp
    src/utils/kalman/KalmanFilter1D.cpp
    src/utils/wavelet/WaveletTransform.cpp
    src/utils/queuetheory/QueueSimulator.cpp
    src/utils/monte/MonteCarloSimulator.cpp
    src/utils/graph/GraphAnalyzer.cpp
    src/utils/signalgen/SignalGenerator.cpp
    src/utils/ringbuf2/RingBufferEx.cpp
    src/utils/bitalloc/BitAllocator.cpp
    src/utils/spinbox/SpinLockQueue.cpp
    src/utils/timerpool/TimerPool.cpp
    src/utils/detrend/DetrendEngine.cpp
    src/utils/zscore/ZScoreNormalizer.cpp
    src/utils/medoid/KMedoidClusterer.cpp
    src/utils/votecomp/VotingCompositor.cpp
    src/utils/markov2/MarkovChain.cpp
    src/utils/fuzzy2/FuzzyLogicEngine.cpp
    src/utils/crc64b/Crc64Engine.cpp
    src/utils/rle/RunLengthCodec.cpp
    src/utils/lzw/LzwCompressor.cpp
    src/utils/huffman/HuffmanCodec.cpp
    src/utils/shannon/ShannonCoder.cpp
    src/utils/haar/HaarWavelet.cpp
    src/utils/bezier/BezierSpline.cpp
    src/utils/voicedetect/VoiceActivityDetector.cpp
    src/utils/dynhistogram/DynamicHistogram.cpp
    src/utils/ticker/TickEngine.cpp
    src/utils/segmenter/StreamSegmenter.cpp
    src/utils/cluster/KMeansClusterer.cpp
    src/utils/changept/ChangepointDetector.cpp
    src/utils/eye/EyeDiagramEngine.cpp
    src/utils/symbol/SymbolDecoder.cpp

    # ── 滚动折线图 ──
    src/widgets/chart/ScrollChartWidget.cpp
    src/widgets/chart/ScrollChartWidgetStats.cpp

    # ── 示波器通道管理 ──
    src/widgets/scope/ScopeChannelManager.cpp
    src/widgets/scope/ScopeChannelManagerStats.cpp

    # ── LED矩阵模拟器 ──
    src/widgets/led_matrix/LedMatrixSimulator.cpp
    src/widgets/led_matrix/LedMatrixSimulatorStats.cpp

    # ── F32: 十六进制内存编辑器 ──
    src/utils/hex_editor/MemoryHexEditor.cpp
    src/utils/hex_editor/MemoryHexEditorExport.cpp
    src/utils/hex_editor/MemoryHexEditorInternal.cpp
    src/utils/hex_editor/MemoryHexEditorStats.cpp

    # ── 波形模式检测 ──
    src/core/analysis/WaveformPatternDetector.cpp
    src/core/analysis/WaveformPatternDetectorStats.cpp

    # ── F29: 协议定时分析器 ──
    src/utils/protocol_timer/ProtocolTimerWidget.cpp
    src/utils/protocol_timer/ProtocolTimerWidgetStats.cpp

    # ── F33: 设备配置档案管理 ──
    src/core/settings/ConfigProfileManager.cpp
    src/core/settings/ConfigProfileManagerStats.cpp

    # ── 数据流过滤器 ──
    src/core/filter/DataStreamFilter.cpp
    src/core/filter/DataStreamFilterStats.cpp

    # ── 协议响应模拟器 ──
    src/protocol/simulator/ProtocolSimulator.cpp
    src/protocol/simulator/ProtocolSimulatorStats.cpp

    # ── 协议字段编辑器 ──
    src/protocol/field_editor/ProtocolFieldEditor.cpp
    src/protocol/field_editor/ProtocolFieldEditorUI.cpp
    src/protocol/field_editor/ProtocolFieldEditorStats.cpp

    # ── F34: JS 可脚本化协议引擎 ──

    # ── 通知历史 ──
    src/core/notification/NotificationHistory.cpp
    src/core/notification/NotificationHistoryStats.cpp

    # ── 串口流量分析 ──
    src/serial/profiler/SerialPortProfiler.cpp
    src/serial/profiler/SerialPortProfilerStats.cpp

    # ── 串口时序分析 ──
    src/serial/timing/SerialTimingAnalyzer.cpp
    src/serial/timing/SerialTimingAnalyzerStats.cpp

    # ── 环形缓冲区可视化 ──
    src/widgets/circular/CircularBufferWidget.cpp
    src/widgets/circular/CircularBufferWidgetStats.cpp

    # ── 波形发生器 ──
    src/utils/waveform/WaveformGenerator.cpp
    src/utils/waveform/WaveformGeneratorExport.cpp
    src/utils/waveform/WaveformGeneratorStats.cpp

    # ── F35: 数据流分割器 ──
    src/utils/splitter/DataStreamSplitter.cpp
    src/utils/splitter/DataStreamSplitterStats.cpp

    # ── F34: 数据速率计算器 ──
    src/utils/rate/DataRateCalculator.cpp
    src/utils/rate/DataRateCalculatorStats.cpp

    # ── F33: 数据编解码机 ──
    src/utils/encoder/SerialDataEncoder.cpp
    src/utils/encoder/SerialDataEncoderStats.cpp

    # ── F36: 丢包检测器 ──
    src/utils/loss/PacketLossDetector.cpp
    src/utils/loss/PacketLossDetectorStats.cpp

    # ── F38: 字节频率分析器 ──
    src/utils/frequency2/ByteFrequencyAnalyzer.cpp
    src/utils/frequency2/ByteFrequencyAnalyzerStats.cpp

    # ── F39: 协议模糊测试器 ──
    src/utils/fuzzer/SerialProtocolFuzzer.cpp
    src/utils/fuzzer/SerialProtocolFuzzerStats.cpp

    # ── F40: 帧模板库 ──
    src/utils/template_lib/FrameTemplateLibrary.cpp
    src/utils/template_lib/FrameTemplateLibraryStats.cpp

    # ── F41: 帧解码器 ──
    src/utils/decoder/SerialFrameDecoder.cpp
    src/utils/decoder/SerialFrameDecoderBuild.cpp

    # -- F42: 数据加密引擎 --
    src/utils/crypto/DataEncryptionEngine.cpp

    # -- F46: 波形滤波器 --
    src/chart/filter/WaveformFilter.cpp
    src/chart/filter/WaveformFilterCompute.cpp

    # -- F47: 数据流录制器 --
    src/utils/recorder/StreamCaptureRecorder.cpp
    src/utils/recorder/StreamCaptureRecorderExport.cpp

    # -- F48: Hex差异引擎 --
    src/utils/hex_diff/HexDiffEngine.cpp
    src/utils/hex_diff/HexDiffEngineDiff.cpp
    src/utils/hex_diff/HexDiffEnginePatch.cpp

    # -- F43: 高级数据日志 --
    src/utils/logger2/SerialDataLogger.cpp
    src/utils/logger2/SerialDataLoggerRotation.cpp
    src/utils/logger2/SerialDataLoggerStats.cpp
    src/utils/logger2/SerialDataLoggerExport.cpp

    # -- F45: 数据包可视化 --
    src/protocol/visual/PacketVisualizer.cpp
    src/protocol/visual/PacketVisualizerPaint.cpp
    src/protocol/visual/PacketVisualizerStats.cpp

    # -- F49: 协议会话管理 --
    src/protocol/session/ProtocolSession.cpp
    src/protocol/session/ProtocolSessionStats.cpp

    # -- F50: 串口脚本引擎 --
    src/serial/script/SerialScriptEngine.cpp
    src/serial/script/SerialScriptEngineStats.cpp

    # -- F51: 数据同步引擎 --
    src/utils/synchronizer/DataSynchronizer.cpp

    # -- F52: 数据速率限制器 --
    src/utils/rate/DataRateLimiter.cpp

    # -- F52: 滑动窗口聚合器 --
    src/utils/aggregator2/WindowedAggregator.cpp

    # -- F53: 数据降采样引擎 --
    src/utils/reductor/DataReducer.cpp

    # -- F54: 数据缓存 --
    src/utils/cache/DataCache.cpp

    # -- F55: 串口设备模拟器 --
    src/serial/simulator/SerialSimulator.cpp
    src/serial/simulator/SerialSimulatorProfiles.cpp

    # -- F53: 字节流解析器 --
    src/protocol/parser2/ByteStreamParser.cpp
    src/protocol/parser2/ByteStreamParserStateMachine.cpp

    # -- F56: 数据验证器 --
    src/utils/validator/DataValidator.cpp
    src/utils/validator/DataValidatorRules.cpp

    # -- F55: 数据压缩器 --
    src/utils/compress/DataCompressor.cpp
    src/utils/compress/DataCompressorAlgo.cpp

    # -- F62: 数据包组装器 --
    src/protocol/assembly/PacketAssembler.cpp
    src/protocol/assembly/PacketAssemblyOps.cpp

    # -- F60: 协议流量复制 --
    src/protocol/duplicate/ProtocolDuplicator.cpp
    src/protocol/duplicate/ProtocolDuplicatorStats.cpp
    src/protocol/duplicate/ProtocolDuplicatorIO.cpp

    # -- F61: 串口健康监控 --
    src/serial/health/SerialHealthMonitor.cpp

    # -- F57: 数据流量计 --
    src/utils/flow/DataFlowMeter.cpp

    # -- F58: 协议回放引擎 --
    src/protocol/replay/ProtocolReplayEngine.cpp

    # -- F59: 数据变换管线 --
    src/utils/transform/DataTransformer.cpp

    # -- F63: 数据批处理器 --
    src/utils/batch/DataBatchProcessor.cpp

    # -- F64: 数据采样器 --
    src/utils/sampler/DataSampler.cpp

    # -- F65: 多流数据合并器 --
    src/utils/merger/DataStreamMerger.cpp

    # -- F66: 环形缓冲区 --
    src/utils/ring/DataRingBuffer.cpp

    # -- F67: 协议帧验证器 --
    src/protocol/validator/ProtocolFrameValidator.cpp

    # -- F68: 数据关联器 --
    src/utils/correlator/DataCorrelator.cpp

    # -- F69: 数据插值器 --
    src/utils/interpolator/DataInterpolator.cpp

    # -- F70: 数据质量评分 --
    src/utils/quality/DataQualityScorer.cpp

    # -- F71: 事件时间线 --
    src/utils/timeline/EventTimeline.cpp

    # -- F128~F142: Wave 22 工具模块 --
    src/utils/fir/FirFilterDesigner.cpp
    src/utils/iir/IirFilterDesigner.cpp
    src/utils/lpc/LinearPredictiveCoder.cpp
    src/utils/envelope/EnvelopeDetector.cpp
    src/utils/stft/StftAnalyzer.cpp
    src/utils/zcr/ZeroCrossingRate.cpp
    src/utils/thd/ThdAnalyzer.cpp
    src/utils/hmm/HiddenMarkovModel.cpp
    src/utils/viterbi/ViterbiDecoder.cpp
    src/utils/gmm/GaussianMixtureModel.cpp
    src/utils/covariance/CovarianceMatrix.cpp
    src/utils/percentile/PercentileCalculator.cpp
    src/utils/scrambler/Scrambler.cpp
    src/utils/bloom/BloomFilter.cpp

    # -- F143~F157: Wave 23 工具模块 --
    src/utils/agc/AutomaticGainControl.cpp
    src/utils/rms/RmsMeter.cpp
    src/utils/levenshtein/LevenshteinDistance.cpp
    src/utils/ngram/NgramAnalyzer.cpp
    src/utils/aho/AhoCorasickMatcher.cpp
    src/utils/rabin/RabinKarpMatcher.cpp
    src/utils/interleaver/BlockInterleaver.cpp
    src/utils/cepstrum/CepstrumAnalyzer.cpp
    src/utils/gabor/GaborTransform.cpp
    src/utils/pitch/PitchDetector.cpp
    src/utils/ttest/TTestCalculator.cpp
    src/utils/moment/StatisticalMoments.cpp
    src/utils/ks/KsTestCalculator.cpp
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
    src/core/recording/DataStreamRecorder.h
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
    src/chart/preview/WaveformPreviewWidget.h
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

    # ── 数据历史环形缓冲区 ──
    src/core/data/DataHistoryBuffer.h
    src/core/data/DataHistoryBuffer.ipp

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
    src/utils/pipeline/DataPipeline.h
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
    src/dashboard/MiniChartWidget.h
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
    # -- F44: Modbus Master --
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
    src/utils/checksum/DataChecksumVerifier.h

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

    # ── 数据检查器 ──
    src/utils/data_inspector/DataInspectorWidget.h

    # ── F26: 设备注册表 ──
    src/core/device/DeviceProfile.h
    src/core/device/DeviceRegistry.h
    src/core/device/DeviceProfilePanel.h

    # ── 信号发生器 ──
    src/utils/signal_gen/SignalGeneratorWidget.h

    # ── 频率计数器 ──
    src/utils/frequency/FrequencyCounterWidget.h

    # ── F37: 数据比较器 ──
    src/utils/compare/DataComparator.h

    # ── F28: 数据流标注管理 ──
    src/utils/annotation/DataAnnotationWidget.h

    # ── F29: 报文模板库 ──
    src/utils/packet_lib/PacketTemplateLib.h

    # ── F30: 数据掩码编辑器 ──
    src/utils/bitmask/DataMaskEditor.h

    # ── F31: 数据包重组引擎 ──
    src/protocol/reassembly/PacketReassembler.h

    # ── F31: 字节模式搜索器 ──
    src/utils/pattern/BytePatternAnalyzer.h

    # ── 滚动折线图 ──
    src/widgets/chart/ScrollChartWidget.h

    # ── 示波器通道管理 ──
    src/widgets/scope/ScopeChannelManager.h

    # ── LED矩阵模拟器 ──
    src/widgets/led_matrix/LedMatrixSimulator.h

    # ── F32: 十六进制内存编辑器 ──
    src/utils/hex_editor/MemoryHexEditor.h

    # ── F29: 协议定时分析器 ──
    src/utils/protocol_timer/ProtocolTimerWidget.h

    # ── 波形模式检测 ──
    src/core/analysis/WaveformPatternDetector.h

    # ── F33: 设备配置档案管理 ──
    src/core/settings/ConfigProfileManager.h

    # ── 数据流过滤器 ──
    src/core/filter/DataStreamFilter.h

    # ── 连接配置档案 ──
    src/core/profiles/ConnectionProfileManager.h

    # ── 协议响应模拟器 ──
    src/protocol/simulator/ProtocolSimulator.h

    # ── 协议字段编辑器 ──
    src/protocol/field_editor/ProtocolFieldEditor.h

    # ── F34: JS 可脚本化协议引擎 ──

    # ── 通知历史 ──
    src/core/notification/NotificationHistory.h

    # ── 串口流量分析 ──
    src/serial/profiler/SerialPortProfiler.h

    # ── 串口时序分析 ──
    src/serial/timing/SerialTimingAnalyzer.h

    # ── 环形缓冲区可视化 ──
    src/widgets/circular/CircularBufferWidget.h

    # ── 波形发生器 ──
    src/utils/waveform/WaveformGenerator.h

    # ── F35: 数据流分割器 ──
    src/utils/splitter/DataStreamSplitter.h

    # ── F34: 数据速率计算器 ──
    src/utils/rate/DataRateCalculator.h

    # -- F52: 数据速率限制器 --
    src/utils/rate/DataRateLimiter.h

    # ── F33: 数据编解码机 ──
    src/utils/encoder/SerialDataEncoder.h

    # ── F36: 丢包检测器 ──
    src/utils/loss/PacketLossDetector.h

    # ── F38: 字节频率分析器 ──
    src/utils/frequency2/ByteFrequencyAnalyzer.h

    # ── F39: 协议模糊测试器 ──
    src/utils/fuzzer/SerialProtocolFuzzer.h

    # ── F40: 帧模板库 ──
    src/utils/template_lib/FrameTemplateLibrary.h

    # ── F41: 帧解码器 ──
    src/utils/decoder/SerialFrameDecoder.h

    # -- F42: 数据加密引擎 --
    src/utils/crypto/DataEncryptionEngine.h

    # -- F46: 波形滤波器 --
    src/chart/filter/WaveformFilter.h

    # -- F47: 数据流录制器 --
    src/utils/recorder/StreamCaptureRecorder.h

    # -- F48: Hex差异引擎 --
    src/utils/hex_diff/HexDiffEngine.h

    # -- F43: 高级数据日志 --
    src/utils/logger2/SerialDataLogger.h

    # -- F45: 数据包可视化 --
    src/protocol/visual/PacketVisualizer.h

    # -- F49: 协议会话管理 --
    src/protocol/session/ProtocolSession.h

    # -- F50: 串口脚本引擎 --
    src/serial/script/SerialScriptEngine.h

    # -- F51: 数据同步引擎 --
    src/utils/synchronizer/DataSynchronizer.h

    # -- F52: 滑动窗口聚合器 --
    src/utils/aggregator2/WindowedAggregator.h

    # -- F53: 数据降采样引擎 --
    src/utils/reductor/DataReducer.h

    # -- F53: 字节流解析器 --
    src/protocol/parser2/ByteStreamParser.h

    # -- F55: 串口设备模拟器 --
    src/serial/simulator/SerialSimulator.h

    # -- F54: 数据缓存 --
    src/utils/cache/DataCache.h

    # -- F56: 数据验证器 --
    src/utils/validator/DataValidator.h

    # -- F55: 数据压缩器 --
    src/utils/compress/DataCompressor.h

    # -- F62: 数据包组装器 --
    src/protocol/assembly/PacketAssembler.h

    # -- F60: 协议流量复制 --
    src/protocol/duplicate/ProtocolDuplicator.h

    # -- F61: 串口健康监控 --
    src/serial/health/SerialHealthMonitor.h

    # -- F57: 数据流量计 --
    src/utils/flow/DataFlowMeter.h

    # -- F58: 协议回放引擎 --
    src/protocol/replay/ProtocolReplayEngine.h

    # -- F59: 数据变换管线 --
    src/utils/transform/DataTransformer.h

    # -- F63: 数据批处理器 --
    src/utils/batch/DataBatchProcessor.h

    # -- F64: 数据采样器 --
    src/utils/sampler/DataSampler.h

    # -- F65: 多流数据合并器 --
    src/utils/merger/DataStreamMerger.h

    # -- F66: 环形缓冲区 --
    src/utils/ring/DataRingBuffer.h

    # -- F67: 协议帧验证器 --
    src/protocol/validator/ProtocolFrameValidator.h

    # -- F68: 数据关联器 --
    src/utils/correlator/DataCorrelator.h

    # -- F69: 数据插值器 --
    src/utils/interpolator/DataInterpolator.h

    # -- F70: 数据质量评分 --
    src/utils/quality/DataQualityScorer.h

    # -- F71: 事件时间线 --
    src/utils/timeline/EventTimeline.h

    # -- F72: 异常检测器 --
    src/utils/anomaly/AnomalyDetector.h

    # -- F73: 数字滤波器 --
    src/utils/filter2/DigitalFilter.h

    # -- F74: 趋势预测器 --
    src/utils/predictor/TrendPredictor.h

    # -- F75: 直方图构建器 --
    src/utils/histogram/HistogramBuilder.h

    # -- F76: 频谱分析器 --
    src/utils/spectrum/SpectrumAnalyzer.h

    # -- F77: 统计分布 --
    src/utils/statdist/StatDistribution.h

    # -- F78: 熵计算器 --
    src/utils/entropy/EntropyCalculator.h

    # -- F78b: 数据归一化器 --
    src/utils/normalize/DataNormalizer.h

    # -- F79: 互相关分析器 --
    src/utils/crosscorr/CrossCorrelator.h

    # -- F80: 数值微分器 --
    src/utils/derivative/NumericalDerivative.h

    # -- F81: 数据窗口管理器 --
    src/utils/window/DataWindowManager.h

    # -- F82: 数据分类器 --
    src/utils/classifier/DataClassifier.h

    # -- F83: 数据量化器 --
    src/utils/quantize/DataQuantizer.h

    # -- F84: 数据重采样器 --
    src/utils/resample/DataResampler.h

    # -- F85: 自相关器 --
    src/utils/autocorr/AutoCorrelator.h

    # -- F86: 脊线检测器 --
    src/utils/peakdetect/RidgeDetector.h

    # -- F87: 高精度频率计数器 --
    src/utils/freq/FrequencyCounter2.h

    # -- F88: 数值积分器 --
    src/utils/integrator/NumericalIntegrator.h

    # -- F89: 卷积引擎 --
    src/utils/convolution/ConvolutionEngine.h

    # -- F90: 解调器 --
    src/utils/modulation/Demodulator.h

    # -- F91: 离群值检测器 --
    src/utils/outlier/OutlierDetector.h

    # -- F92: 数据流分段器 --
    src/utils/segmenter/StreamSegmenter.h

    # -- F93: K-Means聚类器 --
    src/utils/cluster/KMeansClusterer.h

    # -- F94: 变点检测器 --
    src/utils/changept/ChangepointDetector.h

    # -- F95: PID控制器 --
    src/utils/pid/PidController.h

    # -- F96: FFT管道 --
    src/utils/fft2/FftPipeline.h

    # -- F97: 贝叶斯分类器 --
    src/utils/bayes/BayesClassifier.h

    # -- F98: 回归引擎 --
    src/utils/regress/RegressionEngine.h

    # -- F99: PCA主成分分析器 --
    src/utils/pca/PcaAnalyzer.h

    # -- F100: 快速DTW距离 --
    src/utils/dtw2/FastDtw.h
    src/utils/eye/EyeDiagramEngine.h

    # -- F96: 符号解码器 --
    src/utils/symbol/SymbolDecoder.h

    # -- F101: 一维卡尔曼滤波器 --
    src/utils/kalman/KalmanFilter1D.h

    # -- F102: 小波变换引擎 --
    src/utils/wavelet/WaveletTransform.h

    # -- F103: 位域解析器 --
    src/utils/bitfield/BitFieldParser.h

    # -- F104: 统计环形缓冲区 --
    src/utils/circbuf/CircularStatsBuffer.h

    # -- F105: 滞后滤波器 --
    src/utils/hysteresis/HysteresisFilter.h

    # -- F110: 队列理论模拟器 --
    src/utils/queuetheory/QueueSimulator.h

    # -- F111: 蒙特卡洛模拟器 --
    src/utils/monte/MonteCarloSimulator.h

    # -- F112: 图分析器 --
    src/utils/graph/GraphAnalyzer.h

    # -- F113: 信号发生器 --
    src/utils/signalgen/SignalGenerator.h

    # -- F114: 扩展环形缓冲区 --
    src/utils/ringbuf2/RingBufferEx.h

    # -- F115: 位分配器 --
    src/utils/bitalloc/BitAllocator.h

    # -- F116: 自旋锁队列 --
    src/utils/spinbox/SpinLockQueue.h

    # -- F117: 定时器池 --
    src/utils/timerpool/TimerPool.h

    # -- F118: 去趋势引擎 --
    src/utils/detrend/DetrendEngine.h

    # -- F119: Z-Score标准化器 --
    src/utils/zscore/ZScoreNormalizer.h

    # -- F120: K-Medoid聚类器 --
    src/utils/medoid/KMedoidClusterer.h

    # -- F121: 投票组合器 --
    src/utils/votecomp/VotingCompositor.h

    # -- F122: 马尔可夫链 --
    src/utils/markov2/MarkovChain.h

    # -- F123: 模糊逻辑引擎 --
    src/utils/fuzzy2/FuzzyLogicEngine.h

    # -- F124: CRC-64校验引擎 --
    src/utils/crc64b/Crc64Engine.h

    # -- F125: 游程编码 --
    src/utils/rle/RunLengthCodec.h

    # -- F126: LZW压缩 --
    src/utils/lzw/LzwCompressor.h

    # -- F127: 霍夫曼编码 --
    src/utils/huffman/HuffmanCodec.h
    src/utils/shannon/ShannonCoder.h
    src/utils/haar/HaarWavelet.h
    src/utils/bezier/BezierSpline.h
    src/utils/voicedetect/VoiceActivityDetector.h
    src/utils/dynhistogram/DynamicHistogram.h
    src/utils/ticker/TickEngine.h

    # -- F128~F142: Wave 22 工具模块 --
    src/utils/fir/FirFilterDesigner.h
    src/utils/iir/IirFilterDesigner.h
    src/utils/lpc/LinearPredictiveCoder.h
    src/utils/envelope/EnvelopeDetector.h
    src/utils/stft/StftAnalyzer.h
    src/utils/zcr/ZeroCrossingRate.h
    src/utils/thd/ThdAnalyzer.h
    src/utils/lru/LruCache.h
    src/utils/bloom/BloomFilter.h
    src/utils/hmm/HiddenMarkovModel.h
    src/utils/viterbi/ViterbiDecoder.h
    src/utils/gmm/GaussianMixtureModel.h
    src/utils/covariance/CovarianceMatrix.h
    src/utils/percentile/PercentileCalculator.h
    src/utils/scrambler/Scrambler.h

    # -- F143~F157: Wave 23 工具模块 --
    src/utils/agc/AutomaticGainControl.h
    src/utils/rms/RmsMeter.h
    src/utils/levenshtein/LevenshteinDistance.h
    src/utils/ngram/NgramAnalyzer.h
    src/utils/aho/AhoCorasickMatcher.h
    src/utils/rabin/RabinKarpMatcher.h
    src/utils/interleaver/BlockInterleaver.h
    src/utils/cepstrum/CepstrumAnalyzer.h
    src/utils/gabor/GaborTransform.h
    src/utils/pitch/PitchDetector.h
    src/utils/ttest/TTestCalculator.h
    src/utils/moment/StatisticalMoments.h
    src/utils/ks/KsTestCalculator.h
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
    Qt6::Qml
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

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
    # src/utils/logviewer/DataLogViewer.cpp (file missing)
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
    # DUPLICATE REMOVED: src/terminal/search/TerminalSearchBuild.cpp
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
    # DUPLICATE REMOVED: src/protocol/bridge/ProtocolBridgeManagerConfig.cpp
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
    src/chart/eye/EyeDiagramEngine.cpp
    src/chart/eye/EyeDiagramEngineStats.cpp
    src/chart/eye/EyeDiagramWidget.cpp
    src/chart/eye/EyeDiagramWidgetStats.cpp
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
    # DUPLICATE REMOVED: src/ota/protocols/zmodem/ZModemTransferFrames.cpp
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
    # DUPLICATE REMOVED: src/dashboard/DashboardSerializerProfileCrud.cpp
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
    # src/utils/peaks/PeakDetector.cpp  # removed: duplicate of peak/PeakDetector
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
    src/utils/pid/PidSimulator.cpp
    src/utils/pid/PidSimulatorStats.cpp
    src/utils/pid/PidTunerWidget.cpp
    src/utils/pid/PidTunerWidgetStats.cpp
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
    src/utils/circqueue/CircularPriorityQueue.cpp
    src/utils/rollingrank/RollingRank.cpp
    src/utils/expmove/ExponentialMoving.cpp
    src/utils/polyreg/PolynomialRegression.cpp
    src/utils/gof/GoodnessOfFit.cpp
    src/utils/ringbuffer3/LockFreeRingBuffer.cpp
    src/utils/tokenizer/StreamTokenizer.cpp
    src/utils/suffixarray/SuffixArray.cpp
    src/utils/minhash/MinHashSignature.cpp
    src/utils/timerwheel/TimerWheel.cpp
    src/utils/slidingpercentile/SlidingPercentile.cpp
    # src/utils/autopcor/AutoCorrelation.cpp  # removed: duplicate class
    src/utils/deadzone/DeadZoneFilter.cpp
    src/utils/movingmax/MovingMinMax.cpp
    src/utils/ringhash/RollingHash.cpp
    src/utils/countmin/CountMinSketch.cpp
    # src/utils/cuckoo/CuckooFilter.cpp  # removed: duplicate class
    # src/utils/bloomfilter2/ScalableBloomFilter.cpp  # removed: duplicate class
    src/utils/ratechange/RateOfChange.cpp
    src/utils/simplex/SimplexNoise.cpp
    src/utils/holtwinters/HoltWinters.cpp
    src/utils/multiproc/ProcessMonitor.cpp
    src/utils/slidingwindow3/SlidingWindowCounter.cpp
    src/utils/segmenter/StreamSegmenter.cpp
    src/utils/cluster/KMeansClusterer.cpp
    src/utils/changept/ChangepointDetector.cpp
    src/utils/symbol/SymbolDecoder.cpp
    src/utils/multiset/CountingMultiSet.cpp
    src/utils/deque/IndexedDeque.cpp
    src/utils/gradient/NumericalGradient.cpp
    src/utils/armodel/AutoRegressiveModel.cpp
    src/utils/fractal/FractalDimension.cpp
    src/utils/hurst/HurstExponent.cpp
    src/utils/permutation/PermutationGenerator.cpp
    src/utils/combination/CombinationGenerator.cpp
    src/utils/bloomfilter2/CountingBloomFilter.cpp
    src/utils/rscode/ReedSolomonCodec.cpp
    src/utils/spectrum/SpectralFlatness.cpp
    # src/utils/convolutional/ConvolutionalCoder.cpp  # removed: duplicate of code4/ConvolutionalCoder
    src/utils/cosinedist/CosineDistance.cpp
    src/utils/edgedetect/EdgeDetector.cpp
    src/utils/mass/MassEstimator.cpp
    src/utils/lcs2/LcsSolver.cpp
    src/utils/reservoir/ReservoirSampler.cpp

    # -- F180~F182: 增强编码/排序 --
    src/utils/rle2/RunLengthEncoder2.cpp
    src/utils/shannon2/HuffmanTreeBuilder.cpp
    src/utils/sort2/RadixSorter.cpp

    # -- F172~F177: Wave 25 工具模块 --
    src/utils/wiener/WienerFilter.cpp
    src/utils/circularbuffer2/CircularPowerBuffer.cpp
    src/utils/golomb/GolombCoder.cpp
    src/utils/peakhold/PeakHoldDetector.cpp
    src/utils/lomb/LombPeriodogram.cpp
    src/utils/bintree/BinaryIntervalTree.cpp

    # -- F178~F183: Wave 31 工具模块 --
    src/utils/movingaverage2/SynchronizedMovingAverage.cpp
    src/utils/histogram3/StreamingHistogram.cpp
    src/utils/qr/QrDecomposition.cpp
    src/utils/kalman2/ExtendedKalmanFilter.cpp
    src/utils/tfidf/TfidfVectorizer.cpp
    src/utils/pagerank/PageRank.cpp

    # -- F184~F191: Wave 32 工具模块(linter) --
    src/utils/bitmap/BitmapIndex.cpp
    src/utils/delta/DeltaEncoder.cpp
    src/utils/dwt2/DaubechiesWavelet.cpp
    src/utils/huffman2/AdaptiveHuffman.cpp
    # src/utils/hyperloglog/HyperLogLog.cpp  # removed: duplicate class
    src/utils/lifting/LiftingScheme.cpp
    src/utils/lzw2/LzwCodec2.cpp
    src/utils/mergesort/MergeSorter.cpp

    # -- F192~F203: Wave 33 工具模块 --
    src/utils/whitening/SpectralWhitener.cpp
    src/utils/peaks2/MultiPeakDetector.cpp
    src/utils/pingpong/PingPongBuffer.cpp
    # DUPLICATE REMOVED: src/utils/rle2/RunLengthEncoder2.cpp
    # DUPLICATE REMOVED: src/utils/shannon2/HuffmanTreeBuilder.cpp
    src/utils/graph2/TopologicalSorter.cpp
    src/utils/segment2/SegmentTree.cpp
    src/utils/sort2/QuickSelect.cpp
    # DUPLICATE REMOVED: src/utils/sort2/RadixSorter.cpp

    # -- F204~F210: Wave 34 工具模块(linter) --
    src/utils/bloomfilter3/ScalableBloomFilter.cpp
    src/utils/fenwick/FenwickTree.cpp
    src/utils/lfu2/TinyLfuCache.cpp
    src/utils/merkle/MerkleTree.cpp
    src/utils/roaring/RoaringBitmap.cpp
    src/utils/skiplist2/ConcurrentSkipList.cpp
    src/utils/sparsae/SparseArray.cpp

    # -- F211~F215: Wave 35 工具模块 --
    src/utils/svd/SingularValueDecomposition.cpp
    src/utils/kalman3/UnscentedKalmanFilter.cpp
    src/utils/graph3/DijkstraShortestPath.cpp
    src/utils/heap2/FibonacciHeap.cpp
    src/utils/quadtree/QuadTree.cpp

    # -- F216~F225: Wave 36 工具模块(linter) --
    src/utils/btree/BPlusTree.cpp
    src/utils/convhull/ConvexHull.cpp
    src/utils/disjoint/DisjointSet.cpp
    src/utils/radixtree/RadixTree.cpp
    src/utils/rle3/BurrowsWheelerTransform.cpp
    src/utils/suffix/SuffixAutomaton.cpp
    src/utils/voronoi/VoronoiDiagram.cpp
    # src/utils/wavelet2/HaarWavelet2D.cpp  # removed: duplicate class

    # -- F226~F231: Wave 39 DSP工具模块 --
    src/utils/bessel/BesselFilter.cpp
    src/utils/butterworth/ButterworthFilter.cpp
    src/utils/chebyshev/ChebyshevFilter.cpp
    # goertzel/GoertzelAlgorithm.cpp removed (duplicate of fft8)
    src/utils/welch/WelchSpectrum.cpp
    src/utils/yulewalker/YuleWalker.cpp

    # -- Wave 38 --
    src/utils/catboost/CatBoostEstimator.cpp
    src/utils/lru3/ArcCache.cpp
    src/utils/quantile/QuantileSketch.cpp
    src/utils/rle4/LempelZivWelch.cpp
    src/utils/fft3/RealFFT.cpp
    src/utils/sort3/TimSort.cpp

    # -- Wave 39: 信号处理V2 --
    # src/utils/wiener2/WienerFilter2.cpp  # removed: duplicate of signal6/WienerFilter
    src/utils/kalman4/ParticleFilter.cpp
    src/utils/ekf/ExtendedKalman.cpp
    src/utils/complement/ComplementaryFilter.cpp
    src/utils/savitzky/SavitzkyGolay.cpp
    src/utils/medfilt/MedianFilter.cpp
    src/utils/notch/NotchFilter.cpp
    # src/utils/peakdetect2/PeakDetector2.cpp  # removed: duplicate class
    src/utils/align/SignalAligner.cpp
    # src/utils/window2/WindowFunction.cpp  # removed: duplicate of dsp6/WindowFunction

    # -- Wave 40: 特征提取 --
    src/utils/shorttime/ShortTimeEnergy.cpp
    src/utils/zcr2/ZeroCrossingRate2.cpp
    src/utils/spectral/SpectralFeatures.cpp
    src/utils/mfcc/MfccExtractor.cpp
    src/utils/pitch2/PitchDetector2.cpp
    src/utils/ode/RungeKuttaSolver.cpp
    src/utils/decomp/LuDecomposition.cpp
    src/utils/decomp2/CholeskyDecomposition.cpp
    src/utils/graph4/BellmanFord.cpp
    src/utils/graph5/FloydWarshall.cpp
    src/utils/graph5/AStarSearch.cpp
    src/utils/mst/KruskalMST.cpp
    src/utils/mst2/PrimMST.cpp
    src/utils/stringmatch/KmpMatcher.cpp
    src/utils/stringmatch2/BoyerMooreMatcher.cpp
    src/utils/treap/Treap.cpp
    src/utils/splay/SplayTree.cpp
    src/utils/kdtree/KDTree.cpp
    src/utils/cic/CicFilter.cpp
    src/utils/optimize/SimulatedAnnealing.cpp
    src/utils/optimize2/GeneticOptimizer.cpp
    src/utils/rabin2/RabinKarpMulti.cpp
    src/utils/interp2/CubicInterpolator.cpp
    src/utils/phase/PhaseDetector.cpp
    src/utils/dft2/GoertzelSpectrum.cpp
    src/utils/qr2/HouseholderQR.cpp
    # src/utils/eigen2/PowerIteration.cpp  # removed: duplicate class
    src/utils/bfgs/BfgsOptimizer.cpp
    src/utils/dct/DctTransform.cpp
    # src/utils/distance/DistanceMetric.cpp  # removed: duplicate class
    src/utils/cluster2/DbScan.cpp
    src/utils/gmixture/GaussianMixture.cpp
    src/utils/bayes3/NaiveBayesClassifier.cpp
    src/utils/crossval/CrossValidator.cpp
    src/utils/emd/EarthMoverDistance.cpp
    src/utils/peakhold2/PeakTracker.cpp
    src/utils/cepstrum3/CepstralAnalysis.cpp
    src/utils/entropy2/RenyiEntropy.cpp
    src/utils/pca2/KernelPca.cpp
    src/utils/arima/ArimaModel.cpp
    src/utils/stacked/StackedEnsemble.cpp
    src/utils/glm/GeneralizedLinearModel.cpp
    src/utils/ensemble/BaggingEnsemble.cpp
    src/utils/bernstein/BernsteinPolynomial.cpp
    src/utils/taylor/TaylorSeries.cpp
    src/utils/hermite/HermiteInterpolation.cpp

    # ── Wave 44: 数值方法/ODE求根/积分 ──
    src/utils/conjugate/ConjugateGradient.cpp
    src/utils/lagrange/LagrangeInterpolation.cpp
    src/utils/bisect/BisectionSolver.cpp
    src/utils/romberg/RombergIntegration.cpp
    src/utils/adams/AdamsBashforth.cpp
    src/utils/predictor_corrector/PredictorCorrector.cpp
    src/utils/greville/GrevilleAbscissae.cpp
    src/utils/laguerre/LaguerreSolver.cpp
    src/utils/horner/HornerScheme.cpp
    # src/utils/newtonroot/NewtonRaphson.cpp  # removed: duplicate class
    src/utils/gaussquad/GaussLegendre.cpp
    src/utils/richardson/RichardsonExtrapolation.cpp
    src/utils/autodiff/numericalDifferentiator.cpp
    src/utils/rk2/Rk2Solver.cpp
    src/utils/collocation/CollocationSolver.cpp

    # ── Wave 45: 线性代数/特征值/逼近 ──
    src/utils/minimizer/GoldenSectionSearch.cpp
    src/utils/backtrack/BacktrackingLineSearch.cpp
    src/utils/divided_diff/DividedDifference.cpp
    src/utils/tridiag/TridiagonalSolver.cpp
    src/utils/schur/SchurDecomposition.cpp
    src/utils/rational/RationalInterpolation.cpp
    src/utils/chebyshev2/ChebyshevApproximation.cpp
    src/utils/bandmat/BandMatrixSolver.cpp
    # src/utils/sylvester/SylvesterSolver.cpp  # removed: duplicate class (superseded by matrix29)
    src/utils/pade/PadeApproximant.cpp
    src/utils/sturm/SturmSequence.cpp
    # src/utils/qrstep/QrEigenSolver.cpp  # removed: duplicate class
    src/utils/hessenberg/HessenbergReduction.cpp
    src/utils/butterfly/ButterflyOperation.cpp
    src/utils/circulant/CirculantSolver.cpp

    # ── Wave 46: 优化/特征值/迭代求解 ──
    src/utils/gram_schmidt/GramSchmidt.cpp
    src/utils/nelder_mead/NelderMead.cpp
    src/utils/davidson/DavidsonEigen.cpp
    src/utils/lanczos/LanczosEigen.cpp
    src/utils/powerquad/PowerMethodGeneralized.cpp
    src/utils/lobatto/LobattoIntegration.cpp
    src/utils/clenshaw_curtis/ClenshawCurtis.cpp
    src/utils/toeplitz/ToeplitzSolver.cpp
    src/utils/hankel/HankelSolver.cpp
    src/utils/vandermonde/VandermondeSolver.cpp
    src/utils/sparse_lu/SparseDirectSolver.cpp
    src/utils/ilu/IncompleteLU.cpp
    src/utils/sor/SorSolver.cpp
    src/utils/jacobi_iter/JacobiSolver.cpp
    src/utils/gauss_seidel/GaussSeidelSolver.cpp

    # ── Wave 47: 线性求解器/特征值/分解更新 ──
    src/utils/cgls/CglsSolver.cpp
    src/utils/bicgstab/BicgstabSolver.cpp
    src/utils/gmres/GmresSolver.cpp
    src/utils/minres/MinresSolver.cpp
    src/utils/symm_eigen/SymmetricEigenSolver.cpp
    src/utils/tridiag_eigen/TridiagEigenSolver.cpp
    src/utils/qz/QzDecomposition.cpp
    src/utils/svd2/SvdSolver.cpp
    # src/utils/polar/PolarDecomposition.cpp  # removed: duplicate class
    src/utils/qrupdate/QrUpdate.cpp
    src/utils/cholupdate/CholeskyUpdate.cpp
    src/utils/schur_update/SchurReorder.cpp
    src/utils/arnoldi/ArnoldiProcess.cpp
    src/utils/power2/InverseIteration.cpp
    src/utils/nystrom/NystromApproximation.cpp

    # ── Wave 48: 算法/字符串/多项式 ──
    src/utils/interp3/AkimaInterpolation.cpp
    src/utils/spectral2/SpectralClustering.cpp
    src/utils/procrustes/ProcrustesAnalysis.cpp
    src/utils/mahal/MahalanobisDistance.cpp
    src/utils/hungarian/HungarianAlgorithm.cpp
    src/utils/tsp/TspSolver.cpp
    src/utils/edmonds/EdmondsMatching.cpp
    src/utils/lcp_array/LcpArray.cpp
    src/utils/min_rotation/MinimalRotation.cpp
    src/utils/polynomial_root/PolynomialRoots.cpp
    # src/utils/resultant/PolynomialResultant.cpp  # removed: duplicate class
    src/utils/groebner/GroebnerBasis.cpp
    src/utils/grobner2/MultivariatePolynomial.cpp

    # ── Wave 49: 计算几何/空间索引 ──
    src/utils/bsp_tree/BspTree2d.cpp
    src/utils/interval_tree/IntervalTreeSearch.cpp
    src/utils/segment_tree/SegmentTreeRange.cpp
    src/utils/delaunay2/DelaunayFlip.cpp
    src/utils/voronoi2/VoronoiDiagram.cpp
    src/utils/convex_hull2/ChanConvexHull.cpp
    src/utils/polygon/PolygonTriangulation.cpp
    src/utils/line_sweep/LineSweepIntersect.cpp
    src/utils/geo_dist/GeodesicDistance.cpp
    src/utils/r_tree/RTreeSearch.cpp
    src/utils/kd_tree2/KdTreeBalancer.cpp
    src/utils/quadtree2/QuadtreeRegion.cpp
    src/utils/octree/OctreeVolume.cpp
    src/utils/point_location/PointLocation.cpp
    src/utils/nearest_neighbor2/ApproxNearestNeighbor.cpp

    # -- Wave 44-45: 新工具模块 --
    src/utils/fft4/FastCorrelation.cpp
    src/utils/dwt3/SymmetricWavelet.cpp
    src/utils/lz77/Lz77Compressor.cpp
    src/utils/huffman3/CanonicalHuffman.cpp
    src/utils/segment3/IntervalTree.cpp
    src/utils/sort4/IntroSort.cpp
    src/utils/detrend2/LinearDetrend.cpp
    src/utils/normalize2/MinMaxScaler.cpp

    # -- Wave 46-47 --
    src/utils/wavelet3/DaubechiesD4.cpp
    src/utils/fft5/OverlapSaveFFT.cpp
    src/utils/filter3/NotchFilter.cpp
    src/utils/peakdetect2/ZeroCrossingDetector.cpp
    src/utils/crc64b2/Crc64Ecma.cpp
    src/utils/compress2/LzssCompressor.cpp
    src/utils/heap3/BinaryHeap.cpp
    src/utils/graph5/GraphColoring.cpp
    src/utils/align/NeedlemanWunsch.cpp
    src/utils/cluster3/KMedoids.cpp

    # ── Wave 54: 新增工具模块 ──
    src/utils/cluster2/AgglomerativeClusterer.cpp
    src/utils/compress2/Lz4Compressor.cpp
    src/utils/detrend2/PolynomialDetrend.cpp
    src/utils/interpolate2/BicubicInterpolator.cpp
    src/utils/lts2/LtsRegression.cpp
    src/utils/optim2/SimulatedAnnealing2.cpp
    src/utils/phase2/PhaseUnwrapper.cpp
    src/utils/priority2/BinomialHeap.cpp
    src/utils/savitzky2/SavitzkyGolay2D.cpp
    src/utils/segment2/GraphSegmenter.cpp
    src/utils/set2/DisjointSetForest.cpp
    src/utils/signal3/VoiceActivityDetector2.cpp
    src/utils/streaming3/StreamingQuantile2.cpp

    # ── Wave 55: 新增工具模块 ──
    src/utils/morpho2/MorphologicalOps.cpp
    src/utils/fit3/SplineInterpolation.cpp
    src/utils/conv2/Convolution2D.cpp
    # src/utils/linalg4/LuDecomposition.cpp  # removed: conflicts with decomp/LuDecomposition (same class name)
    # src/utils/signal5/EnvelopeDetector.cpp  # removed: file does not exist
    src/utils/graph9/MinimumSpanningTree.cpp
    src/utils/code3/HammingCode.cpp
    src/utils/cluster3/GaussianMixture2.cpp

    # ── Wave 71: 节律/解码/编码/螺旋 ──
    src/utils/circadian/CircadianRhythm.cpp
    src/utils/viterbi2/ViterbiDecoder2.cpp
    src/utils/golomb2/GolombRiceCoder.cpp
    src/utils/spiral/SpiralMatrix.cpp

    # ── Wave 72: 组合/模拟/三角剖分 ──
    src/utils/nqueens/NQueensSolver.cpp
    src/utils/hanoi/HanoiTower.cpp
    src/utils/life/GameOfLife.cpp
    src/utils/delaunay3/DelaunayTriangulation2.cpp

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

    # -- F118: 波形数学引擎 --
    src/chart/math/WaveformMath.cpp
    src/chart/math/WaveformMathStats.cpp
    src/chart/math/MathExpressionParser.cpp
    src/chart/math/MathExpressionParserStats.cpp

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

    # -- F63: 数据批处理器 (file missing) --
    # src/utils/batch/DataBatchProcessor.cpp

    # -- F64: 数据采样器 --
    # DUPLICATE REMOVED: src/utils/sampler/DataSampler.cpp

    # -- F65: 多流数据合并器 --
    src/utils/merger/DataStreamMerger.cpp

    # -- F66: 环形缓冲区 --
    src/utils/ring/DataRingBuffer.cpp

    # -- F67: 协议帧验证器 --
    src/protocol/validator/ProtocolFrameValidator.cpp

    # -- F68: 数据关联器 --
    # DUPLICATE REMOVED: src/utils/correlator/DataCorrelator.cpp

    # -- F69: 数据插值器 --
    # DUPLICATE REMOVED: src/utils/interpolator/DataInterpolator.cpp

    # -- F70: 数据质量评分 --
    # DUPLICATE REMOVED: src/utils/quality/DataQualityScorer.cpp

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
    # src/utils/viterbi/ViterbiDecoder.cpp  # removed: duplicate of signal16/ViterbiDecoder
    src/utils/gmm/GaussianMixtureModel.cpp
    src/utils/covariance/CovarianceMatrix.cpp
    src/utils/percentile/PercentileCalculator.cpp
    # src/utils/scrambler/Scrambler.cpp  (duplicate of code23)
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
    # src/utils/ttest/TTestCalculator.cpp  # removed: duplicate class
    src/utils/moment/StatisticalMoments.cpp
    src/utils/ks/KsTestCalculator.cpp

    # -- F158~F171: Wave 24 工具模块 --
    src/utils/bootstrap/BootstrapResampler.cpp
    src/utils/chisq/ChiSquareTest.cpp
    src/utils/anova/AnovaTest.cpp
    src/utils/lrc/LrcChecksum.cpp
    src/utils/fcs/FcsChecker.cpp
    src/utils/snr/SnrEstimator.cpp
    src/utils/jaro/JaroWinkler.cpp

    # -- F172~F185: Wave 25 工具模块 --
    # DUPLICATE REMOVED: src/utils/lzw2/LzwCodec2.cpp
    src/utils/runlength2/RunLengthEncoder.cpp
    # DUPLICATE REMOVED: src/utils/huffman2/AdaptiveHuffman.cpp
    # DUPLICATE REMOVED: src/utils/delta/DeltaEncoder.cpp
    # DUPLICATE REMOVED: src/utils/bitmap/BitmapIndex.cpp
    # DUPLICATE REMOVED: src/utils/mergesort/MergeSorter.cpp
    # DUPLICATE REMOVED: src/utils/hyperloglog/HyperLogLog.cpp
    src/utils/tdigest/TDigest.cpp
    # DUPLICATE REMOVED: src/utils/dwt2/DaubechiesWavelet.cpp
    # DUPLICATE REMOVED: src/utils/lifting/LiftingScheme.cpp
    src/utils/quantize2/MuLawEncoder.cpp
    src/utils/polar/PolarCode.cpp
    # turbo/TurboDecoder.cpp removed (duplicate of code6)

    # -- F186~F200: Wave 35 工具模块 --
    # DUPLICATE REMOVED: src/utils/rle2/RunLengthEncoder2.cpp
    # DUPLICATE REMOVED: src/utils/shannon2/HuffmanTreeBuilder.cpp
    # DUPLICATE REMOVED: src/utils/sort2/RadixSorter.cpp
    # DUPLICATE REMOVED: src/utils/sort2/QuickSelect.cpp
    # DUPLICATE REMOVED: src/utils/graph2/TopologicalSorter.cpp
    # DUPLICATE REMOVED: src/utils/segment2/SegmentTree.cpp
    # DUPLICATE REMOVED: src/utils/fenwick/FenwickTree.cpp
    # DUPLICATE REMOVED: src/utils/sparsae/SparseArray.cpp
    # DUPLICATE REMOVED: src/utils/roaring/RoaringBitmap.cpp
    src/utils/ringhash/RingHash.cpp
    # DUPLICATE REMOVED: src/utils/bloomfilter3/ScalableBloomFilter.cpp
    # DUPLICATE REMOVED: src/utils/merkle/MerkleTree.cpp
    # DUPLICATE REMOVED: src/utils/skiplist2/ConcurrentSkipList.cpp
    # DUPLICATE REMOVED: src/utils/lfu2/TinyLfuCache.cpp

    # -- F201~F220: Wave 33+Linter 工具模块 --
    # src/utils/graph3/DijkstraShortestPath.cpp  # removed: duplicate (already in Wave 40)
    # DUPLICATE REMOVED: src/utils/heap2/FibonacciHeap.cpp
    # DUPLICATE REMOVED: src/utils/kalman3/UnscentedKalmanFilter.cpp
    # DUPLICATE REMOVED: src/utils/quadtree/QuadTree.cpp
    # DUPLICATE REMOVED: src/utils/svd/SingularValueDecomposition.cpp
    # DUPLICATE REMOVED: src/utils/btree/BPlusTree.cpp
    # DUPLICATE REMOVED: src/utils/suffix/SuffixAutomaton.cpp
    # DUPLICATE REMOVED: src/utils/radixtree/RadixTree.cpp
    # DUPLICATE REMOVED: src/utils/disjoint/DisjointSet.cpp
    # DUPLICATE REMOVED: src/utils/convhull/ConvexHull.cpp
    # DUPLICATE REMOVED: src/utils/voronoi/VoronoiDiagram.cpp
    # DUPLICATE REMOVED: src/utils/wavelet2/HaarWavelet2D.cpp
    # DUPLICATE REMOVED: src/utils/rle3/BurrowsWheelerTransform.cpp
    # DUPLICATE REMOVED: src/utils/autocorr/AutoCorrelator.cpp
    # DUPLICATE REMOVED: src/utils/crosscorr/CrossCorrelator.cpp
    # DUPLICATE REMOVED: src/utils/resample/DataResampler.cpp
    src/utils/wavwriter/WavWriter.cpp
    src/utils/wavreader/WavReader.cpp
    src/utils/hilbert/HilbertTransform.cpp
    # src/utils/lomb2/LombScargle.cpp  # removed: duplicate class
    # -- Wave 73: linter未跟踪模块集成 --
    src/utils/aho2/CommentzWalter.cpp
    src/utils/btree4/BStarTree.cpp
    src/utils/dict/TrieMap.cpp
    # src/utils/friedman/FriedmanTest.cpp  # removed: duplicate class
    src/utils/golay/GolayCode.cpp
    src/utils/haar2/HaarWavelet2D.cpp
    # src/utils/horner2/HornerScheme.cpp  # removed: duplicate class
    src/utils/inv_perm/InversePermutation.cpp
    src/utils/levenshtein2/DamerauLevenshtein.cpp
    src/utils/lfsr/LfsrSequence.cpp
    src/utils/median3/MedianFilter3D.cpp
    src/utils/median_abs/MedianAbsoluteDeviation.cpp
    src/utils/min_cut/MinCut.cpp
    src/utils/poly_roots/PolynomialRoots.cpp
    src/utils/quadtree3/QuadtreeBalancer.cpp
    # src/utils/reed_solomon2/ReedSolomon2.cpp  # removed: duplicate class (superseded by code29)
    src/utils/simon/SimonSpeckCipher.cpp
    src/utils/suffix2/SuffixArray2.cpp
    src/utils/tensor3/TensorDecomposition.cpp
    src/utils/treap2/ImplicitTreap.cpp
    src/utils/cuckoo_hash/CuckooHashTable.cpp
    src/utils/qr3/HouseholderQR.cpp
    src/utils/sturm2/SturmSequence.cpp
    src/utils/whitening2/ZcaWhitening.cpp

    # ── Wave 83: AES/NFA/符号求导/2D树状数组/Fisher精确检验 ──
    src/utils/aes2/AesCbc.cpp
    src/utils/automata2/NfaSimulator.cpp
    src/utils/derivative3/SymbolicDerivative.cpp
    src/utils/fenwick2/FenwickTree2D.cpp
    src/utils/fisher/FisherExactTest.cpp
    src/utils/heap6/BinaryHeap2.cpp
    src/utils/interleaver2/TurboInterleaver.cpp
    src/utils/tpsa/TpsaEngine.cpp

    # ── Wave 84: SHA-3/SHA-512 ──
    src/utils/sha3/Sha3Hash.cpp
    src/utils/sha512/Sha512Hash.cpp

    # ── Wave 84: 哈希函数集合(.cpp) ──
    src/utils/farmhash/FarmHash.cpp
    src/utils/metro/MetroHash.cpp
    src/utils/murmur64/MurmurHash64.cpp
    src/utils/polyhash/PolynomialHash.cpp
    src/utils/spooky/SpookyHash.cpp

    # ── Wave 89: Cauchy分布/原子环形缓冲/MCMC采样/Catmull-Rom样条 ──
    src/utils/cauchy/CauchyDistribution.cpp
    src/utils/ring4/RingBufferAtomic.cpp
    src/utils/mcmc/MetropolisHastings.cpp
    src/utils/spline3/CatmullRomSpline.cpp

    # ── Wave 90: 子串搜索/L1正则化/矩阵乘法/SPDY压缩 ──
    src/utils/substring/SubstringSearch.cpp
    src/utils/l1/L1Regularization.cpp
    src/utils/matmul/MatrixMultiply.cpp
    src/utils/spdy/SpdyCompressor.cpp

    # ── Wave 85: 图算法/插值/压缩/线性代数 ──
    # src/utils/dijkstra/DijkstraShortestPath.cpp  # removed: conflicts with graph3/DijkstraShortestPath
    # src/utils/graph12/BellmanFord.cpp  # removed: conflicts with graph4/BellmanFord
    src/utils/hmm2/HiddenMarkovModel2.cpp
    # src/utils/interp5/AkimaInterpolation.cpp  # removed: conflicts with interp3/AkimaInterpolation
    src/utils/lzw3/LzwEncoder.cpp
    src/utils/qr4/GivensQRUpdate.cpp
    src/utils/rolling_hash/RollingHash3.cpp

    # ── Wave 86: 哈希模块 ──
    src/utils/echohash/EchoHash.cpp
    src/utils/md6/Md6Hash.cpp
    src/utils/jh/JhHash.cpp

    # ── Wave 91: SGD/小波包/B-Tree/GMM/欧拉路径/格基规约/后缀自动机 ──
    src/utils/sgd/StochasticGradientDescent.cpp
    src/utils/dwt4/WaveletPacket.cpp
    src/utils/btree5/BTreeMap.cpp
    src/utils/gmm/GaussianMixture3.cpp
    src/utils/graph13/EulerianPath.cpp
    src/utils/lattice/LatticeReducer.cpp
    # suffix3/SuffixAutomaton.cpp removed (duplicate of suffix)

    # ── Wave 92: 核PCA增强 ──
    # DUPLICATE REMOVED: src/utils/pca2/KernelPca.cpp

    # ── Wave 93: FFT卷积/配对堆/正则引擎/DTW ──
    src/utils/conv3/FftConvolution.cpp
    # src/utils/heap7/PairingHeap.cpp  # removed: duplicate of heap4/PairingHeap
    src/utils/regex2/RegexEngine.cpp
    src/utils/distance3/DynamicTimeWarping.cpp

    # ── Wave 94: SCC/红黑树/三角插值/线段树 ──
    # src/utils/graph14/StronglyConnected.cpp  # removed: duplicate of graph7/StronglyConnected
    # src/utils/tree6/RedBlackTree2.cpp  # removed: duplicate of tree4/RedBlackTree2
    src/utils/interp6/TrigonometricInterp.cpp
    src/utils/segment4/LazySegmentTree.cpp

    # ── Wave 95: 基数排序/计数布隆/拓扑排序/CSR ──
    src/utils/sort8/RadixSort.cpp
    src/utils/bloom7/CountingBloomFilter.cpp
    # src/utils/graph15/TopologicalSort.cpp  # removed: duplicate class
    src/utils/matrix5/SparseMatrixCSR.cpp

    # ── Wave 96: 斐波那契队列/KD树/Shannon/DFA ──
    src/utils/priority3/FibonacciPriorityQueue.cpp
    src/utils/kdtree3/KdTreeNearest.cpp
    src/utils/entropy4/ShannonCoding.cpp
    # src/utils/automata3/DfaMinimizer.cpp  # removed: duplicate of automata/DfaMinimizer

    # ── Wave 97: 正态性检验/自适应FFT/并查集/Huffman ──
    src/utils/norm_test/NormalityTest.cpp
    src/utils/adaptive_fft/AdaptiveFFT.cpp
    src/utils/disjoint2/WeightedDisjointSet.cpp
    src/utils/huffman4/AdaptiveHuffman.cpp

    # ── Wave 98: 多项式插值/三角剖分/小波去噪/LRU ──
    src/utils/poly_interp/PolynomialInterpolator.cpp
    src/utils/triangulate2/EarClippingTriangulator.cpp
    src/utils/wavelet4/WaveletDenoiser.cpp
    src/utils/cache2/LRUCache.cpp

    # ── Wave 87-88: Softmax/边缘检测/Goertzel/特征值/Levenshtein ──
    # src/utils/softmax/SoftmaxClassifier.cpp  # removed: GCC14 default member initializer bug
    src/utils/detector/EdgeDetector2.cpp
    # src/utils/fft8/GoertzelAlgorithm.cpp  # removed: duplicate class (superseded by fft33)
    src/utils/matrix5/SymmetricEigen.cpp
    src/utils/string5/LevenshteinAutomaton.cpp

    # ── Wave 85b: Renyi熵/Zipf/增量PCA/懒传播线段树 ──
    # shannon3/RenyiEntropy.cpp removed (duplicate of entropy2)
    src/utils/zipf/ZipfGenerator.cpp
    src/utils/pca3/IncrementalPca.cpp
    src/utils/segment_tree2/SegmentTreeLazy.cpp

    # ── 哈希模块补充cpp ──
    src/utils/blake3/Blake3Hash.cpp
    src/utils/keccak/KeccakHash.cpp
    src/utils/siphash/SipHash.cpp
    src/utils/wyhash/WyHash.cpp
    src/utils/xxhash/XxHash.cpp

    # ── Wave 105-106: 关节点/SG滤波器/斜堆/SVD/Introsort/区间树/多边形裁剪/在线分位数 ──
    src/utils/graph18/ArticulationPoint.cpp
    src/utils/detrend3/SavitzkyGolayFilter.cpp
    src/utils/heap8/SkewHeap.cpp
    src/utils/linalg6/SvdSolver.cpp
    src/utils/sort10/Introsort2.cpp
    src/utils/tree9/IntervalTree2.cpp
    src/utils/geo3/PolygonClipper.cpp
    src/utils/quantile3/OnlineQuantile.cpp

    # ── Wave 107-118 additional sources ──
    src/utils/dsp7/AllpassFilter.cpp
    src/utils/cluster5/GaussianNaiveBayes.cpp
    # src/utils/code6/TurboDecoder.cpp  # removed: duplicate class (superseded by code28)
    src/utils/hash2/CuckooFilter.cpp
    src/utils/interp8/BSplineInterp.cpp
    src/utils/matrix8/BandMatrix2.cpp
    # src/utils/graph19/GraphIsomorphism.cpp  # removed: duplicate class (superseded by graph40)
    src/utils/poly4/LaguerrePoly.cpp
    src/utils/string7/RabinKarp2.cpp
    # src/utils/cluster6/SpectralCluster2.cpp  # removed: duplicate class (superseded by cluster28)
    src/utils/code7/PolarCode.cpp
    src/utils/fft10/GoertzelBank.cpp
    src/utils/graph20/BridgeTree.cpp
    src/utils/graph21/GraphDrawing.cpp
    src/utils/loss5/TripletLoss.cpp
    src/utils/matrix10/BlockMatrix.cpp
    src/utils/segment5/PersistentSegmentTree.cpp
    src/utils/tree11/VanEmdeBoas.cpp
    src/utils/matrix9/SchurDecomp.cpp
    src/utils/dsp8/FractionalDelay.cpp
    src/utils/signal9/PeakDetector2.cpp

    # ── Wave 110: 亲和力/扰码器/采样率转换/快速Hartley/最大团/稀疏QR/谱白化/配对堆 ──
    src/utils/cluster22/AffinityCluster.cpp
    src/utils/code23/Scrambler.cpp
    src/utils/dsp24/SampleRateConv.cpp
    src/utils/fft25/FastHartley.cpp
    src/utils/graph38/MaxClique.cpp
    src/utils/matrix27/SparseQR.cpp
    src/utils/signal26/SpectralWhitening.cpp
    src/utils/tree27/PairingHeap2.cpp

    # Wave 111-138: 大规模工具模块实现
    src/utils/cluster23/HierarchicalDensity.cpp
    src/utils/code24/ConvolutionalInterleaver.cpp
    src/utils/dsp25/EQMatch.cpp
    src/utils/fft26/DCTFast.cpp
    src/utils/graph39/MaximumMatching.cpp
    src/utils/matrix28/SparseSOR.cpp
    src/utils/signal27/TransientDetect.cpp
    src/utils/tree28/LeftistTree.cpp
    src/utils/cluster24/PlanarCheck.cpp
    src/utils/code25/LdpcEncoder.cpp
    src/utils/dsp26/ChorusEffect.cpp
    src/utils/fft27/RecursiveDFT.cpp
    src/utils/graph40/StronglyConnected2.cpp
    src/utils/matrix24/BandedSolver.cpp
    src/utils/signal23/CepstrumLifter.cpp
    src/utils/tree25/BinomialQueue.cpp
    src/utils/cluster25/GaussianMixture5.cpp
    src/utils/code26/CrcAccelerator.cpp
    src/utils/dsp27/WavetableOsc.cpp
    src/utils/fft28/FastNumberTheory.cpp
    src/utils/graph41/GraphPartition.cpp
    src/utils/matrix29/SylvesterSolver.cpp
    src/utils/signal24/SpectralEstimator.cpp
    src/utils/tree26/FibonacciHeap2.cpp
    src/utils/cluster26/HierClassifier.cpp
    src/utils/code27/PolarDecoder.cpp
    src/utils/dsp28/DynamicCompressor.cpp
    src/utils/fft29/ChirpZTransform.cpp
    # src/utils/graph42/GraphIsomorphism.cpp  # removed: duplicate class (superseded by newer wave)
    src/utils/matrix30/GeneralizedEigen2.cpp
    src/utils/signal25/AudioFingerprint.cpp
    src/utils/tree24/ScapegoatTree3.cpp
    src/utils/cluster27/OpticsClustering2.cpp
    src/utils/code28/TurboDecoder.cpp
    src/utils/dsp29/MultibandEQ.cpp
    src/utils/fft30/DiscreteCosine3.cpp
    src/utils/graph43/VertexCover2.cpp
    src/utils/matrix31/SymmetricEigen4.cpp
    src/utils/signal28/VoiceActivityDetect.cpp
    src/utils/tree29/RedBlackTree3.cpp
    src/utils/cluster28/SpectralCluster2.cpp
    src/utils/code29/ReedSolomon2.cpp
    src/utils/dsp30/Phaser.cpp
    src/utils/fft31/PrunedFFT2.cpp
    src/utils/graph44/DominatingSet.cpp
    src/utils/matrix32/SchurDecomp3.cpp
    src/utils/signal29/BeatDetector.cpp
    src/utils/tree30/AVLTree3.cpp
    src/utils/cluster29/DBSCAN4.cpp
    src/utils/code30/ConvolutionalCode3.cpp
    src/utils/dsp31/Flanger.cpp
    src/utils/fft32/ConstantQ2.cpp
    src/utils/graph45/EulerTour2.cpp
    src/utils/matrix33/SparseCholesky2.cpp
    src/utils/signal30/NoiseProfile.cpp
    src/utils/tree31/Treap3.cpp
    src/utils/cluster30/AgglomerativeCluster2.cpp
    src/utils/code31/PolarCode3.cpp
    src/utils/dsp32/Limiter.cpp
    src/utils/fft33/GoertzelAlgorithm.cpp
    src/utils/graph46/MaximumFlow2.cpp
    src/utils/matrix34/ToeplitzSolver2.cpp
    src/utils/signal31/Chromagram.cpp
    src/utils/tree32/SplayTree3.cpp
    src/utils/cluster31/BirchClustering2.cpp
    src/utils/code32/SpinalCode2.cpp
    src/utils/dsp33/NoiseGate2.cpp
    src/utils/fft34/PolyphaseFilterbank2.cpp
    src/utils/graph47/MinCostFlow2.cpp
    src/utils/matrix36/KrylovSolver2.cpp
    src/utils/signal35/OnsetDetector.cpp
    src/utils/cluster32/SubspaceCluster2.cpp
    src/utils/code33/GrayCode3.cpp
    src/utils/dsp34/Crossfader.cpp
    src/utils/fft35/SlidingDFT2.cpp
    src/utils/graph48/Matching3.cpp
    src/utils/matrix37/QRDecomp3.cpp
    src/utils/signal36/HarmonicProduct2.cpp
    src/utils/tree37/CartesianTree3.cpp
    src/utils/cluster33/GaussianMixture7.cpp
    src/utils/code34/HammingCode3.cpp
    src/utils/dsp35/EnvelopeShaper.cpp
    src/utils/fft36/ZoomFFT2.cpp
    src/utils/graph49/StronglyConnected3.cpp
    src/utils/matrix35/SymmetricEigen5.cpp
    src/utils/signal32/EnvelopeDetect2.cpp
    src/utils/tree33/SegmentTree3.cpp
    src/utils/cluster34/SpectralBicluster2.cpp
    src/utils/code35/ErasureCode2.cpp
    src/utils/dsp36/StereoProcessor2.cpp
    src/utils/fft37/MultiTaper2.cpp
    src/utils/graph50/FlowNetwork3.cpp
    src/utils/matrix39/GeneralizedEigen3.cpp
    src/utils/signal33/Beamformer2.cpp
    src/utils/tree34/PersistentTree2.cpp
    src/utils/cluster35/DBSCAN5.cpp
    src/utils/code36/TurboCode3.cpp
    src/utils/dsp37/Compressor2.cpp
    src/utils/fft38/DCTFast2.cpp
    src/utils/graph51/MaxClique2.cpp
    src/utils/matrix40/SparseBiCGSTAB.cpp
    src/utils/signal39/SpectralSubtract2.cpp
    src/utils/tree35/IntervalTree3.cpp
    src/utils/cluster36/KMeans4.cpp
    src/utils/code37/ConvolutionalInterleaver2.cpp
    src/utils/dsp38/SampleRateConv2.cpp
    src/utils/fft39/WalshHadamard2.cpp
    src/utils/graph52/EdgeColoring2.cpp
    src/utils/matrix41/SparseLU2.cpp
    src/utils/signal40/PitchTracker2.cpp
    src/utils/tree40/FenwickTree2D2.cpp
    src/utils/cluster37/GaussianMixture8.cpp
    # src/utils/code38/ReedSolomon3.cpp  # removed: conflicts with code9/ReedSolomon3
    src/utils/dsp39/DynamicEQ.cpp
    src/utils/fft40/PrunedFFT3.cpp
    src/utils/graph53/MinCut2.cpp
    src/utils/matrix42/SparseCholesky3.cpp
    src/utils/signal41/MelFilterbank2.cpp
    src/utils/tree41/SplayTree4.cpp
    src/utils/cluster38/OPTICS3.cpp
    src/utils/code39/ConvolutionalCode4.cpp
    src/utils/dsp40/Deesser.cpp
    src/utils/fft41/ConstantQ3.cpp
    # src/utils/graph54/ChinesePostman.cpp  # removed: conflicts with graph27/ChinesePostman
    src/utils/matrix43/SchurDecomp4.cpp
    src/utils/signal42/PhaseCorrelator2.cpp
    src/utils/tree42/RedBlackTree4.cpp
    src/utils/cluster39/BirchClustering3.cpp
    src/utils/code40/SpinalCode3.cpp
    src/utils/dsp41/Vocoder.cpp
    src/utils/fft42/GoertzelAlgorithm2.cpp
    src/utils/graph55/BipartiteMatch3.cpp
    src/utils/matrix44/SymmetricEigen6.cpp
    # src/utils/signal43/SpectralClustering3.cpp  # removed: conflicts with cluster12/SpectralClustering3
    src/utils/tree43/AVLTree4.cpp
    src/utils/cluster40/AgglomerativeCluster3.cpp
    src/utils/code41/PolarCode4.cpp
    src/utils/dsp42/GranularProcessor.cpp
    src/utils/fft43/ZoomFFT3.cpp
    src/utils/graph56/TravelingSalesman2.cpp
    src/utils/matrix45/SVD4.cpp
    src/utils/signal44/EnvelopeDetect3.cpp
    src/utils/tree44/Treap4.cpp

    # ── Wave 45+: Extended Algorithm Sources ──
    src/utils/cluster41/HierarchicalCluster4.cpp
    src/utils/cluster42/MeanShift4.cpp
    src/utils/cluster43/SpectralCluster4.cpp
    src/utils/cluster44/DBSCAN6.cpp
    src/utils/cluster45/GaussianMixture9.cpp
    src/utils/cluster46/SubspaceCluster3.cpp
    src/utils/cluster47/KMeans5.cpp
    src/utils/cluster48/HierarchicalCluster5.cpp
    src/utils/cluster49/DBSCAN7.cpp
    src/utils/cluster50/OPTICS4.cpp
    src/utils/cluster51/GaussianMixture10.cpp
    src/utils/cluster52/BirchClustering4.cpp
    src/utils/cluster53/SpectralClustering2.cpp
    src/utils/cluster54/MeanShift3.cpp
    src/utils/cluster55/DBSCAN8.cpp
    src/utils/cluster56/GaussianMixture11.cpp
    src/utils/cluster57/KMedoids4.cpp
    src/utils/cluster58/AffinityProp3.cpp
    src/utils/cluster59/IsolationForest3.cpp
    src/utils/cluster60/SpectralClustering3.cpp
    src/utils/cluster61/HierarchicalCluster6.cpp
    src/utils/cluster62/DBSCAN9.cpp
    src/utils/cluster63/KMeans6.cpp
    src/utils/cluster64/BirchClustering5.cpp
    src/utils/cluster65/OPTICS5.cpp
    src/utils/cluster66/GaussianMixture12.cpp
    src/utils/cluster67/KMeans7.cpp
    src/utils/cluster68/BirchClustering6.cpp
    src/utils/cluster69/DBSCAN10.cpp
    src/utils/cluster70/MeanShift4.cpp
    src/utils/cluster71/KMeans8.cpp
    src/utils/cluster72/DBSCAN11.cpp
    src/utils/cluster73/GaussianMixture13.cpp
    src/utils/cluster74/HierarchicalCluster7.cpp
    src/utils/cluster75/MeanShift5.cpp
    src/utils/cluster76/SpectralClustering4.cpp
    src/utils/cluster77/AffinityProp4.cpp
    src/utils/cluster78/IsolationForest4.cpp
    src/utils/cluster79/DBSCAN12.cpp
    src/utils/cluster80/GaussianMixture14.cpp
    src/utils/cluster81/HierarchicalCluster8.cpp
    src/utils/cluster82/KMeans9.cpp
    src/utils/cluster83/BirchClustering7.cpp
    src/utils/cluster84/MeanShift6.cpp
    src/utils/cluster85/SpectralClustering5.cpp
    src/utils/cluster86/IsolationForest5.cpp
    src/utils/cluster87/KMeans10.cpp
    src/utils/cluster88/HierarchicalCluster9.cpp
    src/utils/cluster89/SpectralClustering6.cpp
    src/utils/cluster90/MeanShift7.cpp
    src/utils/cluster91/BirchClustering8.cpp
    src/utils/cluster92/DBSCAN13.cpp
    src/utils/cluster93/OPTICS6.cpp
    src/utils/cluster94/GaussianMixture15.cpp
    src/utils/cluster95/IsolationForest6.cpp
    src/utils/cluster96/HierarchicalCluster10.cpp
    src/utils/cluster97/SpectralClustering7.cpp
    src/utils/cluster98/MeanShift8.cpp
    src/utils/cluster99/BirchClustering9.cpp
    src/utils/cluster100/DBSCAN14.cpp
    src/utils/cluster101/OPTICS7.cpp
    src/utils/cluster102/GaussianMixture16.cpp
    src/utils/cluster103/KMeans11.cpp
    src/utils/cluster104/HierarchicalCluster11.cpp
    src/utils/cluster105/SpectralClustering8.cpp
    src/utils/cluster106/MeanShift9.cpp
    src/utils/cluster107/BirchClustering10.cpp
    src/utils/cluster108/DBSCAN15.cpp
    src/utils/cluster109/OPTICS8.cpp
    src/utils/cluster110/GaussianMixture17.cpp
    src/utils/cluster111/KMeans12.cpp
    src/utils/cluster112/HierarchicalCluster12.cpp
    src/utils/cluster113/SpectralClustering9.cpp
    src/utils/cluster114/MeanShift10.cpp
    src/utils/cluster115/BirchClustering11.cpp
    src/utils/cluster116/DBSCAN16.cpp
    src/utils/cluster117/OPTICS9.cpp
    src/utils/cluster118/GaussianMixture18.cpp
    src/utils/cluster158/KMedoids13.cpp
    src/utils/code42/HammingCode4.cpp
    src/utils/code43/BchCode4.cpp
    src/utils/code44/LdpcDecoder4.cpp
    src/utils/code45/TurboCode4.cpp
    src/utils/code46/ErasureCode3.cpp
    src/utils/code47/ConvolutionalCode5.cpp
    src/utils/code48/ReedSolomon4.cpp
    src/utils/code49/PolarCode5.cpp
    src/utils/code50/LdpcEncoder2.cpp
    src/utils/code51/GrayCode4.cpp
    src/utils/code52/HammingCode5.cpp
    src/utils/code53/SpinalCode4.cpp
    src/utils/code54/ConvolutionalCode2.cpp
    src/utils/code55/TurboCode2.cpp
    src/utils/code56/LDPCCode3.cpp
    src/utils/code57/ReedSolomon5.cpp
    src/utils/code58/CascadeCode2.cpp
    src/utils/code59/SpinalCode5.cpp
    src/utils/code60/PolarCode6.cpp
    src/utils/code61/TurboCode3.cpp
    src/utils/code62/BchCode5.cpp
    src/utils/code63/ConvolutionalCode3.cpp
    src/utils/code64/HammingCode6.cpp
    src/utils/code65/GrayCode5.cpp
    src/utils/code66/ReedSolomon6.cpp
    src/utils/code67/LDPCCode4.cpp
    src/utils/code68/SpinalCode6.cpp
    src/utils/code69/ConvolutionalCode4.cpp
    src/utils/code70/TurboCode4.cpp
    src/utils/code71/PolarCode7.cpp
    src/utils/code72/ReedSolomon7.cpp
    src/utils/code73/BchCode6.cpp
    src/utils/code74/ConvolutionalCode5.cpp
    src/utils/code75/LDPCCode5.cpp
    src/utils/code76/TurboCode5.cpp
    src/utils/code77/PolarCode8.cpp
    src/utils/code78/SpinalCode7.cpp
    src/utils/code79/CascadeCode3.cpp
    src/utils/code80/TurboCode6.cpp
    src/utils/code81/BchCode7.cpp
    src/utils/code82/ReedSolomon8.cpp
    src/utils/code83/LDPCCode5.cpp
    src/utils/code84/GrayCode6.cpp
    src/utils/code85/SpinalCode8.cpp
    src/utils/code86/PolarCode9.cpp
    src/utils/code87/CascadeCode4.cpp
    src/utils/code88/ReedSolomon9.cpp
    src/utils/code89/ConvCode5.cpp
    src/utils/code90/BchCode8.cpp
    src/utils/code91/PolarCode10.cpp
    src/utils/code92/SpinalCode9.cpp
    src/utils/code93/TurboCode7.cpp
    src/utils/code94/GrayCode7.cpp
    src/utils/code95/CascadeCode5.cpp
    src/utils/code96/ReedSolomon10.cpp
    src/utils/code97/ConvCode6.cpp
    src/utils/code98/BchCode9.cpp
    src/utils/code99/PolarCode11.cpp
    src/utils/code100/SpinalCode10.cpp
    src/utils/code101/TurboCode8.cpp
    src/utils/code102/GrayCode8.cpp
    src/utils/code103/CascadeCode6.cpp
    src/utils/code104/LDPCCode6.cpp
    src/utils/code105/ReedSolomon11.cpp
    src/utils/code106/BchCode10.cpp
    src/utils/code107/PolarCode12.cpp
    src/utils/code108/SpinalCode11.cpp
    src/utils/code109/TurboCode9.cpp
    src/utils/code110/GrayCode9.cpp
    src/utils/code111/CascadeCode7.cpp
    src/utils/code112/LDPCCode7.cpp
    src/utils/code113/ReedSolomon12.cpp
    src/utils/code114/BchCode11.cpp
    src/utils/code115/PolarCode13.cpp
    src/utils/code116/SpinalCode12.cpp
    src/utils/code117/TurboCode10.cpp
    src/utils/code118/GrayCode10.cpp
    src/utils/code119/CascadeCode8.cpp
    src/utils/code159/TurboCode11.cpp
    src/utils/dsp43/ConvolutionReverb.cpp
    src/utils/dsp44/PitchShift2.cpp
    src/utils/dsp45/SpectralGate.cpp
    src/utils/dsp46/StereoProcessor3.cpp
    src/utils/dsp47/MultibandCompressor.cpp
    src/utils/dsp48/TransientShaper.cpp
    src/utils/dsp49/DynamicCompressor2.cpp
    src/utils/dsp50/SampleRateConv3.cpp
    src/utils/dsp51/DelayLine3.cpp
    src/utils/dsp52/NoiseGate3.cpp
    src/utils/dsp53/EQMatch2.cpp
    src/utils/dsp54/ChorusEffect2.cpp
    src/utils/dsp55/TransientShaper2.cpp
    src/utils/dsp56/MultibandCompress2.cpp
    src/utils/dsp57/Limiter3.cpp
    src/utils/dsp58/DeEsser3.cpp
    src/utils/dsp59/GateExpand3.cpp
    src/utils/dsp60/PhaseVocoder3.cpp
    src/utils/dsp61/StereoWidth3.cpp
    src/utils/dsp62/DynamicEQ3.cpp
    src/utils/dsp63/Compressor4.cpp
    src/utils/dsp64/Reverb3.cpp
    src/utils/dsp65/DelayLine4.cpp
    src/utils/dsp66/Phaser2.cpp
    src/utils/dsp67/Flanger2.cpp
    src/utils/dsp68/StereoEnhance2.cpp
    src/utils/dsp69/SampleRateConv4.cpp
    src/utils/dsp70/NoiseGate4.cpp
    src/utils/dsp71/ChorusEffect3.cpp
    src/utils/dsp72/StereoProcessor3.cpp
    src/utils/dsp73/DynamicCompressor3.cpp
    src/utils/dsp74/Limiter4.cpp
    src/utils/dsp75/TransientShaper3.cpp
    src/utils/dsp76/MultibandCompress3.cpp
    src/utils/dsp77/DeEsser4.cpp
    src/utils/dsp78/GateExpand4.cpp
    src/utils/dsp79/PhaseVocoder4.cpp
    src/utils/dsp80/StereoWidth4.cpp
    src/utils/dsp81/ChorusEffect4.cpp
    src/utils/dsp82/DynamicEQ4.cpp
    src/utils/dsp83/Flanger3.cpp
    src/utils/dsp84/Phaser3.cpp
    src/utils/dsp85/NoiseGate5.cpp
    src/utils/dsp86/Reverb4.cpp
    src/utils/dsp87/DelayLine5.cpp
    src/utils/dsp88/StereoEnhance3.cpp
    src/utils/dsp89/Compressor4.cpp
    src/utils/dsp90/Expander3.cpp
    src/utils/dsp91/Gate3.cpp
    src/utils/dsp92/Limiter4.cpp
    src/utils/dsp93/MultibandComp3.cpp
    src/utils/dsp94/StereoWidener3.cpp
    src/utils/dsp95/DynamicEQ5.cpp
    src/utils/dsp96/Phaser4.cpp
    src/utils/dsp97/Compressor5.cpp
    src/utils/dsp98/Expander4.cpp
    src/utils/dsp99/Gate4.cpp
    src/utils/dsp100/Limiter5.cpp
    src/utils/dsp101/MultibandComp4.cpp
    src/utils/dsp102/StereoWidener4.cpp
    src/utils/dsp103/DynamicEQ6.cpp
    src/utils/dsp104/Phaser5.cpp
    src/utils/dsp105/Compressor6.cpp
    src/utils/dsp106/Expander5.cpp
    src/utils/dsp107/Gate5.cpp
    src/utils/dsp108/Limiter6.cpp
    src/utils/dsp109/MultibandComp5.cpp
    src/utils/dsp110/StereoWidener5.cpp
    src/utils/dsp111/DynamicEQ7.cpp
    src/utils/dsp112/Phaser6.cpp
    src/utils/dsp113/Compressor7.cpp
    src/utils/dsp114/Expander6.cpp
    src/utils/dsp115/Gate6.cpp
    src/utils/dsp116/Limiter7.cpp
    src/utils/dsp117/MultibandComp6.cpp
    src/utils/dsp118/StereoWidener6.cpp
    src/utils/dsp119/DynamicEQ8.cpp
    src/utils/dsp120/Phaser7.cpp
    src/utils/dsp160/DynamicRangeCompressor.cpp
    src/utils/fft44/ShortTimeFFT3.cpp
    src/utils/fft45/DiscreteCosine4.cpp
    src/utils/fft46/ChirpZ3.cpp
    src/utils/fft47/PolyphaseFilterbank3.cpp
    src/utils/fft48/MultiTaper3.cpp
    src/utils/fft49/SlidingDFT3.cpp
    src/utils/fft50/FastHartley2.cpp
    src/utils/fft51/WalshHadamard3.cpp
    src/utils/fft52/DCTFast3.cpp
    src/utils/fft53/GoertzelAlgorithm3.cpp
    src/utils/fft54/RecursiveDFT2.cpp
    src/utils/fft55/PrunedFFT4.cpp
    src/utils/fft56/ShortTimeFFT4.cpp
    src/utils/fft57/ChirpZ3.cpp
    src/utils/fft58/ZoomFFT3.cpp
    src/utils/fft59/NumberTheoretic3.cpp
    src/utils/fft60/Multitaper3.cpp
    src/utils/fft61/FastConv3.cpp
    src/utils/fft62/PolyPhase2.cpp
    src/utils/fft63/WignerVille2.cpp
    src/utils/fft64/HilbertHuang2.cpp
    src/utils/fft65/AdaptiveSTFT2.cpp
    src/utils/fft66/Cepstrum3.cpp
    src/utils/fft67/SlidingDFT3.cpp
    src/utils/fft68/ConstantQ3.cpp
    src/utils/fft69/MelSpectrogram3.cpp
    src/utils/fft70/GaborTransform2.cpp
    src/utils/fft71/WalshHadamard4.cpp
    src/utils/fft72/GoertzelAlgorithm4.cpp
    src/utils/fft73/DCTFast4.cpp
    src/utils/fft74/FastHartley3.cpp
    src/utils/fft75/RecursiveDFT3.cpp
    src/utils/fft76/ShortTimeFFT5.cpp
    src/utils/fft77/ChirpZ4.cpp
    src/utils/fft78/NumberTheoretic4.cpp
    src/utils/fft79/Multitaper4.cpp
    src/utils/fft80/ZoomFFT4.cpp
    src/utils/fft81/PolyPhase3.cpp
    src/utils/fft82/GoertzelAlgorithm5.cpp
    src/utils/fft83/ConstantQ4.cpp
    src/utils/fft84/MelSpectrogram4.cpp
    src/utils/fft85/GaborTransform3.cpp
    src/utils/fft86/SlidingDFT4.cpp
    src/utils/fft87/Cepstrum4.cpp
    src/utils/fft88/WalshHadamard5.cpp
    src/utils/fft89/FastConv4.cpp
    src/utils/fft90/ZoomFFT4.cpp
    src/utils/fft91/PrunedFFT3.cpp
    src/utils/fft92/RecursiveDFT3.cpp
    src/utils/fft93/DCTFast3.cpp
    src/utils/fft94/ChirpZTransform3.cpp
    src/utils/fft95/GoertzelAlgorithm6.cpp
    src/utils/fft96/ConstantQ5.cpp
    src/utils/fft97/MelSpectrogram5.cpp
    src/utils/fft98/ZoomFFT5.cpp
    src/utils/fft99/PrunedFFT4.cpp
    src/utils/fft100/RecursiveDFT4.cpp
    src/utils/fft101/DCTFast4.cpp
    src/utils/fft102/ChirpZTransform4.cpp
    src/utils/fft103/GoertzelAlgorithm7.cpp
    src/utils/fft104/ConstantQ6.cpp
    src/utils/fft105/MelSpectrogram6.cpp
    src/utils/fft106/ZoomFFT6.cpp
    src/utils/fft107/PrunedFFT5.cpp
    src/utils/fft108/RecursiveDFT5.cpp
    src/utils/fft109/DCTFast5.cpp
    src/utils/fft110/ChirpZTransform5.cpp
    src/utils/fft111/GoertzelAlgorithm8.cpp
    src/utils/fft112/ConstantQ7.cpp
    src/utils/fft113/MelSpectrogram7.cpp
    src/utils/fft114/ZoomFFT7.cpp
    src/utils/fft115/PrunedFFT6.cpp
    src/utils/fft116/RecursiveDFT6.cpp
    src/utils/fft117/DCTFast6.cpp
    src/utils/fft118/ChirpZTransform6.cpp
    src/utils/fft119/GoertzelAlgorithm9.cpp
    src/utils/fft120/ConstantQ8.cpp
    src/utils/fft121/MelSpectrogram8.cpp
    src/utils/fft161/ShortTimeFourier.cpp
    src/utils/graph57/Bridges2.cpp
    src/utils/graph58/TopologicalSort2.cpp
    src/utils/graph59/SteinerTree2.cpp
    src/utils/graph60/DominatingSet2.cpp
    src/utils/graph61/GraphColoring4.cpp
    src/utils/graph62/IndependentSet2.cpp
    src/utils/graph63/MaxClique3.cpp
    src/utils/graph64/StronglyConnected4.cpp
    src/utils/graph65/Matching4.cpp
    src/utils/graph66/EulerTour3.cpp
    src/utils/graph67/VertexCover3.cpp
    src/utils/graph68/FlowNetwork4.cpp
    src/utils/graph69/BipartiteMatch4.cpp
    src/utils/graph70/ChinesePostman2.cpp
    src/utils/graph71/MinSpanningTree6.cpp
    src/utils/graph72/SteinerTree3.cpp
    src/utils/graph73/GraphColoring5.cpp
    src/utils/graph74/IndependentSet3.cpp
    src/utils/graph75/DominatingSet3.cpp
    src/utils/graph76/MaxClique4.cpp
    src/utils/graph77/GraphIsomorph3.cpp
    src/utils/graph78/Biconnected3.cpp
    src/utils/graph79/BridgeDetect3.cpp
    src/utils/graph80/FlowNetwork5.cpp
    src/utils/graph81/MinSpanningTree7.cpp
    src/utils/graph82/TopologicalSort3.cpp
    src/utils/graph83/ShortestPath4.cpp
    src/utils/graph84/BipartiteMatch5.cpp
    src/utils/graph85/EulerTour4.cpp
    src/utils/graph86/Matching5.cpp
    src/utils/graph87/VertexCover4.cpp
    src/utils/graph88/EdgeColoring3.cpp
    src/utils/graph89/SteinerTree4.cpp
    src/utils/graph90/ChinesePostman3.cpp
    src/utils/graph91/MinSpanningTree8.cpp
    src/utils/graph92/GraphColoring6.cpp
    src/utils/graph93/IndependentSet4.cpp
    src/utils/graph94/DominatingSet4.cpp
    src/utils/graph95/DominatingSet5.cpp
    src/utils/graph96/TopologicalSort4.cpp
    src/utils/graph97/FlowNetwork6.cpp
    src/utils/graph98/Biconnected4.cpp
    src/utils/graph99/BridgeDetect4.cpp
    src/utils/graph100/MaxClique5.cpp
    src/utils/graph101/ShortestPath5.cpp
    src/utils/graph102/GraphIsomorph4.cpp
    src/utils/graph103/GraphColoring7.cpp
    src/utils/graph104/IndependentSet5.cpp
    src/utils/graph105/MinSpanningTree8.cpp
    src/utils/graph106/StronglyConnected3.cpp
    src/utils/graph107/FlowNetwork7.cpp
    src/utils/graph108/EulerTour5.cpp
    src/utils/graph109/DominatingSet6.cpp
    src/utils/graph110/Biconnected5.cpp
    src/utils/graph111/GraphColoring8.cpp
    src/utils/graph112/IndependentSet6.cpp
    src/utils/graph113/MinSpanningTree9.cpp
    src/utils/graph114/StronglyConnected4.cpp
    src/utils/graph115/FlowNetwork8.cpp
    src/utils/graph116/EulerTour6.cpp
    src/utils/graph117/DominatingSet7.cpp
    src/utils/graph118/Biconnected6.cpp
    src/utils/graph119/GraphColoring9.cpp
    src/utils/graph120/IndependentSet7.cpp
    src/utils/graph121/MinSpanningTree9.cpp
    src/utils/graph122/StronglyConnected5.cpp
    src/utils/graph123/FlowNetwork9.cpp
    src/utils/graph124/EulerTour7.cpp
    src/utils/graph125/DominatingSet8.cpp
    src/utils/graph126/Biconnected7.cpp
    src/utils/graph127/GraphColoring10.cpp
    src/utils/graph128/IndependentSet8.cpp
    src/utils/graph129/MinSpanningTree11.cpp
    src/utils/graph130/StronglyConnected6.cpp
    src/utils/graph131/FlowNetwork10.cpp
    src/utils/graph132/EulerTour8.cpp
    src/utils/graph133/DominatingSet9.cpp
    src/utils/graph134/Biconnected8.cpp
    src/utils/graph174/BridgeDetector.cpp
    src/utils/matrix46/GeneralizedEigen4.cpp
    src/utils/matrix47/ToeplitzSolver3.cpp
    src/utils/matrix48/SparseGMRES2.cpp
    src/utils/matrix49/SparseBiCGSTAB2.cpp
    src/utils/matrix50/KrylovSolver3.cpp
    src/utils/matrix51/SparseCholesky4.cpp
    src/utils/matrix53/SparseQR2.cpp
    src/utils/matrix54/SymmetricEigen7.cpp
    src/utils/matrix55/TridiagonalSolver3.cpp
    src/utils/matrix56/SVD5.cpp
    src/utils/matrix57/SchurDecomp5.cpp
    src/utils/matrix58/GeneralizedEigen5.cpp
    src/utils/matrix59/SparseLU2.cpp
    src/utils/matrix60/GeneralizedEigen2.cpp
    src/utils/matrix61/TridiagonalEigen2.cpp
    src/utils/matrix62/BandSolver2.cpp
    src/utils/matrix63/ConditionNumber2.cpp
    src/utils/matrix64/SylvesterSolver2.cpp
    src/utils/matrix65/SchurComplement2.cpp
    src/utils/matrix66/SparseCholesky2.cpp
    src/utils/matrix67/KroneckerProduct2.cpp
    src/utils/matrix68/ToeplitzSolver2.cpp
    src/utils/matrix69/SparseQR3.cpp
    src/utils/matrix70/Hessenberg2.cpp
    src/utils/matrix71/SymmetricEigen8.cpp
    src/utils/matrix72/SVD6.cpp
    src/utils/matrix73/TridiagonalSolver4.cpp
    src/utils/matrix74/HouseholderQR2.cpp
    src/utils/matrix75/SparseBiCG2.cpp
    src/utils/matrix76/SymmetricEigen9.cpp
    src/utils/matrix77/SparseGMRES2.cpp
    src/utils/matrix78/TridiagonalEigen3.cpp
    src/utils/matrix79/GeneralizedEigen3.cpp
    src/utils/matrix80/SparseLU3.cpp
    src/utils/matrix81/BandSolver3.cpp
    src/utils/matrix82/SchurDecomp6.cpp
    src/utils/matrix83/ConditionNumber3.cpp
    src/utils/matrix84/SylvesterSolver3.cpp
    src/utils/matrix85/SparseCholesky3.cpp
    src/utils/matrix86/KroneckerProduct3.cpp
    src/utils/matrix87/ToeplitzSolver3.cpp
    src/utils/matrix88/Hessenberg3.cpp
    src/utils/matrix89/HouseholderQR3.cpp
    src/utils/matrix90/SparseQR4.cpp
    src/utils/matrix91/SVD7.cpp
    src/utils/matrix92/SymmetricEigen10.cpp
    src/utils/matrix93/SparseLU3.cpp
    src/utils/matrix94/SymmetricEigen11.cpp
    src/utils/matrix95/GeneralizedEigen3.cpp
    src/utils/matrix96/SchurDecomp4.cpp
    src/utils/matrix97/SparseCholesky4.cpp
    src/utils/matrix98/Hessenberg4.cpp
    src/utils/matrix99/ToeplitzSolver4.cpp
    src/utils/matrix100/KrylovSolver3.cpp
    src/utils/matrix101/SparseLU4.cpp
    src/utils/matrix102/SymmetricEigen12.cpp
    src/utils/matrix103/GeneralizedEigen4.cpp
    src/utils/matrix104/SchurDecomp5.cpp
    src/utils/matrix105/SparseCholesky5.cpp
    src/utils/matrix106/Hessenberg5.cpp
    src/utils/matrix107/ToeplitzSolver5.cpp
    src/utils/matrix108/KrylovSolver4.cpp
    src/utils/matrix109/SparseLU5.cpp
    src/utils/matrix110/SymmetricEigen13.cpp
    src/utils/matrix111/GeneralizedEigen5.cpp
    src/utils/matrix112/SchurDecomp6.cpp
    src/utils/matrix113/SparseCholesky6.cpp
    src/utils/matrix114/Hessenberg6.cpp
    src/utils/matrix115/ToeplitzSolver6.cpp
    src/utils/matrix116/KrylovSolver5.cpp
    src/utils/matrix117/SparseLU6.cpp
    src/utils/matrix118/SymmetricEigen14.cpp
    src/utils/matrix119/GeneralizedEigen6.cpp
    src/utils/matrix120/SchurDecomp7.cpp
    src/utils/matrix121/SparseCholesky7.cpp
    src/utils/matrix122/Hessenberg7.cpp
    src/utils/matrix123/ToeplitzSolver7.cpp
    src/utils/matrix124/KrylovSolver6.cpp
    src/utils/matrix164/BandEigenSolver.cpp
    src/utils/signal45/Chromagram2.cpp
    src/utils/signal46/FormantTracker2.cpp
    src/utils/signal47/NoiseProfile2.cpp
    src/utils/signal48/SpectralWhitening2.cpp
    src/utils/signal49/BeatDetector2.cpp
    src/utils/signal50/HarmonicProduct3.cpp
    src/utils/signal51/SpectralSubtract3.cpp
    src/utils/signal52/VoiceActivityDetect2.cpp
    src/utils/signal53/EnvelopeDetect4.cpp
    src/utils/signal54/OnsetDetector2.cpp
    src/utils/signal55/PitchDetector2.cpp
    src/utils/signal56/BlindSourceSep2.cpp
    src/utils/signal57/BeatDetector2.cpp
    src/utils/signal58/HarmonicProduct2.cpp
    src/utils/signal59/Resampler3.cpp
    src/utils/signal60/Chromagram2.cpp
    src/utils/signal61/MFCC2.cpp
    src/utils/signal62/SpectralCentroid2.cpp
    src/utils/signal63/ZeroCrossing2.cpp
    src/utils/signal64/SpectralFlatness2.cpp
    src/utils/signal65/SpectralRolloff2.cpp
    src/utils/signal66/SpectralContrast2.cpp
    src/utils/signal67/EnvelopeFollower3.cpp
    src/utils/signal68/SpectralFlux2.cpp
    src/utils/signal69/SpectralBrightness2.cpp
    src/utils/signal70/VoiceActivityDetect3.cpp
    src/utils/signal71/PitchTrack3.cpp
    src/utils/signal72/EnvelopeDetect5.cpp
    src/utils/signal73/BlindSourceSep3.cpp
    src/utils/signal74/SpectralSubtract4.cpp
    src/utils/signal75/SpectralLeakage2.cpp
    src/utils/signal76/PitchDetector3.cpp
    src/utils/signal77/OnsetDetector3.cpp
    src/utils/signal78/HarmonicProduct3.cpp
    src/utils/signal79/Chromagram3.cpp
    src/utils/signal80/Resampler4.cpp
    src/utils/signal81/MFCC3.cpp
    src/utils/signal82/ZeroCrossing3.cpp
    src/utils/signal83/BeatDetector3.cpp
    src/utils/signal84/SpectralFlux3.cpp
    src/utils/signal85/MFCC4.cpp
    src/utils/signal86/SpectralBrightness3.cpp
    src/utils/signal87/EnvelopeDetect6.cpp
    src/utils/signal88/PitchTrack4.cpp
    src/utils/signal89/Chromagram4.cpp
    src/utils/signal90/ZeroCrossing4.cpp
    src/utils/signal91/SpectralFlatness3.cpp
    src/utils/signal92/Tonality3.cpp
    src/utils/signal93/NoiseEstimate3.cpp
    src/utils/signal94/VoiceActivity3.cpp
    src/utils/signal95/TransientDetect3.cpp
    src/utils/signal96/Chromagram5.cpp
    src/utils/signal97/EnvelopeDetect7.cpp
    src/utils/signal98/PitchTrack5.cpp
    src/utils/signal99/SpectralFlatness4.cpp
    src/utils/signal100/Tonality4.cpp
    src/utils/signal101/NoiseEstimate4.cpp
    src/utils/signal102/VoiceActivity4.cpp
    src/utils/signal103/TransientDetect4.cpp
    src/utils/signal104/Chromagram6.cpp
    src/utils/signal105/EnvelopeDetect8.cpp
    src/utils/signal106/PitchTrack6.cpp
    src/utils/signal107/SpectralFlatness5.cpp
    src/utils/signal108/Tonality5.cpp
    src/utils/signal109/NoiseEstimate5.cpp
    src/utils/signal110/VoiceActivity5.cpp
    src/utils/signal111/TransientDetect5.cpp
    src/utils/signal112/Chromagram7.cpp
    src/utils/signal113/EnvelopeDetect9.cpp
    src/utils/signal114/PitchTrack7.cpp
    src/utils/signal115/SpectralFlatness6.cpp
    src/utils/signal116/Tonality6.cpp
    src/utils/signal117/NoiseEstimate6.cpp
    src/utils/signal118/VoiceActivity6.cpp
    src/utils/signal119/TransientDetect6.cpp
    src/utils/signal120/Chromagram8.cpp
    src/utils/signal121/EnvelopeDetect10.cpp
    src/utils/signal122/PitchTrack8.cpp
    src/utils/signal162/SpectralSubtraction.cpp
    src/utils/tree45/BPlusTree5.cpp
    src/utils/tree46/SuffixTree2.cpp
    src/utils/tree47/SegmentTree4.cpp
    src/utils/tree48/CartesianTree4.cpp
    src/utils/tree49/WeightBalancedTree3.cpp
    src/utils/tree50/FenwickTree3.cpp
    src/utils/tree51/RedBlackTree5.cpp
    src/utils/tree52/BPlusTree6.cpp
    src/utils/tree53/AVLTree5.cpp
    src/utils/tree54/SplayTree5.cpp
    src/utils/tree55/Treap5.cpp
    src/utils/tree56/SegmentTree5.cpp
    src/utils/tree57/IntervalTree4.cpp
    src/utils/tree58/QuadTree3.cpp
    src/utils/tree59/PrioritySearchTree2.cpp
    src/utils/tree60/KDTree4.cpp
    src/utils/tree61/SuffixArray3.cpp
    src/utils/tree62/RangeTree3.cpp
    src/utils/tree63/WaveletTree3.cpp
    src/utils/tree64/SegmentTree6.cpp
    src/utils/tree65/FenwickTree3.cpp
    src/utils/tree66/AVLTree6.cpp
    src/utils/tree67/RedBlackTree6.cpp
    src/utils/tree68/Treap6.cpp
    src/utils/tree69/SplayTree6.cpp
    src/utils/tree70/BPlusTree7.cpp
    src/utils/tree71/WeightBalancedTree3.cpp
    src/utils/tree72/PersistentTree3.cpp
    src/utils/tree73/AVLTree7.cpp
    src/utils/tree74/RedBlackTree7.cpp
    src/utils/tree75/BPlusTree8.cpp
    src/utils/tree76/IntervalTree5.cpp
    src/utils/tree77/QuadTree4.cpp
    src/utils/tree78/KDTree5.cpp
    src/utils/tree79/PrioritySearchTree3.cpp
    src/utils/tree80/SuffixArray4.cpp
    src/utils/tree81/RangeTree4.cpp
    src/utils/tree82/WaveletTree4.cpp
    src/utils/tree83/RedBlackTree8.cpp
    src/utils/tree84/SplayTree7.cpp
    src/utils/tree85/Treap7.cpp
    src/utils/tree86/AVLTree8.cpp
    src/utils/tree87/BPlusTree9.cpp
    src/utils/tree88/IntervalTree6.cpp
    src/utils/tree89/QuadTree5.cpp
    src/utils/tree90/KDTree6.cpp
    src/utils/tree91/SuffixTree4.cpp
    src/utils/tree92/BinaryHeap8.cpp
    src/utils/tree93/FenwickTree6.cpp
    src/utils/tree94/AVLTree9.cpp
    src/utils/tree95/SplayTree8.cpp
    src/utils/tree96/RedBlackTree9.cpp
    src/utils/tree97/Treap8.cpp
    src/utils/tree98/BPlusTree10.cpp
    src/utils/tree99/SuffixTree5.cpp
    src/utils/tree100/BinaryHeap9.cpp
    src/utils/tree101/FenwickTree7.cpp
    src/utils/tree102/AVLTree10.cpp
    src/utils/tree103/SplayTree9.cpp
    src/utils/tree104/RedBlackTree10.cpp
    src/utils/tree105/Treap9.cpp
    src/utils/tree106/BPlusTree11.cpp
    src/utils/tree107/SuffixTree6.cpp
    src/utils/tree108/BinaryHeap10.cpp
    src/utils/tree109/FenwickTree8.cpp
    src/utils/tree110/AVLTree11.cpp
    src/utils/tree111/SplayTree10.cpp
    src/utils/tree112/RedBlackTree11.cpp
    src/utils/tree113/Treap10.cpp
    src/utils/tree114/BPlusTree12.cpp
    src/utils/tree115/SuffixTree7.cpp
    src/utils/tree116/BinaryHeap11.cpp
    src/utils/tree117/FenwickTree9.cpp
    src/utils/tree118/AVLTree12.cpp
    src/utils/tree119/SplayTree11.cpp
    src/utils/tree120/RedBlackTree12.cpp
    src/utils/tree121/Treap11.cpp
    src/utils/tree122/BPlusTree13.cpp
    src/utils/tree161/IntervalHeap.cpp
    src/utils/cluster159/DBSCAN8.cpp
    src/utils/code160/PolarCode4.cpp
    src/utils/dsp161/MultibandGate.cpp
    src/utils/fft162/ConstantQTransform.cpp
    src/utils/graph175/ChromaticIndex.cpp
    src/utils/matrix165/SingularValueSolver.cpp
    src/utils/signal163/WienerFilter.cpp
    src/utils/tree162/BStarTree.cpp
    src/utils/cluster160/MeanShift7.cpp
    src/utils/code161/GolayCode3.cpp
    src/utils/dsp162/StereoEnhancer.cpp
    src/utils/fft163/HaarWaveletTransform.cpp
    src/utils/graph176/FlowNetwork.cpp
    src/utils/matrix166/LeastSquaresSolver.cpp
    src/utils/signal164/NoiseProfiler.cpp
    src/utils/tree163/LeftistHeap.cpp
    src/utils/cluster161/GaussianMixture11.cpp
    src/utils/code162/BCHCode4.cpp
    src/utils/dsp163/VocalRemover.cpp
    src/utils/fft164/DiscreteCosineTransform.cpp
    src/utils/graph177/MinCostFlow.cpp
    src/utils/matrix167/ConditionEstimator.cpp
    src/utils/signal165/ChannelEqualizer.cpp
    src/utils/tree164/PairingHeap.cpp
    src/utils/cluster162/OPTICS5.cpp
    src/utils/code163/ReedMullerCode2.cpp
    src/utils/dsp164/PhaseVocoder.cpp
    src/utils/fft165/DiscreteSineTransform.cpp
    src/utils/graph178/BipartiteMatch.cpp
    src/utils/matrix168/EigenValueSolver.cpp
    src/utils/signal166/Beamformer2.cpp
    src/utils/tree165/FibonacciHeap.cpp
    src/utils/cluster163/Agglomerative6.cpp
    src/utils/code164/ConvolutionalCode6.cpp
    src/utils/dsp165/GranularSynthesis.cpp
    src/utils/fft166/ModifiedDCT.cpp
    src/utils/graph179/Hungarian.cpp
    src/utils/matrix169/SparseSolver.cpp
    src/utils/signal167/PitchDetector.cpp
    src/utils/tree166/VanEmdeBoas.cpp
    src/utils/cluster164/BirchClustering5.cpp
    src/utils/code165/ErasureCode4.cpp
    src/utils/dsp166/Deconvolver.cpp
    src/utils/fft167/WalshHadamard.cpp
    src/utils/graph180/StoerWagner.cpp
    src/utils/matrix170/CholeskySolver.cpp
    src/utils/signal168/HarmonicAnalyzer.cpp
    src/utils/tree167/ScapegoatTree4.cpp
    src/utils/cluster165/FuzzyCMeans6.cpp
    src/utils/code166/TrellisCode.cpp
    src/utils/dsp167/ConvolutionReverb.cpp
    src/utils/fft168/NumberTheoreticTransform.cpp
    src/utils/graph181/EulerTour.cpp
    src/utils/matrix171/LUDecomposition.cpp
    src/utils/signal169/SignalSynchronizer.cpp
    src/utils/tree168/WeightBalancedTree4.cpp
    src/utils/cluster166/SpectralCluster6.cpp
    src/utils/code167/GrayCode3.cpp
    src/utils/dsp168/BinauralProcessor.cpp
    src/utils/fft169/DiscreteHartleyTransform.cpp
    src/utils/graph182/StrongConnectivity.cpp
    src/utils/matrix172/QRDecomposition.cpp
    src/utils/signal170/AdaptiveLineEnhancer2.cpp
    src/utils/tree169/BPlusTree6.cpp
    src/utils/cluster167/KMeans15.cpp
    src/utils/code168/HuffmanCode3.cpp
    src/utils/dsp169/DynamicEQ.cpp
    src/utils/fft170/SlidingDFT4.cpp
    src/utils/graph183/TopologicalSort.cpp
    src/utils/matrix173/SVD2.cpp
    src/utils/signal171/Resampler.cpp
    src/utils/tree170/RedBlackTree8.cpp
    src/utils/cluster168/DBSCAN9.cpp
    src/utils/code169/ArithmeticCode.cpp
    src/utils/dsp170/SpectralGate.cpp
    src/utils/fft171/ZoomFFT.cpp
    src/utils/graph184/TransitiveClosure.cpp
    src/utils/matrix174/EigenVectorSolver.cpp
    src/utils/signal172/EnvelopeDetector2.cpp
    src/utils/tree171/AA_Tree3.cpp
    src/utils/cluster169/HierarchicalCluster6.cpp
    src/utils/code170/LempelZivWelch3.cpp
    src/utils/dsp171/MultibandCompressor2.cpp
    src/utils/fft172/PrimeFactorFFT.cpp
    src/utils/graph185/GraphColoring5.cpp
    src/utils/matrix175/TridiagonalSolver.cpp
    src/utils/signal173/SignalGenerator2.cpp
    src/utils/tree172/CartesianTree5.cpp
    src/utils/cluster170/SubspaceCluster4.cpp
    src/utils/code171/RunLengthCode3.cpp
    src/utils/dsp172/TransientShaper2.cpp
    src/utils/fft173/SplitRadixFFT.cpp
    src/utils/graph186/DominatingSet3.cpp
    src/utils/matrix176/BandedSolver.cpp
    src/utils/signal174/CorrelationAnalyzer.cpp
    src/utils/tree173/Treap6.cpp
    src/utils/cluster171/GaussianMixture12.cpp
    src/utils/code172/DeltaCode.cpp
    src/utils/dsp173/SpectralRepair.cpp
    src/utils/fft174/WinogradFFT.cpp
    src/utils/graph187/GraphMatching2.cpp
    src/utils/matrix177/IterativeSolver.cpp
    src/utils/signal175/FeatureExtractor.cpp
    src/utils/tree174/SplayTree6.cpp
    src/utils/cluster172/KMedoids14.cpp
    src/utils/code173/CaesarCode.cpp
    src/utils/dsp174/Limiter3.cpp
    src/utils/fft175/DistributedArithmetic4.cpp
    src/utils/graph188/MaximumClique.cpp
    src/utils/matrix178/SparseLU.cpp
    src/utils/signal176/WaveletDenoiser2.cpp
    src/utils/tree175/AvlTree3.cpp
    src/utils/cluster173/MeanShift8.cpp
    src/utils/code174/VigenereCode.cpp
    src/utils/dsp175/NoiseGate3.cpp
    src/utils/fft176/Goertzel4.cpp
    src/utils/graph189/IndependentSet3.cpp
    src/utils/matrix179/SchurDecomposition4.cpp
    src/utils/signal177/Periodogram.cpp
    src/utils/tree176/BTree4.cpp
    src/utils/cluster174/Agglomerative7.cpp
    src/utils/code175/SubstitutionCode.cpp
    src/utils/dsp176/Compressor2.cpp
    src/utils/fft177/ChirpZ4.cpp
    src/utils/graph190/Biconnected9.cpp
    src/utils/matrix180/SymmetricEigenSolver.cpp
    src/utils/signal178/WindowFunction.cpp
    src/utils/tree177/FenwickTree4.cpp
    src/utils/cluster175/BirchClustering6.cpp
    src/utils/code176/TranspositionCode.cpp
    src/utils/dsp177/Expander2.cpp
    src/utils/fft178/PrimeFactorFFT4.cpp
    src/utils/graph191/DominatingSet4.cpp
    src/utils/matrix181/GeneralizedEigenSolver.cpp
    src/utils/signal179/FilterDesign.cpp
    src/utils/tree178/SegmentTree4.cpp
    src/utils/cluster176/FuzzyCMeans7.cpp
    src/utils/code177/PlayfairCode.cpp
    src/utils/dsp178/Gate3.cpp
    src/utils/fft179/RaderFFT4.cpp
    src/utils/graph192/VertexCover4.cpp
    src/utils/matrix182/SVD3.cpp
    src/utils/signal180/Resampler2.cpp
    src/utils/tree179/IntervalTree4.cpp
    src/utils/cluster177/SpectralCluster7.cpp
    src/utils/code178/ADFGVXCode.cpp
    src/utils/dsp179/Limiter4.cpp
    src/utils/fft180/WinogradFFT4.cpp
    src/utils/graph193/EdgeColoring.cpp
    src/utils/matrix183/SparseQR.cpp
    src/utils/signal181/Correlator.cpp
    src/utils/tree180/SuffixTree4.cpp
    src/utils/cluster178/KMeans16.cpp
    src/utils/code179/RailFenceCode.cpp
    src/utils/dsp180/MultibandCompressor3.cpp
    src/utils/fft181/BruunFFT4.cpp
    src/utils/graph194/TravelingSalesman.cpp
    src/utils/matrix184/ConjugateGradient.cpp
    src/utils/signal182/WaveletDenoiser3.cpp
    src/utils/tree181/Rope4.cpp
    src/utils/cluster179/DBSCAN10.cpp
    src/utils/code180/AffineCode.cpp
    src/utils/dsp181/SpectralGate2.cpp
    src/utils/fft182/MixedRadixFFT4.cpp
    src/utils/graph195/ChinesePostman.cpp
    src/utils/matrix185/BiCGSTAB.cpp
    src/utils/signal183/AdaptiveFilter3.cpp
    src/utils/tree182/AA4.cpp
    src/utils/cluster180/GaussianMixture13.cpp
    src/utils/code181/BeaufortCode.cpp
    src/utils/dsp182/Deesser2.cpp
    src/utils/fft183/SplitRadixFFT4.cpp
    src/utils/graph196/GraphIsomorphism.cpp
    src/utils/matrix186/GMRES.cpp
    src/utils/signal184/SignalClassifier.cpp
    src/utils/tree183/Treap7.cpp
    src/utils/cluster181/KMedoids15.cpp
    src/utils/code182/AutokeyCode.cpp
    src/utils/dsp183/PhaseCorrelator.cpp
    src/utils/fft184/HexFFT4.cpp
    src/utils/graph197/MaximumFlow.cpp
    src/utils/matrix187/Cholesky3.cpp
    src/utils/signal185/PitchDetector3.cpp
    src/utils/tree184/WAVL4.cpp
    src/utils/cluster182/HierarchicalCluster7.cpp
    src/utils/code183/FourSquareCode.cpp
    src/utils/dsp184/Convolver2.cpp
    src/utils/fft185/DCT4.cpp
    src/utils/graph198/MinimumSpanningTree.cpp
    src/utils/matrix188/ThomasAlgorithm.cpp
    src/utils/signal186/EnvelopeDetector3.cpp
    src/utils/tree185/RedBlackTree9.cpp
    src/utils/cluster183/SubspaceCluster5.cpp
    src/utils/code184/BifidCode.cpp
    src/utils/dsp185/DynamicEQ2.cpp
    src/utils/fft186/DST4.cpp
    src/utils/graph199/StrongConnectivity3.cpp
    src/utils/matrix189/GaussSeidel.cpp
    src/utils/signal187/SignalSynchronizer2.cpp
    src/utils/tree186/ScapegoatTree5.cpp
    src/utils/cluster184/OPTICS6.cpp
    src/utils/code185/NihilistCode.cpp
    src/utils/dsp186/TransientShaper3.cpp
    src/utils/fft187/WHT4.cpp
    src/utils/graph200/EulerTour3.cpp
    src/utils/matrix190/SparseLU2.cpp
    src/utils/signal188/FeatureExtractor2.cpp
    src/utils/tree187/WeightBalancedTree5.cpp
    src/utils/cluster185/GaussianMixture14.cpp
    src/utils/code186/TrifidCode.cpp
    src/utils/dsp187/MultibandGate2.cpp
    src/utils/fft188/NumberTheoreticTransform2.cpp
    src/utils/graph201/TransitiveClosure2.cpp
    src/utils/matrix191/IterativeRefinement.cpp
    src/utils/signal189/SignalGenerator3.cpp
    src/utils/tree188/CartesianTree6.cpp
    src/utils/cluster186/KMeans17.cpp
    src/utils/code187/GronsfeldCode.cpp
    src/utils/dsp188/SpectralRepair2.cpp
    src/utils/fft189/DistributedArithmetic5.cpp
    src/utils/graph202/MaximumClique2.cpp
    src/utils/matrix192/BandSolver2.cpp
    src/utils/signal190/WaveletDenoiser4.cpp
    src/utils/tree189/AvlTree4.cpp
    src/utils/cluster187/MeanShift9.cpp
    src/utils/code188/Chaocipher.cpp
    src/utils/dsp189/Delay3.cpp
    src/utils/fft190/SlidingDFT5.cpp
    src/utils/graph203/GraphColoring6.cpp
    src/utils/matrix193/TridiagonalSolver2.cpp
    src/utils/signal191/Resampler3.cpp
    src/utils/tree190/BPlusTree7.cpp
    src/utils/cluster188/Agglomerative8.cpp
    src/utils/code189/BazeleriesCode.cpp
    src/utils/dsp190/Reverb2.cpp
    src/utils/fft191/ZoomFFT2.cpp
    src/utils/graph204/HamiltonianCycle.cpp
    src/utils/matrix194/EigenVectorSolver2.cpp
    src/utils/signal192/SignalClassifier2.cpp
    src/utils/tree191/SplayTree7.cpp
    src/utils/cluster189/BirchClustering7.cpp
    src/utils/code190/DoubleTranspositionCode.cpp
    src/utils/dsp191/Chorus2.cpp
    src/utils/fft192/Goertzel5.cpp
    src/utils/graph205/IndependentSet4.cpp
    src/utils/matrix195/SchurDecomposition5.cpp
    src/utils/signal193/Periodogram2.cpp
    src/utils/tree192/VanEmdeBoas4.cpp
    src/utils/cluster190/FuzzyCMeans8.cpp
    src/utils/code191/TwoSquareCode.cpp
    src/utils/dsp192/Flanger2.cpp
    src/utils/fft193/DHT4.cpp
    src/utils/graph206/DominatingSet5.cpp
    src/utils/matrix196/SymmetricEigenSolver2.cpp
    src/utils/signal194/WindowFunction2.cpp
    src/utils/tree193/FenwickTree5.cpp
    src/utils/cluster191/SpectralCluster8.cpp
    src/utils/code192/StraddlingCheckerboard.cpp
    src/utils/dsp193/Phaser2.cpp
    src/utils/fft194/ChirpZ5.cpp
    src/utils/graph207/Biconnected10.cpp
    src/utils/matrix197/GeneralizedEigenSolver2.cpp
    src/utils/signal195/FilterDesign2.cpp
    src/utils/tree194/SegmentTree5.cpp
    src/utils/cluster192/GaussianMixture15.cpp
    src/utils/code193/PolluxCode.cpp
    src/utils/dsp194/Compressor3.cpp
    src/utils/fft195/PrimeFactorFFT5.cpp
    src/utils/graph208/VertexCover5.cpp
    src/utils/matrix198/SVD4.cpp
    src/utils/signal196/Resampler4.cpp
    src/utils/tree195/IntervalTree5.cpp
    src/utils/cluster193/KMeans18.cpp
    src/utils/code194/MorbitCode.cpp
    src/utils/dsp195/Limiter5.cpp
    src/utils/fft196/WinogradFFT5.cpp
    src/utils/graph209/EdgeColoring2.cpp
    src/utils/matrix199/SparseQR2.cpp
    src/utils/signal197/Correlator2.cpp
    src/utils/tree196/SuffixTree5.cpp
    src/utils/cluster194/BirchClustering8.cpp
    src/utils/code195/FractionatedMorse.cpp
    src/utils/dsp196/MultibandCompressor4.cpp
    src/utils/fft197/ChirpZ6.cpp
    src/utils/graph210/TravelingSalesman2.cpp
    src/utils/matrix200/ConjugateGradient2.cpp
    src/utils/signal198/WaveletDenoiser5.cpp
    src/utils/tree197/Rope5.cpp
    src/utils/cluster195/DBSCAN11.cpp
    src/utils/code196/BaudotCode.cpp
    src/utils/dsp197/Expander3.cpp
    src/utils/fft198/HexFFT5.cpp
    src/utils/graph211/ChinesePostman2.cpp
    src/utils/matrix201/BiCGSTAB2.cpp
    src/utils/signal199/AdaptiveFilter4.cpp
    src/utils/tree198/AA5.cpp
    src/utils/cluster196/GaussianMixture16.cpp
    src/utils/code197/TapirCode.cpp
    src/utils/dsp198/Deesser3.cpp
    src/utils/fft199/MixedRadixFFT5.cpp
    src/utils/graph212/GraphIsomorphism2.cpp
    src/utils/matrix202/GMRES2.cpp
    src/utils/signal200/SignalClassifier3.cpp
    src/utils/tree199/Treap8.cpp
    src/utils/cluster197/KMedoids16.cpp
    src/utils/code198/RouteCipher.cpp
    src/utils/dsp199/PhaseCorrelator2.cpp
    src/utils/fft200/RaderFFT5.cpp
    src/utils/graph213/MaximumFlow2.cpp
    src/utils/matrix203/Cholesky4.cpp
    src/utils/signal201/PitchDetector4.cpp
    src/utils/tree200/WAVL5.cpp
    src/utils/cluster198/HierarchicalCluster8.cpp
    src/utils/code199/TwoSquareCode2.cpp
    src/utils/dsp200/Convolver3.cpp
    src/utils/fft201/DCT5.cpp
    src/utils/graph214/MinimumSpanningTree2.cpp
    src/utils/matrix204/ThomasAlgorithm2.cpp
    src/utils/signal202/FilterDesign3.cpp
    src/utils/tree201/RedBlackTree10.cpp
    src/utils/cluster199/SubspaceCluster6.cpp
    src/utils/code200/FoursquareCode2.cpp
    src/utils/dsp201/Gate4.cpp
    src/utils/fft202/DST5.cpp
    src/utils/graph215/StrongConnectivity4.cpp
    src/utils/matrix205/GaussSeidel2.cpp
    src/utils/signal203/SignalSynchronizer3.cpp
    src/utils/tree202/IntervalTree6.cpp
    src/utils/cluster200/OPTICS7.cpp
    src/utils/code201/DigrafidCode.cpp
    src/utils/dsp202/Limiter6.cpp
    src/utils/fft203/WinogradFFT6.cpp
    src/utils/graph216/EdgeColoring3.cpp
    src/utils/matrix206/SparseQR3.cpp
    src/utils/signal204/Correlator3.cpp
    src/utils/tree203/SuffixTree6.cpp
    src/utils/cluster201/SpectralCluster9.cpp
    src/utils/code202/SeriatedPlayfair.cpp
    src/utils/dsp203/SpectralGate3.cpp
    src/utils/fft204/BruunFFT5.cpp
    src/utils/graph217/TravelingSalesman3.cpp
    src/utils/matrix207/ConjugateGradient3.cpp
    src/utils/signal205/WaveletDenoiser6.cpp
    src/utils/tree204/Rope6.cpp
    src/utils/cluster202/KMeans19.cpp
    src/utils/code203/ColumnarTransposition.cpp
    src/utils/dsp204/Deesser4.cpp
    src/utils/fft205/SplitRadixFFT5.cpp
    src/utils/graph218/GraphIsomorphism3.cpp
    src/utils/matrix208/BiCGSTAB3.cpp
    src/utils/signal206/SignalClassifier4.cpp
    src/utils/tree205/Treap9.cpp
    src/utils/cluster203/Agglomerative9.cpp
    src/utils/code204/PolluxCode2.cpp
    src/utils/dsp205/NoiseGate4.cpp
    src/utils/fft206/Goertzel6.cpp
    src/utils/graph219/DominatingSet6.cpp
    src/utils/matrix209/EigenVectorSolver3.cpp
    src/utils/signal207/Periodogram3.cpp
    src/utils/tree206/AvlTree5.cpp
    src/utils/cluster204/BirchClustering9.cpp
    src/utils/code205/FoursquareCode3.cpp
    src/utils/dsp206/MultibandGate3.cpp
    src/utils/fft207/ChirpZ7.cpp
    src/utils/graph220/Biconnected11.cpp
    src/utils/matrix210/SchurDecomposition6.cpp
    src/utils/signal208/WindowFunction3.cpp
    src/utils/tree207/BPlusTree8.cpp
    src/utils/cluster205/FuzzyCMeans9.cpp
    src/utils/code206/StraddlingCheckerboard2.cpp
    src/utils/dsp207/Expander4.cpp
    src/utils/fft208/RaderFFT6.cpp
    src/utils/graph221/TravelingSalesman4.cpp
    src/utils/matrix211/ConjugateGradient4.cpp
    src/utils/signal209/FilterDesign4.cpp
    src/utils/tree208/ScapegoatTree6.cpp
    src/utils/cluster206/SpectralCluster10.cpp
    src/utils/code207/SeriatedPlayfair2.cpp
    src/utils/dsp208/SpectralGate4.cpp
    src/utils/fft209/BruunFFT6.cpp
    src/utils/graph222/ChinesePostman3.cpp
    src/utils/matrix212/BiCGSTAB4.cpp
    src/utils/signal210/WaveletDenoiser7.cpp
    src/utils/tree209/VanEmdeBoas5.cpp
    src/utils/cluster207/GaussianMixture17.cpp
    src/utils/code208/DoubleTranspositionCode2.cpp
    src/utils/dsp209/Chorus3.cpp
    src/utils/fft210/PrimeFactorFFT6.cpp
    src/utils/graph223/GraphIsomorphism4.cpp
    src/utils/matrix213/SymmetricEigenSolver3.cpp
    src/utils/signal211/Correlator4.cpp
    src/utils/tree210/RedBlackTree11.cpp
    src/utils/cluster208/KMedoids17.cpp
    src/utils/code209/FractionatedMorse2.cpp
    src/utils/dsp210/Flanger3.cpp
    src/utils/fft211/DCT6.cpp
    src/utils/graph224/HamiltonianCycle2.cpp
    src/utils/matrix214/ThomasAlgorithm3.cpp
    src/utils/signal212/Resampler5.cpp
    src/utils/tree211/SplayTree8.cpp
    src/utils/cluster209/HierarchicalCluster9.cpp
    src/utils/code210/BaudotCode2.cpp
    src/utils/dsp211/Phaser3.cpp
    src/utils/fft212/DST6.cpp
    src/utils/graph225/EulerTour4.cpp
    src/utils/matrix215/SparseLU3.cpp
    src/utils/signal213/SignalSynchronizer4.cpp
    src/utils/tree212/FenwickTree6.cpp
    src/utils/cluster210/SubspaceCluster7.cpp
    src/utils/code211/TapirCode2.cpp
    src/utils/dsp212/Delay4.cpp
    src/utils/fft213/WHT5.cpp
    src/utils/graph226/TransitiveClosure3.cpp
    src/utils/matrix216/IterativeRefinement2.cpp
    src/utils/signal214/FeatureExtractor3.cpp
    src/utils/tree213/CartesianTree7.cpp
    src/utils/cluster211/OPTICS8.cpp
    src/utils/code212/DigrafidCode2.cpp
    src/utils/dsp213/Reverb3.cpp
    src/utils/fft214/DistributedArithmetic6.cpp
    src/utils/graph227/MaximumClique3.cpp
    src/utils/matrix217/GaussSeidel3.cpp
    src/utils/signal215/SignalGenerator4.cpp
    src/utils/tree214/WeightBalancedTree6.cpp
    src/utils/cluster212/GaussianMixture18.cpp
    src/utils/code213/MorbitCode2.cpp
    src/utils/dsp214/Limiter7.cpp
    src/utils/fft215/ZoomFFT3.cpp
    src/utils/graph228/EdgeColoring4.cpp
    src/utils/matrix218/GMRES3.cpp
    src/utils/signal216/EnvelopeDetector4.cpp
    src/utils/tree215/Treap10.cpp
    src/utils/cluster213/KMeans20.cpp
    src/utils/code214/RouteCipher2.cpp
    src/utils/dsp215/Compressor4.cpp
    src/utils/fft216/SlidingDFT6.cpp
    src/utils/graph229/GraphColoring7.cpp
    src/utils/matrix219/TridiagonalSolver3.cpp
    src/utils/signal217/AdaptiveFilter5.cpp
    src/utils/tree216/Rope7.cpp
    src/utils/cluster214/DBSCAN12.cpp
    src/utils/code215/BazeleriesCode2.cpp
    src/utils/dsp216/MultibandCompressor5.cpp
    src/utils/fft217/NumberTheoreticTransform3.cpp
    src/utils/graph230/VertexCover6.cpp
    src/utils/matrix220/SVD5.cpp
    src/utils/signal218/PitchDetector5.cpp
    src/utils/tree217/BTree5.cpp
    src/utils/cluster215/GaussianMixture19.cpp
    src/utils/code216/BaudotCode3.cpp
    src/utils/dsp217/Expander5.cpp
    src/utils/fft218/HexFFT6.cpp
    src/utils/graph231/MaximumFlow3.cpp
    src/utils/matrix221/Cholesky5.cpp
    src/utils/signal219/SignalClassifier5.cpp
    src/utils/tree218/SegmentTree6.cpp
    src/utils/cluster216/KMeans21.cpp
    src/utils/code217/FoursquareCode4.cpp
    src/utils/dsp218/Deesser5.cpp
    src/utils/fft219/MixedRadixFFT6.cpp
    src/utils/graph232/ChinesePostman4.cpp
    src/utils/matrix222/BiCGSTAB5.cpp
    src/utils/signal220/WaveletDenoiser8.cpp
    src/utils/tree219/AA6.cpp
    src/utils/cluster217/Agglomerative10.cpp
    src/utils/code218/PolluxCode3.cpp
    src/utils/dsp219/NoiseGate5.cpp
    src/utils/fft220/SplitRadixFFT6.cpp
    src/utils/graph233/GraphIsomorphism5.cpp
    src/utils/matrix223/EigenVectorSolver4.cpp
    src/utils/signal221/Periodogram4.cpp
    src/utils/tree220/AvlTree6.cpp
    src/utils/cluster218/BirchClustering10.cpp
    src/utils/code219/StraddlingCheckerboard3.cpp
    src/utils/dsp220/MultibandGate4.cpp
    src/utils/fft221/Goertzel7.cpp
    src/utils/graph234/DominatingSet7.cpp
    src/utils/matrix224/SchurDecomposition7.cpp
    src/utils/signal222/WindowFunction4.cpp
    src/utils/tree221/BPlusTree9.cpp
    src/utils/cluster219/FuzzyCMeans10.cpp
    src/utils/code220/SeriatedPlayfair3.cpp
    src/utils/dsp221/Expander6.cpp
    src/utils/fft222/RaderFFT7.cpp
    src/utils/graph235/TravelingSalesman5.cpp
    src/utils/matrix225/ConjugateGradient5.cpp
    src/utils/signal223/FilterDesign5.cpp
    src/utils/tree222/ScapegoatTree7.cpp
    src/utils/cluster220/SpectralCluster11.cpp
    src/utils/code221/DoubleTranspositionCode3.cpp
    src/utils/dsp222/Chorus4.cpp
    src/utils/fft223/PrimeFactorFFT7.cpp
    src/utils/graph236/ChinesePostman5.cpp
    src/utils/matrix226/BiCGSTAB6.cpp
    src/utils/signal224/WaveletDenoiser9.cpp
    src/utils/tree223/VanEmdeBoas6.cpp
    src/utils/cluster221/GaussianMixture20.cpp
    src/utils/code222/FoursquareCode5.cpp
    src/utils/dsp223/SpectralGate5.cpp
    src/utils/fft224/BruunFFT7.cpp
    src/utils/graph237/GraphIsomorphism6.cpp
    src/utils/matrix227/SymmetricEigenSolver4.cpp
    src/utils/signal225/Correlator5.cpp
    src/utils/tree224/RedBlackTree12.cpp
    src/utils/cluster222/KMedoids18.cpp
    src/utils/code223/FractionatedMorse3.cpp
    src/utils/dsp224/Flanger4.cpp
    src/utils/fft225/DCT7.cpp
    src/utils/graph238/HamiltonianCycle3.cpp
    src/utils/matrix228/ThomasAlgorithm4.cpp
    src/utils/signal226/Resampler6.cpp
    src/utils/tree225/SplayTree9.cpp
    src/utils/cluster223/HierarchicalCluster10.cpp
    src/utils/code224/BaudotCode4.cpp
    src/utils/dsp225/Phaser4.cpp
    src/utils/fft226/DST7.cpp
    src/utils/graph239/EulerTour5.cpp
    src/utils/matrix229/SparseLU4.cpp
    src/utils/signal227/SignalSynchronizer5.cpp
    src/utils/tree226/FenwickTree7.cpp
    src/utils/cluster224/SubspaceCluster8.cpp
    src/utils/code225/TapirCode3.cpp
    src/utils/dsp226/Delay5.cpp
    src/utils/fft227/WHT6.cpp
    src/utils/graph240/TransitiveClosure4.cpp
    src/utils/matrix230/IterativeRefinement3.cpp
    src/utils/signal228/FeatureExtractor4.cpp
    src/utils/tree227/CartesianTree8.cpp
    src/utils/cluster225/OPTICS9.cpp
    src/utils/code226/DigrafidCode3.cpp
    src/utils/dsp227/Reverb4.cpp
    src/utils/fft228/DistributedArithmetic7.cpp
    src/utils/graph241/MaximumClique4.cpp
    src/utils/matrix231/GaussSeidel4.cpp
    src/utils/signal229/SignalGenerator5.cpp
    src/utils/tree228/WeightBalancedTree7.cpp
    src/utils/cluster226/GaussianMixture21.cpp
    src/utils/code227/MorbitCode3.cpp
    src/utils/dsp228/Limiter8.cpp
    src/utils/fft229/ZoomFFT4.cpp
    src/utils/graph242/EdgeColoring5.cpp
    src/utils/matrix232/GMRES4.cpp
    src/utils/signal230/EnvelopeDetector5.cpp
    src/utils/tree229/Treap11.cpp
    src/utils/cluster227/KMeans22.cpp
    src/utils/code228/RouteCipher3.cpp
    src/utils/dsp229/Compressor5.cpp
    src/utils/fft230/SlidingDFT7.cpp
    src/utils/graph243/GraphColoring8.cpp
    src/utils/matrix233/TridiagonalSolver4.cpp
    src/utils/signal231/AdaptiveFilter6.cpp
    src/utils/tree230/Rope8.cpp
    src/utils/cluster228/DBSCAN13.cpp
    src/utils/code229/BazeleriesCode3.cpp
    src/utils/dsp230/MultibandCompressor6.cpp
    src/utils/fft231/NumberTheoreticTransform4.cpp
    src/utils/graph244/VertexCover7.cpp
    src/utils/matrix234/SVD6.cpp
    src/utils/signal232/PitchDetector6.cpp
    src/utils/tree231/BTree6.cpp
    src/utils/cluster229/GaussianMixture22.cpp
    src/utils/code230/BaudotCode5.cpp
    src/utils/dsp231/Expander7.cpp
    src/utils/fft232/HexFFT7.cpp
    src/utils/graph245/MaximumFlow4.cpp
    src/utils/matrix235/Cholesky6.cpp
    src/utils/signal233/SignalClassifier6.cpp
    src/utils/tree232/SegmentTree7.cpp
    src/utils/cluster230/KMeans23.cpp
    src/utils/code231/FoursquareCode6.cpp
    src/utils/dsp232/Deesser6.cpp
    src/utils/fft233/MixedRadixFFT7.cpp
    src/utils/graph246/ChinesePostman6.cpp
    src/utils/matrix236/BiCGSTAB7.cpp
    src/utils/signal234/WaveletDenoiser10.cpp
    src/utils/tree233/AA7.cpp
    src/utils/cluster231/Agglomerative11.cpp
    src/utils/code232/PolluxCode4.cpp
    src/utils/dsp233/NoiseGate6.cpp
    src/utils/fft234/SplitRadixFFT7.cpp
    src/utils/graph247/GraphIsomorphism7.cpp
    src/utils/matrix237/EigenVectorSolver5.cpp
    src/utils/signal235/Periodogram5.cpp
    src/utils/tree234/AvlTree7.cpp
    src/utils/cluster232/BirchClustering11.cpp
    src/utils/code233/StraddlingCheckerboard4.cpp
    src/utils/dsp234/MultibandGate4.cpp
    src/utils/fft235/Goertzel8.cpp
    src/utils/graph248/DominatingSet8.cpp
    src/utils/matrix238/SchurDecomposition8.cpp
    src/utils/signal236/WindowFunction4.cpp
    src/utils/tree235/BPlusTree10.cpp
    src/utils/cluster233/FuzzyCMeans11.cpp
    src/utils/code234/SeriatedPlayfair4.cpp
    src/utils/dsp235/Expander8.cpp
    src/utils/fft236/RaderFFT8.cpp
    src/utils/graph249/TravelingSalesman6.cpp
    src/utils/matrix239/ConjugateGradient6.cpp
    src/utils/signal237/FilterDesign6.cpp
    src/utils/tree236/ScapegoatTree8.cpp
    src/utils/cluster234/SpectralCluster12.cpp
    src/utils/code235/DoubleTranspositionCode4.cpp
    src/utils/dsp236/Chorus5.cpp
    src/utils/fft237/PrimeFactorFFT8.cpp
    src/utils/graph250/ChinesePostman7.cpp
    src/utils/matrix240/BiCGSTAB8.cpp
    src/utils/signal238/WaveletDenoiser11.cpp
    src/utils/tree237/VanEmdeBoas7.cpp
    src/utils/cluster235/GaussianMixture23.cpp
    src/utils/code236/FoursquareCode7.cpp
    src/utils/dsp237/SpectralGate5.cpp
    src/utils/fft238/BruunFFT8.cpp
    src/utils/graph251/GraphIsomorphism8.cpp
    src/utils/matrix241/SymmetricEigenSolver5.cpp
    src/utils/signal239/Correlator6.cpp
    src/utils/tree238/RedBlackTree13.cpp
    src/utils/cluster236/KMedoids19.cpp
    src/utils/code237/FractionatedMorse4.cpp
    src/utils/dsp238/Flanger5.cpp
    src/utils/fft239/DCT8.cpp
    src/utils/graph252/HamiltonianCycle4.cpp
    src/utils/matrix242/ThomasAlgorithm4.cpp
    src/utils/signal240/Resampler7.cpp
    src/utils/tree239/SplayTree10.cpp
    src/utils/cluster237/HierarchicalCluster11.cpp
    src/utils/code238/BaudotCode6.cpp
    src/utils/dsp239/Phaser5.cpp
    src/utils/fft240/DST8.cpp
    src/utils/graph253/EulerTour6.cpp
    src/utils/matrix243/SparseLU4.cpp
    src/utils/signal241/SignalSynchronizer6.cpp
    src/utils/tree240/FenwickTree8.cpp
    src/utils/cluster238/SubspaceCluster9.cpp
    src/utils/code239/TapirCode4.cpp
    src/utils/dsp240/Delay6.cpp
    src/utils/fft241/WHT7.cpp
    src/utils/graph254/TransitiveClosure5.cpp
    src/utils/matrix244/IterativeRefinement4.cpp
    src/utils/signal242/FeatureExtractor5.cpp
    src/utils/tree241/CartesianTree9.cpp
    src/utils/cluster239/OPTICS10.cpp
    src/utils/code240/DigrafidCode4.cpp
    src/utils/dsp241/Reverb5.cpp
    src/utils/fft242/DistributedArithmetic8.cpp
    src/utils/graph255/MaximumClique5.cpp
    src/utils/matrix245/GaussSeidel5.cpp
    src/utils/signal243/SignalGenerator6.cpp
    src/utils/tree242/WeightBalancedTree8.cpp
    src/utils/cluster240/GaussianMixture24.cpp
    src/utils/code241/MorbitCode4.cpp
    src/utils/dsp242/Limiter9.cpp
    src/utils/fft243/ZoomFFT5.cpp
    src/utils/graph256/EdgeColoring6.cpp
    src/utils/matrix246/GMRES5.cpp
    src/utils/signal244/EnvelopeDetector6.cpp
    src/utils/tree243/Treap12.cpp

    # ── Wave 50: New Feature Modules (Sources) ──
    src/protocol/svd/SvdParser.cpp
    src/protocol/svd/SvdParserStats.cpp
    src/protocol/svd/SvdRegisterTreeModel.cpp
    src/protocol/svd/SvdBitFieldWidget.cpp
    src/protocol/svd/SvdViewerWidget.cpp
    src/protocol/svd/SvdViewerWidgetStats.cpp
    src/protocol/logic/LogicSampler.cpp
    src/protocol/logic/LogicSamplerStats.cpp
    src/protocol/logic/ProtocolDecoder.cpp
    src/protocol/logic/ProtocolDecoderStats.cpp
    src/utils/firmware/FirmwareDiffer.cpp
    src/utils/firmware/FirmwareDifferStats.cpp
    src/utils/firmware/FirmwarePatchGenerator.cpp
    src/utils/firmware/FirmwarePatchGeneratorStats.cpp
    src/connection/bridge/PortBridge.cpp
    src/connection/bridge/PortBridgeStats.cpp
    src/connection/bridge/BridgeConfigPanel.cpp
    src/connection/bridge/BridgeConfigPanelStats.cpp
    src/chart/math/MathExpressionParser.cpp
    src/chart/math/MathExpressionParserStats.cpp
    # src/utils/pid/PidController.cpp  # removed: duplicate (already in Wave 1 section)

    # ── F73: 状态机设计器 ──
    src/utils/statemachine/StateMachineDesigner.cpp
    src/utils/statemachine/StateMachineDesignerStats.cpp
    src/utils/statemachine/StateMachineWidget.cpp
    src/utils/statemachine/StateMachineWidgetStats.cpp

    # ── F74: 网络拓扑发现 (Sources) ──
    src/utils/network/NetworkScanner.cpp
    src/utils/network/NetworkScannerStats.cpp
    src/utils/network/NetworkTopologyWidget.cpp
    src/utils/network/NetworkTopologyWidgetStats.cpp

    # ── F75: Eye Diagram Analyzer (Sources) ──
    src/chart/eye/EyeDiagramEngine.cpp
    src/chart/eye/EyeDiagramEngineStats.cpp
    src/chart/eye/EyeDiagramWidget.cpp
    src/chart/eye/EyeDiagramWidgetStats.cpp

    # ── F77: Protocol Sequencer ──
    src/protocol/sequencer/ProtocolSequencer.cpp
    src/protocol/sequencer/ProtocolSequencerStats.cpp
    src/protocol/sequencer/SequenceEditorWidget.cpp
    src/protocol/sequencer/SequenceEditorWidgetStats.cpp

    # ── F76: Waveform Math Engine (Sources) ──
    src/chart/math/WaveformMath.cpp
    src/chart/math/WaveformMathStats.cpp

    # ── F77: Digital Filter Designer ──
    src/utils/filter_design/FilterDesigner.cpp
    src/utils/filter_design/FilterDesignerStats.cpp
    src/utils/filter_design/FilterDesignerWidget.cpp
    src/utils/filter_design/FilterDesignerWidgetStats.cpp

    # ── F82: Waveform Generator ──
    src/utils/wavegen/WaveformGenerator.cpp
    src/utils/wavegen/WaveformGeneratorStats.cpp
    src/utils/wavegen/WaveformGeneratorWidget.cpp
    src/utils/wavegen/WaveformGeneratorWidgetStats.cpp

    # ── F85: MQTT Client Engine ──
    src/connection/mqtt_client/MqttClientEngine.cpp
    src/connection/mqtt_client/MqttClientEnginePacket.cpp
    src/connection/mqtt_client/MqttClientEngineStats.cpp
    src/connection/mqtt_client/MqttClientPanel.cpp
    src/connection/mqtt_client/MqttClientPanelStats.cpp

    # ── F88: Script Engine ──
    src/utils/scripting/ScriptEngine.cpp
    src/utils/scripting/ScriptEngineStats.cpp
    src/utils/scripting/ScriptEditorWidget.cpp
    src/utils/scripting/ScriptEditorWidgetStats.cpp
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
    src/chart/eye/EyeTypes.h
    src/chart/eye/EyeDiagramEngine.h
    src/chart/eye/EyeDiagramWidget.h
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

    # -- F118: 波形数学引擎 --
    src/chart/math/MathTypes.h
    src/chart/math/WaveformMath.h
    src/chart/math/MathExpressionParser.h

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

    # -- F63: 数据批处理器 (file missing) --
    # src/utils/batch/DataBatchProcessor.h

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
    src/utils/pid/PidSimulator.h
    src/utils/pid/PidTunerWidget.h

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

    # -- F96: 符号解码器 --
    src/utils/symbol/SymbolDecoder.h

    # -- F104: 计数多重集合 --
    src/utils/multiset/CountingMultiSet.h
    # -- F105: 索引双端队列 --
    src/utils/deque/IndexedDeque.h
    # -- F106: AVL平衡二叉搜索树 --
    src/utils/avltree/AvlTree.h
    # -- F107: 红黑树 --
    src/utils/redblack/RedBlackTree.h
    # -- F108: 汉明码编解码器 (duplicate removed) --
    # -- F109: 数值梯度计算器 --
    src/utils/gradient/NumericalGradient.h
    # -- F110: 自回归模型 --
    src/utils/armodel/AutoRegressiveModel.h
    # -- F111: 分形维数估计器 --
    src/utils/fractal/FractalDimension.h
    # -- F112: Hurst指数 --
    src/utils/hurst/HurstExponent.h
    # -- F113: 排列生成器 --
    src/utils/permutation/PermutationGenerator.h
    # -- F114: 组合生成器 --
    src/utils/combination/CombinationGenerator.h
    # -- F115: 计数型布隆过滤器 --
    src/utils/bloomfilter2/CountingBloomFilter.h
    # -- F116: Reed-Solomon编解码器 --
    src/utils/rscode/ReedSolomonCodec.h
    # -- F117: 频谱平坦度 --
    src/utils/spectrum/SpectralFlatness.h

    # -- F180~F182: 增强编码/排序 --
    src/utils/rle2/RunLengthEncoder2.h
    src/utils/shannon2/HuffmanTreeBuilder.h
    src/utils/sort2/RadixSorter.h
    src/utils/sort2/QuickSelect.h

    # -- F172~F177: Wave 25 工具模块 --
    src/utils/wiener/WienerFilter.h
    src/utils/circularbuffer2/CircularPowerBuffer.h
    src/utils/golomb/GolombCoder.h
    src/utils/peakhold/PeakHoldDetector.h
    src/utils/lomb/LombPeriodogram.h
    src/utils/bintree/BinaryIntervalTree.h

    # -- F178~F183: Wave 31 工具模块 --
    src/utils/movingaverage2/SynchronizedMovingAverage.h
    src/utils/histogram3/StreamingHistogram.h
    src/utils/qr/QrDecomposition.h
    src/utils/kalman2/ExtendedKalmanFilter.h
    src/utils/tfidf/TfidfVectorizer.h
    src/utils/pagerank/PageRank.h

    # -- F184~F191: Wave 32 工具模块(linter) --
    src/utils/bitmap/BitmapIndex.h
    src/utils/delta/DeltaEncoder.h
    src/utils/dwt2/DaubechiesWavelet.h
    src/utils/huffman2/AdaptiveHuffman.h
    src/utils/hyperloglog/HyperLogLog.h
    # src/utils/lifting/LiftingScheme.h  # removed: duplicate class
    src/utils/lzw2/LzwCodec2.h
    src/utils/mergesort/MergeSorter.h

    # -- F192~F203: Wave 33 工具模块 --
    src/utils/whitening/SpectralWhitener.h
    src/utils/peaks2/MultiPeakDetector.h
    src/utils/pingpong/PingPongBuffer.h
    src/utils/twoway/TwoWayMap.h
    # DUPLICATE REMOVED: src/utils/rle2/RunLengthEncoder2.h
    # DUPLICATE REMOVED: src/utils/shannon2/HuffmanTreeBuilder.h
    src/utils/graph2/TopologicalSorter.h
    src/utils/graph2/UnionFind.h
    src/utils/segment2/SegmentTree.h
    # DUPLICATE REMOVED: src/utils/sort2/QuickSelect.h
    # DUPLICATE REMOVED: src/utils/sort2/RadixSorter.h

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

    src/utils/ratechange/RateOfChange.h
    src/utils/simplex/SimplexNoise.h
    src/utils/holtwinters/HoltWinters.h
    src/utils/multiproc/ProcessMonitor.h
    src/utils/slidingwindow3/SlidingWindowCounter.h
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
    src/utils/circqueue/CircularPriorityQueue.h
    src/utils/rollingrank/RollingRank.h
    src/utils/expmove/ExponentialMoving.h
    src/utils/polyreg/PolynomialRegression.h
    src/utils/gof/GoodnessOfFit.h
    src/utils/ringbuffer3/LockFreeRingBuffer.h
    src/utils/tokenizer/StreamTokenizer.h
    src/utils/suffixarray/SuffixArray.h
    src/utils/minhash/MinHashSignature.h
    src/utils/timerwheel/TimerWheel.h
    src/utils/slidingpercentile/SlidingPercentile.h
    src/utils/autopcor/AutoCorrelation.h
    src/utils/deadzone/DeadZoneFilter.h
    # src/utils/movingmax/MovingMinMax.h  # removed: duplicate class
    src/utils/ringhash/RollingHash.h
    src/utils/countmin/CountMinSketch.h
    src/utils/cuckoo/CuckooFilter.h
    src/utils/bloomfilter2/ScalableBloomFilter.h

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
    # src/utils/hmm/HiddenMarkovModel.h  # removed: duplicate class
    # src/utils/viterbi/ViterbiDecoder.h  # removed: duplicate of signal16/ViterbiDecoder
    src/utils/gmm/GaussianMixtureModel.h
    src/utils/covariance/CovarianceMatrix.h
    src/utils/percentile/PercentileCalculator.h
    # src/utils/scrambler/Scrambler.h  (duplicate of code23)

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
    # src/utils/moment/StatisticalMoments.h  # removed: duplicate class
    src/utils/ks/KsTestCalculator.h

    # -- F158~F171: Wave 24 工具模块 --
    src/utils/bootstrap/BootstrapResampler.h
    src/utils/lfu/LfuCache.h
    src/utils/trie/TrieMap.h
    src/utils/skiplist/SkipList.h
    src/utils/heap/GenHeap.h
    src/utils/chisq/ChiSquareTest.h
    src/utils/anova/AnovaTest.h
    src/utils/lrc/LrcChecksum.h
    src/utils/fcs/FcsChecker.h
    src/utils/snr/SnrEstimator.h
    src/utils/jaro/JaroWinkler.h
    # src/utils/convolutional/ConvolutionalCoder.h  # removed: duplicate of code4/ConvolutionalCoder
    src/utils/cosinedist/CosineDistance.h
    src/utils/edgedetect/EdgeDetector.h
    src/utils/mass/MassEstimator.h
    src/utils/lcs2/LcsSolver.h
    src/utils/reservoir/ReservoirSampler.h

    # -- F172~F185: Wave 25 工具模块 --
    # DUPLICATE REMOVED: src/utils/lzw2/LzwCodec2.h
    src/utils/runlength2/RunLengthEncoder.h
    # DUPLICATE REMOVED: src/utils/huffman2/AdaptiveHuffman.h
    # DUPLICATE REMOVED: src/utils/delta/DeltaEncoder.h
    # DUPLICATE REMOVED: src/utils/bitmap/BitmapIndex.h
    # DUPLICATE REMOVED: src/utils/mergesort/MergeSorter.h
    # DUPLICATE REMOVED: src/utils/hyperloglog/HyperLogLog.h
    src/utils/tdigest/TDigest.h
    # DUPLICATE REMOVED: src/utils/dwt2/DaubechiesWavelet.h
    # DUPLICATE REMOVED: src/utils/lifting/LiftingScheme.h
    src/utils/quantize2/MuLawEncoder.h
    src/utils/polar/PolarCode.h
    # turbo/TurboDecoder.h removed (duplicate of code6)

    # -- F186~F200: Wave 35 工具模块 --
    # DUPLICATE REMOVED: src/utils/rle2/RunLengthEncoder2.h
    # DUPLICATE REMOVED: src/utils/shannon2/HuffmanTreeBuilder.h
    # DUPLICATE REMOVED: src/utils/sort2/RadixSorter.h
    # DUPLICATE REMOVED: src/utils/sort2/QuickSelect.h
    # DUPLICATE REMOVED: src/utils/graph2/TopologicalSorter.h
    # DUPLICATE REMOVED: src/utils/graph2/UnionFind.h
    # DUPLICATE REMOVED: src/utils/segment2/SegmentTree.h
    src/utils/fenwick/FenwickTree.h
    src/utils/sparsae/SparseArray.h
    src/utils/roaring/RoaringBitmap.h
    src/utils/ringhash/RingHash.h
    src/utils/bloomfilter3/ScalableBloomFilter.h
    src/utils/merkle/MerkleTree.h
    src/utils/skiplist2/ConcurrentSkipList.h
    src/utils/lfu2/TinyLfuCache.h

    # -- F211~F215: Wave 35 工具模块 --
    src/utils/svd/SingularValueDecomposition.h
    src/utils/kalman3/UnscentedKalmanFilter.h
    src/utils/graph3/DijkstraShortestPath.h
    src/utils/heap2/FibonacciHeap.h
    src/utils/quadtree/QuadTree.h

    # -- F216~F225: Wave 36 工具模块(linter) --
    src/utils/btree/BPlusTree.h
    src/utils/convhull/ConvexHull.h
    src/utils/disjoint/DisjointSet.h
    src/utils/radixtree/RadixTree.h
    src/utils/rle3/BurrowsWheelerTransform.h
    src/utils/suffix/SuffixAutomaton.h
    src/utils/voronoi/VoronoiDiagram.h
    src/utils/wavelet2/HaarWavelet2D.h

    # -- F226~F231: Wave 39 DSP工具模块 --
    src/utils/bessel/BesselFilter.h
    src/utils/butterworth/ButterworthFilter.h
    src/utils/chebyshev/ChebyshevFilter.h
    # goertzel/GoertzelAlgorithm.h removed (duplicate of fft8)
    src/utils/welch/WelchSpectrum.h
    src/utils/yulewalker/YuleWalker.h

    # -- F226~F232: Wave 33 工具模块(续) --
    # DUPLICATE REMOVED: src/utils/autocorr/AutoCorrelator.h
    # DUPLICATE REMOVED: src/utils/crosscorr/CrossCorrelator.h
    # DUPLICATE REMOVED: src/utils/resample/DataResampler.h
    src/utils/wavwriter/WavWriter.h
    src/utils/wavreader/WavReader.h
    src/utils/hilbert/HilbertTransform.h
    src/utils/lomb2/LombScargle.h

    # -- Wave 38 --
    src/utils/catboost/CatBoostEstimator.h
    src/utils/lru3/ArcCache.h
    src/utils/quantile/QuantileSketch.h
    src/utils/rle4/LempelZivWelch.h
    src/utils/fft3/RealFFT.h
    src/utils/sort3/TimSort.h

    # -- Wave 39: 信号处理V2 --
    # src/utils/wiener2/WienerFilter2.h  # removed: duplicate of signal6/WienerFilter
    src/utils/kalman4/ParticleFilter.h
    src/utils/ekf/ExtendedKalman.h
    src/utils/complement/ComplementaryFilter.h
    src/utils/savitzky/SavitzkyGolay.h
    src/utils/medfilt/MedianFilter.h
    src/utils/notch/NotchFilter.h
    src/utils/peakdetect2/PeakDetector2.h
    # src/utils/align/SignalAligner.h  # removed: duplicate class
    # src/utils/window2/WindowFunction.h  # removed: duplicate of dsp6/WindowFunction
    src/utils/shorttime/ShortTimeEnergy.h
    src/utils/zcr2/ZeroCrossingRate2.h
    src/utils/spectral/SpectralFeatures.h
    src/utils/mfcc/MfccExtractor.h
    src/utils/pitch2/PitchDetector2.h
    src/utils/ode/RungeKuttaSolver.h
    src/utils/decomp/LuDecomposition.h
    src/utils/decomp2/CholeskyDecomposition.h
    src/utils/graph4/BellmanFord.h
    src/utils/graph5/FloydWarshall.h
    src/utils/mst/KruskalMST.h
    src/utils/mst2/PrimMST.h
    src/utils/stringmatch/KmpMatcher.h
    src/utils/stringmatch2/BoyerMooreMatcher.h
    src/utils/treap/Treap.h
    src/utils/splay/SplayTree.h
    src/utils/kdtree/KDTree.h
    src/utils/cic/CicFilter.h
    src/utils/optimize/SimulatedAnnealing.h
    src/utils/optimize2/GeneticOptimizer.h
    src/utils/rabin2/RabinKarpMulti.h
    src/utils/interp2/CubicInterpolator.h
    src/utils/phase/PhaseDetector.h
    src/utils/dft2/GoertzelSpectrum.h
    src/utils/qr2/HouseholderQR.h
    src/utils/eigen2/PowerIteration.h
    # src/utils/bfgs/BfgsOptimizer.h  # removed: duplicate class
    src/utils/dct/DctTransform.h
    src/utils/distance/DistanceMetric.h
    # src/utils/cluster2/DbScan.h  # removed: duplicate class

    # -- Wave 42 头文件 --
    src/utils/arima/ArimaModel.h
    src/utils/bayes3/NaiveBayesClassifier.h
    src/utils/bernstein/BernsteinPolynomial.h
    src/utils/cepstrum3/CepstralAnalysis.h
    src/utils/crossval/CrossValidator.h
    src/utils/emd/EarthMoverDistance.h
    src/utils/ensemble/BaggingEnsemble.h
    src/utils/entropy2/RenyiEntropy.h
    src/utils/glm/GeneralizedLinearModel.h
    src/utils/gmixture/GaussianMixture.h
    src/utils/hermite/HermiteInterpolation.h
    src/utils/pca2/KernelPca.h
    src/utils/peakhold2/PeakTracker.h
    src/utils/stacked/StackedEnsemble.h
    src/utils/taylor/TaylorSeries.h

    # ── Wave 44: 数值方法/ODE求根/积分 ──
    src/utils/conjugate/ConjugateGradient.h
    src/utils/lagrange/LagrangeInterpolation.h
    src/utils/bisect/BisectionSolver.h
    src/utils/romberg/RombergIntegration.h
    src/utils/adams/AdamsBashforth.h
    src/utils/predictor_corrector/PredictorCorrector.h
    src/utils/greville/GrevilleAbscissae.h
    src/utils/laguerre/LaguerreSolver.h
    src/utils/horner/HornerScheme.h
    src/utils/newtonroot/NewtonRaphson.h
    # src/utils/gaussquad/GaussLegendre.h  # removed: duplicate class
    src/utils/richardson/RichardsonExtrapolation.h
    src/utils/autodiff/numericalDifferentiator.h
    src/utils/rk2/Rk2Solver.h
    src/utils/collocation/CollocationSolver.h

    # ── Wave 45: 线性代数/特征值/逼近 ──
    src/utils/minimizer/GoldenSectionSearch.h
    src/utils/backtrack/BacktrackingLineSearch.h
    src/utils/divided_diff/DividedDifference.h
    src/utils/tridiag/TridiagonalSolver.h
    src/utils/schur/SchurDecomposition.h
    src/utils/rational/RationalInterpolation.h
    src/utils/chebyshev2/ChebyshevApproximation.h
    src/utils/bandmat/BandMatrixSolver.h
    # src/utils/sylvester/SylvesterSolver.h  # removed: duplicate class (superseded by matrix29)
    src/utils/pade/PadeApproximant.h
    src/utils/sturm/SturmSequence.h
    src/utils/qrstep/QrEigenSolver.h
    # src/utils/hessenberg/HessenbergReduction.h  # removed: duplicate class
    src/utils/butterfly/ButterflyOperation.h
    src/utils/circulant/CirculantSolver.h

    # ── Wave 46: 优化/特征值/迭代求解 ──
    src/utils/gram_schmidt/GramSchmidt.h
    src/utils/nelder_mead/NelderMead.h
    src/utils/davidson/DavidsonEigen.h
    src/utils/lanczos/LanczosEigen.h
    src/utils/powerquad/PowerMethodGeneralized.h
    src/utils/lobatto/LobattoIntegration.h
    src/utils/clenshaw_curtis/ClenshawCurtis.h
    src/utils/toeplitz/ToeplitzSolver.h
    src/utils/hankel/HankelSolver.h
    src/utils/vandermonde/VandermondeSolver.h
    src/utils/sparse_lu/SparseDirectSolver.h
    src/utils/ilu/IncompleteLU.h
    src/utils/sor/SorSolver.h
    src/utils/jacobi_iter/JacobiSolver.h
    src/utils/gauss_seidel/GaussSeidelSolver.h

    # ── Wave 47: 线性求解器/特征值/分解更新 ──
    src/utils/cgls/CglsSolver.h
    src/utils/bicgstab/BicgstabSolver.h
    src/utils/gmres/GmresSolver.h
    src/utils/minres/MinresSolver.h
    src/utils/symm_eigen/SymmetricEigenSolver.h
    src/utils/tridiag_eigen/TridiagEigenSolver.h
    src/utils/qz/QzDecomposition.h
    src/utils/svd2/SvdSolver.h
    src/utils/polar/PolarDecomposition.h
    # src/utils/qrupdate/QrUpdate.h  # removed: duplicate class
    src/utils/cholupdate/CholeskyUpdate.h
    src/utils/schur_update/SchurReorder.h
    src/utils/arnoldi/ArnoldiProcess.h
    src/utils/power2/InverseIteration.h
    src/utils/nystrom/NystromApproximation.h

    # ── Wave 48: 算法/字符串/多项式 ──
    src/utils/interp3/AkimaInterpolation.h
    src/utils/spectral2/SpectralClustering.h
    src/utils/procrustes/ProcrustesAnalysis.h
    src/utils/mahal/MahalanobisDistance.h
    src/utils/hungarian/HungarianAlgorithm.h
    src/utils/tsp/TspSolver.h
    src/utils/edmonds/EdmondsMatching.h
    src/utils/lcp_array/LcpArray.h
    src/utils/min_rotation/MinimalRotation.h
    src/utils/polynomial_root/PolynomialRoots.h
    src/utils/resultant/PolynomialResultant.h
    # src/utils/groebner/GroebnerBasis.h  # removed: duplicate class
    src/utils/grobner2/MultivariatePolynomial.h

    # ── Wave 49: 计算几何/空间索引 ──
    src/utils/bsp_tree/BspTree2d.h
    src/utils/interval_tree/IntervalTreeSearch.h
    src/utils/segment_tree/SegmentTreeRange.h
    src/utils/delaunay2/DelaunayFlip.h
    src/utils/voronoi2/VoronoiDiagram.h
    src/utils/convex_hull2/ChanConvexHull.h
    src/utils/polygon/PolygonTriangulation.h
    src/utils/line_sweep/LineSweepIntersect.h
    src/utils/geo_dist/GeodesicDistance.h
    src/utils/r_tree/RTreeSearch.h
    src/utils/kd_tree2/KdTreeBalancer.h
    src/utils/quadtree2/QuadtreeRegion.h
    src/utils/octree/OctreeVolume.h
    src/utils/point_location/PointLocation.h
    src/utils/nearest_neighbor2/ApproxNearestNeighbor.h

    # -- Wave 44-45: 新工具模块头文件 --
    src/utils/fft4/FastCorrelation.h
    src/utils/dwt3/SymmetricWavelet.h
    src/utils/lz77/Lz77Compressor.h
    src/utils/huffman3/CanonicalHuffman.h
    src/utils/segment3/IntervalTree.h
    src/utils/sort4/IntroSort.h
    src/utils/detrend2/LinearDetrend.h
    src/utils/normalize2/MinMaxScaler.h

    # -- Wave 46-47 --
    src/utils/wavelet3/DaubechiesD4.h
    src/utils/fft5/OverlapSaveFFT.h
    src/utils/filter3/NotchFilter.h
    src/utils/peakdetect2/ZeroCrossingDetector.h
    src/utils/crc64b2/Crc64Ecma.h
    src/utils/compress2/LzssCompressor.h
    src/utils/heap3/BinaryHeap.h
    src/utils/graph5/GraphColoring.h
    src/utils/align/NeedlemanWunsch.h
    src/utils/cluster3/KMedoids.h

    # ── Wave 54: 新增工具模块头文件 ──
    src/utils/cluster2/AgglomerativeClusterer.h
    src/utils/compress2/Lz4Compressor.h
    src/utils/detrend2/PolynomialDetrend.h
    src/utils/graph5/AStarSearch.h
    src/utils/interpolate2/BicubicInterpolator.h
    src/utils/lts2/LtsRegression.h
    src/utils/optim2/SimulatedAnnealing2.h
    src/utils/phase2/PhaseUnwrapper.h
    src/utils/priority2/BinomialHeap.h
    src/utils/savitzky2/SavitzkyGolay2D.h
    src/utils/segment2/GraphSegmenter.h
    src/utils/set2/DisjointSetForest.h
    src/utils/signal3/VoiceActivityDetector2.h
    src/utils/streaming3/StreamingQuantile2.h

    # ── Wave 55: 新增工具模块头文件 ──
    src/utils/morpho2/MorphologicalOps.h
    src/utils/fit3/SplineInterpolation.h
    src/utils/conv2/Convolution2D.h
    # src/utils/linalg4/LuDecomposition.h  # removed: conflicts with decomp/LuDecomposition (same class name)
    # src/utils/signal5/EnvelopeDetector.h  # removed: file does not exist
    src/utils/graph9/MinimumSpanningTree.h
    src/utils/code3/HammingCode.h
    src/utils/cluster3/GaussianMixture2.h

    # ── Wave 71: 节律/解码/编码/螺旋 ──
    src/utils/circadian/CircadianRhythm.h
    src/utils/viterbi2/ViterbiDecoder2.h
    src/utils/golomb2/GolombRiceCoder.h
    src/utils/spiral/SpiralMatrix.h

    # ── Wave 72: 组合/模拟/三角剖分 ──
    src/utils/nqueens/NQueensSolver.h
    src/utils/hanoi/HanoiTower.h
    src/utils/life/GameOfLife.h
    src/utils/delaunay3/DelaunayTriangulation2.h
    # -- Wave 73: linter未跟踪模块集成 --
    src/utils/aho2/CommentzWalter.h
    src/utils/btree4/BStarTree.h
    src/utils/cuckoo_hash/CuckooHashTable.h
    src/utils/dict/TrieMap.h
    src/utils/friedman/FriedmanTest.h
    # src/utils/golay/GolayCode.h  # removed: duplicate class
    src/utils/haar2/HaarWavelet2D.h
    src/utils/horner2/HornerScheme.h
    # src/utils/inv_perm/InversePermutation.h  # removed: duplicate class
    src/utils/levenshtein2/DamerauLevenshtein.h
    src/utils/lfsr/LfsrSequence.h
    src/utils/median3/MedianFilter3D.h
    src/utils/median_abs/MedianAbsoluteDeviation.h
    src/utils/min_cut/MinCut.h
    src/utils/poly_roots/PolynomialRoots.h
    src/utils/qr3/HouseholderQR.h
    src/utils/quadtree3/QuadtreeBalancer.h
    # src/utils/reed_solomon2/ReedSolomon2.h  # removed: duplicate class (superseded by code29)
    src/utils/simon/SimonSpeckCipher.h
    src/utils/sturm2/SturmSequence.h
    src/utils/suffix2/SuffixArray2.h
    src/utils/tensor3/TensorDecomposition.h
    src/utils/treap2/ImplicitTreap.h
    src/utils/whitening2/ZcaWhitening.h

    # ── Wave 83: AES/NFA/符号求导/2D树状数组/Fisher精确检验 ──
    src/utils/aes2/AesCbc.h
    src/utils/automata2/NfaSimulator.h
    src/utils/derivative3/SymbolicDerivative.h
    src/utils/fenwick2/FenwickTree2D.h
    src/utils/fisher/FisherExactTest.h
    src/utils/heap6/BinaryHeap2.h
    src/utils/interleaver2/TurboInterleaver.h
    src/utils/tpsa/TpsaEngine.h

    # ── Wave 84: 哈希函数集合(header-only) ──
    src/utils/blake3/Blake3Hash.h
    src/utils/farmhash/FarmHash.h
    src/utils/keccak/KeccakHash.h
    src/utils/metro/MetroHash.h
    src/utils/murmur64/MurmurHash64.h
    src/utils/polyhash/PolynomialHash.h
    src/utils/siphash/SipHash.h
    src/utils/spooky/SpookyHash.h
    src/utils/wyhash/WyHash.h
    src/utils/xxhash/XxHash.h

    # ── Wave 84: SHA-3/SHA-512 ──
    src/utils/sha3/Sha3Hash.h
    src/utils/sha512/Sha512Hash.h

    # ── Wave 89: Cauchy分布/原子环形缓冲/MCMC采样/CatmullRom样条 ──
    src/utils/cauchy/CauchyDistribution.h
    src/utils/ring4/RingBufferAtomic.h
    src/utils/mcmc/MetropolisHastings.h
    src/utils/spline3/CatmullRomSpline.h

    # ── Wave 90: 子串搜索/L1正则化/矩阵乘法/SPDY压缩 ──
    src/utils/substring/SubstringSearch.h
    src/utils/l1/L1Regularization.h
    src/utils/matmul/MatrixMultiply.h
    src/utils/spdy/SpdyCompressor.h

    # ── Wave 85: 图算法/插值/压缩/线性代数 ──
    # src/utils/dijkstra/DijkstraShortestPath.h  # removed: conflicts with graph3/DijkstraShortestPath
    # src/utils/graph12/BellmanFord.h  # removed: conflicts with graph4/BellmanFord
    src/utils/hmm2/HiddenMarkovModel2.h
    # src/utils/interp5/AkimaInterpolation.h  # removed: conflicts with interp3/AkimaInterpolation
    src/utils/lzw3/LzwEncoder.h
    src/utils/qr4/GivensQRUpdate.h
    src/utils/rolling_hash/RollingHash3.h

    # ── Wave 86: 哈希模块 ──
    src/utils/echohash/EchoHash.h
    src/utils/md6/Md6Hash.h
    src/utils/jh/JhHash.h

    # ── Wave 91-98 headers ──
    src/utils/sgd/StochasticGradientDescent.h
    src/utils/dwt4/WaveletPacket.h
    src/utils/btree5/BTreeMap.h
    src/utils/gmm/GaussianMixture3.h
    src/utils/graph13/EulerianPath.h
    src/utils/lattice/LatticeReducer.h
    # suffix3/SuffixAutomaton.h removed (duplicate of suffix)
    # DUPLICATE REMOVED: src/utils/pca2/KernelPca.h
    src/utils/conv3/FftConvolution.h
    # src/utils/heap7/PairingHeap.h  # removed: duplicate of heap4/PairingHeap
    src/utils/regex2/RegexEngine.h
    src/utils/distance3/DynamicTimeWarping.h
    # src/utils/graph14/StronglyConnected.h  # removed: duplicate of graph7/StronglyConnected
    # src/utils/tree6/RedBlackTree2.h  # removed: duplicate of tree4/RedBlackTree2
    src/utils/interp6/TrigonometricInterp.h
    src/utils/segment4/LazySegmentTree.h
    src/utils/sort8/RadixSort.h
    src/utils/bloom7/CountingBloomFilter.h
    src/utils/graph15/TopologicalSort.h
    # src/utils/matrix5/SparseMatrixCSR.h  # removed: duplicate class
    src/utils/priority3/FibonacciPriorityQueue.h
    src/utils/kdtree3/KdTreeNearest.h
    src/utils/entropy4/ShannonCoding.h
    # src/utils/automata3/DfaMinimizer.h  # removed: duplicate of automata/DfaMinimizer
    src/utils/norm_test/NormalityTest.h
    src/utils/adaptive_fft/AdaptiveFFT.h
    src/utils/disjoint2/WeightedDisjointSet.h
    src/utils/huffman4/AdaptiveHuffman.h
    src/utils/poly_interp/PolynomialInterpolator.h
    src/utils/triangulate2/EarClippingTriangulator.h
    src/utils/wavelet4/WaveletDenoiser.h
    src/utils/cache2/LRUCache.h
    # src/utils/softmax/SoftmaxClassifier.h  # removed: GCC14 default member initializer bug
    src/utils/detector/EdgeDetector2.h
    # src/utils/fft8/GoertzelAlgorithm.h  # removed: duplicate class (superseded by fft33)
    src/utils/matrix5/SymmetricEigen.h
    src/utils/string5/LevenshteinAutomaton.h
    # shannon3/RenyiEntropy.h removed (duplicate of entropy2)
    src/utils/zipf/ZipfGenerator.h
    src/utils/pca3/IncrementalPca.h
    src/utils/segment_tree2/SegmentTreeLazy.h

    # ── Wave 105-106 headers ──
    src/utils/graph18/ArticulationPoint.h
    src/utils/detrend3/SavitzkyGolayFilter.h
    src/utils/heap8/SkewHeap.h
    src/utils/linalg6/SvdSolver.h
    src/utils/sort10/Introsort2.h
    src/utils/tree9/IntervalTree2.h
    src/utils/geo3/PolygonClipper.h
    src/utils/quantile3/OnlineQuantile.h

    # ── Wave 107-118 headers ──
    src/utils/dsp7/AllpassFilter.h
    src/utils/cluster5/GaussianNaiveBayes.h
    # src/utils/code6/TurboDecoder.h  # removed: duplicate class (superseded by code28)
    src/utils/hash2/CuckooFilter.h
    src/utils/interp8/BSplineInterp.h
    src/utils/matrix8/BandMatrix2.h
    # src/utils/graph19/GraphIsomorphism.h  # removed: duplicate class (superseded by graph40)
    src/utils/dsp9/ParametricEq.h
    src/utils/graph22/CommunityDetection.h
    src/utils/matrix11/HouseholderBidiag.h
    src/utils/interp9/CubicSpline2.h
    src/utils/signal10/WaveletTransform2.h
    src/utils/code8/LdpcDecoder2.h
    src/utils/cluster7/DBSCAN2.h
    src/utils/tree12/WeightedBalancedTree.h
    src/utils/poly4/LaguerrePoly.h
    src/utils/poly5/OrthogonalPoly.h
    src/utils/signal11/Resampler.h
    src/utils/string8/AhoCorasick2.h
    src/utils/tree13/ScapegoatTree2.h
    src/utils/dsp10/FirDesigner.h
    src/utils/graph23/MaxBipartiteMatch.h
    src/utils/loss6/FocalLoss2.h
    src/utils/matrix12/CompressedCol.h
    src/utils/cluster8/MeanShift.h
    src/utils/code9/ReedSolomon3.h
    src/utils/fft11/MovingDFT.h
    src/utils/graph24/VertexCover.h
    src/utils/matrix13/SchurComplement.h
    src/utils/optimize4/NewtonMethod.h
    src/utils/signal12/Autocorrelation.h
    src/utils/interp10/RationalInterp.h
    # Wave 119-130
    src/utils/cluster9/OpticsClustering.h
    src/utils/code10/BchCode.h
    src/utils/dsp11/MultiRateFilter.h
    src/utils/fft12/ConstantQTransform.h
    src/utils/graph25/SteinerTree.h
    src/utils/matrix14/StrassenMultiply.h
    src/utils/signal13/PhaseCorrelate.h
    src/utils/tree14/BPlusTree3.h
    src/utils/cluster10/GaussianProcess.h
    src/utils/cluster11/IsolationForest.h
    src/utils/code11/GolayCode2.h
    src/utils/code12/SpinalCode.h
    src/utils/dsp12/FilterBank2.h
    src/utils/dsp13/AdaptiveFilter2.h
    src/utils/fft13/DctTransform.h
    src/utils/fft14/HilbertTransform.h
    src/utils/graph26/MinimumCostFlow.h
    src/utils/graph27/ChinesePostman.h
    # src/utils/matrix15/ToeplitzSolver.h  # removed: duplicate of toeplitz/ToeplitzSolver
    src/utils/matrix16/KrylovSolver.h
    src/utils/signal14/Cepstrum.h
    src/utils/signal15/MusicAlgorithm.h
    src/utils/tree15/SegmentTreeBeats.h
    src/utils/tree16/WeightedUnionFind.h
    src/utils/cluster14/AgglomerativeCluster.h
    src/utils/code15/PolarCode2.h
    src/utils/dsp16/BiquadFilter.h
    src/utils/fft17/ShortTimeFFT.h
    src/utils/graph30/FlowNetwork2.h
    src/utils/matrix19/SparseLU.h
    src/utils/signal18/PitchDetector.h
    # src/utils/tree19/FenwickTree2D.h  # removed: duplicate of fenwick2/FenwickTree2D
    # Wave 125-138
    src/utils/cluster12/SpectralClustering3.h
    src/utils/code13/ReedMullerCode.h
    src/utils/dsp14/WaveShaper.h
    src/utils/fft15/SlidingDFT.h
    src/utils/graph28/DominatorTree.h
    src/utils/matrix17/SchurDecomp2.h
    src/utils/signal16/ViterbiDecoder.h
    # src/utils/tree17/IntervalTree2.h  # removed: duplicate of tree9/IntervalTree2
    src/utils/cluster13/BirchClustering.h
    src/utils/code14/ConvolutionalCode2.h
    src/utils/dsp15/DelayLine.h
    src/utils/fft16/PrunedFFT.h
    src/utils/graph29/EdgeColoring.h
    src/utils/matrix18/TridiagonalSolver2.h
    src/utils/signal17/BlindSourceSep.h
    # src/utils/tree18/PersistentTree2.h  # removed: duplicate class (superseded by tree34)
    src/utils/cluster15/MeanShift2.h
    # src/utils/cluster16/GaussianMixture4.h  # removed: duplicate class
    src/utils/code17/HammingCode2.h
    src/utils/dsp17/Oscillator.h
    src/utils/dsp18/Compressor.h
    src/utils/fft18/ZoomFFT.h
    src/utils/fft19/PolyphaseFilterbank.h
    src/utils/graph31/VertexColoring.h
    src/utils/graph32/MaxFlowPushRelabel.h
    src/utils/matrix20/SparseCholesky.h
    src/utils/matrix21/GeneralizedEigen.h
    src/utils/signal19/SpectralSubtract.h
    # src/utils/signal20/Beamformer.h  # removed: duplicate of dsp5/Beamformer
    src/utils/tree20/LinkCutTree.h
    src/utils/tree21/EulerTourTree.h
    src/utils/cluster17/SubspaceCluster.h
    src/utils/code18/GrayCode2.h
    src/utils/dsp19/NoiseGate.h
    src/utils/fft20/GoertzelBank2.h
    src/utils/graph33/MinCostMaxFlow.h
    src/utils/matrix22/QRDecomp2.h
    src/utils/signal21/HarmonicProduct.h
    src/utils/tree22/Treap2.h
    src/utils/cluster18/SpectralBicluster.h
    src/utils/code19/RsErasure.h
    src/utils/dsp20/StereoProcessor.h
    src/utils/fft21/MultiTaper.h
    src/utils/graph34/Matching2.h
    src/utils/matrix23/SymmetricEigen3.h
    src/utils/signal22/EnvelopeDetect.h
    src/utils/tree23/CartesianTree2.h

    # ── Wave 110: 亲和力/扰码器/采样率转换/快速Hartley/最大团/稀疏QR/谱白化/配对堆 ──
    src/utils/cluster22/AffinityCluster.h
    src/utils/code23/Scrambler.h
    src/utils/dsp24/SampleRateConv.h
    src/utils/fft25/FastHartley.h
    src/utils/graph38/MaxClique.h
    src/utils/matrix27/SparseQR.h
    src/utils/signal26/SpectralWhitening.h
    src/utils/tree27/PairingHeap2.h

    # Wave 111-190 headers
    src/utils/cluster23/HierarchicalDensity.h
    src/utils/code24/ConvolutionalInterleaver.h
    src/utils/dsp25/EQMatch.h
    src/utils/fft26/DCTFast.h
    src/utils/matrix28/SparseSOR.h
    src/utils/signal27/TransientDetect.h
    src/utils/tree28/LeftistTree.h
    src/utils/cluster24/PlanarCheck.h
    src/utils/code25/LdpcEncoder.h
    src/utils/dsp26/ChorusEffect.h
    src/utils/fft27/RecursiveDFT.h
    src/utils/graph40/StronglyConnected2.h
    src/utils/matrix24/BandedSolver.h
    src/utils/signal23/CepstrumLifter.h
    src/utils/tree25/BinomialQueue.h
    src/utils/cluster25/GaussianMixture5.h
    src/utils/code26/CrcAccelerator.h
    src/utils/dsp27/WavetableOsc.h
    src/utils/fft28/FastNumberTheory.h
    src/utils/graph41/GraphPartition.h
    src/utils/matrix29/SylvesterSolver.h
    src/utils/signal24/SpectralEstimator.h
    src/utils/tree26/FibonacciHeap2.h
    src/utils/cluster26/HierClassifier.h
    src/utils/code27/PolarDecoder.h
    src/utils/dsp28/DynamicCompressor.h
    src/utils/fft29/ChirpZTransform.h
    # src/utils/graph42/GraphIsomorphism.h  # removed: duplicate class (superseded by newer wave)
    src/utils/matrix30/GeneralizedEigen2.h
    src/utils/signal25/AudioFingerprint.h
    src/utils/tree24/ScapegoatTree3.h
    src/utils/cluster27/OpticsClustering2.h
    src/utils/code28/TurboDecoder.h
    src/utils/dsp29/MultibandEQ.h
    src/utils/fft30/DiscreteCosine3.h
    src/utils/graph43/VertexCover2.h
    src/utils/matrix31/SymmetricEigen4.h
    src/utils/signal28/VoiceActivityDetect.h
    src/utils/cluster28/SpectralCluster2.h
    src/utils/code29/ReedSolomon2.h
    src/utils/dsp30/Phaser.h
    src/utils/fft31/PrunedFFT2.h
    src/utils/graph44/DominatingSet.h
    src/utils/matrix32/SchurDecomp3.h
    src/utils/signal29/BeatDetector.h
    src/utils/tree30/AVLTree3.h
    src/utils/cluster29/DBSCAN4.h
    src/utils/code30/ConvolutionalCode3.h
    src/utils/dsp31/Flanger.h
    src/utils/fft32/ConstantQ2.h
    src/utils/graph45/EulerTour2.h
    src/utils/matrix33/SparseCholesky2.h
    src/utils/signal30/NoiseProfile.h
    src/utils/tree31/Treap3.h
    src/utils/cluster30/AgglomerativeCluster2.h
    src/utils/code31/PolarCode3.h
    src/utils/dsp32/Limiter.h
    src/utils/fft33/GoertzelAlgorithm.h
    src/utils/graph46/MaximumFlow2.h
    src/utils/matrix34/ToeplitzSolver2.h
    src/utils/signal31/Chromagram.h
    src/utils/tree32/SplayTree3.h
    src/utils/cluster31/BirchClustering2.h
    src/utils/code32/SpinalCode2.h
    src/utils/dsp33/NoiseGate2.h
    src/utils/fft34/PolyphaseFilterbank2.h
    src/utils/graph47/MinCostFlow2.h
    src/utils/matrix36/KrylovSolver2.h
    src/utils/signal35/OnsetDetector.h
    src/utils/cluster32/SubspaceCluster2.h
    src/utils/code33/GrayCode3.h
    src/utils/dsp34/Crossfader.h
    src/utils/fft35/SlidingDFT2.h
    src/utils/graph48/Matching3.h
    src/utils/matrix37/QRDecomp3.h
    src/utils/signal36/HarmonicProduct2.h
    src/utils/tree37/CartesianTree3.h
    src/utils/cluster33/GaussianMixture7.h
    src/utils/code34/HammingCode3.h
    src/utils/dsp35/EnvelopeShaper.h
    src/utils/fft36/ZoomFFT2.h
    src/utils/graph49/StronglyConnected3.h
    src/utils/matrix35/SymmetricEigen5.h
    src/utils/signal32/EnvelopeDetect2.h
    src/utils/tree33/SegmentTree3.h
    src/utils/cluster34/SpectralBicluster2.h
    src/utils/code35/ErasureCode2.h
    src/utils/dsp36/StereoProcessor2.h
    src/utils/fft37/MultiTaper2.h
    src/utils/graph50/FlowNetwork3.h
    src/utils/matrix39/GeneralizedEigen3.h
    src/utils/signal33/Beamformer2.h
    src/utils/tree34/PersistentTree2.h
    src/utils/cluster35/DBSCAN5.h
    src/utils/code36/TurboCode3.h
    src/utils/dsp37/Compressor2.h
    src/utils/fft38/DCTFast2.h
    src/utils/graph51/MaxClique2.h
    src/utils/matrix40/SparseBiCGSTAB.h
    src/utils/signal39/SpectralSubtract2.h
    src/utils/tree35/IntervalTree3.h
    src/utils/cluster36/KMeans4.h
    src/utils/code37/ConvolutionalInterleaver2.h
    src/utils/dsp38/SampleRateConv2.h
    src/utils/fft39/WalshHadamard2.h
    src/utils/graph52/EdgeColoring2.h
    src/utils/matrix41/SparseLU2.h
    src/utils/signal40/PitchTracker2.h
    src/utils/tree40/FenwickTree2D2.h
    src/utils/cluster37/GaussianMixture8.h
    # src/utils/code38/ReedSolomon3.h  # removed: conflicts with code9/ReedSolomon3
    src/utils/dsp39/DynamicEQ.h
    src/utils/fft40/PrunedFFT3.h
    src/utils/graph53/MinCut2.h
    src/utils/matrix42/SparseCholesky3.h
    src/utils/signal41/MelFilterbank2.h
    src/utils/tree41/SplayTree4.h
    src/utils/cluster38/OPTICS3.h
    src/utils/code39/ConvolutionalCode4.h
    src/utils/dsp40/Deesser.h
    src/utils/fft41/ConstantQ3.h
    # src/utils/graph54/ChinesePostman.h  # removed: conflicts with graph27/ChinesePostman
    src/utils/matrix43/SchurDecomp4.h
    src/utils/signal42/PhaseCorrelator2.h
    src/utils/tree42/RedBlackTree4.h
    src/utils/cluster39/BirchClustering3.h
    src/utils/code40/SpinalCode3.h
    src/utils/dsp41/Vocoder.h
    src/utils/fft42/GoertzelAlgorithm2.h
    src/utils/graph55/BipartiteMatch3.h
    src/utils/matrix44/SymmetricEigen6.h
    # src/utils/signal43/SpectralClustering3.h  # removed: conflicts with cluster12/SpectralClustering3
    src/utils/tree43/AVLTree4.h
    src/utils/cluster40/AgglomerativeCluster3.h
    src/utils/code41/PolarCode4.h
    src/utils/dsp42/GranularProcessor.h
    src/utils/fft43/ZoomFFT3.h
    src/utils/graph56/TravelingSalesman2.h
    src/utils/matrix45/SVD4.h
    src/utils/signal44/EnvelopeDetect3.h
    src/utils/tree44/Treap4.h
    src/utils/cluster41/HierarchicalCluster4.h
    src/utils/code42/HammingCode4.h
    src/utils/dsp43/ConvolutionReverb.h
    src/utils/fft44/ShortTimeFFT3.h
    src/utils/graph57/Bridges2.h
    src/utils/matrix46/GeneralizedEigen4.h
    src/utils/signal45/Chromagram2.h
    src/utils/tree45/BPlusTree5.h
    src/utils/cluster42/MeanShift4.h
    src/utils/code43/BchCode4.h
    src/utils/dsp44/PitchShift2.h
    src/utils/fft45/DiscreteCosine4.h
    src/utils/graph58/TopologicalSort2.h
    src/utils/matrix47/ToeplitzSolver3.h
    src/utils/signal46/FormantTracker2.h
    src/utils/tree46/SuffixTree2.h
    src/utils/cluster43/SpectralCluster4.h
    src/utils/code44/LdpcDecoder4.h
    src/utils/dsp45/SpectralGate.h
    src/utils/fft46/ChirpZ3.h
    src/utils/graph59/SteinerTree2.h
    src/utils/matrix48/SparseGMRES2.h
    src/utils/signal47/NoiseProfile2.h
    src/utils/tree47/SegmentTree4.h
    src/utils/cluster44/DBSCAN6.h
    src/utils/code45/TurboCode4.h
    src/utils/dsp46/StereoProcessor3.h
    src/utils/fft47/PolyphaseFilterbank3.h
    src/utils/graph60/DominatingSet2.h
    src/utils/matrix49/SparseBiCGSTAB2.h
    src/utils/signal48/SpectralWhitening2.h
    src/utils/tree48/CartesianTree4.h
    src/utils/cluster45/GaussianMixture9.h
    src/utils/code46/ErasureCode3.h
    src/utils/dsp47/MultibandCompressor.h
    src/utils/fft48/MultiTaper3.h
    src/utils/graph61/GraphColoring4.h
    src/utils/matrix50/KrylovSolver3.h
    src/utils/signal49/BeatDetector2.h
    src/utils/tree49/WeightBalancedTree3.h
    src/utils/cluster46/SubspaceCluster3.h
    src/utils/code47/ConvolutionalCode5.h
    src/utils/dsp48/TransientShaper.h
    src/utils/fft49/SlidingDFT3.h
    src/utils/graph62/IndependentSet2.h
    src/utils/matrix51/SparseCholesky4.h
    src/utils/signal50/HarmonicProduct3.h
    src/utils/tree50/FenwickTree3.h

    # ── Wave 51+: Extended Algorithm Modules (584 files) ──
    src/utils/cluster47/KMeans5.h
    src/utils/cluster48/HierarchicalCluster5.h
    src/utils/cluster49/DBSCAN7.h
    src/utils/cluster50/OPTICS4.h
    src/utils/cluster51/GaussianMixture10.h
    src/utils/cluster52/BirchClustering4.h
    src/utils/cluster53/SpectralClustering2.h
    src/utils/cluster54/MeanShift3.h
    src/utils/cluster55/DBSCAN8.h
    src/utils/cluster56/GaussianMixture11.h
    src/utils/cluster57/KMedoids4.h
    src/utils/cluster58/AffinityProp3.h
    src/utils/cluster59/IsolationForest3.h
    src/utils/cluster60/SpectralClustering3.h
    src/utils/cluster61/HierarchicalCluster6.h
    src/utils/cluster62/DBSCAN9.h
    src/utils/cluster63/KMeans6.h
    src/utils/cluster64/BirchClustering5.h
    src/utils/cluster65/OPTICS5.h
    src/utils/cluster66/GaussianMixture12.h
    src/utils/cluster67/KMeans7.h
    src/utils/cluster68/BirchClustering6.h
    src/utils/cluster69/DBSCAN10.h
    src/utils/cluster70/MeanShift4.h
    src/utils/cluster71/KMeans8.h
    src/utils/cluster72/DBSCAN11.h
    src/utils/cluster73/GaussianMixture13.h
    src/utils/cluster74/HierarchicalCluster7.h
    src/utils/cluster75/MeanShift5.h
    src/utils/cluster76/SpectralClustering4.h
    src/utils/cluster77/AffinityProp4.h
    src/utils/cluster78/IsolationForest4.h
    src/utils/cluster79/DBSCAN12.h
    src/utils/cluster80/GaussianMixture14.h
    src/utils/cluster81/HierarchicalCluster8.h
    src/utils/cluster82/KMeans9.h
    src/utils/cluster83/BirchClustering7.h
    src/utils/cluster84/MeanShift6.h
    src/utils/cluster85/SpectralClustering5.h
    src/utils/cluster86/IsolationForest5.h
    src/utils/cluster87/KMeans10.h
    src/utils/cluster88/HierarchicalCluster9.h
    src/utils/cluster89/SpectralClustering6.h
    src/utils/cluster90/MeanShift7.h
    src/utils/cluster91/BirchClustering8.h
    src/utils/cluster92/DBSCAN13.h
    src/utils/cluster93/OPTICS6.h
    src/utils/cluster94/GaussianMixture15.h
    src/utils/cluster95/IsolationForest6.h
    src/utils/cluster96/HierarchicalCluster10.h
    src/utils/cluster97/SpectralClustering7.h
    src/utils/cluster98/MeanShift8.h
    src/utils/cluster99/BirchClustering9.h
    src/utils/cluster100/DBSCAN14.h
    src/utils/cluster101/OPTICS7.h
    src/utils/cluster102/GaussianMixture16.h
    src/utils/cluster103/KMeans11.h
    src/utils/cluster104/HierarchicalCluster11.h
    src/utils/cluster105/SpectralClustering8.h
    src/utils/cluster106/MeanShift9.h
    src/utils/cluster107/BirchClustering10.h
    src/utils/cluster108/DBSCAN15.h
    src/utils/cluster109/OPTICS8.h
    src/utils/cluster110/GaussianMixture17.h
    src/utils/cluster111/KMeans12.h
    src/utils/cluster112/HierarchicalCluster12.h
    src/utils/cluster113/SpectralClustering9.h
    src/utils/cluster114/MeanShift10.h
    src/utils/cluster115/BirchClustering11.h
    src/utils/cluster116/DBSCAN16.h
    src/utils/cluster117/OPTICS9.h
    src/utils/cluster118/GaussianMixture18.h
    src/utils/cluster158/KMedoids13.h
    src/utils/code48/ReedSolomon4.h
    src/utils/code49/PolarCode5.h
    src/utils/code50/LdpcEncoder2.h
    src/utils/code51/GrayCode4.h
    src/utils/code52/HammingCode5.h
    src/utils/code53/SpinalCode4.h
    src/utils/code54/ConvolutionalCode2.h
    src/utils/code55/TurboCode2.h
    src/utils/code56/LDPCCode3.h
    src/utils/code57/ReedSolomon5.h
    src/utils/code58/CascadeCode2.h
    src/utils/code59/SpinalCode5.h
    src/utils/code60/PolarCode6.h
    src/utils/code61/TurboCode3.h
    src/utils/code62/BchCode5.h
    src/utils/code63/ConvolutionalCode3.h
    src/utils/code64/HammingCode6.h
    src/utils/code65/GrayCode5.h
    src/utils/code66/ReedSolomon6.h
    src/utils/code67/LDPCCode4.h
    src/utils/code68/SpinalCode6.h
    src/utils/code69/ConvolutionalCode4.h
    src/utils/code70/TurboCode4.h
    src/utils/code71/PolarCode7.h
    src/utils/code72/ReedSolomon7.h
    src/utils/code73/BchCode6.h
    src/utils/code74/ConvolutionalCode5.h
    src/utils/code75/LDPCCode5.h
    src/utils/code76/TurboCode5.h
    src/utils/code77/PolarCode8.h
    src/utils/code78/SpinalCode7.h
    src/utils/code79/CascadeCode3.h
    src/utils/code80/TurboCode6.h
    src/utils/code81/BchCode7.h
    src/utils/code82/ReedSolomon8.h
    src/utils/code83/LDPCCode5.h
    src/utils/code84/GrayCode6.h
    src/utils/code85/SpinalCode8.h
    src/utils/code86/PolarCode9.h
    src/utils/code87/CascadeCode4.h
    src/utils/code88/ReedSolomon9.h
    src/utils/code89/ConvCode5.h
    src/utils/code90/BchCode8.h
    src/utils/code91/PolarCode10.h
    src/utils/code92/SpinalCode9.h
    src/utils/code93/TurboCode7.h
    src/utils/code94/GrayCode7.h
    src/utils/code95/CascadeCode5.h
    src/utils/code96/ReedSolomon10.h
    src/utils/code97/ConvCode6.h
    src/utils/code98/BchCode9.h
    src/utils/code99/PolarCode11.h
    src/utils/code100/SpinalCode10.h
    src/utils/code101/TurboCode8.h
    src/utils/code102/GrayCode8.h
    src/utils/code103/CascadeCode6.h
    src/utils/code104/LDPCCode6.h
    src/utils/code105/ReedSolomon11.h
    src/utils/code106/BchCode10.h
    src/utils/code107/PolarCode12.h
    src/utils/code108/SpinalCode11.h
    src/utils/code109/TurboCode9.h
    src/utils/code110/GrayCode9.h
    src/utils/code111/CascadeCode7.h
    src/utils/code112/LDPCCode7.h
    src/utils/code113/ReedSolomon12.h
    src/utils/code114/BchCode11.h
    src/utils/code115/PolarCode13.h
    src/utils/code116/SpinalCode12.h
    src/utils/code117/TurboCode10.h
    src/utils/code118/GrayCode10.h
    src/utils/code119/CascadeCode8.h
    src/utils/code159/TurboCode11.h
    src/utils/dsp49/DynamicCompressor2.h
    src/utils/dsp50/SampleRateConv3.h
    src/utils/dsp51/DelayLine3.h
    src/utils/dsp52/NoiseGate3.h
    src/utils/dsp53/EQMatch2.h
    src/utils/dsp54/ChorusEffect2.h
    src/utils/dsp55/TransientShaper2.h
    src/utils/dsp56/MultibandCompress2.h
    src/utils/dsp57/Limiter3.h
    src/utils/dsp58/DeEsser3.h
    src/utils/dsp59/GateExpand3.h
    src/utils/dsp60/PhaseVocoder3.h
    src/utils/dsp61/StereoWidth3.h
    src/utils/dsp62/DynamicEQ3.h
    src/utils/dsp63/Compressor4.h
    src/utils/dsp64/Reverb3.h
    src/utils/dsp65/DelayLine4.h
    src/utils/dsp66/Phaser2.h
    src/utils/dsp67/Flanger2.h
    src/utils/dsp68/StereoEnhance2.h
    src/utils/dsp69/SampleRateConv4.h
    src/utils/dsp70/NoiseGate4.h
    src/utils/dsp71/ChorusEffect3.h
    src/utils/dsp72/StereoProcessor3.h
    src/utils/dsp73/DynamicCompressor3.h
    src/utils/dsp74/Limiter4.h
    src/utils/dsp75/TransientShaper3.h
    src/utils/dsp76/MultibandCompress3.h
    src/utils/dsp77/DeEsser4.h
    src/utils/dsp78/GateExpand4.h
    src/utils/dsp79/PhaseVocoder4.h
    src/utils/dsp80/StereoWidth4.h
    src/utils/dsp81/ChorusEffect4.h
    src/utils/dsp82/DynamicEQ4.h
    src/utils/dsp83/Flanger3.h
    src/utils/dsp84/Phaser3.h
    src/utils/dsp85/NoiseGate5.h
    src/utils/dsp86/Reverb4.h
    src/utils/dsp87/DelayLine5.h
    src/utils/dsp88/StereoEnhance3.h
    src/utils/dsp89/Compressor4.h
    src/utils/dsp90/Expander3.h
    src/utils/dsp91/Gate3.h
    src/utils/dsp92/Limiter4.h
    src/utils/dsp93/MultibandComp3.h
    src/utils/dsp94/StereoWidener3.h
    src/utils/dsp95/DynamicEQ5.h
    src/utils/dsp96/Phaser4.h
    src/utils/dsp97/Compressor5.h
    src/utils/dsp98/Expander4.h
    src/utils/dsp99/Gate4.h
    src/utils/dsp100/Limiter5.h
    src/utils/dsp101/MultibandComp4.h
    src/utils/dsp102/StereoWidener4.h
    src/utils/dsp103/DynamicEQ6.h
    src/utils/dsp104/Phaser5.h
    src/utils/dsp105/Compressor6.h
    src/utils/dsp106/Expander5.h
    src/utils/dsp107/Gate5.h
    src/utils/dsp108/Limiter6.h
    src/utils/dsp109/MultibandComp5.h
    src/utils/dsp110/StereoWidener5.h
    src/utils/dsp111/DynamicEQ7.h
    src/utils/dsp112/Phaser6.h
    src/utils/dsp113/Compressor7.h
    src/utils/dsp114/Expander6.h
    src/utils/dsp115/Gate6.h
    src/utils/dsp116/Limiter7.h
    src/utils/dsp117/MultibandComp6.h
    src/utils/dsp118/StereoWidener6.h
    src/utils/dsp119/DynamicEQ8.h
    src/utils/dsp120/Phaser7.h
    src/utils/dsp160/DynamicRangeCompressor.h
    src/utils/fft50/FastHartley2.h
    src/utils/fft51/WalshHadamard3.h
    src/utils/fft52/DCTFast3.h
    src/utils/fft53/GoertzelAlgorithm3.h
    src/utils/fft54/RecursiveDFT2.h
    src/utils/fft55/PrunedFFT4.h
    src/utils/fft56/ShortTimeFFT4.h
    src/utils/fft57/ChirpZ3.h
    src/utils/fft58/ZoomFFT3.h
    src/utils/fft59/NumberTheoretic3.h
    src/utils/fft60/Multitaper3.h
    src/utils/fft61/FastConv3.h
    src/utils/fft62/PolyPhase2.h
    src/utils/fft63/WignerVille2.h
    src/utils/fft64/HilbertHuang2.h
    src/utils/fft65/AdaptiveSTFT2.h
    src/utils/fft66/Cepstrum3.h
    src/utils/fft67/SlidingDFT3.h
    src/utils/fft68/ConstantQ3.h
    src/utils/fft69/MelSpectrogram3.h
    src/utils/fft70/GaborTransform2.h
    src/utils/fft71/WalshHadamard4.h
    src/utils/fft72/GoertzelAlgorithm4.h
    src/utils/fft73/DCTFast4.h
    src/utils/fft74/FastHartley3.h
    src/utils/fft75/RecursiveDFT3.h
    src/utils/fft76/ShortTimeFFT5.h
    src/utils/fft77/ChirpZ4.h
    src/utils/fft78/NumberTheoretic4.h
    src/utils/fft79/Multitaper4.h
    src/utils/fft80/ZoomFFT4.h
    src/utils/fft81/PolyPhase3.h
    src/utils/fft82/GoertzelAlgorithm5.h
    src/utils/fft83/ConstantQ4.h
    src/utils/fft84/MelSpectrogram4.h
    src/utils/fft85/GaborTransform3.h
    src/utils/fft86/SlidingDFT4.h
    src/utils/fft87/Cepstrum4.h
    src/utils/fft88/WalshHadamard5.h
    src/utils/fft89/FastConv4.h
    src/utils/fft90/ZoomFFT4.h
    src/utils/fft91/PrunedFFT3.h
    src/utils/fft92/RecursiveDFT3.h
    src/utils/fft93/DCTFast3.h
    src/utils/fft94/ChirpZTransform3.h
    src/utils/fft95/GoertzelAlgorithm6.h
    src/utils/fft96/ConstantQ5.h
    src/utils/fft97/MelSpectrogram5.h
    src/utils/fft98/ZoomFFT5.h
    src/utils/fft99/PrunedFFT4.h
    src/utils/fft100/RecursiveDFT4.h
    src/utils/fft101/DCTFast4.h
    src/utils/fft102/ChirpZTransform4.h
    src/utils/fft103/GoertzelAlgorithm7.h
    src/utils/fft104/ConstantQ6.h
    src/utils/fft105/MelSpectrogram6.h
    src/utils/fft106/ZoomFFT6.h
    src/utils/fft107/PrunedFFT5.h
    src/utils/fft108/RecursiveDFT5.h
    src/utils/fft109/DCTFast5.h
    src/utils/fft110/ChirpZTransform5.h
    src/utils/fft111/GoertzelAlgorithm8.h
    src/utils/fft112/ConstantQ7.h
    src/utils/fft113/MelSpectrogram7.h
    src/utils/fft114/ZoomFFT7.h
    src/utils/fft115/PrunedFFT6.h
    src/utils/fft116/RecursiveDFT6.h
    src/utils/fft117/DCTFast6.h
    src/utils/fft118/ChirpZTransform6.h
    src/utils/fft119/GoertzelAlgorithm9.h
    src/utils/fft120/ConstantQ8.h
    src/utils/fft121/MelSpectrogram8.h
    src/utils/fft161/ShortTimeFourier.h
    src/utils/graph63/MaxClique3.h
    src/utils/graph64/StronglyConnected4.h
    src/utils/graph65/Matching4.h
    src/utils/graph66/EulerTour3.h
    src/utils/graph67/VertexCover3.h
    src/utils/graph68/FlowNetwork4.h
    src/utils/graph69/BipartiteMatch4.h
    src/utils/graph70/ChinesePostman2.h
    src/utils/graph71/MinSpanningTree6.h
    src/utils/graph72/SteinerTree3.h
    src/utils/graph73/GraphColoring5.h
    src/utils/graph74/IndependentSet3.h
    src/utils/graph75/DominatingSet3.h
    src/utils/graph76/MaxClique4.h
    src/utils/graph77/GraphIsomorph3.h
    src/utils/graph78/Biconnected3.h
    src/utils/graph79/BridgeDetect3.h
    src/utils/graph80/FlowNetwork5.h
    src/utils/graph81/MinSpanningTree7.h
    src/utils/graph82/TopologicalSort3.h
    src/utils/graph83/ShortestPath4.h
    src/utils/graph84/BipartiteMatch5.h
    src/utils/graph85/EulerTour4.h
    src/utils/graph86/Matching5.h
    src/utils/graph87/VertexCover4.h
    src/utils/graph88/EdgeColoring3.h
    src/utils/graph89/SteinerTree4.h
    src/utils/graph90/ChinesePostman3.h
    src/utils/graph91/MinSpanningTree8.h
    src/utils/graph92/GraphColoring6.h
    src/utils/graph93/IndependentSet4.h
    src/utils/graph94/DominatingSet4.h
    src/utils/graph95/DominatingSet5.h
    src/utils/graph96/TopologicalSort4.h
    src/utils/graph97/FlowNetwork6.h
    src/utils/graph98/Biconnected4.h
    src/utils/graph99/BridgeDetect4.h
    src/utils/graph100/MaxClique5.h
    src/utils/graph101/ShortestPath5.h
    src/utils/graph102/GraphIsomorph4.h
    src/utils/graph103/GraphColoring7.h
    src/utils/graph104/IndependentSet5.h
    src/utils/graph105/MinSpanningTree8.h
    src/utils/graph106/StronglyConnected3.h
    src/utils/graph107/FlowNetwork7.h
    src/utils/graph108/EulerTour5.h
    src/utils/graph109/DominatingSet6.h
    src/utils/graph110/Biconnected5.h
    src/utils/graph111/GraphColoring8.h
    src/utils/graph112/IndependentSet6.h
    src/utils/graph113/MinSpanningTree9.h
    src/utils/graph114/StronglyConnected4.h
    src/utils/graph115/FlowNetwork8.h
    src/utils/graph116/EulerTour6.h
    src/utils/graph117/DominatingSet7.h
    src/utils/graph118/Biconnected6.h
    src/utils/graph119/GraphColoring9.h
    src/utils/graph120/IndependentSet7.h
    src/utils/graph121/MinSpanningTree9.h
    src/utils/graph122/StronglyConnected5.h
    src/utils/graph123/FlowNetwork9.h
    src/utils/graph124/EulerTour7.h
    src/utils/graph125/DominatingSet8.h
    src/utils/graph126/Biconnected7.h
    src/utils/graph127/GraphColoring10.h
    src/utils/graph128/IndependentSet8.h
    src/utils/graph129/MinSpanningTree11.h
    src/utils/graph130/StronglyConnected6.h
    src/utils/graph131/FlowNetwork10.h
    src/utils/graph132/EulerTour8.h
    src/utils/graph133/DominatingSet9.h
    src/utils/graph134/Biconnected8.h
    src/utils/graph174/BridgeDetector.h
    src/utils/matrix53/SparseQR2.h
    src/utils/matrix54/SymmetricEigen7.h
    src/utils/matrix55/TridiagonalSolver3.h
    src/utils/matrix56/SVD5.h
    src/utils/matrix57/SchurDecomp5.h
    src/utils/matrix58/GeneralizedEigen5.h
    src/utils/matrix59/SparseLU2.h
    src/utils/matrix60/GeneralizedEigen2.h
    src/utils/matrix61/TridiagonalEigen2.h
    src/utils/matrix62/BandSolver2.h
    src/utils/matrix63/ConditionNumber2.h
    src/utils/matrix64/SylvesterSolver2.h
    src/utils/matrix65/SchurComplement2.h
    src/utils/matrix66/SparseCholesky2.h
    src/utils/matrix67/KroneckerProduct2.h
    src/utils/matrix68/ToeplitzSolver2.h
    src/utils/matrix69/SparseQR3.h
    src/utils/matrix70/Hessenberg2.h
    src/utils/matrix71/SymmetricEigen8.h
    src/utils/matrix72/SVD6.h
    src/utils/matrix73/TridiagonalSolver4.h
    src/utils/matrix74/HouseholderQR2.h
    src/utils/matrix75/SparseBiCG2.h
    src/utils/matrix76/SymmetricEigen9.h
    src/utils/matrix77/SparseGMRES2.h
    src/utils/matrix78/TridiagonalEigen3.h
    src/utils/matrix79/GeneralizedEigen3.h
    src/utils/matrix80/SparseLU3.h
    src/utils/matrix81/BandSolver3.h
    src/utils/matrix82/SchurDecomp6.h
    src/utils/matrix83/ConditionNumber3.h
    src/utils/matrix84/SylvesterSolver3.h
    src/utils/matrix85/SparseCholesky3.h
    src/utils/matrix86/KroneckerProduct3.h
    src/utils/matrix87/ToeplitzSolver3.h
    src/utils/matrix88/Hessenberg3.h
    src/utils/matrix89/HouseholderQR3.h
    src/utils/matrix90/SparseQR4.h
    src/utils/matrix91/SVD7.h
    src/utils/matrix92/SymmetricEigen10.h
    src/utils/matrix93/SparseLU3.h
    src/utils/matrix94/SymmetricEigen11.h
    src/utils/matrix95/GeneralizedEigen3.h
    src/utils/matrix96/SchurDecomp4.h
    src/utils/matrix97/SparseCholesky4.h
    src/utils/matrix98/Hessenberg4.h
    src/utils/matrix99/ToeplitzSolver4.h
    src/utils/matrix100/KrylovSolver3.h
    src/utils/matrix101/SparseLU4.h
    src/utils/matrix102/SymmetricEigen12.h
    src/utils/matrix103/GeneralizedEigen4.h
    src/utils/matrix104/SchurDecomp5.h
    src/utils/matrix105/SparseCholesky5.h
    src/utils/matrix106/Hessenberg5.h
    src/utils/matrix107/ToeplitzSolver5.h
    src/utils/matrix108/KrylovSolver4.h
    src/utils/matrix109/SparseLU5.h
    src/utils/matrix110/SymmetricEigen13.h
    src/utils/matrix111/GeneralizedEigen5.h
    src/utils/matrix112/SchurDecomp6.h
    src/utils/matrix113/SparseCholesky6.h
    src/utils/matrix114/Hessenberg6.h
    src/utils/matrix115/ToeplitzSolver6.h
    src/utils/matrix116/KrylovSolver5.h
    src/utils/matrix117/SparseLU6.h
    src/utils/matrix118/SymmetricEigen14.h
    src/utils/matrix119/GeneralizedEigen6.h
    src/utils/matrix120/SchurDecomp7.h
    src/utils/matrix121/SparseCholesky7.h
    src/utils/matrix122/Hessenberg7.h
    src/utils/matrix123/ToeplitzSolver7.h
    src/utils/matrix124/KrylovSolver6.h
    src/utils/matrix164/BandEigenSolver.h
    src/utils/signal51/SpectralSubtract3.h
    src/utils/signal52/VoiceActivityDetect2.h
    src/utils/signal53/EnvelopeDetect4.h
    src/utils/signal54/OnsetDetector2.h
    src/utils/signal55/PitchDetector2.h
    src/utils/signal56/BlindSourceSep2.h
    src/utils/signal57/BeatDetector2.h
    src/utils/signal58/HarmonicProduct2.h
    src/utils/signal59/Resampler3.h
    src/utils/signal60/Chromagram2.h
    src/utils/signal61/MFCC2.h
    src/utils/signal62/SpectralCentroid2.h
    src/utils/signal63/ZeroCrossing2.h
    src/utils/signal64/SpectralFlatness2.h
    src/utils/signal65/SpectralRolloff2.h
    src/utils/signal66/SpectralContrast2.h
    src/utils/signal67/EnvelopeFollower3.h
    src/utils/signal68/SpectralFlux2.h
    src/utils/signal69/SpectralBrightness2.h
    src/utils/signal70/VoiceActivityDetect3.h
    src/utils/signal71/PitchTrack3.h
    src/utils/signal72/EnvelopeDetect5.h
    src/utils/signal73/BlindSourceSep3.h
    src/utils/signal74/SpectralSubtract4.h
    src/utils/signal75/SpectralLeakage2.h
    src/utils/signal76/PitchDetector3.h
    src/utils/signal77/OnsetDetector3.h
    src/utils/signal78/HarmonicProduct3.h
    src/utils/signal79/Chromagram3.h
    src/utils/signal80/Resampler4.h
    src/utils/signal81/MFCC3.h
    src/utils/signal82/ZeroCrossing3.h
    src/utils/signal83/BeatDetector3.h
    src/utils/signal84/SpectralFlux3.h
    src/utils/signal85/MFCC4.h
    src/utils/signal86/SpectralBrightness3.h
    src/utils/signal87/EnvelopeDetect6.h
    src/utils/signal88/PitchTrack4.h
    src/utils/signal89/Chromagram4.h
    src/utils/signal90/ZeroCrossing4.h
    src/utils/signal91/SpectralFlatness3.h
    src/utils/signal92/Tonality3.h
    src/utils/signal93/NoiseEstimate3.h
    src/utils/signal94/VoiceActivity3.h
    src/utils/signal95/TransientDetect3.h
    src/utils/signal96/Chromagram5.h
    src/utils/signal97/EnvelopeDetect7.h
    src/utils/signal98/PitchTrack5.h
    src/utils/signal99/SpectralFlatness4.h
    src/utils/signal100/Tonality4.h
    src/utils/signal101/NoiseEstimate4.h
    src/utils/signal102/VoiceActivity4.h
    src/utils/signal103/TransientDetect4.h
    src/utils/signal104/Chromagram6.h
    src/utils/signal105/EnvelopeDetect8.h
    src/utils/signal106/PitchTrack6.h
    src/utils/signal107/SpectralFlatness5.h
    src/utils/signal108/Tonality5.h
    src/utils/signal109/NoiseEstimate5.h
    src/utils/signal110/VoiceActivity5.h
    src/utils/signal111/TransientDetect5.h
    src/utils/signal112/Chromagram7.h
    src/utils/signal113/EnvelopeDetect9.h
    src/utils/signal114/PitchTrack7.h
    src/utils/signal115/SpectralFlatness6.h
    src/utils/signal116/Tonality6.h
    src/utils/signal117/NoiseEstimate6.h
    src/utils/signal118/VoiceActivity6.h
    src/utils/signal119/TransientDetect6.h
    src/utils/signal120/Chromagram8.h
    src/utils/signal121/EnvelopeDetect10.h
    src/utils/signal122/PitchTrack8.h
    src/utils/signal162/SpectralSubtraction.h
    src/utils/tree51/RedBlackTree5.h
    src/utils/tree52/BPlusTree6.h
    src/utils/tree53/AVLTree5.h
    src/utils/tree54/SplayTree5.h
    src/utils/tree55/Treap5.h
    src/utils/tree56/SegmentTree5.h
    src/utils/tree57/IntervalTree4.h
    src/utils/tree58/QuadTree3.h
    src/utils/tree59/PrioritySearchTree2.h
    src/utils/tree60/KDTree4.h
    src/utils/tree61/SuffixArray3.h
    src/utils/tree62/RangeTree3.h
    src/utils/tree63/WaveletTree3.h
    src/utils/tree64/SegmentTree6.h
    src/utils/tree65/FenwickTree3.h
    src/utils/tree66/AVLTree6.h
    src/utils/tree67/RedBlackTree6.h
    src/utils/tree68/Treap6.h
    src/utils/tree69/SplayTree6.h
    src/utils/tree70/BPlusTree7.h
    src/utils/tree71/WeightBalancedTree3.h
    src/utils/tree72/PersistentTree3.h
    src/utils/tree73/AVLTree7.h
    src/utils/tree74/RedBlackTree7.h
    src/utils/tree75/BPlusTree8.h
    src/utils/tree76/IntervalTree5.h
    src/utils/tree77/QuadTree4.h
    src/utils/tree78/KDTree5.h
    src/utils/tree79/PrioritySearchTree3.h
    src/utils/tree80/SuffixArray4.h
    src/utils/tree81/RangeTree4.h
    src/utils/tree82/WaveletTree4.h
    src/utils/tree83/RedBlackTree8.h
    src/utils/tree84/SplayTree7.h
    src/utils/tree85/Treap7.h
    src/utils/tree86/AVLTree8.h
    src/utils/tree87/BPlusTree9.h
    src/utils/tree88/IntervalTree6.h
    src/utils/tree89/QuadTree5.h
    src/utils/tree90/KDTree6.h
    src/utils/tree91/SuffixTree4.h
    src/utils/tree92/BinaryHeap8.h
    src/utils/tree93/FenwickTree6.h
    src/utils/tree94/AVLTree9.h
    src/utils/tree95/SplayTree8.h
    src/utils/tree96/RedBlackTree9.h
    src/utils/tree97/Treap8.h
    src/utils/tree98/BPlusTree10.h
    src/utils/tree99/SuffixTree5.h
    src/utils/tree100/BinaryHeap9.h
    src/utils/tree101/FenwickTree7.h
    src/utils/tree102/AVLTree10.h
    src/utils/tree103/SplayTree9.h
    src/utils/tree104/RedBlackTree10.h
    src/utils/tree105/Treap9.h
    src/utils/tree106/BPlusTree11.h
    src/utils/tree107/SuffixTree6.h
    src/utils/tree108/BinaryHeap10.h
    src/utils/tree109/FenwickTree8.h
    src/utils/tree110/AVLTree11.h
    src/utils/tree111/SplayTree10.h
    src/utils/tree112/RedBlackTree11.h
    src/utils/tree113/Treap10.h
    src/utils/tree114/BPlusTree12.h
    src/utils/tree115/SuffixTree7.h
    src/utils/tree116/BinaryHeap11.h
    src/utils/tree117/FenwickTree9.h
    src/utils/tree118/AVLTree12.h
    src/utils/tree119/SplayTree11.h
    src/utils/tree120/RedBlackTree12.h
    src/utils/tree121/Treap11.h
    src/utils/tree122/BPlusTree13.h
    src/utils/tree161/IntervalHeap.h
    src/utils/cluster159/DBSCAN8.h
    src/utils/code160/PolarCode4.h
    src/utils/dsp161/MultibandGate.h
    src/utils/fft162/ConstantQTransform.h
    src/utils/graph175/ChromaticIndex.h
    src/utils/matrix165/SingularValueSolver.h
    src/utils/signal163/WienerFilter.h
    src/utils/tree162/BStarTree.h
    src/utils/cluster160/MeanShift7.h
    src/utils/code161/GolayCode3.h
    src/utils/dsp162/StereoEnhancer.h
    src/utils/fft163/HaarWaveletTransform.h
    src/utils/graph176/FlowNetwork.h
    src/utils/matrix166/LeastSquaresSolver.h
    src/utils/signal164/NoiseProfiler.h
    src/utils/tree163/LeftistHeap.h
    src/utils/cluster161/GaussianMixture11.h
    src/utils/code162/BCHCode4.h
    src/utils/dsp163/VocalRemover.h
    src/utils/fft164/DiscreteCosineTransform.h
    src/utils/graph177/MinCostFlow.h
    src/utils/matrix167/ConditionEstimator.h
    src/utils/signal165/ChannelEqualizer.h
    src/utils/tree164/PairingHeap.h
    src/utils/cluster162/OPTICS5.h
    src/utils/code163/ReedMullerCode2.h
    src/utils/dsp164/PhaseVocoder.h
    src/utils/fft165/DiscreteSineTransform.h
    src/utils/graph178/BipartiteMatch.h
    src/utils/matrix168/EigenValueSolver.h
    src/utils/signal166/Beamformer2.h
    src/utils/tree165/FibonacciHeap.h
    src/utils/cluster163/Agglomerative6.h
    src/utils/code164/ConvolutionalCode6.h
    src/utils/dsp165/GranularSynthesis.h
    src/utils/fft166/ModifiedDCT.h
    src/utils/graph179/Hungarian.h
    src/utils/matrix169/SparseSolver.h
    src/utils/signal167/PitchDetector.h
    src/utils/tree166/VanEmdeBoas.h
    src/utils/cluster164/BirchClustering5.h
    src/utils/code165/ErasureCode4.h
    src/utils/dsp166/Deconvolver.h
    src/utils/fft167/WalshHadamard.h
    src/utils/graph180/StoerWagner.h
    src/utils/matrix170/CholeskySolver.h
    src/utils/signal168/HarmonicAnalyzer.h
    src/utils/tree167/ScapegoatTree4.h
    src/utils/cluster165/FuzzyCMeans6.h
    src/utils/code166/TrellisCode.h
    src/utils/dsp167/ConvolutionReverb.h
    src/utils/fft168/NumberTheoreticTransform.h
    src/utils/graph181/EulerTour.h
    src/utils/matrix171/LUDecomposition.h
    src/utils/signal169/SignalSynchronizer.h
    src/utils/tree168/WeightBalancedTree4.h
    src/utils/cluster166/SpectralCluster6.h
    src/utils/code167/GrayCode3.h
    src/utils/dsp168/BinauralProcessor.h
    src/utils/fft169/DiscreteHartleyTransform.h
    src/utils/graph182/StrongConnectivity.h
    src/utils/matrix172/QRDecomposition.h
    src/utils/signal170/AdaptiveLineEnhancer2.h
    src/utils/tree169/BPlusTree6.h
    src/utils/cluster167/KMeans15.h
    src/utils/code168/HuffmanCode3.h
    src/utils/dsp169/DynamicEQ.h
    src/utils/fft170/SlidingDFT4.h
    src/utils/graph183/TopologicalSort.h
    src/utils/matrix173/SVD2.h
    src/utils/signal171/Resampler.h
    src/utils/tree170/RedBlackTree8.h
    src/utils/cluster168/DBSCAN9.h
    src/utils/code169/ArithmeticCode.h
    src/utils/dsp170/SpectralGate.h
    src/utils/fft171/ZoomFFT.h
    src/utils/graph184/TransitiveClosure.h
    src/utils/matrix174/EigenVectorSolver.h
    src/utils/signal172/EnvelopeDetector2.h
    src/utils/tree171/AA_Tree3.h
    src/utils/cluster169/HierarchicalCluster6.h
    src/utils/code170/LempelZivWelch3.h
    src/utils/dsp171/MultibandCompressor2.h
    src/utils/fft172/PrimeFactorFFT.h
    src/utils/graph185/GraphColoring5.h
    src/utils/matrix175/TridiagonalSolver.h
    src/utils/signal173/SignalGenerator2.h
    src/utils/tree172/CartesianTree5.h
    src/utils/cluster170/SubspaceCluster4.h
    src/utils/code171/RunLengthCode3.h
    src/utils/dsp172/TransientShaper2.h
    src/utils/fft173/SplitRadixFFT.h
    src/utils/graph186/DominatingSet3.h
    src/utils/matrix176/BandedSolver.h
    src/utils/signal174/CorrelationAnalyzer.h
    src/utils/tree173/Treap6.h
    src/utils/cluster171/GaussianMixture12.h
    src/utils/code172/DeltaCode.h
    src/utils/dsp173/SpectralRepair.h
    src/utils/fft174/WinogradFFT.h
    src/utils/graph187/GraphMatching2.h
    src/utils/matrix177/IterativeSolver.h
    src/utils/signal175/FeatureExtractor.h
    src/utils/tree174/SplayTree6.h
    src/utils/cluster172/KMedoids14.h
    src/utils/code173/CaesarCode.h
    src/utils/dsp174/Limiter3.h
    src/utils/fft175/DistributedArithmetic4.h
    src/utils/graph188/MaximumClique.h
    src/utils/matrix178/SparseLU.h
    src/utils/signal176/WaveletDenoiser2.h
    src/utils/tree175/AvlTree3.h
    src/utils/cluster173/MeanShift8.h
    src/utils/code174/VigenereCode.h
    src/utils/dsp175/NoiseGate3.h
    src/utils/fft176/Goertzel4.h
    src/utils/graph189/IndependentSet3.h
    src/utils/matrix179/SchurDecomposition4.h
    src/utils/signal177/Periodogram.h
    src/utils/tree176/BTree4.h
    src/utils/cluster174/Agglomerative7.h
    src/utils/code175/SubstitutionCode.h
    src/utils/dsp176/Compressor2.h
    src/utils/fft177/ChirpZ4.h
    src/utils/graph190/Biconnected9.h
    src/utils/matrix180/SymmetricEigenSolver.h
    src/utils/signal178/WindowFunction.h
    src/utils/tree177/FenwickTree4.h
    src/utils/cluster175/BirchClustering6.h
    src/utils/code176/TranspositionCode.h
    src/utils/dsp177/Expander2.h
    src/utils/fft178/PrimeFactorFFT4.h
    src/utils/graph191/DominatingSet4.h
    src/utils/matrix181/GeneralizedEigenSolver.h
    src/utils/signal179/FilterDesign.h
    src/utils/tree178/SegmentTree4.h
    src/utils/cluster176/FuzzyCMeans7.h
    src/utils/code177/PlayfairCode.h
    src/utils/dsp178/Gate3.h
    src/utils/fft179/RaderFFT4.h
    src/utils/graph192/VertexCover4.h
    src/utils/matrix182/SVD3.h
    src/utils/signal180/Resampler2.h
    src/utils/tree179/IntervalTree4.h
    src/utils/cluster177/SpectralCluster7.h
    src/utils/code178/ADFGVXCode.h
    src/utils/dsp179/Limiter4.h
    src/utils/fft180/WinogradFFT4.h
    src/utils/graph193/EdgeColoring.h
    src/utils/matrix183/SparseQR.h
    src/utils/signal181/Correlator.h
    src/utils/tree180/SuffixTree4.h
    src/utils/cluster178/KMeans16.h
    src/utils/code179/RailFenceCode.h
    src/utils/dsp180/MultibandCompressor3.h
    src/utils/fft181/BruunFFT4.h
    src/utils/graph194/TravelingSalesman.h
    src/utils/matrix184/ConjugateGradient.h
    src/utils/signal182/WaveletDenoiser3.h
    src/utils/tree181/Rope4.h
    src/utils/cluster179/DBSCAN10.h
    src/utils/code180/AffineCode.h
    src/utils/dsp181/SpectralGate2.h
    src/utils/fft182/MixedRadixFFT4.h
    src/utils/graph195/ChinesePostman.h
    src/utils/matrix185/BiCGSTAB.h
    src/utils/signal183/AdaptiveFilter3.h
    src/utils/tree182/AA4.h
    src/utils/cluster180/GaussianMixture13.h
    src/utils/code181/BeaufortCode.h
    src/utils/dsp182/Deesser2.h
    src/utils/fft183/SplitRadixFFT4.h
    src/utils/graph196/GraphIsomorphism.h
    src/utils/matrix186/GMRES.h
    src/utils/signal184/SignalClassifier.h
    src/utils/tree183/Treap7.h
    src/utils/cluster181/KMedoids15.h
    src/utils/code182/AutokeyCode.h
    src/utils/dsp183/PhaseCorrelator.h
    src/utils/fft184/HexFFT4.h
    src/utils/graph197/MaximumFlow.h
    src/utils/matrix187/Cholesky3.h
    src/utils/signal185/PitchDetector3.h
    src/utils/tree184/WAVL4.h
    src/utils/cluster182/HierarchicalCluster7.h
    src/utils/code183/FourSquareCode.h
    src/utils/dsp184/Convolver2.h
    src/utils/fft185/DCT4.h
    src/utils/graph198/MinimumSpanningTree.h
    src/utils/matrix188/ThomasAlgorithm.h
    src/utils/signal186/EnvelopeDetector3.h
    src/utils/tree185/RedBlackTree9.h
    src/utils/cluster183/SubspaceCluster5.h
    src/utils/code184/BifidCode.h
    src/utils/dsp185/DynamicEQ2.h
    src/utils/fft186/DST4.h
    src/utils/graph199/StrongConnectivity3.h
    src/utils/matrix189/GaussSeidel.h
    src/utils/signal187/SignalSynchronizer2.h
    src/utils/tree186/ScapegoatTree5.h
    src/utils/cluster184/OPTICS6.h
    src/utils/code185/NihilistCode.h
    src/utils/dsp186/TransientShaper3.h
    src/utils/fft187/WHT4.h
    src/utils/graph200/EulerTour3.h
    src/utils/matrix190/SparseLU2.h
    src/utils/signal188/FeatureExtractor2.h
    src/utils/tree187/WeightBalancedTree5.h
    src/utils/cluster185/GaussianMixture14.h
    src/utils/code186/TrifidCode.h
    src/utils/dsp187/MultibandGate2.h
    src/utils/fft188/NumberTheoreticTransform2.h
    src/utils/graph201/TransitiveClosure2.h
    src/utils/matrix191/IterativeRefinement.h
    src/utils/signal189/SignalGenerator3.h
    src/utils/tree188/CartesianTree6.h
    src/utils/cluster186/KMeans17.h
    src/utils/code187/GronsfeldCode.h
    src/utils/dsp188/SpectralRepair2.h
    src/utils/fft189/DistributedArithmetic5.h
    src/utils/graph202/MaximumClique2.h
    src/utils/matrix192/BandSolver2.h
    src/utils/signal190/WaveletDenoiser4.h
    src/utils/tree189/AvlTree4.h
    src/utils/cluster187/MeanShift9.h
    src/utils/code188/Chaocipher.h
    src/utils/dsp189/Delay3.h
    src/utils/fft190/SlidingDFT5.h
    src/utils/graph203/GraphColoring6.h
    src/utils/matrix193/TridiagonalSolver2.h
    src/utils/signal191/Resampler3.h
    src/utils/tree190/BPlusTree7.h
    src/utils/cluster188/Agglomerative8.h
    src/utils/code189/BazeleriesCode.h
    src/utils/dsp190/Reverb2.h
    src/utils/fft191/ZoomFFT2.h
    src/utils/graph204/HamiltonianCycle.h
    src/utils/matrix194/EigenVectorSolver2.h
    src/utils/signal192/SignalClassifier2.h
    src/utils/tree191/SplayTree7.h
    src/utils/cluster189/BirchClustering7.h
    src/utils/code190/DoubleTranspositionCode.h
    src/utils/dsp191/Chorus2.h
    src/utils/fft192/Goertzel5.h
    src/utils/graph205/IndependentSet4.h
    src/utils/matrix195/SchurDecomposition5.h
    src/utils/signal193/Periodogram2.h
    src/utils/tree192/VanEmdeBoas4.h
    src/utils/cluster190/FuzzyCMeans8.h
    src/utils/code191/TwoSquareCode.h
    src/utils/dsp192/Flanger2.h
    src/utils/fft193/DHT4.h
    src/utils/graph206/DominatingSet5.h
    src/utils/matrix196/SymmetricEigenSolver2.h
    src/utils/signal194/WindowFunction2.h
    src/utils/tree193/FenwickTree5.h
    src/utils/cluster191/SpectralCluster8.h
    src/utils/code192/StraddlingCheckerboard.h
    src/utils/dsp193/Phaser2.h
    src/utils/fft194/ChirpZ5.h
    src/utils/graph207/Biconnected10.h
    src/utils/matrix197/GeneralizedEigenSolver2.h
    src/utils/signal195/FilterDesign2.h
    src/utils/tree194/SegmentTree5.h
    src/utils/cluster192/GaussianMixture15.h
    src/utils/code193/PolluxCode.h
    src/utils/dsp194/Compressor3.h
    src/utils/fft195/PrimeFactorFFT5.h
    src/utils/graph208/VertexCover5.h
    src/utils/matrix198/SVD4.h
    src/utils/signal196/Resampler4.h
    src/utils/tree195/IntervalTree5.h
    src/utils/cluster193/KMeans18.h
    src/utils/code194/MorbitCode.h
    src/utils/dsp195/Limiter5.h
    src/utils/fft196/WinogradFFT5.h
    src/utils/graph209/EdgeColoring2.h
    src/utils/matrix199/SparseQR2.h
    src/utils/signal197/Correlator2.h
    src/utils/tree196/SuffixTree5.h
    src/utils/cluster194/BirchClustering8.h
    src/utils/code195/FractionatedMorse.h
    src/utils/dsp196/MultibandCompressor4.h
    src/utils/fft197/ChirpZ6.h
    src/utils/graph210/TravelingSalesman2.h
    src/utils/matrix200/ConjugateGradient2.h
    src/utils/signal198/WaveletDenoiser5.h
    src/utils/tree197/Rope5.h
    src/utils/cluster195/DBSCAN11.h
    src/utils/code196/BaudotCode.h
    src/utils/dsp197/Expander3.h
    src/utils/fft198/HexFFT5.h
    src/utils/graph211/ChinesePostman2.h
    src/utils/matrix201/BiCGSTAB2.h
    src/utils/signal199/AdaptiveFilter4.h
    src/utils/tree198/AA5.h
    src/utils/cluster196/GaussianMixture16.h
    src/utils/code197/TapirCode.h
    src/utils/dsp198/Deesser3.h
    src/utils/fft199/MixedRadixFFT5.h
    src/utils/graph212/GraphIsomorphism2.h
    src/utils/matrix202/GMRES2.h
    src/utils/signal200/SignalClassifier3.h
    src/utils/tree199/Treap8.h
    src/utils/cluster197/KMedoids16.h
    src/utils/code198/RouteCipher.h
    src/utils/dsp199/PhaseCorrelator2.h
    src/utils/fft200/RaderFFT5.h
    src/utils/graph213/MaximumFlow2.h
    src/utils/matrix203/Cholesky4.h
    src/utils/signal201/PitchDetector4.h
    src/utils/tree200/WAVL5.h
    src/utils/cluster198/HierarchicalCluster8.h
    src/utils/code199/TwoSquareCode2.h
    src/utils/dsp200/Convolver3.h
    src/utils/fft201/DCT5.h
    src/utils/graph214/MinimumSpanningTree2.h
    src/utils/matrix204/ThomasAlgorithm2.h
    src/utils/signal202/FilterDesign3.h
    src/utils/tree201/RedBlackTree10.h
    src/utils/cluster199/SubspaceCluster6.h
    src/utils/code200/FoursquareCode2.h
    src/utils/dsp201/Gate4.h
    src/utils/fft202/DST5.h
    src/utils/graph215/StrongConnectivity4.h
    src/utils/matrix205/GaussSeidel2.h
    src/utils/signal203/SignalSynchronizer3.h
    src/utils/tree202/IntervalTree6.h
    src/utils/cluster200/OPTICS7.h
    src/utils/code201/DigrafidCode.h
    src/utils/dsp202/Limiter6.h
    src/utils/fft203/WinogradFFT6.h
    src/utils/graph216/EdgeColoring3.h
    src/utils/matrix206/SparseQR3.h
    src/utils/signal204/Correlator3.h
    src/utils/tree203/SuffixTree6.h
    src/utils/cluster201/SpectralCluster9.h
    src/utils/code202/SeriatedPlayfair.h
    src/utils/dsp203/SpectralGate3.h
    src/utils/fft204/BruunFFT5.h
    src/utils/graph217/TravelingSalesman3.h
    src/utils/matrix207/ConjugateGradient3.h
    src/utils/signal205/WaveletDenoiser6.h
    src/utils/tree204/Rope6.h
    src/utils/cluster202/KMeans19.h
    src/utils/code203/ColumnarTransposition.h
    src/utils/dsp204/Deesser4.h
    src/utils/fft205/SplitRadixFFT5.h
    src/utils/graph218/GraphIsomorphism3.h
    src/utils/matrix208/BiCGSTAB3.h
    src/utils/signal206/SignalClassifier4.h
    src/utils/tree205/Treap9.h
    src/utils/cluster203/Agglomerative9.h
    src/utils/code204/PolluxCode2.h
    src/utils/dsp205/NoiseGate4.h
    src/utils/fft206/Goertzel6.h
    src/utils/graph219/DominatingSet6.h
    src/utils/matrix209/EigenVectorSolver3.h
    src/utils/signal207/Periodogram3.h
    src/utils/tree206/AvlTree5.h
    src/utils/cluster204/BirchClustering9.h
    src/utils/code205/FoursquareCode3.h
    src/utils/dsp206/MultibandGate3.h
    src/utils/fft207/ChirpZ7.h
    src/utils/graph220/Biconnected11.h
    src/utils/matrix210/SchurDecomposition6.h
    src/utils/signal208/WindowFunction3.h
    src/utils/tree207/BPlusTree8.h
    src/utils/cluster205/FuzzyCMeans9.h
    src/utils/code206/StraddlingCheckerboard2.h
    src/utils/dsp207/Expander4.h
    src/utils/fft208/RaderFFT6.h
    src/utils/graph221/TravelingSalesman4.h
    src/utils/matrix211/ConjugateGradient4.h
    src/utils/signal209/FilterDesign4.h
    src/utils/tree208/ScapegoatTree6.h
    src/utils/cluster206/SpectralCluster10.h
    src/utils/code207/SeriatedPlayfair2.h
    src/utils/dsp208/SpectralGate4.h
    src/utils/fft209/BruunFFT6.h
    src/utils/graph222/ChinesePostman3.h
    src/utils/matrix212/BiCGSTAB4.h
    src/utils/signal210/WaveletDenoiser7.h
    src/utils/tree209/VanEmdeBoas5.h
    src/utils/cluster207/GaussianMixture17.h
    src/utils/code208/DoubleTranspositionCode2.h
    src/utils/dsp209/Chorus3.h
    src/utils/fft210/PrimeFactorFFT6.h
    src/utils/graph223/GraphIsomorphism4.h
    src/utils/matrix213/SymmetricEigenSolver3.h
    src/utils/signal211/Correlator4.h
    src/utils/tree210/RedBlackTree11.h
    src/utils/cluster208/KMedoids17.h
    src/utils/code209/FractionatedMorse2.h
    src/utils/dsp210/Flanger3.h
    src/utils/fft211/DCT6.h
    src/utils/graph224/HamiltonianCycle2.h
    src/utils/matrix214/ThomasAlgorithm3.h
    src/utils/signal212/Resampler5.h
    src/utils/tree211/SplayTree8.h
    src/utils/cluster209/HierarchicalCluster9.h
    src/utils/code210/BaudotCode2.h
    src/utils/dsp211/Phaser3.h
    src/utils/fft212/DST6.h
    src/utils/graph225/EulerTour4.h
    src/utils/matrix215/SparseLU3.h
    src/utils/signal213/SignalSynchronizer4.h
    src/utils/tree212/FenwickTree6.h
    src/utils/cluster210/SubspaceCluster7.h
    src/utils/code211/TapirCode2.h
    src/utils/dsp212/Delay4.h
    src/utils/fft213/WHT5.h
    src/utils/graph226/TransitiveClosure3.h
    src/utils/matrix216/IterativeRefinement2.h
    src/utils/signal214/FeatureExtractor3.h
    src/utils/tree213/CartesianTree7.h
    src/utils/cluster211/OPTICS8.h
    src/utils/code212/DigrafidCode2.h
    src/utils/dsp213/Reverb3.h
    src/utils/fft214/DistributedArithmetic6.h
    src/utils/graph227/MaximumClique3.h
    src/utils/matrix217/GaussSeidel3.h
    src/utils/signal215/SignalGenerator4.h
    src/utils/tree214/WeightBalancedTree6.h
    src/utils/cluster212/GaussianMixture18.h
    src/utils/code213/MorbitCode2.h
    src/utils/dsp214/Limiter7.h
    src/utils/fft215/ZoomFFT3.h
    src/utils/graph228/EdgeColoring4.h
    src/utils/matrix218/GMRES3.h
    src/utils/signal216/EnvelopeDetector4.h
    src/utils/tree215/Treap10.h
    src/utils/cluster213/KMeans20.h
    src/utils/code214/RouteCipher2.h
    src/utils/dsp215/Compressor4.h
    src/utils/fft216/SlidingDFT6.h
    src/utils/graph229/GraphColoring7.h
    src/utils/matrix219/TridiagonalSolver3.h
    src/utils/signal217/AdaptiveFilter5.h
    src/utils/tree216/Rope7.h
    src/utils/cluster214/DBSCAN12.h
    src/utils/code215/BazeleriesCode2.h
    src/utils/dsp216/MultibandCompressor5.h
    src/utils/fft217/NumberTheoreticTransform3.h
    src/utils/graph230/VertexCover6.h
    src/utils/matrix220/SVD5.h
    src/utils/signal218/PitchDetector5.h
    src/utils/tree217/BTree5.h
    src/utils/cluster215/GaussianMixture19.h
    src/utils/code216/BaudotCode3.h
    src/utils/dsp217/Expander5.h
    src/utils/fft218/HexFFT6.h
    src/utils/graph231/MaximumFlow3.h
    src/utils/matrix221/Cholesky5.h
    src/utils/signal219/SignalClassifier5.h
    src/utils/tree218/SegmentTree6.h
    src/utils/cluster216/KMeans21.h
    src/utils/code217/FoursquareCode4.h
    src/utils/dsp218/Deesser5.h
    src/utils/fft219/MixedRadixFFT6.h
    src/utils/graph232/ChinesePostman4.h
    src/utils/matrix222/BiCGSTAB5.h
    src/utils/signal220/WaveletDenoiser8.h
    src/utils/tree219/AA6.h
    src/utils/cluster217/Agglomerative10.h
    src/utils/code218/PolluxCode3.h
    src/utils/dsp219/NoiseGate5.h
    src/utils/fft220/SplitRadixFFT6.h
    src/utils/graph233/GraphIsomorphism5.h
    src/utils/matrix223/EigenVectorSolver4.h
    src/utils/signal221/Periodogram4.h
    src/utils/tree220/AvlTree6.h
    src/utils/cluster218/BirchClustering10.h
    src/utils/code219/StraddlingCheckerboard3.h
    src/utils/dsp220/MultibandGate4.h
    src/utils/fft221/Goertzel7.h
    src/utils/graph234/DominatingSet7.h
    src/utils/matrix224/SchurDecomposition7.h
    src/utils/signal222/WindowFunction4.h
    src/utils/tree221/BPlusTree9.h
    src/utils/cluster219/FuzzyCMeans10.h
    src/utils/code220/SeriatedPlayfair3.h
    src/utils/dsp221/Expander6.h
    src/utils/fft222/RaderFFT7.h
    src/utils/graph235/TravelingSalesman5.h
    src/utils/matrix225/ConjugateGradient5.h
    src/utils/signal223/FilterDesign5.h
    src/utils/tree222/ScapegoatTree7.h
    src/utils/cluster220/SpectralCluster11.h
    src/utils/code221/DoubleTranspositionCode3.h
    src/utils/dsp222/Chorus4.h
    src/utils/fft223/PrimeFactorFFT7.h
    src/utils/graph236/ChinesePostman5.h
    src/utils/matrix226/BiCGSTAB6.h
    src/utils/signal224/WaveletDenoiser9.h
    src/utils/tree223/VanEmdeBoas6.h
    src/utils/cluster221/GaussianMixture20.h
    src/utils/code222/FoursquareCode5.h
    src/utils/dsp223/SpectralGate5.h
    src/utils/fft224/BruunFFT7.h
    src/utils/graph237/GraphIsomorphism6.h
    src/utils/matrix227/SymmetricEigenSolver4.h
    src/utils/signal225/Correlator5.h
    src/utils/tree224/RedBlackTree12.h
    src/utils/cluster222/KMedoids18.h
    src/utils/code223/FractionatedMorse3.h
    src/utils/dsp224/Flanger4.h
    src/utils/fft225/DCT7.h
    src/utils/graph238/HamiltonianCycle3.h
    src/utils/matrix228/ThomasAlgorithm4.h
    src/utils/signal226/Resampler6.h
    src/utils/tree225/SplayTree9.h
    src/utils/cluster223/HierarchicalCluster10.h
    src/utils/code224/BaudotCode4.h
    src/utils/dsp225/Phaser4.h
    src/utils/fft226/DST7.h
    src/utils/graph239/EulerTour5.h
    src/utils/matrix229/SparseLU4.h
    src/utils/signal227/SignalSynchronizer5.h
    src/utils/tree226/FenwickTree7.h
    src/utils/cluster224/SubspaceCluster8.h
    src/utils/code225/TapirCode3.h
    src/utils/dsp226/Delay5.h
    src/utils/fft227/WHT6.h
    src/utils/graph240/TransitiveClosure4.h
    src/utils/matrix230/IterativeRefinement3.h
    src/utils/signal228/FeatureExtractor4.h
    src/utils/tree227/CartesianTree8.h
    src/utils/cluster225/OPTICS9.h
    src/utils/code226/DigrafidCode3.h
    src/utils/dsp227/Reverb4.h
    src/utils/fft228/DistributedArithmetic7.h
    src/utils/graph241/MaximumClique4.h
    src/utils/matrix231/GaussSeidel4.h
    src/utils/signal229/SignalGenerator5.h
    src/utils/tree228/WeightBalancedTree7.h
    src/utils/cluster226/GaussianMixture21.h
    src/utils/code227/MorbitCode3.h
    src/utils/dsp228/Limiter8.h
    src/utils/fft229/ZoomFFT4.h
    src/utils/graph242/EdgeColoring5.h
    src/utils/matrix232/GMRES4.h
    src/utils/signal230/EnvelopeDetector5.h
    src/utils/tree229/Treap11.h
    src/utils/cluster227/KMeans22.h
    src/utils/code228/RouteCipher3.h
    src/utils/dsp229/Compressor5.h
    src/utils/fft230/SlidingDFT7.h
    src/utils/graph243/GraphColoring8.h
    src/utils/matrix233/TridiagonalSolver4.h
    src/utils/signal231/AdaptiveFilter6.h
    src/utils/tree230/Rope8.h
    src/utils/cluster228/DBSCAN13.h
    src/utils/code229/BazeleriesCode3.h
    src/utils/dsp230/MultibandCompressor6.h
    src/utils/fft231/NumberTheoreticTransform4.h
    src/utils/graph244/VertexCover7.h
    src/utils/matrix234/SVD6.h
    src/utils/signal232/PitchDetector6.h
    src/utils/tree231/BTree6.h
    src/utils/cluster229/GaussianMixture22.h
    src/utils/code230/BaudotCode5.h
    src/utils/dsp231/Expander7.h
    src/utils/fft232/HexFFT7.h
    src/utils/graph245/MaximumFlow4.h
    src/utils/matrix235/Cholesky6.h
    src/utils/signal233/SignalClassifier6.h
    src/utils/tree232/SegmentTree7.h
    src/utils/cluster230/KMeans23.h
    src/utils/code231/FoursquareCode6.h
    src/utils/dsp232/Deesser6.h
    src/utils/fft233/MixedRadixFFT7.h
    src/utils/graph246/ChinesePostman6.h
    src/utils/matrix236/BiCGSTAB7.h
    src/utils/signal234/WaveletDenoiser10.h
    src/utils/tree233/AA7.h
    src/utils/cluster231/Agglomerative11.h
    src/utils/code232/PolluxCode4.h
    src/utils/dsp233/NoiseGate6.h
    src/utils/fft234/SplitRadixFFT7.h
    src/utils/graph247/GraphIsomorphism7.h
    src/utils/matrix237/EigenVectorSolver5.h
    src/utils/signal235/Periodogram5.h
    src/utils/tree234/AvlTree7.h
    src/utils/cluster232/BirchClustering11.h
    src/utils/code233/StraddlingCheckerboard4.h
    src/utils/dsp234/MultibandGate4.h
    src/utils/fft235/Goertzel8.h
    src/utils/graph248/DominatingSet8.h
    src/utils/matrix238/SchurDecomposition8.h
    src/utils/signal236/WindowFunction4.h
    src/utils/tree235/BPlusTree10.h
    src/utils/cluster233/FuzzyCMeans11.h
    src/utils/code234/SeriatedPlayfair4.h
    src/utils/dsp235/Expander8.h
    src/utils/fft236/RaderFFT8.h
    src/utils/graph249/TravelingSalesman6.h
    src/utils/matrix239/ConjugateGradient6.h
    src/utils/signal237/FilterDesign6.h
    src/utils/tree236/ScapegoatTree8.h
    src/utils/cluster234/SpectralCluster12.h
    src/utils/code235/DoubleTranspositionCode4.h
    src/utils/dsp236/Chorus5.h
    src/utils/fft237/PrimeFactorFFT8.h
    src/utils/graph250/ChinesePostman7.h
    src/utils/matrix240/BiCGSTAB8.h
    src/utils/signal238/WaveletDenoiser11.h
    src/utils/tree237/VanEmdeBoas7.h
    src/utils/cluster235/GaussianMixture23.h
    src/utils/code236/FoursquareCode7.h
    src/utils/dsp237/SpectralGate5.h
    src/utils/fft238/BruunFFT8.h
    src/utils/graph251/GraphIsomorphism8.h
    src/utils/matrix241/SymmetricEigenSolver5.h
    src/utils/signal239/Correlator6.h
    src/utils/tree238/RedBlackTree13.h
    src/utils/cluster236/KMedoids19.h
    src/utils/code237/FractionatedMorse4.h
    src/utils/dsp238/Flanger5.h
    src/utils/fft239/DCT8.h
    src/utils/graph252/HamiltonianCycle4.h
    src/utils/matrix242/ThomasAlgorithm4.h
    src/utils/signal240/Resampler7.h
    src/utils/tree239/SplayTree10.h
    src/utils/cluster237/HierarchicalCluster11.h
    src/utils/code238/BaudotCode6.h
    src/utils/dsp239/Phaser5.h
    src/utils/fft240/DST8.h
    src/utils/graph253/EulerTour6.h
    src/utils/matrix243/SparseLU4.h
    src/utils/signal241/SignalSynchronizer6.h
    src/utils/tree240/FenwickTree8.h
    src/utils/cluster238/SubspaceCluster9.h
    src/utils/code239/TapirCode4.h
    src/utils/dsp240/Delay6.h
    src/utils/fft241/WHT7.h
    src/utils/graph254/TransitiveClosure5.h
    src/utils/matrix244/IterativeRefinement4.h
    src/utils/signal242/FeatureExtractor5.h
    src/utils/tree241/CartesianTree9.h
    src/utils/cluster239/OPTICS10.h
    src/utils/code240/DigrafidCode4.h
    src/utils/dsp241/Reverb5.h
    src/utils/fft242/DistributedArithmetic8.h
    src/utils/graph255/MaximumClique5.h
    src/utils/matrix245/GaussSeidel5.h
    src/utils/signal243/SignalGenerator6.h
    src/utils/tree242/WeightBalancedTree8.h
    src/utils/cluster240/GaussianMixture24.h
    src/utils/code241/MorbitCode4.h
    src/utils/dsp242/Limiter9.h
    src/utils/fft243/ZoomFFT5.h
    src/utils/graph256/EdgeColoring6.h
    src/utils/matrix246/GMRES5.h
    src/utils/signal244/EnvelopeDetector6.h
    src/utils/tree243/Treap12.h

    # ── Wave 50: New Feature Modules (Headers) ──
    src/protocol/svd/SvdTypes.h
    src/protocol/svd/SvdParser.h
    src/protocol/svd/SvdRegisterTreeModel.h
    src/protocol/svd/SvdBitFieldWidget.h
    src/protocol/svd/SvdViewerWidget.h
    src/protocol/logic/LogicTypes.h
    src/protocol/logic/LogicSampler.h
    src/protocol/logic/ProtocolDecoder.h
    src/utils/firmware/FirmwareTypes.h
    src/utils/firmware/FirmwareDiffer.h
    src/utils/firmware/FirmwarePatchGenerator.h
    src/connection/bridge/BridgeTypes.h
    src/connection/bridge/PortBridge.h
    src/connection/bridge/BridgeConfigPanel.h
    src/chart/math/MathTypes.h
    src/chart/math/MathExpressionParser.h
    # src/utils/pid/PidController.h  # removed: duplicate (already in Wave 1 section)

    # ── F73: 状态机设计器 (Headers) ──
    src/utils/statemachine/SmTypes.h
    src/utils/statemachine/StateMachineDesigner.h
    src/utils/statemachine/StateMachineWidget.h

    # ── F74: 网络拓扑发现 (Headers) ──
    src/utils/network/NetworkTypes.h
    src/utils/network/NetworkScanner.h
    src/utils/network/NetworkTopologyWidget.h

    # ── F75: Eye Diagram Analyzer (Headers) ──
    src/chart/eye/EyeTypes.h
    src/chart/eye/EyeDiagramEngine.h
    src/chart/eye/EyeDiagramWidget.h

    # ── F77: Protocol Sequencer (Headers) ──
    src/protocol/sequencer/SequencerTypes.h
    src/protocol/sequencer/ProtocolSequencer.h
    src/protocol/sequencer/SequenceEditorWidget.h

    # ── F76: Waveform Math Engine (Headers) ──
    src/chart/math/WaveformMath.h

    # ── F77: Digital Filter Designer ──
    src/utils/filter_design/FilterTypes.h
    src/utils/filter_design/FilterDesigner.h
    src/utils/filter_design/FilterDesignerWidget.h

    # ── PID Tuner ──
    src/utils/pid/PidTunerTypes.h

    # ── GPS/NMEA 数据可视化 ──
    src/utils/gps/GpsTypes.h
    src/utils/gps/NmeaParser.h
    src/utils/gps/GpsTrackWidget.h

    # ── F82: Waveform Generator ──
    src/utils/wavegen/WaveGenTypes.h
    src/utils/wavegen/WaveformGenerator.h
    src/utils/wavegen/WaveformGeneratorWidget.h

    # ── F85: MQTT Client Engine ──
    src/connection/mqtt_client/MqttClientTypes.h
    src/connection/mqtt_client/MqttClientEngine.h
    src/connection/mqtt_client/MqttClientPanel.h

    # ── F88: Script Engine ──
    src/utils/scripting/ScriptTypes.h
    src/utils/scripting/ScriptEngine.h
    src/utils/scripting/ScriptEditorWidget.h
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

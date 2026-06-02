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
    SerialPort
    Charts
    Network
)

# 源文件
set(SOURCES
    src/main.cpp
    src/core/mainwindow/MainWindow.cpp
    src/core/mainwindow/MainWindowSignalConnect.cpp
    src/core/mainwindow/MainWindowPanelConnect.cpp
    src/core/connect/ConnectionController.cpp
    src/core/connect/ConnectionControllerReconnect.cpp
    src/core/connect/ConnectionControllerHandlers.cpp
    src/core/recording/RecordingController.cpp
    src/core/navigation/NavigationController.cpp
    src/core/navigation/NavigationControllerAnimations.cpp
    src/core/send/SendController.cpp
    src/core/settings/SettingsController.cpp
    src/core/toolbar/ToolbarController.cpp
    src/core/terminal/TerminalController.cpp
    src/core/panels/PanelManager.cpp
    src/core/background/BackgroundWidget.cpp
    src/core/background/BackgroundSettingsPopup.cpp
    src/core/widgets/AnimatedButton.cpp
    src/core/connect/ConnectionManager.cpp
    src/core/connect/ConnectionFactory.cpp
    src/core/theme/ThemeManager.cpp
    src/connection/serial_port/SerialConnection.cpp
    src/connection/serial_port/SerialConnectionError.cpp
    src/connection/network/TcpConnection.cpp
    src/connection/network/UdpConnection.cpp
    src/terminal/widget/TerminalWidget.cpp
    src/terminal/widget/TerminalWidgetPaint.cpp
    src/terminal/model/TerminalModel.cpp
    src/terminal/search/TerminalSearchBar.cpp
    src/terminal/types/DirectionFilter.cpp
    src/terminal/layout/TerminalLayoutManager.cpp
    src/terminal/selection/TerminalSelectionManager.cpp
    src/terminal/search/TerminalSearchManager.cpp
    src/terminal/search/TerminalSearchRenderer.cpp
    src/terminal/menu/TerminalContextMenuManager.cpp
    src/serial/config/SerialConfigPanel.cpp
    src/serial/config/SerialConfigPanelUI.cpp
    src/serial/commands/QuickCommandBar.cpp
    src/serial/data/BookmarkWidget.cpp
    src/serial/commands/TimedSender.cpp
    src/serial/commands/SendHistory.cpp
    src/serial/data/DataStatistics.cpp
    src/serial/port/SerialDriverDetector.cpp
    src/serial/port/PortWatcher.cpp
    src/protocol/parser/FrameParser.cpp
    src/protocol/parser/FrameParserHelpers.cpp
    src/protocol/parser/FrameParserStateHandlers.cpp
    src/protocol/view/ProtocolView.cpp
    src/protocol/editor/FrameVisualEditor.cpp
    src/protocol/editor/FrameVisualEditorUI.cpp
    src/protocol/hex/IntelHexParser.cpp
    src/protocol/bridge/JustFloatBridge.cpp
    src/protocol/bridge/FireWaterBridge.cpp
    src/protocol/bridge/ProtocolBridgeManager.cpp
    src/chart/widget/ChartWidget.cpp
    src/chart/model/ChannelConfig.cpp
    src/chart/model/ChartModel.cpp
    src/chart/scale/YAxisManager.cpp
    src/chart/overlay/CursorOverlay.cpp
    src/chart/zoom/ZoomController.cpp
    src/chart/fft/FftEngine.cpp
    src/chart/fft/FftWidget.cpp
    src/chart/stats/HistogramWidget.cpp
    src/chart/stats/ScatterWidget.cpp
    src/ota/manager/OtaManager.cpp
    src/ota/widget/OtaWidget.cpp
    src/ota/widget/OtaWidgetSlots.cpp
    src/ota/history/OtaHistoryModel.cpp
    src/ota/protocols/base/BaseTransfer.cpp
    src/ota/protocols/xmodem/XModemTransfer.cpp
    src/ota/protocols/xmodem/XModemTransferHandlers.cpp
    src/ota/protocols/ymodem/YModemTransfer.cpp
    src/ota/protocols/ymodem/YModemTransferHandlers.cpp
    src/ota/protocols/zmodem/ZModemTransfer.cpp
    src/ota/protocols/zmodem/ZModemTransferHandlers.cpp
    src/utils/settings/SettingsManager.cpp
    src/utils/export/DataExporter.cpp
    src/utils/export/DataExporterStreamed.cpp
    src/utils/export/DataExporterEdl.cpp
    src/utils/log/DataLogger.cpp
    src/utils/log/DataLoggerEdl.cpp
    src/protocol/parser/FrameDefinition.cpp
    src/core/settings/SessionManager.cpp

    # ── F1: 数据录制回放 ──
    src/core/recording/RecordingTimeline.cpp
    src/core/recording/PlaybackController.cpp
    src/core/recording/PlaybackWidget.cpp
    src/core/recording/RecordingMarker.cpp
    src/utils/log/RecordingFileFormat.cpp

    # ── F3: 多通道数据导出 ──
    src/utils/export/ChartExporter.cpp
    src/utils/export/ExportDialog.cpp

    # ── F2: 自定义协议引擎 ──
    src/protocol/schema/ProtocolSchema.cpp
    src/protocol/engine/ProtocolEngine.cpp
    src/protocol/schema/ProtocolTemplateLibrary.cpp
    src/protocol/editor/ProtocolSchemaEditor.cpp
    src/protocol/engine/ProtocolFieldMapper.cpp

    # ── F4: 终端增强 ──
    src/terminal/layout/TerminalSplitter.cpp
    src/terminal/layout/TerminalTabManager.cpp
    src/terminal/layout/TerminalDualView.cpp

    # ── F5: 仪表盘 ──
    src/dashboard/DashboardWidget.cpp
    src/dashboard/GaugeWidget.cpp
    src/dashboard/ProgressBarWidget.cpp
    src/dashboard/LedIndicatorWidget.cpp
    src/dashboard/NumericDisplayWidget.cpp
    src/dashboard/DashboardModel.cpp

    # ── F6: J-Link RTT ──
    src/rtt/JLinkRttConnection.cpp
    src/rtt/JLinkSdkLoader.cpp
    src/rtt/RttConfigPanel.cpp
    src/rtt/RttChannelManager.cpp

    # ── F7: 自动化触发 ──
    src/automation/TriggerEngine.cpp
    src/automation/TriggerAction.cpp
    src/automation/TriggerManager.cpp
    src/automation/TriggerListPanel.cpp

    # ── F8: 项目管理 ──
    src/core/settings/ProjectManager.cpp
    src/core/settings/ProjectWelcomeDialog.cpp

    # ── F9: 串口高级调试 ──
    src/serial/signals/SignalLineMonitor.cpp
    src/serial/signals/SignalLineWidget.cpp
    src/serial/data/TrafficMonitor.cpp
    src/serial/data/TrafficMonitorWidget.cpp

    # ── F10: 性能监控 ──
    src/utils/perf/PerformanceMonitor.cpp
    src/utils/perf/PerformanceOverlay.cpp

    # ── F11: 插件系统 ──
    src/plugin/PluginManager.cpp
    src/plugin/PluginApi.cpp
    src/plugin/PluginConfigPanel.cpp

    # ── F12: Bluetooth/BLE 调试 ──
    src/connection/ble/BleScanner.cpp
    src/connection/ble/BleConnection.cpp
    src/connection/ble/BleGattModel.cpp
    src/connection/ble/BleGattBrowser.cpp
    src/connection/ble/BleConfigPanel.cpp
    src/connection/ble/BluetoothAtConsole.cpp

    # ── F13: CAN/CAN-FD 总线 ──
    src/connection/can/CanConnection.cpp
    src/connection/can/CanFrameParser.cpp
    src/connection/can/CanBusMonitor.cpp
    src/connection/can/CanConfigPanel.cpp

    # ── F14: MQTT 客户端 ──
    src/connection/mqtt/MqttConnection.cpp
    src/connection/mqtt/MqttTopicModel.cpp
    src/connection/mqtt/MqttSubscriptionPanel.cpp
    src/connection/mqtt/MqttConfigPanel.cpp

    # ── F15: TCP/UDP 增强 ──
    src/connection/tcp/TcpServerConnection.cpp
    src/connection/tcp/TcpMultiConnectionManager.cpp
    src/connection/tcp/UdpMulticastConnection.cpp
    src/connection/tcp/TlsConnection.cpp
    src/connection/tcp/MultiConnectionPanel.cpp

    # ── F16: SPI/I2C 桥接 ──
    src/connection/spi_i2c/SpiConnection.cpp
    src/connection/spi_i2c/I2cConnection.cpp
    src/connection/spi_i2c/SpiI2cConfigPanel.cpp
    src/connection/spi_i2c/RegisterEditor.cpp

    # ── F17: WebSocket 客户端 ──
    src/connection/ws/WebSocketConnection.cpp
    src/connection/ws/WsConfigPanel.cpp

    # ── F18: Modbus 主站/从站 ──
    src/protocol/modbus/ModbusMaster.cpp
    src/protocol/modbus/ModbusSlave.cpp
    src/protocol/modbus/ModbusScanWidget.cpp
    src/protocol/modbus/ModbusConfigPanel.cpp

    # ── F19: Protobuf/FlatBuffers 解码 ──
    src/protocol/protobuf/ProtobufDecoder.cpp
    src/protocol/protobuf/FlatBuffersDecoder.cpp
    src/protocol/protobuf/SchemaViewer.cpp

    # ── F20: USB 调试 ──
    src/connection/usb/UsbConnection.cpp
    src/connection/usb/UsbDeviceDetector.cpp
    src/connection/usb/UsbDescriptorViewer.cpp
    src/connection/usb/UsbConfigPanel.cpp

    # ── F21: 正则终端过滤器 ──
    src/terminal/filter/TerminalFilter.cpp
    src/terminal/filter/TerminalFilterBar.cpp
    src/terminal/filter/TerminalHighlighter.cpp

    # ── F22: 校验和计算器 ──
    src/utils/checksum/ChecksumCalculator.cpp
    src/utils/checksum/ChecksumPanel.cpp

    # ── F23: ASCII/表格转换器 ──
    src/utils/converter/DataConverter.cpp
    src/utils/converter/ConverterPanel.cpp

    # ── F24: 时间戳分析器 ──
    src/utils/timestamp/TimestampAnalyzer.cpp
    src/utils/timestamp/TimestampPanel.cpp

    # ── F25: 数据包构建器 ──
    src/utils/packet/PacketBuilder.cpp
    src/utils/packet/PacketBuilderPanel.cpp

    # ── F26: 设备注册表 ──
    src/core/device/DeviceRegistry.cpp
    src/core/device/DeviceProfilePanel.cpp
)

set(HEADERS
    src/core/mainwindow/MainWindow.h
    src/core/connect/ConnectionController.h

    src/core/widgets/AnimatedButton.h
    src/core/recording/RecordingController.h
    src/core/navigation/NavigationController.h
    src/core/send/SendController.h
    src/core/settings/SettingsController.h
    src/core/toolbar/ToolbarController.h
    src/core/terminal/TerminalController.h
    src/core/panels/PanelManager.h
    src/core/background/BackgroundWidget.h
    src/core/background/BackgroundSettingsPopup.h
    src/core/connect/ConnectionManager.h
    src/core/connect/ConnectionFactory.h
    src/core/theme/ThemeManager.h
    src/core/widgets/ToastWidget.h
    src/core/navigation/NavIndicatorWidget.h
    src/core/theme/Constants.h
    src/core/settings/SessionManager.h
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

    # ── F14: MQTT 客户端 ──
    src/connection/mqtt/MqttConnection.h
    src/connection/mqtt/MqttTopicModel.h
    src/connection/mqtt/MqttSubscriptionPanel.h
    src/connection/mqtt/MqttConfigPanel.h

    # ── F15: TCP/UDP 增强 ──
    src/connection/tcp/TcpServerConnection.h
    src/connection/tcp/TcpMultiConnectionManager.h
    src/connection/tcp/UdpMulticastConnection.h
    src/connection/tcp/TlsConnection.h
    src/connection/tcp/MultiConnectionPanel.h

    # ── F16: SPI/I2C 桥接 ──
    src/connection/spi_i2c/SpiConnection.h
    src/connection/spi_i2c/I2cConnection.h
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

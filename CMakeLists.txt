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

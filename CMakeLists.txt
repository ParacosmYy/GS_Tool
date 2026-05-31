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
    src/core/MainWindow.cpp
    src/core/MainWindowSignalConnect.cpp
    src/core/ConnectionController.cpp
    src/core/RecordingController.cpp
    src/core/NavigationController.cpp
    src/core/SendController.cpp
    src/core/SettingsController.cpp
    src/core/ToolbarController.cpp
    src/core/TerminalController.cpp
    src/core/PanelManager.cpp
    src/core/BackgroundWidget.cpp
    src/core/BackgroundSettingsPopup.cpp
    src/core/ConnectionManager.cpp
    src/core/ConnectionFactory.cpp
    src/core/ThemeManager.cpp
    src/connection/SerialConnection.cpp
    src/connection/TcpConnection.cpp
    src/connection/UdpConnection.cpp
    src/terminal/TerminalWidget.cpp
    src/terminal/TerminalModel.cpp
    src/terminal/TerminalSearchBar.cpp
    src/terminal/DirectionFilter.cpp
    src/terminal/TerminalLayoutManager.cpp
    src/terminal/TerminalSelectionManager.cpp
    src/terminal/TerminalSearchManager.cpp
    src/serial/SerialConfigPanel.cpp
    src/serial/QuickCommandBar.cpp
    src/serial/TimedSender.cpp
    src/serial/SendHistory.cpp
    src/serial/DataStatistics.cpp
    src/serial/SerialDriverDetector.cpp
    src/serial/PortWatcher.cpp
    src/protocol/FrameParser.cpp
    src/protocol/FrameParserHelpers.cpp
    src/protocol/ProtocolView.cpp
    src/protocol/FrameVisualEditor.cpp
    src/protocol/IntelHexParser.cpp
    src/protocol/JustFloatBridge.cpp
    src/protocol/FireWaterBridge.cpp
    src/protocol/ProtocolBridgeManager.cpp
    src/chart/ChartWidget.cpp
    src/chart/ChannelConfig.cpp
    src/chart/ChartModel.cpp
    src/ota/OtaManager.cpp
    src/ota/OtaWidget.cpp
    src/ota/OtaHistoryModel.cpp
    src/ota/protocols/BaseTransfer.cpp
    src/ota/protocols/XModemTransfer.cpp
    src/ota/protocols/YModemTransfer.cpp
    src/ota/protocols/ZModemTransfer.cpp
    src/utils/SettingsManager.cpp
    src/utils/DataExporter.cpp
    src/utils/DataLogger.cpp
    src/protocol/FrameDefinition.cpp
    src/core/SessionManager.cpp
)

set(HEADERS
    src/core/MainWindow.h
    src/core/ConnectionController.h

    src/core/RecordingController.h
    src/core/NavigationController.h
    src/core/SendController.h
    src/core/SettingsController.h
    src/core/ToolbarController.h
    src/core/TerminalController.h
    src/core/PanelManager.h
    src/core/BackgroundWidget.h
    src/core/BackgroundSettingsPopup.h
    src/core/ConnectionManager.h
    src/core/ConnectionFactory.h
    src/core/ThemeManager.h
    src/core/Constants.h
    src/core/SessionManager.h
    src/connection/IConnection.h
    src/connection/SerialConnection.h
    src/connection/TcpConnection.h
    src/connection/UdpConnection.h
    src/terminal/TerminalWidget.h
    src/terminal/TerminalModel.h
    src/terminal/TerminalSearchBar.h
    src/terminal/DirectionFilter.h
    src/terminal/TerminalLayoutManager.h
    src/terminal/TerminalSelectionManager.h
    src/terminal/TerminalSearchManager.h
    src/terminal/TerminalTypes.h
    src/serial/SerialConfigPanel.h
    src/serial/QuickCommandBar.h
    src/serial/TimedSender.h
    src/serial/SendHistory.h
    src/serial/DataStatistics.h
    src/serial/SerialDriverDetector.h
    src/serial/PortWatcher.h
    src/protocol/FrameDefinition.h
    src/protocol/FrameParser.h
    src/protocol/ProtocolView.h
    src/protocol/FrameVisualEditor.h
    src/protocol/IntelHexParser.h
    src/protocol/IProtocolBridge.h
    src/protocol/JustFloatBridge.h
    src/protocol/FireWaterBridge.h
    src/protocol/ProtocolBridgeManager.h
    src/chart/ChartWidget.h
    src/chart/ChartColors.h
    src/chart/ChannelConfig.h
    src/chart/ChartModel.h
    src/ota/OtaManager.h
    src/ota/OtaWidget.h
    src/ota/OtaHistoryModel.h
    src/ota/protocols/BaseTransfer.h
    src/ota/protocols/XModemTransfer.h
    src/ota/protocols/YModemTransfer.h
    src/ota/protocols/ZModemTransfer.h
    src/utils/SettingsManager.h
    src/utils/DataExporter.h
    src/utils/DataLogger.h
    src/utils/ByteFormat.h
    src/utils/CRC.h
    src/utils/HexConverter.h
    src/utils/RingBuffer.h
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

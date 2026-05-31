cmake_minimum_required(VERSION 3.20)

project(EmbedDebug VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
    src/core/RecordingController.cpp
    src/core/NavigationController.cpp
    src/core/ConnectionManager.cpp
    src/core/ConnectionFactory.cpp
    src/core/ThemeManager.cpp
    src/connection/SerialConnection.cpp
    src/connection/TcpConnection.cpp
    src/connection/UdpConnection.cpp
    src/terminal/TerminalWidget.cpp
    src/terminal/TerminalModel.cpp
    src/terminal/TerminalSearchBar.cpp
    src/terminal/TerminalTypes.h
    src/serial/SerialConfigPanel.cpp
    src/serial/QuickCommandBar.cpp
    src/serial/TimedSender.cpp
    src/serial/SendHistory.cpp
    src/serial/DataStatistics.cpp
    src/protocol/FrameParser.cpp
    src/protocol/ProtocolView.cpp
    src/protocol/FrameVisualEditor.cpp
    src/protocol/IntelHexParser.cpp
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
    src/utils/HexConverter.h
    src/utils/RingBuffer.h
    src/utils/CRC.h
    src/protocol/FrameDefinition.h
)

set(HEADERS
    src/core/MainWindow.h
    src/core/RecordingController.h
    src/core/NavigationController.h
    src/core/ConnectionManager.h
    src/core/ConnectionFactory.h
    src/core/ThemeManager.h
    src/core/Constants.h
    src/connection/IConnection.h
    src/connection/SerialConnection.h
    src/connection/TcpConnection.h
    src/connection/UdpConnection.h
    src/terminal/TerminalWidget.h
    src/terminal/TerminalModel.h
    src/terminal/TerminalSearchBar.h
    src/terminal/TerminalTypes.h
    src/serial/SerialConfigPanel.h
    src/serial/QuickCommandBar.h
    src/serial/TimedSender.h
    src/serial/SendHistory.h
    src/serial/DataStatistics.h
    src/protocol/FrameDefinition.h
    src/protocol/FrameParser.h
    src/protocol/ProtocolView.h
    src/protocol/FrameVisualEditor.h
    src/protocol/IntelHexParser.h
    src/chart/ChartWidget.h
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

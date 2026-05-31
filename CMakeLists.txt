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
    src/core/ConnectionManager.cpp
    src/core/ThemeManager.cpp
    src/connection/SerialConnection.cpp
    src/terminal/TerminalWidget.cpp
    src/terminal/TerminalModel.cpp
    src/serial/SerialConfigPanel.cpp
    src/serial/QuickCommandBar.cpp
    src/serial/TimedSender.cpp
    src/utils/SettingsManager.cpp
    src/utils/HexConverter.h
    src/utils/RingBuffer.h
    src/utils/CRC.h
)

set(HEADERS
    src/core/MainWindow.h
    src/core/ConnectionManager.h
    src/core/ThemeManager.h
    src/core/Constants.h
    src/connection/IConnection.h
    src/connection/SerialConnection.h
    src/terminal/TerminalWidget.h
    src/terminal/TerminalModel.h
    src/serial/SerialConfigPanel.h
    src/serial/QuickCommandBar.h
    src/serial/TimedSender.h
    src/utils/SettingsManager.h
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

# 测试
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

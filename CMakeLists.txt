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
# 源文件清单集中维护，根 CMake 只保留构建入口与目标规则。
include(cmake/EmbedDebugSources.cmake)

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

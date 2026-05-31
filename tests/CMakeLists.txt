cmake_minimum_required(VERSION 3.20)

find_package(Qt6 REQUIRED COMPONENTS Test)

# ============================================================
# 协议桥单元测试可执行目标
# ============================================================

# 需要被测试的源文件(不含main.cpp)
set(BRIDGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/protocol/JustFloatBridge.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/FireWaterBridge.cpp
)

set(BRIDGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/protocol/IProtocolBridge.h
    ${CMAKE_SOURCE_DIR}/src/protocol/JustFloatBridge.h
    ${CMAKE_SOURCE_DIR}/src/protocol/FireWaterBridge.h
)

add_executable(test_protocol_bridges
    test_protocol_bridges.cpp
    ${BRIDGE_SOURCES}
    ${BRIDGE_HEADERS}
)

target_include_directories(test_protocol_bridges PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_protocol_bridges PRIVATE
    Qt6::Core
    Qt6::Test
)

# Windows平台特殊设置
if(WIN32)
    target_compile_definitions(test_protocol_bridges PRIVATE
        UNICODE
        _UNICODE
    )
endif()

# 注册为CTest测试
add_test(NAME ProtocolBridges COMMAND test_protocol_bridges)
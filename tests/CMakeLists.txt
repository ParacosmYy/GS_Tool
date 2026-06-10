cmake_minimum_required(VERSION 3.20)

find_package(Qt6 REQUIRED COMPONENTS Test)

# ============================================================
# 协议桥单元测试可执行目标
# ============================================================

# 需要被测试的源文件(不含main.cpp)
set(BRIDGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/protocol/bridge/JustFloatBridge.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/bridge/FireWaterBridge.cpp
)

set(BRIDGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/protocol/bridge/IProtocolBridge.h
    ${CMAKE_SOURCE_DIR}/src/protocol/bridge/JustFloatBridge.h
    ${CMAKE_SOURCE_DIR}/src/protocol/bridge/FireWaterBridge.h
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

# ============================================================
# 面板元数据描述结构测试
# ============================================================

add_executable(test_panel_descriptor
    test_panel_descriptor.cpp
)

target_include_directories(test_panel_descriptor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_panel_descriptor PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_panel_descriptor PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME PanelDescriptor COMMAND test_panel_descriptor)

# ============================================================
# 导航面板映射元数据测试
# ============================================================

add_executable(test_nav_panel_mapping
    test_nav_panel_mapping.cpp
)

target_include_directories(test_nav_panel_mapping PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_nav_panel_mapping PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_nav_panel_mapping PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME NavPanelMapping COMMAND test_nav_panel_mapping)

# ============================================================
# 图标导航栏真实图标测试
# ============================================================

add_executable(test_icon_nav_bar
    test_icon_nav_bar.cpp
    ${CMAKE_SOURCE_DIR}/src/core/navigation/IconNavBar.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/IconManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
    ${CMAKE_SOURCE_DIR}/resources/app.qrc
)

target_include_directories(test_icon_nav_bar PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(test_icon_nav_bar PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_icon_nav_bar PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME IconNavBar COMMAND test_icon_nav_bar)

# ============================================================
# 导航分类跳转测试
# ============================================================

add_executable(test_navigation_controller_category
    test_navigation_controller_category.cpp
    ${CMAKE_SOURCE_DIR}/src/core/navigation/NavigationController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/navigation/NavigationControllerAnimations.cpp
    ${CMAKE_SOURCE_DIR}/src/core/navigation/NavigationControllerQuery.cpp
    ${CMAKE_SOURCE_DIR}/src/core/navigation/NavigationControllerStatus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
)

target_include_directories(test_navigation_controller_category PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_navigation_controller_category PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_navigation_controller_category PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME NavigationControllerCategory COMMAND test_navigation_controller_category)

# ============================================================
# MainWindow布局状态计算测试
# ============================================================

add_executable(test_mainwindow_layout_state
    test_mainwindow_layout_state.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mainwindow/MainWindowLayoutState.cpp
)

target_include_directories(test_mainwindow_layout_state PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_mainwindow_layout_state PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_mainwindow_layout_state PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME MainWindowLayoutState COMMAND test_mainwindow_layout_state)

# ============================================================
# 命令面板键盘交互测试
# ============================================================

add_executable(test_command_palette
    test_command_palette.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/CommandPalette.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/CommandPaletteFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
)

target_include_directories(test_command_palette PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_command_palette PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_command_palette PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME CommandPalette COMMAND test_command_palette)

# ============================================================
# 终端清屏状态测试
# ============================================================

add_executable(test_terminal_widget_clear
    test_terminal_widget_clear.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidgetConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidgetEvents.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidgetSlots.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidgetSearch.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/widget/TerminalWidgetPaint.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModel.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModelQuery.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModelStats.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/types/DirectionFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/selection/TerminalSelectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/selection/TerminalSelectionManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBuilders.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBuild.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/menu/TerminalContextMenuManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
)

target_include_directories(test_terminal_widget_clear PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_widget_clear PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_widget_clear PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalWidgetClear COMMAND test_terminal_widget_clear)

# ============================================================
# 终端搜索管理器状态测试
# ============================================================

add_executable(test_terminal_search_manager
    test_terminal_search_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBuild.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBuilders.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/types/DirectionFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
)

target_include_directories(test_terminal_search_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_search_manager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_search_manager PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalSearchManager COMMAND test_terminal_search_manager)

# ============================================================
# 终端搜索栏交互测试
# ============================================================

add_executable(test_terminal_search_bar
    test_terminal_search_bar.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBar.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBarHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/search/TerminalSearchBarAnimation.cpp
)

target_include_directories(test_terminal_search_bar PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_search_bar PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_search_bar PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalSearchBar COMMAND test_terminal_search_bar)

# ============================================================
# 发送栏输入校验测试
# ============================================================

add_executable(test_send_controller_validation
    test_send_controller_validation.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendControllerSend.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendControllerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendHistoryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendHistoryManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/AnimatedButton.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/SmartAutoComplete.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistoryStats.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/TimedSender.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/TimedSenderStats.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModel.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModelQuery.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/model/TerminalModelStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerEdl.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerSession.cpp
)

target_include_directories(test_send_controller_validation PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_send_controller_validation PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_send_controller_validation PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SendControllerValidation COMMAND test_send_controller_validation)

# ============================================================
# 快捷指令栏交互测试
# ============================================================

add_executable(test_quick_command_bar
    test_quick_command_bar.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/QuickCommandBar.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/QuickCommandBarActions.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/QuickCommandBarDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/AnimatedButton.cpp
)

target_include_directories(test_quick_command_bar PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_quick_command_bar PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_quick_command_bar PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME QuickCommandBar COMMAND test_quick_command_bar)

# ============================================================
# 终端过滤栏交互测试
# ============================================================

add_executable(test_terminal_filter_bar
    test_terminal_filter_bar.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/filter/TerminalFilterBar.cpp
)

target_include_directories(test_terminal_filter_bar PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_filter_bar PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_filter_bar PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalFilterBar COMMAND test_terminal_filter_bar)

# ============================================================
# 终端导出路径规范化测试
# ============================================================

add_executable(test_terminal_export_options
    test_terminal_export_options.cpp
    ${CMAKE_SOURCE_DIR}/src/core/terminal/TerminalExportOptions.cpp
)

target_include_directories(test_terminal_export_options PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_export_options PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_export_options PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalExportOptions COMMAND test_terminal_export_options)

# ============================================================
# 数据导出错误反馈测试
# ============================================================

add_executable(test_data_exporter_errors
    test_data_exporter_errors.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterFormats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterStreamed.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterStreamedFormats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterUtils.cpp
)

target_include_directories(test_data_exporter_errors PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_data_exporter_errors PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_data_exporter_errors PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME DataExporterErrors COMMAND test_data_exporter_errors)

# ============================================================
# 发送历史补全数据测试
# ============================================================

add_executable(test_send_history
    test_send_history.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistoryStats.cpp
)

target_include_directories(test_send_history PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_send_history PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_send_history PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SendHistory COMMAND test_send_history)

# ============================================================
# 工作区状态完整性测试
# ============================================================

add_executable(test_workspace_manager
    test_workspace_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/workspace/WorkspaceManager.cpp
)

target_include_directories(test_workspace_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_workspace_manager PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_workspace_manager PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME WorkspaceManager COMMAND test_workspace_manager)

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
# 启动参数解析测试
# ============================================================

add_executable(test_startup_options
    test_startup_options.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mainwindow/StartupOptions.cpp
)

target_include_directories(test_startup_options PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_startup_options PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_startup_options PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME StartupOptions COMMAND test_startup_options)

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
    ${CMAKE_SOURCE_DIR}/src/connection/interface/IConnection.h
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
# 定时发送器测试
# ============================================================

add_executable(test_timed_sender
    test_timed_sender.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/TimedSender.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/TimedSenderStats.cpp
)

target_include_directories(test_timed_sender PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_timed_sender PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_timed_sender PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TimedSender COMMAND test_timed_sender)

# ============================================================
# 发送历史补全管理统计测试
# ============================================================

add_executable(test_send_history_manager
    test_send_history_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendHistoryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/send/SendHistoryManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/core/widgets/SmartAutoComplete.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/serial/commands/SendHistoryStats.cpp
)

target_include_directories(test_send_history_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_send_history_manager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_send_history_manager PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SendHistoryManager COMMAND test_send_history_manager)

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
# 终端过滤器核心测试
# ============================================================

add_executable(test_terminal_filter
    test_terminal_filter.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/filter/TerminalFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/filter/TerminalFilterApply.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/filter/TerminalFilterCompat.cpp
    ${CMAKE_SOURCE_DIR}/src/terminal/filter/TerminalFilterStats.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerApply.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerColor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme/ThemeManagerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
)

target_include_directories(test_terminal_filter PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_terminal_filter PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_terminal_filter PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME TerminalFilter COMMAND test_terminal_filter)

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
    ${CMAKE_SOURCE_DIR}/src/utils/export/DataExporterEdl.cpp
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
# 数据流录制器错误反馈测试
# ============================================================

add_executable(test_data_stream_recorder
    test_data_stream_recorder.cpp
    ${CMAKE_SOURCE_DIR}/src/core/recording/DataStreamRecorder.cpp
)

target_include_directories(test_data_stream_recorder PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_data_stream_recorder PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_data_stream_recorder PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME DataStreamRecorder COMMAND test_data_stream_recorder)

# ============================================================
# 数据日志会话交互测试
# ============================================================

add_executable(test_data_logger_session
    test_data_logger_session.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerEdl.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerSession.cpp
)

target_include_directories(test_data_logger_session PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_data_logger_session PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_data_logger_session PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME DataLoggerSession COMMAND test_data_logger_session)

# ============================================================
# 录制文件格式路径校验测试
# ============================================================

add_executable(test_recording_file_format
    test_recording_file_format.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/RecordingFileFormat.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/RecordingFileIndex.cpp
)

target_include_directories(test_recording_file_format PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_recording_file_format PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_recording_file_format PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME RecordingFileFormat COMMAND test_recording_file_format)

# ============================================================
# 图表导出错误反馈测试
# ============================================================

add_executable(test_chart_exporter_errors
    test_chart_exporter_errors.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ChartExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ChartExporterFormats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ChartExporterImage.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ChartExporterJson.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChartModel.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChartModelSlots.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChartModelStats.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChannelConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChannelConfigSet.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChannelConfigSetMethods.cpp
    ${CMAKE_SOURCE_DIR}/src/chart/model/ChannelConfigStyle.cpp
)

target_include_directories(test_chart_exporter_errors PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_chart_exporter_errors PRIVATE
    Qt6::Charts
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Test
    Qt6::Widgets
)

if(WIN32)
    target_compile_definitions(test_chart_exporter_errors PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME ChartExporterErrors COMMAND test_chart_exporter_errors)

# ============================================================
# 导出对话框输入校验测试
# ============================================================

add_executable(test_export_dialog_validation
    test_export_dialog_validation.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ExportDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/export/ExportDialogSlots.cpp
)

target_include_directories(test_export_dialog_validation PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_export_dialog_validation PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    Qt6::Widgets
)

if(WIN32)
    target_compile_definitions(test_export_dialog_validation PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME ExportDialogValidation COMMAND test_export_dialog_validation)

# ============================================================
# 录制控制器工具栏状态测试
# ============================================================

add_executable(test_recording_controller_actions
    test_recording_controller_actions.cpp
    ${CMAKE_SOURCE_DIR}/src/core/recording/RecordingController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/recording/RecordingControllerStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerEdl.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/DataLoggerSession.cpp
)

target_include_directories(test_recording_controller_actions PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_recording_controller_actions PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    Qt6::Widgets
)

if(WIN32)
    target_compile_definitions(test_recording_controller_actions PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME RecordingControllerActions COMMAND test_recording_controller_actions)

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

# ============================================================
# Serial Station ASCII文本协议测试
# ============================================================

add_executable(test_ascii_text_protocol
    serial_station/test_ascii_text_protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
)

target_include_directories(test_ascii_text_protocol PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_ascii_text_protocol PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_ascii_text_protocol PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME AsciiTextProtocol COMMAND test_ascii_text_protocol)

# ============================================================
# Serial Station JustFloat协议测试
# ============================================================

add_executable(test_just_float_protocol
    serial_station/test_just_float_protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
)

target_include_directories(test_just_float_protocol PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_just_float_protocol PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_just_float_protocol PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME JustFloatProtocol COMMAND test_just_float_protocol)

# ============================================================
# Serial Station协议注册表测试
# ============================================================

add_executable(test_serial_protocol_registry
    serial_station/test_serial_protocol_registry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_protocol_registry PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_protocol_registry PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_protocol_registry PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialProtocolRegistry COMMAND test_serial_protocol_registry)

# ============================================================
# Serial Station Custom MD协议测试
# ============================================================

add_executable(test_custom_md_protocol
    serial_station/test_custom_md_protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
)

target_include_directories(test_custom_md_protocol PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_custom_md_protocol PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_custom_md_protocol PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME CustomMdProtocol COMMAND test_custom_md_protocol)

# ============================================================
# Serial Station Modbus RTU协议测试
# ============================================================

add_executable(test_modbus_rtu_protocol
    serial_station/test_modbus_rtu_protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_modbus_rtu_protocol PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_modbus_rtu_protocol PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_modbus_rtu_protocol PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME ModbusRtuProtocol COMMAND test_modbus_rtu_protocol)

# ============================================================
# Serial Station会话状态测试
# ============================================================

add_executable(test_serial_session
    serial_station/test_serial_session.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
)

target_include_directories(test_serial_session PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_session PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_session PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialSession COMMAND test_serial_session)

# ============================================================
# Serial Station配置测试
# ============================================================

add_executable(test_serial_station_config
    serial_station/test_serial_station_config.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
)

target_include_directories(test_serial_station_config PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_config PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

add_test(NAME SerialStationConfig COMMAND test_serial_station_config)

# ============================================================
# Serial Station UART配置面板测试
# ============================================================

add_executable(test_serial_port_panel
    serial_station/test_serial_port_panel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialPortPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
)

target_include_directories(test_serial_port_panel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_port_panel PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_port_panel PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialPortPanel COMMAND test_serial_port_panel)

# ============================================================
# Serial Station命令面板测试
# ============================================================

add_executable(test_serial_command_panel
    serial_station/test_serial_command_panel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandHistoryModel.cpp
)

target_include_directories(test_serial_command_panel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_command_panel PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_command_panel PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialCommandPanel COMMAND test_serial_command_panel)

# ============================================================
# Serial Station协议选择面板测试
# ============================================================

add_executable(test_serial_protocol_panel
    serial_station/test_serial_protocol_panel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialProtocolPanel.cpp
)

target_include_directories(test_serial_protocol_panel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_protocol_panel PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_protocol_panel PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialProtocolPanel COMMAND test_serial_protocol_panel)

# ============================================================
# Serial Station发送编码器测试
# ============================================================

add_executable(test_serial_codec
    serial_station/test_serial_codec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
)

target_include_directories(test_serial_codec PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_codec PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_codec PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialCodec COMMAND test_serial_codec)

# ============================================================
# Serial Station Dispatcher测试
# ============================================================

add_executable(test_serial_dispatcher
    serial_station/test_serial_dispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
)

target_include_directories(test_serial_dispatcher PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_dispatcher PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_dispatcher PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialDispatcher COMMAND test_serial_dispatcher)

# ============================================================
# Serial Station日志服务测试
# ============================================================

add_executable(test_serial_log_service
    serial_station/test_serial_log_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
)

target_include_directories(test_serial_log_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_log_service PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_log_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialLogService COMMAND test_serial_log_service)

# ============================================================
# Serial Station导出服务测试
# ============================================================

add_executable(test_serial_export_service
    serial_station/test_serial_export_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
)

target_include_directories(test_serial_export_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_export_service PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_export_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialExportService COMMAND test_serial_export_service)

# ============================================================
# Serial Station回放服务测试
# ============================================================

add_executable(test_serial_replay_service
    serial_station/test_serial_replay_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialReplayService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
)

target_include_directories(test_serial_replay_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_replay_service PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_replay_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialReplayService COMMAND test_serial_replay_service)

# ============================================================
# Serial Station配置档案服务测试
# ============================================================

add_executable(test_serial_profile_service
    serial_station/test_serial_profile_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
)

target_include_directories(test_serial_profile_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_profile_service PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_profile_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialProfileService COMMAND test_serial_profile_service)

# ============================================================
# Serial Station配置档案索引服务测试
# ============================================================

add_executable(test_serial_profile_catalog_service
    serial_station/test_serial_profile_catalog_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileCatalogService.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
)

target_include_directories(test_serial_profile_catalog_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_profile_catalog_service PRIVATE
    Qt6::Core
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_profile_catalog_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialProfileCatalogService COMMAND test_serial_profile_catalog_service)

# ============================================================
# Serial Station UI回放预览闭环测试
# ============================================================

add_executable(test_serial_station_replay_flow
    serial_station/test_serial_station_replay_flow.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationWindowProfiles.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationController.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerExport.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerReplay.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialPortPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialProtocolPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandHistoryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialLogPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialStatusBar.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialPort.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialManager.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileCatalogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialReplayService.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_station_replay_flow PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_replay_flow PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_station_replay_flow PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialStationReplayFlow COMMAND test_serial_station_replay_flow)

# ============================================================
# Serial Station控制器日志服务接入测试
# ============================================================

add_executable(test_serial_station_controller_log_service
    serial_station/test_serial_station_controller_log_service.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationController.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerExport.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialPort.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialManager.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_station_controller_log_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_controller_log_service PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_station_controller_log_service PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialStationControllerLogService COMMAND test_serial_station_controller_log_service)

# ============================================================
# Serial Station日志导出闭环测试
# ============================================================

add_executable(test_serial_station_export_flow
    serial_station/test_serial_station_export_flow.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationController.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerExport.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialPort.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialManager.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_station_export_flow PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_export_flow PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_station_export_flow PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialStationExportFlow COMMAND test_serial_station_export_flow)

# ============================================================
# Serial Station控制器发送闭环测试
# ============================================================

add_executable(test_serial_station_controller
    serial_station/test_serial_station_controller.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationController.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerExport.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialPort.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialManager.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_station_controller PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_controller PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_station_controller PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialStationController COMMAND test_serial_station_controller)

# ============================================================
# Serial Station工作台壳层测试
# ============================================================

add_executable(test_serial_station_workbench
    serial_station/test_serial_station_workbench.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationWindowProfiles.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationController.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerExport.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationControllerReplay.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/SerialStationConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialPortPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialProtocolPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialCommandHistoryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialLogPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/ui/SerialStatusBar.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialPort.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialManager.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/core/SerialSession.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialExportService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialLogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileCatalogService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialProfileService.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/services/SerialReplayService.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerGroups.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/settings/SettingsManagerConvenience.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/SerialProtocolRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/custom_md/CustomMdProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/just_float/JustFloatProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.cpp
)

target_include_directories(test_serial_station_workbench PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_serial_station_workbench PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Test
)

if(WIN32)
    target_compile_definitions(test_serial_station_workbench PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME SerialStationWorkbench COMMAND test_serial_station_workbench)

# ============================================================
# Qt6兼容回归测试
# ============================================================

add_executable(test_qt6_compat_regressions
    test_qt6_compat_regressions.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/svd/SvdParser.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/svd/SvdParserStats.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/svd/SvdRegisterTreeModel.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/svd/SvdViewerWidgetStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/scripting/ScriptEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/scripting/ScriptEngineStats.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/l1/L1Regularization.cpp
    ${CMAKE_SOURCE_DIR}/src/connection/mqtt_client/MqttClientEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/connection/mqtt_client/MqttClientEnginePacket.cpp
    ${CMAKE_SOURCE_DIR}/src/connection/mqtt_client/MqttClientEngineStats.cpp
)

target_include_directories(test_qt6_compat_regressions PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_qt6_compat_regressions PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Qml
    Qt6::Test
    Qt6::Widgets
)

if(WIN32)
    target_compile_definitions(test_qt6_compat_regressions PRIVATE
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME Qt6CompatRegressions COMMAND test_qt6_compat_regressions)

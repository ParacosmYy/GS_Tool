/**
 * @file PanelManagerQuery.cpp
 * @brief PanelManager 面板映射表与面板列表
 *
 * 从 PanelManagerGetters.cpp 拆分而来，包含:
 *   - panelMappings(): 导航树面板映射表，使用QT_TRANSLATE_NOOP标记翻译键
 *   - allPanels(): 所有可切换面板列表，用于NavigationController::switchToPanel()
 *
 * BasePanel包装器逻辑见PanelManagerWrap.cpp。
 */

#include "core/panels/PanelManager.h"
#include "core/widgets/BasePanel.h"

// ---- 核心 ----
#include "serial/config/SerialConfigPanel.h"
#include "serial/data/DataStatistics.h"
#include "serial/commands/QuickCommandBar.h"
#include "protocol/view/ProtocolView.h"
#include "protocol/editor/FrameVisualEditor.h"
#include "chart/widget/ChartWidget.h"
#include "ota/widget/OtaWidget.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/model/TerminalModel.h"
#include "terminal/search/TerminalSearchBar.h"
#include "serial/data/BookmarkWidget.h"

// ---- 录制回放 / 仪表盘 / 终端增强 ----
#include "core/recording/PlaybackWidget.h"
#include "dashboard/DashboardWidget.h"
#include "terminal/filter/TerminalFilterBar.h"
#include "core/widgets/ScriptRecorder.h"

// ---- 连接层 ----
#include "connection/ble/BleConfigPanel.h"
#include "connection/ble/BleGattBrowser.h"
#include "connection/can/CanConfigPanel.h"
#include "connection/can/CanBusMonitor.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttSubscriptionPanel.h"
#include "connection/tcp/MultiConnectionPanel.h"
#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include "connection/ws/WsConfigPanel.h"
#include "connection/usb/UsbConfigPanel.h"
#include "connection/usb/UsbDescriptorViewer.h"

// ---- 协议层 ----
#include "protocol/editor/ProtocolSchemaEditor.h"
#include "protocol/modbus/ModbusConfigPanel.h"
#include "protocol/modbus/ModbusScanWidget.h"
#include "protocol/protobuf/SchemaViewer.h"

// ---- 调试层 ----
#include "rtt/RttConfigPanel.h"
#include "connection/spi_i2c/RegisterEditor.h"
#include "serial/signals/SignalLineWidget.h"
#include "serial/data/TrafficMonitorWidget.h"
#include "automation/TriggerListPanel.h"

// ---- 图表扩展 ----
#include "chart/fft/FftWidget.h"
#include "chart/stats/ScatterWidget.h"
#include "chart/stats/HistogramWidget.h"

// ---- 工具层 ----
#include "utils/checksum/ChecksumPanel.h"
#include "utils/converter/ConverterPanel.h"
#include "utils/timestamp/TimestampPanel.h"
#include "utils/packet/PacketBuilderPanel.h"
#include "core/widgets/DataDiffWidget.h"

// ---- 系统层 ----
#include "plugin/PluginConfigPanel.h"
#include "core/settings/ProjectWelcomeDialog.h"
#include "core/device/DeviceProfilePanel.h"
#include "utils/perf/PerformanceOverlay.h"

#include <algorithm>

// ==================== 映射表 ====================

QVector<PanelDescriptor> PanelManager::panelDescriptors() const
{
    QVector<PanelDescriptor> descriptors;
    descriptors.reserve(47);

    auto add = [&descriptors](const char* id,
                              const char* objectName,
                              const char* groupKey,
                              const char* titleKey,
                              const char* iconName,
                              QWidget* rawWidget,
                              PanelWrapperPolicy wrapperPolicy,
                              int navOrder,
                              int stackOrder,
                              bool navVisible = true,
                              bool includeInPanelStack = true) {
        descriptors.append(PanelDescriptor{
            id,
            objectName,
            groupKey,
            titleKey,
            iconName,
            rawWidget,
            wrapperPolicy,
            navOrder,
            stackOrder,
            navVisible,
            includeInPanelStack
        });
    };

    add("serial.config", "serialConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "配置"), "cable", m_serialConfig, PanelWrapperPolicy::Wrapped, 0, 0);
    add("connection.ble.config", "bleConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE配置"), "bluetooth", m_bleConfigPanel, PanelWrapperPolicy::Wrapped, 1, 14);
    add("connection.ble.gatt", "bleGattBrowserPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE浏览"), "bluetooth-connected", m_bleGattBrowser, PanelWrapperPolicy::Wrapped, 2, 15);
    add("connection.can.config", "canConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN配置"), "circle-gauge", m_canConfigPanel, PanelWrapperPolicy::Wrapped, 3, 16);
    add("connection.can.monitor", "canBusMonitorPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN监控"), "activity", m_canBusMonitor, PanelWrapperPolicy::Wrapped, 4, 17);
    add("connection.mqtt.config", "mqttConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT配置"), "radio-tower", m_mqttConfigPanel, PanelWrapperPolicy::Wrapped, 5, 18);
    add("connection.mqtt.subscription", "mqttSubscriptionPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT订阅"), "rss", m_mqttSubscriptionPanel, PanelWrapperPolicy::Wrapped, 6, 19);
    add("connection.tcp.multi", "multiConnectionPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "TCP多连接"), "network", m_multiConnectionPanel, PanelWrapperPolicy::Wrapped, 7, 20);
    add("connection.spi_i2c", "spiI2cConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "SPI/I2C"), "cpu", m_spiI2cConfigPanel, PanelWrapperPolicy::Wrapped, 8, 21);
    add("connection.websocket", "wsConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "WebSocket"), "globe", m_wsConfigPanel, PanelWrapperPolicy::Wrapped, 9, 22);
    add("connection.usb.config", "usbConfigPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB配置"), "usb", m_usbConfigPanel, PanelWrapperPolicy::Wrapped, 10, 23);
    add("connection.usb.descriptor", "usbDescriptorViewerPanel", QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB描述符"), "file-search", m_usbDescriptorViewer, PanelWrapperPolicy::Wrapped, 11, 24);

    add("terminal.main", "terminalPanel", QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端"), "terminal", m_terminal, PanelWrapperPolicy::RawPersistent, 12, 6);
    add("terminal.stats", "dataStatsPanel", QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "统计"), "chart-no-axes-combined", m_dataStats, PanelWrapperPolicy::Wrapped, 13, 1);
    add("terminal.playback", "playbackWidgetPanel", QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "录制回放"), "history", m_playbackWidget, PanelWrapperPolicy::Wrapped, 14, 10);
    add("terminal.filter", "terminalFilterBarPanel", QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端过滤"), "filter", m_terminalFilterBar, PanelWrapperPolicy::Wrapped, 15, 12);
    add("terminal.script", "scriptRecorderPanel", QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "脚本录制"), "file-clock", m_scriptRecorder, PanelWrapperPolicy::Wrapped, 16, 13);

    add("chart.main", "chartWidgetPanel", QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "波形图"), "chart-line", m_chartWidget, PanelWrapperPolicy::Wrapped, 17, 4);
    add("chart.fft", "fftWidgetPanel", QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "FFT频谱"), "audio-lines", m_fftWidget, PanelWrapperPolicy::Wrapped, 18, 35);
    add("chart.dashboard", "dashboardWidgetPanel", QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "仪表盘"), "layout-dashboard", m_dashboardWidget, PanelWrapperPolicy::Wrapped, 19, 11);
    add("chart.scatter", "scatterWidgetPanel", QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "散点图"), "scatter-chart", m_scatterWidget, PanelWrapperPolicy::Wrapped, 20, 36);
    add("chart.histogram", "histogramWidgetPanel", QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "直方图"), "bar-chart-3", m_histogramWidget, PanelWrapperPolicy::Wrapped, 21, 37);

    add("protocol.view", "protocolViewPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "协议"), "braces", m_protocolView, PanelWrapperPolicy::Wrapped, 22, 2);
    add("protocol.frame_editor", "frameEditorPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"), "blocks", m_frameEditor, PanelWrapperPolicy::Wrapped, 23, 3);
    add("protocol.schema_editor", "protocolSchemaEditorPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "自定义协议"), "file-code-2", m_protocolSchemaEditor, PanelWrapperPolicy::Wrapped, 24, 25);
    add("protocol.modbus.config", "modbusConfigPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus配置"), "sliders-horizontal", m_modbusConfigPanel, PanelWrapperPolicy::Wrapped, 25, 26);
    add("protocol.modbus.scan", "modbusScanWidgetPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus扫描"), "scan-search", m_modbusScanWidget, PanelWrapperPolicy::Wrapped, 26, 27);
    add("protocol.protobuf.schema", "schemaViewerPanel", QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Protobuf查看"), "file-json", m_schemaViewer, PanelWrapperPolicy::Wrapped, 27, 28);

    add("tool.ota", "otaWidgetPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "OTA升级"), "upload", m_otaWidget, PanelWrapperPolicy::Wrapped, 28, 5);
    add("tool.bookmark", "bookmarkWidgetPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "书签"), "bookmark", m_bookmarkWidget, PanelWrapperPolicy::Wrapped, 29, 9);
    add("tool.checksum", "checksumPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "校验计算"), "badge-check", m_checksumPanel, PanelWrapperPolicy::Wrapped, 30, 38);
    add("tool.converter", "converterPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据转换"), "repeat", m_converterPanel, PanelWrapperPolicy::Wrapped, 31, 39);
    add("tool.timestamp", "timestampPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "时间戳"), "clock", m_timestampPanel, PanelWrapperPolicy::Wrapped, 32, 40);
    add("tool.packet_builder", "packetBuilderPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据包构建"), "package-plus", m_packetBuilderPanel, PanelWrapperPolicy::Wrapped, 33, 41);
    add("tool.data_diff", "dataDiffPanel", QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据对比"), "git-compare", m_dataDiffPanel, PanelWrapperPolicy::Wrapped, 34, 42);

    add("debug.rtt", "rttConfigPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "RTT配置"), "waypoints", m_rttConfigPanel, PanelWrapperPolicy::Wrapped, 35, 29);
    add("debug.register", "registerEditorPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "寄存器编辑"), "table-properties", m_registerEditor, PanelWrapperPolicy::Wrapped, 36, 30);
    add("debug.signal_line", "signalLineWidgetPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "信号线"), "activity", m_signalLineWidget, PanelWrapperPolicy::Wrapped, 37, 31);
    add("debug.traffic", "trafficMonitorWidgetPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "流量监控"), "area-chart", m_trafficMonitorWidget, PanelWrapperPolicy::Wrapped, 38, 32);
    add("debug.trigger", "triggerListPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "触发器"), "zap", m_triggerListPanel, PanelWrapperPolicy::Wrapped, 39, 33);
    add("debug.performance", "performanceOverlayPanel", QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "性能监控"), "gauge", m_performanceOverlay, PanelWrapperPolicy::Wrapped, 40, 46);

    add("system.plugin", "pluginConfigPanel", QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "插件系统"), "puzzle", m_pluginConfigPanel, PanelWrapperPolicy::Wrapped, 41, 43);
    add("system.project", "projectWelcomeDialogPanel", QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "项目管理"), "folder-kanban", m_projectWelcomeDialog, PanelWrapperPolicy::Wrapped, 42, 44);
    add("system.device_profile", "deviceProfilePanel", QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "设备档案"), "microchip", m_deviceProfilePanel, PanelWrapperPolicy::Wrapped, 43, 45);

    add("terminal.search", "searchBarPanel", "", "", "search", m_searchBar, PanelWrapperPolicy::Overlay, -1, 7, false);
    add("terminal.quick_command", "quickCmdBarPanel", "", "", "send", m_quickCmdBar, PanelWrapperPolicy::FixedBar, -1, 8, false);

    return descriptors;
}

static QWidget* panelWidgetForDescriptor(const PanelManager* manager, const PanelDescriptor& descriptor)
{
    return descriptor.wrapperPolicy == PanelWrapperPolicy::Wrapped
        ? manager->wrapper(descriptor.rawWidget)
        : descriptor.rawWidget;
}

/** @brief 获取导航树面板映射表，使用QT_TRANSLATE_NOOP标记翻译键 */
QVector<NavPanelMapping> PanelManager::panelMappings() const
{
    auto descriptors = panelDescriptors();
    std::sort(descriptors.begin(), descriptors.end(), [](const PanelDescriptor& lhs, const PanelDescriptor& rhs) {
        return lhs.navOrder < rhs.navOrder;
    });

    QVector<NavPanelMapping> mappings;
    mappings.reserve(descriptors.size());
    for (const auto& descriptor : descriptors) {
        if (!descriptor.navVisible || descriptor.navOrder < 0) {
            continue;
        }
        mappings.append({descriptor.groupKey, descriptor.titleKey, panelWidgetForDescriptor(this, descriptor)});
    }
    return mappings;
}

/** @brief 获取所有可切换面板列表，用于NavigationController::switchToPanel() */
QVector<QWidget*> PanelManager::allPanels() const
{
    auto descriptors = panelDescriptors();
    std::sort(descriptors.begin(), descriptors.end(), [](const PanelDescriptor& lhs, const PanelDescriptor& rhs) {
        return lhs.stackOrder < rhs.stackOrder;
    });

    QVector<QWidget*> panels;
    panels.reserve(descriptors.size());
    for (const auto& descriptor : descriptors) {
        if (!descriptor.includeInPanelStack || descriptor.stackOrder < 0) {
            continue;
        }
        panels.append(panelWidgetForDescriptor(this, descriptor));
    }
    return panels;
}

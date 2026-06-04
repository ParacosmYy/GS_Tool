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

// ==================== 映射表 ====================

/** @brief 获取导航树面板映射表，使用QT_TRANSLATE_NOOP标记翻译键 */
QVector<NavPanelMapping> PanelManager::panelMappings() const
{
    return {
        // ---- 连接 ----
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "配置"),       wrapper(m_serialConfig)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE配置"),    wrapper(m_bleConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE浏览"),    wrapper(m_bleGattBrowser)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN配置"),    wrapper(m_canConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN监控"),    wrapper(m_canBusMonitor)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT配置"),   wrapper(m_mqttConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT订阅"),   wrapper(m_mqttSubscriptionPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "TCP多连接"),  wrapper(m_multiConnectionPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "SPI/I2C"),    wrapper(m_spiI2cConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "WebSocket"),  wrapper(m_wsConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB配置"),    wrapper(m_usbConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB描述符"),  wrapper(m_usbDescriptorViewer)},

        // ---- 终端 ----
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端"),       m_terminal},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "统计"),       wrapper(m_dataStats)},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "录制回放"),   wrapper(m_playbackWidget)},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端过滤"),   wrapper(m_terminalFilterBar)},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "脚本录制"),   wrapper(m_scriptRecorder)},

        // ---- 图表 ----
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "波形图"),     wrapper(m_chartWidget)},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "FFT频谱"),    wrapper(m_fftWidget)},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "仪表盘"),     wrapper(m_dashboardWidget)},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "散点图"),     wrapper(m_scatterWidget)},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "直方图"),     wrapper(m_histogramWidget)},

        // ---- 协议 ----
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "协议"),       wrapper(m_protocolView)},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"),   wrapper(m_frameEditor)},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "自定义协议"), wrapper(m_protocolSchemaEditor)},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus配置"), wrapper(m_modbusConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus扫描"), wrapper(m_modbusScanWidget)},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Protobuf查看"), wrapper(m_schemaViewer)},

        // ---- 工具 ----
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),    wrapper(m_otaWidget)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "书签"),       wrapper(m_bookmarkWidget)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "校验计算"),   wrapper(m_checksumPanel)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据转换"),   wrapper(m_converterPanel)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "时间戳"),     wrapper(m_timestampPanel)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据包构建"), wrapper(m_packetBuilderPanel)},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据对比"),   wrapper(m_dataDiffPanel)},

        // ---- 调试 ----
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "RTT配置"),    wrapper(m_rttConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "寄存器编辑"), wrapper(m_registerEditor)},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "信号线"),     wrapper(m_signalLineWidget)},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "流量监控"),   wrapper(m_trafficMonitorWidget)},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "触发器"),     wrapper(m_triggerListPanel)},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "性能监控"),   wrapper(m_performanceOverlay)},

        // ---- 系统 ----
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "插件系统"),   wrapper(m_pluginConfigPanel)},
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "项目管理"),   wrapper(m_projectWelcomeDialog)},
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "设备档案"),   wrapper(m_deviceProfilePanel)},
    };
}

/** @brief 获取所有可切换面板列表，用于NavigationController::switchToPanel() */
QVector<QWidget*> PanelManager::allPanels() const
{
    return {
        wrapper(m_serialConfig), wrapper(m_dataStats),
        wrapper(m_protocolView), wrapper(m_frameEditor),
        wrapper(m_chartWidget),  wrapper(m_otaWidget),
        m_terminal, m_searchBar, m_quickCmdBar,
        wrapper(m_bookmarkWidget),
        wrapper(m_playbackWidget),
        wrapper(m_dashboardWidget),
        wrapper(m_terminalFilterBar),
        wrapper(m_scriptRecorder),
        // 连接层
        wrapper(m_bleConfigPanel),       wrapper(m_bleGattBrowser),
        wrapper(m_canConfigPanel),       wrapper(m_canBusMonitor),
        wrapper(m_mqttConfigPanel),      wrapper(m_mqttSubscriptionPanel),
        wrapper(m_multiConnectionPanel), wrapper(m_spiI2cConfigPanel),
        wrapper(m_wsConfigPanel),        wrapper(m_usbConfigPanel),
        wrapper(m_usbDescriptorViewer),
        // 协议层
        wrapper(m_protocolSchemaEditor), wrapper(m_modbusConfigPanel),
        wrapper(m_modbusScanWidget),     wrapper(m_schemaViewer),
        // 调试层
        wrapper(m_rttConfigPanel),       wrapper(m_registerEditor),
        wrapper(m_signalLineWidget),     wrapper(m_trafficMonitorWidget),
        wrapper(m_triggerListPanel),
        // 图表扩展
        wrapper(m_fftWidget),     wrapper(m_scatterWidget),  wrapper(m_histogramWidget),
        // 工具层
        wrapper(m_checksumPanel), wrapper(m_converterPanel),
        wrapper(m_timestampPanel), wrapper(m_packetBuilderPanel),
        wrapper(m_dataDiffPanel),
        // 系统层
        wrapper(m_pluginConfigPanel),    wrapper(m_projectWelcomeDialog),
        wrapper(m_deviceProfilePanel),   wrapper(m_performanceOverlay),
    };
}

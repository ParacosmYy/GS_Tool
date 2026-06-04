/**
 * @file PanelManagerWrap.cpp
 * @brief PanelManager BasePanel包装器创建与查找
 *
 * 从PanelManagerQuery.cpp拆分而来，包含:
 *   - wrapper(): 获取原始面板的BasePanel包装器指针
 *   - wrapPanels(): 创建所有BasePanel包装器，必须在createPanels()之后调用
 *
 * 映射表(panelMappings)和面板列表(allPanels)保留在PanelManagerQuery.cpp中。
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

// ==================== BasePanel包装器 ====================

/** @brief 获取原始面板的BasePanel包装器指针，无包装器则返回nullptr */
BasePanel* PanelManager::wrapper(QWidget* rawPanel) const
{
    auto it = m_wrappers.constFind(rawPanel);
    return (it != m_wrappers.constEnd()) ? it.value() : nullptr;
}

/** @brief 创建所有BasePanel包装器，必须在createPanels()之后调用 */
void PanelManager::wrapPanels()
{
    QWidget* widgetParent = qobject_cast<QWidget*>(parent());

    auto wrap = [this, widgetParent](QWidget* panel, const QString& title) {
        if (!panel) return;
        auto* w = new BasePanel(panel, title, widgetParent);
        w->setVisible(false);
        w->setObjectName(panel->objectName() + "Wrapper");
        m_wrappers[panel] = w;
        ++m_totalPanelCreations;
        ++m_totalPanelRegisters;
    };

    // ---- 核心 (排除 m_terminal/m_searchBar/m_quickCmdBar) ----
    wrap(m_serialConfig, tr("配置"));
    wrap(m_dataStats, tr("统计"));
    wrap(m_protocolView, tr("协议"));
    wrap(m_frameEditor, tr("帧编辑器"));
    wrap(m_chartWidget, tr("波形图"));
    wrap(m_otaWidget, tr("OTA升级"));
    wrap(m_bookmarkWidget, tr("书签"));
    // ---- 录制回放 / 仪表盘 / 终端增强 / 脚本录制 ----
    wrap(m_playbackWidget, tr("录制回放"));
    wrap(m_dashboardWidget, tr("仪表盘"));
    wrap(m_terminalFilterBar, tr("终端过滤"));
    wrap(m_scriptRecorder, tr("脚本录制"));
    // ---- 连接层 ----
    wrap(m_bleConfigPanel, tr("BLE配置"));
    wrap(m_bleGattBrowser, tr("BLE浏览"));
    wrap(m_canConfigPanel, tr("CAN配置"));
    wrap(m_canBusMonitor, tr("CAN监控"));
    wrap(m_mqttConfigPanel, tr("MQTT配置"));
    wrap(m_mqttSubscriptionPanel, tr("MQTT订阅"));
    wrap(m_multiConnectionPanel, tr("TCP多连接"));
    wrap(m_spiI2cConfigPanel, tr("SPI/I2C"));
    wrap(m_wsConfigPanel, tr("WebSocket"));
    wrap(m_usbConfigPanel, tr("USB配置"));
    wrap(m_usbDescriptorViewer, tr("USB描述符"));
    // ---- 协议层 ----
    wrap(m_protocolSchemaEditor, tr("自定义协议"));
    wrap(m_modbusConfigPanel, tr("Modbus配置"));
    wrap(m_modbusScanWidget, tr("Modbus扫描"));
    wrap(m_schemaViewer, tr("Protobuf查看"));
    // ---- 调试层 ----
    wrap(m_rttConfigPanel, tr("RTT配置"));
    wrap(m_registerEditor, tr("寄存器编辑"));
    wrap(m_signalLineWidget, tr("信号线"));
    wrap(m_trafficMonitorWidget, tr("流量监控"));
    wrap(m_triggerListPanel, tr("触发器"));
    wrap(m_performanceOverlay, tr("性能监控"));
    // ---- 图表扩展 ----
    wrap(m_fftWidget, tr("FFT频谱"));
    wrap(m_scatterWidget, tr("散点图"));
    wrap(m_histogramWidget, tr("直方图"));
    // ---- 工具层 ----
    wrap(m_checksumPanel, tr("校验计算"));
    wrap(m_converterPanel, tr("数据转换"));
    wrap(m_timestampPanel, tr("时间戳"));
    wrap(m_packetBuilderPanel, tr("数据包构建"));
    wrap(m_dataDiffPanel, tr("数据对比"));
    // ---- 系统层 ----
    wrap(m_pluginConfigPanel, tr("插件系统"));
    wrap(m_projectWelcomeDialog, tr("项目管理"));
    wrap(m_deviceProfilePanel, tr("设备档案"));
}

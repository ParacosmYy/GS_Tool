/**
 * @file PanelManagerCreation.cpp
 * @brief 面板管理器 - createPanels() 方法实现
 *
 * 集中创建所有功能面板 widget，每个面板设置 objectName（QSS 选择器）和初始可见性。
 * 从 PanelManager.cpp 中分离以控制单文件行数 ≤ 500 行。
 */

#include "core/panels/PanelManager.h"

// ---- 核心 ----
#include "serial/config/SerialConfigPanel.h"
#include "serial/data/DataStatistics.h"
#include "serial/commands/QuickCommandBar.h"
#include "protocol/view/ProtocolView.h"
#include "protocol/editor/FrameVisualEditor.h"
#include "chart/widget/ChartWidget.h"
#include "ota/manager/OtaManager.h"
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

/**
 * @brief 创建所有面板 widget（调用一次，在 MainWindow::setupUI 中）
 *
 * @param otaManager     OTA 管理器指针，OtaWidget 构造时注入
 * @param terminalModel  终端数据模型指针，TerminalWidget 需要设置此模型
 *
 * 所有面板初始状态为隐藏（setVisible(false)），由 NavigationController 按需显示。
 * 终端面板（m_terminal）为默认可见面板，不隐藏。
 */
void PanelManager::createPanels(OtaManager* otaManager, TerminalModel* terminalModel)
{
    QWidget* widgetParent = qobject_cast<QWidget*>(parent());

    // ============================================================
    //  核心面板 (Core) — 串口/协议/图表/终端等基础功能
    // ============================================================

    m_serialConfig = new SerialConfigPanel(widgetParent);           // 串口配置
    m_serialConfig->setObjectName("serialConfigPanel");
    m_serialConfig->setVisible(false);

    m_dataStats = new DataStatistics(widgetParent);                 // 数据统计
    m_dataStats->setObjectName("dataStatsPanel");
    m_dataStats->setVisible(false);

    m_protocolView = new ProtocolView(widgetParent);                // 协议视图
    m_protocolView->setObjectName("protocolViewPanel");
    m_protocolView->setVisible(false);

    m_frameEditor = new FrameVisualEditor(widgetParent);            // 帧编辑器
    m_frameEditor->setObjectName("frameEditorPanel");
    m_frameEditor->setVisible(false);

    m_chartWidget = new ChartWidget(widgetParent);                  // 波形图
    m_chartWidget->setObjectName("chartWidgetPanel");
    m_chartWidget->setVisible(false);

    m_otaWidget = new OtaWidget(otaManager, widgetParent);          // OTA升级
    m_otaWidget->setObjectName("otaWidgetPanel");
    m_otaWidget->setVisible(false);

    m_terminal = new TerminalWidget(widgetParent);                  // 终端（默认可见）
    m_terminal->setObjectName("terminalPanel");
    m_terminal->setModel(terminalModel);

    m_searchBar = new TerminalSearchBar(widgetParent);              // 搜索栏
    m_searchBar->setObjectName("searchBarPanel");
    m_searchBar->setVisible(false);

    m_quickCmdBar = new QuickCommandBar(widgetParent);              // 快捷指令
    m_quickCmdBar->setObjectName("quickCmdBarPanel");
    m_quickCmdBar->setCommands({
        {"AT",     "AT\r\n",           false},
        {"Reset",  "AA 55 01 00 FE",   true},
        {"Status", "AT+STATUS?\r\n",   false}
    });
    m_quickCmdBar->setVisible(false);

    m_bookmarkWidget = new BookmarkWidget(widgetParent);            // 书签
    m_bookmarkWidget->setObjectName("bookmarkWidgetPanel");
    m_bookmarkWidget->setVisible(false);

    // ============================================================
    //  录制回放 (Recording)
    // ============================================================

    m_playbackWidget = new PlaybackWidget(widgetParent);            // F1 录制回放
    m_playbackWidget->setObjectName("playbackWidgetPanel");
    m_playbackWidget->setVisible(false);

    // ============================================================
    //  仪表盘 (Dashboard)
    // ============================================================

    m_dashboardWidget = new DashboardWidget(widgetParent);          // F5 仪表盘
    m_dashboardWidget->setObjectName("dashboardWidgetPanel");
    m_dashboardWidget->setVisible(false);

    // ============================================================
    //  终端增强 (Terminal Enhancement)
    // ============================================================

    m_terminalFilterBar = new TerminalFilterBar(widgetParent);      // F21 终端过滤
    m_terminalFilterBar->setObjectName("terminalFilterBarPanel");
    m_terminalFilterBar->setVisible(false);

    // ============================================================
    //  脚本录制 (Script Recorder)
    // ============================================================

    m_scriptRecorder = new ScriptRecorder(widgetParent);            // 脚本录制器
    m_scriptRecorder->setObjectName("scriptRecorderPanel");
    m_scriptRecorder->setVisible(false);

    // ============================================================
    //  连接层 (Connection) — BLE/CAN/MQTT/TCP/SPI/I2C/WS/USB
    // ============================================================

    m_bleConfigPanel = new BleConfigPanel(widgetParent);            // F12 BLE配置
    m_bleConfigPanel->setObjectName("bleConfigPanel");
    m_bleConfigPanel->setVisible(false);

    m_bleGattBrowser = new BleGattBrowser(widgetParent);            // F12 BLE浏览
    m_bleGattBrowser->setObjectName("bleGattBrowserPanel");
    m_bleGattBrowser->setVisible(false);

    m_canConfigPanel = new CanConfigPanel(widgetParent);            // F13 CAN配置
    m_canConfigPanel->setObjectName("canConfigPanel");
    m_canConfigPanel->setVisible(false);

    m_canBusMonitor = new CanBusMonitor(widgetParent);              // F13 CAN监控
    m_canBusMonitor->setObjectName("canBusMonitorPanel");
    m_canBusMonitor->setVisible(false);

    m_mqttConfigPanel = new MqttConfigPanel(widgetParent);          // F14 MQTT配置
    m_mqttConfigPanel->setObjectName("mqttConfigPanel");
    m_mqttConfigPanel->setVisible(false);

    m_mqttSubscriptionPanel = new MqttSubscriptionPanel(widgetParent); // F14 MQTT订阅
    m_mqttSubscriptionPanel->setObjectName("mqttSubscriptionPanel");
    m_mqttSubscriptionPanel->setVisible(false);

    m_multiConnectionPanel = new MultiConnectionPanel(widgetParent); // F15 TCP多连接
    m_multiConnectionPanel->setObjectName("multiConnectionPanel");
    m_multiConnectionPanel->setVisible(false);

    m_spiI2cConfigPanel = new SpiI2cConfigPanel(widgetParent);     // F16 SPI/I2C
    m_spiI2cConfigPanel->setObjectName("spiI2cConfigPanel");
    m_spiI2cConfigPanel->setVisible(false);

    m_wsConfigPanel = new WsConfigPanel(widgetParent);              // F17 WebSocket
    m_wsConfigPanel->setObjectName("wsConfigPanel");
    m_wsConfigPanel->setVisible(false);

    m_usbConfigPanel = new UsbConfigPanel(widgetParent);            // F20 USB配置
    m_usbConfigPanel->setObjectName("usbConfigPanel");
    m_usbConfigPanel->setVisible(false);

    m_usbDescriptorViewer = new UsbDescriptorViewer(widgetParent);  // F20 USB描述符
    m_usbDescriptorViewer->setObjectName("usbDescriptorViewerPanel");
    m_usbDescriptorViewer->setVisible(false);

    // ============================================================
    //  协议层 (Protocol)
    // ============================================================

    m_protocolSchemaEditor = new ProtocolSchemaEditor(widgetParent); // F2 自定义协议
    m_protocolSchemaEditor->setObjectName("protocolSchemaEditorPanel");
    m_protocolSchemaEditor->setVisible(false);

    m_modbusConfigPanel = new ModbusConfigPanel(widgetParent);      // F18 Modbus配置
    m_modbusConfigPanel->setObjectName("modbusConfigPanel");
    m_modbusConfigPanel->setVisible(false);

    m_modbusScanWidget = new ModbusScanWidget(widgetParent);        // F18 Modbus扫描
    m_modbusScanWidget->setObjectName("modbusScanWidgetPanel");
    m_modbusScanWidget->setVisible(false);

    m_schemaViewer = new SchemaViewer(widgetParent);                // F19 Protobuf查看
    m_schemaViewer->setObjectName("schemaViewerPanel");
    m_schemaViewer->setVisible(false);

    // ============================================================
    //  调试层 (Debug)
    // ============================================================

    m_rttConfigPanel = new RttConfigPanel(widgetParent);            // F6 RTT配置
    m_rttConfigPanel->setObjectName("rttConfigPanel");
    m_rttConfigPanel->setVisible(false);

    m_registerEditor = new RegisterEditor(widgetParent);            // F16 寄存器编辑
    m_registerEditor->setObjectName("registerEditorPanel");
    m_registerEditor->setVisible(false);

    m_signalLineWidget = new SignalLineWidget(widgetParent);        // F9 信号线监控
    m_signalLineWidget->setObjectName("signalLineWidgetPanel");
    m_signalLineWidget->setVisible(false);

    m_trafficMonitorWidget = new TrafficMonitorWidget(widgetParent); // F9 流量监控
    m_trafficMonitorWidget->setObjectName("trafficMonitorWidgetPanel");
    m_trafficMonitorWidget->setVisible(false);

    m_triggerListPanel = new TriggerListPanel(widgetParent);        // F7 触发器
    m_triggerListPanel->setObjectName("triggerListPanel");
    m_triggerListPanel->setVisible(false);

    // ============================================================
    //  图表扩展 (Chart Extensions)
    //  注意: FftWidget/ScatterWidget/HistogramWidget 需要 ChartModel*
    //  因此必须在 ChartWidget(m_chartWidget) 创建之后初始化
    // ============================================================

    m_fftWidget = new FftWidget(m_chartWidget->model(), widgetParent);    // FFT频谱
    m_fftWidget->setObjectName("fftWidgetPanel");
    m_fftWidget->setVisible(false);

    m_scatterWidget = new ScatterWidget(m_chartWidget->model(), widgetParent);  // 散点图
    m_scatterWidget->setObjectName("scatterWidgetPanel");
    m_scatterWidget->setVisible(false);

    m_histogramWidget = new HistogramWidget(m_chartWidget->model(), widgetParent);  // 直方图
    m_histogramWidget->setObjectName("histogramWidgetPanel");
    m_histogramWidget->setVisible(false);

    // ============================================================
    //  工具层 (Utils)
    // ============================================================

    m_checksumPanel = new ChecksumPanel(widgetParent);              // F22 校验计算
    m_checksumPanel->setObjectName("checksumPanel");
    m_checksumPanel->setVisible(false);

    m_converterPanel = new ConverterPanel(widgetParent);            // F23 数据转换
    m_converterPanel->setObjectName("converterPanel");
    m_converterPanel->setVisible(false);

    m_timestampPanel = new TimestampPanel(widgetParent);            // F24 时间戳
    m_timestampPanel->setObjectName("timestampPanel");
    m_timestampPanel->setVisible(false);

    m_packetBuilderPanel = new PacketBuilderPanel(widgetParent);    // F25 数据包构建
    m_packetBuilderPanel->setObjectName("packetBuilderPanel");
    m_packetBuilderPanel->setVisible(false);

    m_dataDiffPanel = new DataDiffWidget(widgetParent);             // 数据对比
    m_dataDiffPanel->setObjectName("dataDiffPanel");
    m_dataDiffPanel->setVisible(false);

    // ============================================================
    //  系统层 (System)
    // ============================================================

    m_pluginConfigPanel = new PluginConfigPanel(widgetParent);      // F11 插件系统
    m_pluginConfigPanel->setObjectName("pluginConfigPanel");
    m_pluginConfigPanel->setVisible(false);

    m_projectWelcomeDialog = new ProjectWelcomeDialog(widgetParent); // F8 项目管理
    m_projectWelcomeDialog->setObjectName("projectWelcomeDialog");
    m_projectWelcomeDialog->setVisible(false);

    m_deviceProfilePanel = new DeviceProfilePanel(widgetParent);    // F26 设备档案
    m_deviceProfilePanel->setObjectName("deviceProfilePanel");
    m_deviceProfilePanel->setVisible(false);

    m_performanceOverlay = new PerformanceOverlay(widgetParent);    // F10 性能监控
    m_performanceOverlay->setObjectName("performanceOverlay");
    m_performanceOverlay->setVisible(false);
}

/**
 * @file PanelManagerGetters.cpp
 * @brief PanelManager 面板Getter方法、映射表和BasePanel包装器
 *
 * 从 PanelManager.cpp 拆分而来，包含所有面板 Getter 方法、
 * panelMappings() 导航映射表、allPanels() 面板列表和 wrapPanels() 包装逻辑。
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

// ==================== 核心 Getter ====================

/** @brief 获取串口配置面板 @return 串口配置面板指针 */
SerialConfigPanel*   PanelManager::serialConfig()      const { return m_serialConfig; }
/** @brief 获取数据统计面板 @return 数据统计面板指针 */
DataStatistics*      PanelManager::dataStats()          const { return m_dataStats; }
/** @brief 获取协议解析视图面板 @return 协议视图指针 */
ProtocolView*        PanelManager::protocolView()       const { return m_protocolView; }
/** @brief 获取帧可视化编辑器面板 @return 帧编辑器指针 */
FrameVisualEditor*   PanelManager::frameEditor()        const { return m_frameEditor; }
/** @brief 获取波形图控件 @return 波形图指针 */
ChartWidget*         PanelManager::chartWidget()        const { return m_chartWidget; }
/** @brief 获取OTA升级面板 @return OTA面板指针 */
OtaWidget*           PanelManager::otaWidget()          const { return m_otaWidget; }
/** @brief 获取终端显示控件 @return 终端控件指针 */
TerminalWidget*      PanelManager::terminal()           const { return m_terminal; }
/** @brief 获取终端搜索栏 @return 搜索栏指针 */
TerminalSearchBar*   PanelManager::searchBar()          const { return m_searchBar; }
/** @brief 获取快捷指令栏 @return 快捷指令栏指针 */
QuickCommandBar*     PanelManager::quickCmdBar()        const { return m_quickCmdBar; }
/** @brief 获取书签面板 @return 书签面板指针 */
BookmarkWidget*      PanelManager::bookmarkWidget()     const { return m_bookmarkWidget; }

// ==================== 录制回放 / 仪表盘 / 终端增强 / 脚本录制 ====================

/** @brief 获取录制回放面板 @return 录制回放面板指针 */
PlaybackWidget*      PanelManager::playbackWidget()     const { return m_playbackWidget; }
/** @brief 获取仪表盘面板 @return 仪表盘指针 */
DashboardWidget*     PanelManager::dashboardWidget()    const { return m_dashboardWidget; }
/** @brief 获取终端过滤栏 @return 过滤栏指针 */
TerminalFilterBar*   PanelManager::terminalFilterBar()  const { return m_terminalFilterBar; }
/** @brief 获取脚本录制器 @return 脚本录制器指针 */
ScriptRecorder*      PanelManager::scriptRecorder()     const { return m_scriptRecorder; }

// ==================== 连接层 Getter ====================

/** @brief 获取BLE配置面板 @return BLE配置面板指针 */
BleConfigPanel*          PanelManager::bleConfigPanel()          const { return m_bleConfigPanel; }
/** @brief 获取BLE GATT浏览器 @return GATT浏览器指针 */
BleGattBrowser*          PanelManager::bleGattBrowser()          const { return m_bleGattBrowser; }
/** @brief 获取CAN配置面板 @return CAN配置面板指针 */
CanConfigPanel*          PanelManager::canConfigPanel()          const { return m_canConfigPanel; }
/** @brief 获取CAN总线监控器 @return CAN监控器指针 */
CanBusMonitor*           PanelManager::canBusMonitor()           const { return m_canBusMonitor; }
/** @brief 获取MQTT配置面板 @return MQTT配置面板指针 */
MqttConfigPanel*         PanelManager::mqttConfigPanel()         const { return m_mqttConfigPanel; }
/** @brief 获取MQTT订阅面板 @return MQTT订阅面板指针 */
MqttSubscriptionPanel*   PanelManager::mqttSubscriptionPanel()   const { return m_mqttSubscriptionPanel; }
/** @brief 获取多连接管理面板 @return 多连接面板指针 */
MultiConnectionPanel*    PanelManager::multiConnectionPanel()    const { return m_multiConnectionPanel; }
/** @brief 获取SPI/I2C桥接配置面板 @return SPI/I2C配置面板指针 */
SpiI2cConfigPanel*       PanelManager::spiI2cConfigPanel()       const { return m_spiI2cConfigPanel; }
/** @brief 获取WebSocket配置面板 @return WebSocket配置面板指针 */
WsConfigPanel*           PanelManager::wsConfigPanel()           const { return m_wsConfigPanel; }
/** @brief 获取USB配置面板 @return USB配置面板指针 */
UsbConfigPanel*          PanelManager::usbConfigPanel()          const { return m_usbConfigPanel; }
/** @brief 获取USB描述符查看器 @return USB描述符查看器指针 */
UsbDescriptorViewer*     PanelManager::usbDescriptorViewer()     const { return m_usbDescriptorViewer; }

// ==================== 协议层 Getter ====================

/** @brief 获取协议Schema编辑器 @return Schema编辑器指针 */
ProtocolSchemaEditor* PanelManager::protocolSchemaEditor() const { return m_protocolSchemaEditor; }
/** @brief 获取Modbus配置面板 @return Modbus配置面板指针 */
ModbusConfigPanel*     PanelManager::modbusConfigPanel()   const { return m_modbusConfigPanel; }
/** @brief 获取Modbus扫描面板 @return Modbus扫描面板指针 */
ModbusScanWidget*      PanelManager::modbusScanWidget()    const { return m_modbusScanWidget; }
/** @brief 获取Schema查看器 @return Schema查看器指针 */
SchemaViewer*          PanelManager::schemaViewer()        const { return m_schemaViewer; }

// ==================== 调试层 Getter ====================

/** @brief 获取RTT配置面板 @return RTT配置面板指针 */
RttConfigPanel*        PanelManager::rttConfigPanel()        const { return m_rttConfigPanel; }
/** @brief 获取寄存器编辑器 @return 寄存器编辑器指针 */
RegisterEditor*        PanelManager::registerEditor()        const { return m_registerEditor; }
/** @brief 获取信号线监控面板 @return 信号线监控指针 */
SignalLineWidget*      PanelManager::signalLineWidget()      const { return m_signalLineWidget; }
/** @brief 获取流量监控面板 @return 流量监控指针 */
TrafficMonitorWidget*  PanelManager::trafficMonitorWidget()  const { return m_trafficMonitorWidget; }
/** @brief 获取触发器列表面板 @return 触发器面板指针 */
TriggerListPanel*      PanelManager::triggerListPanel()      const { return m_triggerListPanel; }

// ==================== 图表扩展 Getter ====================

/** @brief 获取FFT频谱分析面板 @return FFT面板指针 */
FftWidget*             PanelManager::fftWidget()             const { return m_fftWidget; }
/** @brief 获取散点图面板 @return 散点图指针 */
ScatterWidget*         PanelManager::scatterWidget()         const { return m_scatterWidget; }
/** @brief 获取直方图面板 @return 直方图指针 */
HistogramWidget*       PanelManager::histogramWidget()       const { return m_histogramWidget; }

// ==================== 工具层 Getter ====================

/** @brief 获取校验和计算面板 @return 校验面板指针 */
ChecksumPanel*         PanelManager::checksumPanel()         const { return m_checksumPanel; }
/** @brief 获取数据转换面板 @return 转换面板指针 */
ConverterPanel*        PanelManager::converterPanel()        const { return m_converterPanel; }
/** @brief 获取时间戳面板 @return 时间戳面板指针 */
TimestampPanel*        PanelManager::timestampPanel()        const { return m_timestampPanel; }
/** @brief 获取数据包构建面板 @return 数据包构建面板指针 */
PacketBuilderPanel*    PanelManager::packetBuilderPanel()    const { return m_packetBuilderPanel; }
/** @brief 获取数据对比面板 @return 数据对比面板指针 */
DataDiffWidget*        PanelManager::dataDiffPanel()         const { return m_dataDiffPanel; }

// ==================== 系统层 Getter ====================

/** @brief 获取插件配置面板 @return 插件配置面板指针 */
PluginConfigPanel*     PanelManager::pluginConfigPanel()     const { return m_pluginConfigPanel; }
/** @brief 获取工程欢迎对话框 @return 欢迎对话框指针 */
ProjectWelcomeDialog*  PanelManager::projectWelcomeDialog()  const { return m_projectWelcomeDialog; }
/** @brief 获取设备配置面板 @return 设备档案面板指针 */
DeviceProfilePanel*    PanelManager::deviceProfilePanel()    const { return m_deviceProfilePanel; }
/** @brief 获取性能监控浮层 @return 性能监控指针 */
PerformanceOverlay*    PanelManager::performanceOverlay()    const { return m_performanceOverlay; }

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

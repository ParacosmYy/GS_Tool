/**
 * @file PanelManager.cpp
 * @brief 面板管理器实现 - 构造函数、Getter、映射表接口
 *
 * 本文件实现 PanelManager 的构造函数、所有面板 Getter 方法、
 * panelMappings() 导航映射表和 allPanels() 面板列表。
 * 面板创建逻辑见 PanelManagerCreation.cpp。
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

// ---- 系统层 ----
#include "plugin/PluginConfigPanel.h"
#include "core/settings/ProjectWelcomeDialog.h"
#include "core/device/DeviceProfilePanel.h"
#include "utils/perf/PerformanceOverlay.h"

// ==================== 构造函数 ====================

/** @brief 构造面板管理器，所有面板指针由头文件初始化为 nullptr */
PanelManager::PanelManager(QObject* parent)
    : QObject(parent)
{
}

// ==================== 核心 Getter ====================

/** @brief 串口配置面板 */
SerialConfigPanel*   PanelManager::serialConfig()      const { return m_serialConfig; }
/** @brief 数据统计面板 */
DataStatistics*      PanelManager::dataStats()          const { return m_dataStats; }
/** @brief 协议解析视图 */
ProtocolView*        PanelManager::protocolView()       const { return m_protocolView; }
/** @brief 帧可视化编辑器 */
FrameVisualEditor*   PanelManager::frameEditor()        const { return m_frameEditor; }
/** @brief 波形图控件 */
ChartWidget*         PanelManager::chartWidget()        const { return m_chartWidget; }
/** @brief OTA升级面板 */
OtaWidget*           PanelManager::otaWidget()          const { return m_otaWidget; }
/** @brief 终端显示控件 */
TerminalWidget*      PanelManager::terminal()           const { return m_terminal; }
/** @brief 终端搜索栏 */
TerminalSearchBar*   PanelManager::searchBar()          const { return m_searchBar; }
/** @brief 快捷指令栏 */
QuickCommandBar*     PanelManager::quickCmdBar()        const { return m_quickCmdBar; }
/** @brief 书签面板 */
BookmarkWidget*      PanelManager::bookmarkWidget()     const { return m_bookmarkWidget; }

// ==================== 录制回放 ====================

/** @brief 录制回放面板 */
PlaybackWidget*      PanelManager::playbackWidget()     const { return m_playbackWidget; }

// ==================== 仪表盘 ====================

/** @brief 仪表盘面板 */
DashboardWidget*     PanelManager::dashboardWidget()    const { return m_dashboardWidget; }

// ==================== 终端增强 ====================

/** @brief 终端过滤栏 */
TerminalFilterBar*   PanelManager::terminalFilterBar()  const { return m_terminalFilterBar; }

// ==================== 连接层 Getter ====================

/** @brief BLE配置面板 */
BleConfigPanel*          PanelManager::bleConfigPanel()          const { return m_bleConfigPanel; }
/** @brief BLE GATT浏览器 */
BleGattBrowser*          PanelManager::bleGattBrowser()          const { return m_bleGattBrowser; }
/** @brief CAN配置面板 */
CanConfigPanel*          PanelManager::canConfigPanel()          const { return m_canConfigPanel; }
/** @brief CAN总线监控 */
CanBusMonitor*           PanelManager::canBusMonitor()           const { return m_canBusMonitor; }
/** @brief MQTT配置面板 */
MqttConfigPanel*         PanelManager::mqttConfigPanel()         const { return m_mqttConfigPanel; }
/** @brief MQTT订阅面板 */
MqttSubscriptionPanel*   PanelManager::mqttSubscriptionPanel()   const { return m_mqttSubscriptionPanel; }
/** @brief TCP多连接面板 */
MultiConnectionPanel*    PanelManager::multiConnectionPanel()    const { return m_multiConnectionPanel; }
/** @brief SPI/I2C配置面板 */
SpiI2cConfigPanel*       PanelManager::spiI2cConfigPanel()       const { return m_spiI2cConfigPanel; }
/** @brief WebSocket配置面板 */
WsConfigPanel*           PanelManager::wsConfigPanel()           const { return m_wsConfigPanel; }
/** @brief USB配置面板 */
UsbConfigPanel*          PanelManager::usbConfigPanel()          const { return m_usbConfigPanel; }
/** @brief USB描述符查看器 */
UsbDescriptorViewer*     PanelManager::usbDescriptorViewer()     const { return m_usbDescriptorViewer; }

// ==================== 协议层 Getter ====================

/** @brief 自定义协议编辑器 */
ProtocolSchemaEditor* PanelManager::protocolSchemaEditor() const { return m_protocolSchemaEditor; }
/** @brief Modbus配置面板 */
ModbusConfigPanel*     PanelManager::modbusConfigPanel()   const { return m_modbusConfigPanel; }
/** @brief Modbus扫描面板 */
ModbusScanWidget*      PanelManager::modbusScanWidget()    const { return m_modbusScanWidget; }
/** @brief Protobuf查看器 */
SchemaViewer*          PanelManager::schemaViewer()        const { return m_schemaViewer; }

// ==================== 调试层 Getter ====================

/** @brief RTT配置面板 */
RttConfigPanel*        PanelManager::rttConfigPanel()        const { return m_rttConfigPanel; }
/** @brief 寄存器编辑器 */
RegisterEditor*        PanelManager::registerEditor()        const { return m_registerEditor; }
/** @brief 信号线监控 */
SignalLineWidget*      PanelManager::signalLineWidget()      const { return m_signalLineWidget; }
/** @brief 流量监控 */
TrafficMonitorWidget*  PanelManager::trafficMonitorWidget()  const { return m_trafficMonitorWidget; }
/** @brief 触发器列表面板 */
TriggerListPanel*      PanelManager::triggerListPanel()      const { return m_triggerListPanel; }

// ==================== 图表扩展 Getter ====================

/** @brief FFT频谱面板 */
FftWidget*             PanelManager::fftWidget()             const { return m_fftWidget; }
/** @brief 散点图面板 */
ScatterWidget*         PanelManager::scatterWidget()         const { return m_scatterWidget; }
/** @brief 直方图面板 */
HistogramWidget*       PanelManager::histogramWidget()       const { return m_histogramWidget; }

// ==================== 工具层 Getter ====================

/** @brief 校验计算面板 */
ChecksumPanel*         PanelManager::checksumPanel()         const { return m_checksumPanel; }
/** @brief 数据转换面板 */
ConverterPanel*        PanelManager::converterPanel()        const { return m_converterPanel; }
/** @brief 时间戳面板 */
TimestampPanel*        PanelManager::timestampPanel()        const { return m_timestampPanel; }
/** @brief 数据包构建面板 */
PacketBuilderPanel*    PanelManager::packetBuilderPanel()    const { return m_packetBuilderPanel; }

// ==================== 系统层 Getter ====================

/** @brief 插件系统配置面板 */
PluginConfigPanel*     PanelManager::pluginConfigPanel()     const { return m_pluginConfigPanel; }
/** @brief 项目管理对话框 */
ProjectWelcomeDialog*  PanelManager::projectWelcomeDialog()  const { return m_projectWelcomeDialog; }
/** @brief 设备档案面板 */
DeviceProfilePanel*    PanelManager::deviceProfilePanel()    const { return m_deviceProfilePanel; }
/** @brief 性能监控覆层 */
PerformanceOverlay*    PanelManager::performanceOverlay()    const { return m_performanceOverlay; }

// ==================== 映射表 ====================

/**
 * @brief 获取导航树面板映射表
 *
 * 使用 QT_TRANSLATE_NOOP("Nav", ...) 标记分组翻译键，
 * QT_TRANSLATE_NOOP("MainWindow", ...) 标记面板名称翻译键。
 * 映射表顺序对应导航树的叶子节点顺序。
 */
QVector<NavPanelMapping> PanelManager::panelMappings() const
{
    return {
        // ---- 连接 ----
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "配置"),       m_serialConfig},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE配置"),    m_bleConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "BLE浏览"),    m_bleGattBrowser},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN配置"),    m_canConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "CAN监控"),    m_canBusMonitor},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT配置"),   m_mqttConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "MQTT订阅"),   m_mqttSubscriptionPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "TCP多连接"),  m_multiConnectionPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "SPI/I2C"),    m_spiI2cConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "WebSocket"),  m_wsConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB配置"),    m_usbConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "连接"), QT_TRANSLATE_NOOP("MainWindow", "USB描述符"),  m_usbDescriptorViewer},

        // ---- 终端 ----
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端"),       m_terminal},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "统计"),       m_dataStats},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "录制回放"),   m_playbackWidget},
        {QT_TRANSLATE_NOOP("Nav", "终端"), QT_TRANSLATE_NOOP("MainWindow", "终端过滤"),   m_terminalFilterBar},

        // ---- 图表 ----
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "波形图"),     m_chartWidget},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "FFT频谱"),    m_fftWidget},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "仪表盘"),     m_dashboardWidget},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "散点图"),     m_scatterWidget},
        {QT_TRANSLATE_NOOP("Nav", "图表"), QT_TRANSLATE_NOOP("MainWindow", "直方图"),     m_histogramWidget},

        // ---- 协议 ----
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "协议"),       m_protocolView},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"),   m_frameEditor},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "自定义协议"), m_protocolSchemaEditor},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus配置"), m_modbusConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Modbus扫描"), m_modbusScanWidget},
        {QT_TRANSLATE_NOOP("Nav", "协议"), QT_TRANSLATE_NOOP("MainWindow", "Protobuf查看"), m_schemaViewer},

        // ---- 工具 ----
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),    m_otaWidget},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "书签"),       m_bookmarkWidget},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "校验计算"),   m_checksumPanel},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据转换"),   m_converterPanel},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "时间戳"),     m_timestampPanel},
        {QT_TRANSLATE_NOOP("Nav", "工具"), QT_TRANSLATE_NOOP("MainWindow", "数据包构建"), m_packetBuilderPanel},

        // ---- 调试 ----
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "RTT配置"),    m_rttConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "寄存器编辑"), m_registerEditor},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "信号线"),     m_signalLineWidget},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "流量监控"),   m_trafficMonitorWidget},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "触发器"),     m_triggerListPanel},
        {QT_TRANSLATE_NOOP("Nav", "调试"), QT_TRANSLATE_NOOP("MainWindow", "性能监控"),   m_performanceOverlay},

        // ---- 系统 ----
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "插件系统"),   m_pluginConfigPanel},
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "项目管理"),   m_projectWelcomeDialog},
        {QT_TRANSLATE_NOOP("Nav", "系统"), QT_TRANSLATE_NOOP("MainWindow", "设备档案"),   m_deviceProfilePanel},
    };
}

/**
 * @brief 获取所有可切换的面板列表
 *
 * 用于 NavigationController::switchToPanel() 中先隐藏所有面板再显示目标面板。
 * 包含所有可通过导航树切换显示的面板 widget（含终端默认面板）。
 */
QVector<QWidget*> PanelManager::allPanels() const
{
    return {
        // 核心面板
        m_serialConfig, m_dataStats, m_protocolView, m_frameEditor,
        m_chartWidget,  m_otaWidget,  m_terminal,     m_searchBar,
        m_quickCmdBar,  m_bookmarkWidget,
        // 录制回放
        m_playbackWidget,
        // 仪表盘
        m_dashboardWidget,
        // 终端增强
        m_terminalFilterBar,
        // 连接层
        m_bleConfigPanel,       m_bleGattBrowser,
        m_canConfigPanel,       m_canBusMonitor,
        m_mqttConfigPanel,      m_mqttSubscriptionPanel,
        m_multiConnectionPanel, m_spiI2cConfigPanel,
        m_wsConfigPanel,        m_usbConfigPanel,
        m_usbDescriptorViewer,
        // 协议层
        m_protocolSchemaEditor, m_modbusConfigPanel,
        m_modbusScanWidget,     m_schemaViewer,
        // 调试层
        m_rttConfigPanel,       m_registerEditor,
        m_signalLineWidget,     m_trafficMonitorWidget,
        m_triggerListPanel,
        // 图表扩展
        m_fftWidget,     m_scatterWidget,  m_histogramWidget,
        // 工具层
        m_checksumPanel, m_converterPanel,
        m_timestampPanel, m_packetBuilderPanel,
        // 系统层
        m_pluginConfigPanel,    m_projectWelcomeDialog,
        m_deviceProfilePanel,   m_performanceOverlay,
    };
}

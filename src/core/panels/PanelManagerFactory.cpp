/**
 * @file PanelManagerFactory.cpp
 * @brief 面板管理器 - 面板工厂辅助方法实现
 *
 * 从 PanelManagerCreation.cpp 拆分而来，包含按层分类的面板创建方法：
 * - createConnectionPanels()  连接层(BLE/CAN/MQTT/TCP/SPI/I2C/WS/USB)
 * - createProtocolPanels()    协议层(自定义协议/Modbus/Protobuf)
 * - createDebugPanels()       调试层(RTT/寄存器/信号线/流量/触发器)
 * - createChartExtensionPanels() 图表扩展(FFT/散点/直方图)
 * - createToolPanels()        工具层(校验/转换/时间戳/数据包/对比)
 * - createSystemPanels()      系统层(插件/项目/设备/性能)
 *
 * 每个方法由 createPanels() 统一调度，确保创建顺序和依赖关系正确。
 */

#include "core/panels/PanelManager.h"

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

// ---- 图表扩展(依赖ChartWidget::model()完整类型) ----
#include "chart/widget/ChartWidget.h"
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

// ==================== 连接层 ====================

/** @brief 创建连接层面板(BLE/CAN/MQTT/TCP/SPI/I2C/WS/USB)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createConnectionPanels(QWidget* parent)
{
    m_bleConfigPanel = new BleConfigPanel(parent);                   // F12 BLE配置
    m_bleConfigPanel->setObjectName("bleConfigPanel");
    m_bleConfigPanel->setVisible(false);

    m_bleGattBrowser = new BleGattBrowser(parent);                   // F12 BLE浏览
    m_bleGattBrowser->setObjectName("bleGattBrowserPanel");
    m_bleGattBrowser->setVisible(false);

    m_canConfigPanel = new CanConfigPanel(parent);                   // F13 CAN配置
    m_canConfigPanel->setObjectName("canConfigPanel");
    m_canConfigPanel->setVisible(false);

    m_canBusMonitor = new CanBusMonitor(parent);                     // F13 CAN监控
    m_canBusMonitor->setObjectName("canBusMonitorPanel");
    m_canBusMonitor->setVisible(false);

    m_mqttConfigPanel = new MqttConfigPanel(parent);                 // F14 MQTT配置
    m_mqttConfigPanel->setObjectName("mqttConfigPanel");
    m_mqttConfigPanel->setVisible(false);

    m_mqttSubscriptionPanel = new MqttSubscriptionPanel(parent);     // F14 MQTT订阅
    m_mqttSubscriptionPanel->setObjectName("mqttSubscriptionPanel");
    m_mqttSubscriptionPanel->setVisible(false);

    m_multiConnectionPanel = new MultiConnectionPanel(parent);       // F15 TCP多连接
    m_multiConnectionPanel->setObjectName("multiConnectionPanel");
    m_multiConnectionPanel->setVisible(false);

    m_spiI2cConfigPanel = new SpiI2cConfigPanel(parent);             // F16 SPI/I2C
    m_spiI2cConfigPanel->setObjectName("spiI2cConfigPanel");
    m_spiI2cConfigPanel->setVisible(false);

    m_wsConfigPanel = new WsConfigPanel(parent);                     // F17 WebSocket
    m_wsConfigPanel->setObjectName("wsConfigPanel");
    m_wsConfigPanel->setVisible(false);

    m_usbConfigPanel = new UsbConfigPanel(parent);                   // F20 USB配置
    m_usbConfigPanel->setObjectName("usbConfigPanel");
    m_usbConfigPanel->setVisible(false);

    m_usbDescriptorViewer = new UsbDescriptorViewer(parent);         // F20 USB描述符
    m_usbDescriptorViewer->setObjectName("usbDescriptorViewerPanel");
    m_usbDescriptorViewer->setVisible(false);
}

// ==================== 协议层 ====================

/** @brief 创建协议层面板(自定义协议/Modbus/Protobuf)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createProtocolPanels(QWidget* parent)
{
    m_protocolSchemaEditor = new ProtocolSchemaEditor(parent);       // F2 自定义协议
    m_protocolSchemaEditor->setObjectName("protocolSchemaEditorPanel");
    m_protocolSchemaEditor->setVisible(false);

    m_modbusConfigPanel = new ModbusConfigPanel(parent);             // F18 Modbus配置
    m_modbusConfigPanel->setObjectName("modbusConfigPanel");
    m_modbusConfigPanel->setVisible(false);

    m_modbusScanWidget = new ModbusScanWidget(parent);               // F18 Modbus扫描
    m_modbusScanWidget->setObjectName("modbusScanWidgetPanel");
    m_modbusScanWidget->setVisible(false);

    m_schemaViewer = new SchemaViewer(parent);                       // F19 Protobuf查看
    m_schemaViewer->setObjectName("schemaViewerPanel");
    m_schemaViewer->setVisible(false);
}

// ==================== 调试层 ====================

/** @brief 创建调试层面板(RTT/寄存器/信号线/流量/触发器)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createDebugPanels(QWidget* parent)
{
    m_rttConfigPanel = new RttConfigPanel(parent);                   // F6 RTT配置
    m_rttConfigPanel->setObjectName("rttConfigPanel");
    m_rttConfigPanel->setVisible(false);

    m_registerEditor = new RegisterEditor(parent);                   // F16 寄存器编辑
    m_registerEditor->setObjectName("registerEditorPanel");
    m_registerEditor->setVisible(false);

    m_signalLineWidget = new SignalLineWidget(parent);               // F9 信号线监控
    m_signalLineWidget->setObjectName("signalLineWidgetPanel");
    m_signalLineWidget->setVisible(false);

    m_trafficMonitorWidget = new TrafficMonitorWidget(parent);       // F9 流量监控
    m_trafficMonitorWidget->setObjectName("trafficMonitorWidgetPanel");
    m_trafficMonitorWidget->setVisible(false);

    m_triggerListPanel = new TriggerListPanel(parent);               // F7 触发器
    m_triggerListPanel->setObjectName("triggerListPanel");
    m_triggerListPanel->setVisible(false);
}

// ==================== 图表扩展 ====================

/** @brief 创建图表扩展面板(FFT/散点/直方图)，依赖m_chartWidget已创建，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createChartExtensionPanels(QWidget* parent)
{
    m_fftWidget = new FftWidget(m_chartWidget->model(), parent);    // FFT频谱
    m_fftWidget->setObjectName("fftWidgetPanel");
    m_fftWidget->setVisible(false);

    m_scatterWidget = new ScatterWidget(m_chartWidget->model(), parent); // 散点图
    m_scatterWidget->setObjectName("scatterWidgetPanel");
    m_scatterWidget->setVisible(false);

    m_histogramWidget = new HistogramWidget(m_chartWidget->model(), parent); // 直方图
    m_histogramWidget->setObjectName("histogramWidgetPanel");
    m_histogramWidget->setVisible(false);
}

// ==================== 工具层 ====================

/** @brief 创建工具层面板(校验/转换/时间戳/数据包/对比)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createToolPanels(QWidget* parent)
{
    m_checksumPanel = new ChecksumPanel(parent);                     // F22 校验计算
    m_checksumPanel->setObjectName("checksumPanel");
    m_checksumPanel->setVisible(false);

    m_converterPanel = new ConverterPanel(parent);                   // F23 数据转换
    m_converterPanel->setObjectName("converterPanel");
    m_converterPanel->setVisible(false);

    m_timestampPanel = new TimestampPanel(parent);                   // F24 时间戳
    m_timestampPanel->setObjectName("timestampPanel");
    m_timestampPanel->setVisible(false);

    m_packetBuilderPanel = new PacketBuilderPanel(parent);           // F25 数据包构建
    m_packetBuilderPanel->setObjectName("packetBuilderPanel");
    m_packetBuilderPanel->setVisible(false);

    m_dataDiffPanel = new DataDiffWidget(parent);                    // 数据对比
    m_dataDiffPanel->setObjectName("dataDiffPanel");
    m_dataDiffPanel->setVisible(false);
}

// ==================== 系统层 ====================

/** @brief 创建系统层面板(插件/项目/设备/性能)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createSystemPanels(QWidget* parent)
{
    m_pluginConfigPanel = new PluginConfigPanel(parent);             // F11 插件系统
    m_pluginConfigPanel->setObjectName("pluginConfigPanel");
    m_pluginConfigPanel->setVisible(false);

    m_projectWelcomeDialog = new ProjectWelcomeDialog(parent);      // F8 项目管理
    m_projectWelcomeDialog->setObjectName("projectWelcomeDialog");
    m_projectWelcomeDialog->setVisible(false);

    m_deviceProfilePanel = new DeviceProfilePanel(parent);           // F26 设备档案
    m_deviceProfilePanel->setObjectName("deviceProfilePanel");
    m_deviceProfilePanel->setVisible(false);

    m_performanceOverlay = new PerformanceOverlay(parent);           // F10 性能监控
    m_performanceOverlay->setObjectName("performanceOverlay");
    m_performanceOverlay->setVisible(false);
}

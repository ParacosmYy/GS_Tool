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

// ---- 调试层/图表扩展/工具层/系统层includes和面板创建见 PanelManagerFactorySystem.cpp ----

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

// ==================== 调试层/图表扩展/工具层/系统层见 PanelManagerFactorySystem.cpp ====================

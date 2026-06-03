/**
 * @file ConnectionFactory.cpp
 * @brief 连接工厂实现 - 根据连接类型创建对应的IConnection子类实例
 *
 * 工厂映射:
 *   Serial    → SerialConnection
 *   TcpClient → TcpConnection (Client模式)
 *   TcpServer → TcpConnection (Server模式，同一类不同配置)
 *   Udp       → UdpConnection
 *   Rtt       → JLinkRttConnection
 *   WebSocket → WebSocketConnection
 *   Mqtt      → MqttConnection
 *   Tls       → TlsConnection
 *   Ble       → BleConnection
 *   Can       → CanConnection
 *   Spi       → SpiConnection
 *   I2c       → I2cConnection
 *   Usb       → UsbConnection
 */

#include "core/connect/ConnectionFactory.h"
#include "connection/interface/IConnection.h"
#include "connection/serial_port/SerialConnection.h"
#include "connection/network/TcpConnection.h"
#include "connection/network/UdpConnection.h"
#include "connection/tcp/TcpServerConnection.h"
#include "connection/tcp/TlsConnection.h"
#include "connection/tcp/UdpMulticastConnection.h"
#include "connection/ws/WebSocketConnection.h"
#include "connection/mqtt/MqttConnection.h"
#include "connection/ble/BleConnection.h"
#include "connection/can/CanConnection.h"
#include "connection/spi_i2c/SpiConnection.h"
#include "connection/spi_i2c/I2cConnection.h"
#include "connection/usb/UsbConnection.h"
#include "rtt/JLinkRttConnection.h"

/* 静态成员初始化 */
quint64 ConnectionFactory::s_totalCreated = 0;
QMap<ConnectionType, quint64> ConnectionFactory::s_totalByType;
quint64 ConnectionFactory::s_errorCount = 0;

/** @brief 根据连接类型枚举创建对应的IConnection子类实例 @param type 连接类型枚举 @param parent 父对象指针 @return 新创建的连接实例，未知类型返回nullptr */
IConnection* ConnectionFactory::create(ConnectionType type, QObject* parent)
{
    IConnection* conn = nullptr;

    switch (type) {
    case ConnectionType::Serial:
        conn = new SerialConnection(parent);
        break;
    case ConnectionType::TcpClient:
        conn = new TcpConnection(parent);
        break;
    case ConnectionType::TcpServer:
        conn = new TcpServerConnection(parent);
        break;
    case ConnectionType::Udp:
        conn = new UdpConnection(parent);
        break;
    case ConnectionType::Rtt:
        conn = new JLinkRttConnection(parent);
        break;
    case ConnectionType::WebSocket:
        conn = new WebSocketConnection(parent);
        break;
    case ConnectionType::Mqtt:
        conn = new MqttConnection(parent);
        break;
    case ConnectionType::Tls:
        conn = new TlsConnection(parent);
        break;
    case ConnectionType::Ble:
        conn = new BleConnection(parent);
        break;
    case ConnectionType::Can:
        conn = new CanConnection(parent);
        break;
    case ConnectionType::Spi:
        conn = new SpiConnection(parent);
        break;
    case ConnectionType::I2c:
        conn = new I2cConnection(parent);
        break;
    case ConnectionType::Usb:
        conn = new UsbConnection(parent);
        break;
    default:
        qWarning() << "ConnectionFactory: unknown connection type" << static_cast<int>(type);
        ++s_errorCount;
        return nullptr;
    }

    /* 更新统计计数器 */
    ++s_totalCreated;
    ++s_totalByType[type];

    return conn;
}

/** @brief 获取累计创建连接总次数 @return 创建总次数 */
quint64 ConnectionFactory::totalCreated() { return s_totalCreated; }

/** @brief 获取指定连接类型的创建次数 @param type 连接类型 @return 该类型累计创建次数 */
quint64 ConnectionFactory::totalByType(ConnectionType type) { return s_totalByType.value(type, 0); }

/** @brief 获取累计创建失败次数 @return 失败总次数 */
quint64 ConnectionFactory::factoryErrorCount() { return s_errorCount; }

/** @brief 重置工厂统计计数器为初始值 */
void ConnectionFactory::resetFactoryStatistics()
{
    s_totalCreated = 0;
    s_totalByType.clear();
    s_errorCount = 0;
}

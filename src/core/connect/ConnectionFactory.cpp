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

IConnection* ConnectionFactory::create(ConnectionType type, QObject* parent)
{
    switch (type) {
    case ConnectionType::Serial:
        return new SerialConnection(parent);
    case ConnectionType::TcpClient:
        return new TcpConnection(parent);
    case ConnectionType::TcpServer:
        return new TcpServerConnection(parent);
    case ConnectionType::Udp:
        return new UdpConnection(parent);
    case ConnectionType::Rtt:
        return new JLinkRttConnection(parent);
    case ConnectionType::WebSocket:
        return new WebSocketConnection(parent);
    case ConnectionType::Mqtt:
        return new MqttConnection(parent);
    case ConnectionType::Tls:
        return new TlsConnection(parent);
    case ConnectionType::Ble:
        return new BleConnection(parent);
    case ConnectionType::Can:
        return new CanConnection(parent);
    case ConnectionType::Spi:
        return new SpiConnection(parent);
    case ConnectionType::I2c:
        return new I2cConnection(parent);
    case ConnectionType::Usb:
        return new UsbConnection(parent);
    default:
        qWarning() << "ConnectionFactory: unknown connection type" << static_cast<int>(type);
        return nullptr;
    }
}

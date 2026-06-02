/**
 * @file ConnectionFactory.cpp
 * @brief 连接工厂实现 - 根据连接类型创建对应的IConnection子类实例
 *
 * 工厂映射:
 *   Serial    → SerialConnection
 *   TcpClient → TcpConnection (Client模式)
 *   TcpServer → TcpConnection (Server模式，同一类不同配置)
 *   Udp       → UdpConnection
 *   Rtt       → nullptr (尚未实现)
 */

#include "core/connect/ConnectionFactory.h"
#include "connection/interface/IConnection.h"
#include "connection/serial_port/SerialConnection.h"
#include "connection/network/TcpConnection.h"
#include "connection/network/UdpConnection.h"

IConnection* ConnectionFactory::create(ConnectionType type, QObject* parent)
{
    switch (type) {
    case ConnectionType::Serial:
        // 串口连接: 封装QSerialPort，支持全双工通信+信号线控制
        return new SerialConnection(parent);
    case ConnectionType::TcpClient:
        // TCP客户端: 主动连接远程TCP服务器
        return new TcpConnection(parent);
    case ConnectionType::TcpServer:
        // TCP服务器: 本地监听端口等待连接（同一TcpConnection类，通过configure区分模式）
        return new TcpConnection(parent);
    case ConnectionType::Udp:
        // UDP连接: 无连接数据报收发，支持单播/广播
        return new UdpConnection(parent);
    case ConnectionType::Rtt:
        // RTT连接: 尚未实现（依赖J-Link SDK），返回nullptr
        return nullptr;
    default:
        qWarning() << "ConnectionFactory: unknown connection type" << static_cast<int>(type);
        return nullptr;
    }
}

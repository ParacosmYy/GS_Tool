#include "core/ConnectionFactory.h"
#include "connection/IConnection.h"
#include "connection/SerialConnection.h"
#include "connection/TcpConnection.h"
#include "connection/UdpConnection.h"

IConnection* ConnectionFactory::create(ConnectionType type, QObject* parent)
{
    switch (type) {
    case ConnectionType::Serial:
        return new SerialConnection(parent);
    case ConnectionType::TcpClient:
        return new TcpConnection(parent);
    case ConnectionType::TcpServer:
        return new TcpConnection(parent);
    case ConnectionType::Udp:
        return new UdpConnection(parent);
    case ConnectionType::Rtt:
        return nullptr;
    }
    return nullptr;
}

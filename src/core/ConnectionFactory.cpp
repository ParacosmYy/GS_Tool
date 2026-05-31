#include "core/ConnectionFactory.h"
#include "connection/IConnection.h"
#include "connection/SerialConnection.h"

// 根据连接类型创建对应的 IConnection 子类实例
//
// 实现思路:
//   通过 switch-case 匹配 ConnectionType 枚举值，
//   为每种类型构造对应的连接对象。对于尚未实现的类型，
//   返回 nullptr，上层代码应检查返回值。
//
// 内存管理:
//   创建的连接对象通过 QObject 父子树管理生命周期，
//   如果传入 parent，则在 parent 销毁时自动释放。
//   如果 parent 为 nullptr，则调用方需要手动 delete。
IConnection* ConnectionFactory::create(ConnectionType type, QObject* parent)
{
    switch (type) {
    case ConnectionType::Serial:
        // 串口连接 - 已实现
        return new SerialConnection(parent);

    case ConnectionType::TcpClient:
        // TCP 客户端 - 后期实现
        return nullptr;

    case ConnectionType::TcpServer:
        // TCP 服务端 - 后期实现
        return nullptr;

    case ConnectionType::Udp:
        // UDP 连接 - 后期实现
        return nullptr;

    case ConnectionType::Rtt:
        // SEGGER RTT (通过 J-Link) - 后期实现
        return nullptr;
    }

    return nullptr;
}

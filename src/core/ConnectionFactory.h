#ifndef CONNECTION_FACTORY_H
#define CONNECTION_FACTORY_H

#include "core/Constants.h"

class QObject;
class IConnection;

// 连接工厂 - 工厂模式，统一创建各种连接类型
// MainWindow 不需要知道具体连接类的名字
//
// 设计说明:
//   使用简单工厂模式（静态工厂方法），将连接对象的创建逻辑集中在一处。
//   上层（如 MainWindow、ConnectionManager）只需要传入 ConnectionType 枚举，
//   即可获得对应的 IConnection 子类实例，无需了解具体实现类。
//
//   对于尚未实现的连接类型（TCP/UDP/RTT），返回 nullptr，
//   调用方应当检查返回值是否为空。
class ConnectionFactory {
public:
    // 根据类型创建连接对象，返回 nullptr 表示该类型尚未实现
    //
    // 参数:
    //   type   - 连接类型枚举（Serial/TcpClient/TcpServer/Udp/Rtt）
    //   parent - QObject 父对象，用于 Qt 对象树自动管理内存
    //
    // 返回:
    //   IConnection* - 创建成功的连接实例（所有权归调用方或 parent）
    //   nullptr      - 该连接类型尚未实现
    static IConnection* create(ConnectionType type, QObject* parent = nullptr);
};

#endif // CONNECTION_FACTORY_H

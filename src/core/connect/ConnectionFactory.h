/**
 * @file ConnectionFactory.h
 * @brief 连接工厂 - 使用工厂模式统一创建各种连接类型
 *
 * 职责:
 *   1. 根据ConnectionType枚举创建对应的IConnection子类实例
 *   2. 将连接对象的创建逻辑集中在一处，上层无需知道具体实现类
 *   3. 对未实现的连接类型返回nullptr，调用方应检查返回值
 *
 * 设计模式: 简单工厂模式（静态工厂方法）
 *
 * 协作关系:
 *   - ConnectionManager: 通过工厂创建连接实例
 *   - IConnection: 工厂产出的抽象产品接口
 *   - SerialConnection/TcpConnection/UdpConnection: 具体产品类
 */

#ifndef CONNECTION_FACTORY_H
#define CONNECTION_FACTORY_H

#include <QMap>
#include "core/theme/Constants.h"

class QObject;
class IConnection;

/**
 * @brief 连接工厂 - 工厂模式的集中创建点
 *
 * 使用示例:
 * @code
 *   IConnection* conn = ConnectionFactory::create(ConnectionType::Serial);
 *   if (conn) { conn->configure(params); conn->open(); }
 * @endcode
 *
 * 所有创建的连接对象所有权归调用方或parent参数指定的QObject。
 */
class ConnectionFactory {
public:
    /**
     * @brief 根据类型创建连接对象
     *
     * 支持的类型:
     *   - Serial: 创建SerialConnection实例
     *   - TcpClient/TcpServer: 创建TcpConnection实例
     *   - Udp: 创建UdpConnection实例
     *   - Rtt: 尚未实现，返回nullptr
     *
     * @param type 连接类型枚举（Serial/TcpClient/TcpServer/Udp/Rtt）
     * @param parent QObject父对象，用于Qt对象树自动管理内存
     * @return IConnection* 创建成功的连接实例，nullptr表示该类型尚未实现
     */
    static IConnection* create(ConnectionType type, QObject* parent = nullptr);

    // ---- 工厂统计 getter ----

    /** @brief 获取累计创建连接总次数 */
    static quint64 totalCreated();

    /**
     * @brief 获取指定连接类型的创建次数
     * @param type 连接类型
     * @return 该类型累计创建次数
     */
    static quint64 totalByType(ConnectionType type);

    /** @brief 获取累计创建失败次数（类型未实现等） */
    static quint64 factoryErrorCount();

    /** @brief 重置工厂统计计数器为初始值 */
    static void resetFactoryStatistics();

private:
    static quint64 s_totalCreated;                                ///< 累计创建连接总次数
    static QMap<ConnectionType, quint64> s_totalByType;           ///< 各类型创建次数
    static quint64 s_errorCount;                                  ///< 累计创建失败次数
};

#endif // CONNECTION_FACTORY_H

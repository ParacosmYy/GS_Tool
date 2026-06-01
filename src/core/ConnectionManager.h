/**
 * @file ConnectionManager.h
 * @brief 连接管理器 - 统一管理所有连接实例(串口/TCP/UDP)的生命周期
 *
 * 职责:
 *   1. 创建和销毁IConnection实例（通过ConnectionFactory委托创建）
 *   2. 维护活跃连接列表，提供查询接口
 *   3. 在removeConnection时自动调用close()并释放内存
 *
 * 协作关系:
 *   - ConnectionFactory: 委托创建具体连接实例
 *   - ConnectionController: 上层控制器，通过本管理器管理连接
 *   - IConnection: 管理的连接抽象接口
 */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QList>
#include "connection/IConnection.h"

/**
 * @brief 连接管理器 - 统一管理所有IConnection实例
 *
 * 管理连接的完整生命周期：创建 → 配置 → 使用 → 销毁。
 * 所有连接对象的内存由本管理器负责（通过removeConnection释放）。
 *
 * 使用示例:
 * @code
 *   ConnectionManager mgr;
 *   IConnection* conn = mgr.createSerialConnection();
 *   conn->configure(params);
 *   conn->open();
 *   // ... 使用连接 ...
 *   mgr.removeConnection(conn);  // 自动 close + delete
 * @endcode
 */
class ConnectionManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造连接管理器
     * @param parent 父对象
     */
    explicit ConnectionManager(QObject* parent = nullptr);

    /** @brief 析构，关闭并删除所有活跃连接 */
    ~ConnectionManager() override;

    /**
     * @brief 创建指定类型的连接（委托给ConnectionFactory）
     * @param type 连接类型枚举
     * @return 新创建的连接实例，失败返回nullptr
     */
    IConnection* createConnection(ConnectionType type);

    /**
     * @brief 串口便捷方法（内部调用createConnection(Serial)）
     * @return 新创建的串口连接实例
     */
    IConnection* createSerialConnection();

    /**
     * @brief 删除一个连接（自动close + delete）
     * @param conn 要删除的连接指针，从内部列表移除后释放内存
     */
    void removeConnection(IConnection* conn);

    /** @brief 获取所有活跃连接列表 */
    QList<IConnection*> connections() const;

    /**
     * @brief 获取指定索引的连接
     * @param index 连接索引（基于connections()列表顺序）
     * @return 连接指针，索引越界返回nullptr
     */
    IConnection* connection(int index) const;

    /** @brief 获取当前活跃连接数量 */
    int count() const;

private:
    QList<IConnection*> m_connections;  ///< 活跃连接列表
};

#endif // CONNECTIONMANAGER_H

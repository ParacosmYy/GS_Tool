/**
 * @file TcpMultiConnectionManager.h
 * @brief TCP多连接管理器 - 管理多个独立的TCP客户端连接
 *
 * 职责:
 *   1. 创建和管理多个TCP客户端连接
 *   2. 提供统一的发送接口(sendToAll)和状态查询
 *   3. 通过信号通知上层连接变化和数据接收
 *
 * 协作关系:
 *   - MultiConnectionPanel: 提供UI管理界面
 *   - IConnection: 每个连接都是IConnection实例
 */

#ifndef TCPMULTICONNECTIONMANAGER_H
#define TCPMULTICONNECTIONMANAGER_H

#include <QObject>
#include <QMap>

class IConnection;

/**
 * @brief TCP多连接管理器 - 管理多个独立的TCP连接实例
 *
 * 每个连接通过唯一ID标识，支持添加、移除和批量发送。
 */
class TcpMultiConnectionManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
    explicit TcpMultiConnectionManager(QObject* parent = nullptr);

    /** @brief 析构，关闭并释放所有连接 */
    ~TcpMultiConnectionManager();

    /**
     * @brief 添加一个新的TCP连接
     * @param host 目标主机地址
     * @param port 目标端口号
     * @return 新连接的唯一ID，-1表示创建失败
     */
    int addConnection(const QString& host, int port);

    /**
     * @brief 移除指定ID的连接
     * @param id 连接ID
     */
    void removeConnection(int id);

    /**
     * @brief 获取当前连接数量
     * @return 活跃连接数
     */
    int connectionCount() const;

    /**
     * @brief 向所有连接发送数据
     * @param data 待发送的字节数据
     * @return 成功发送的连接数量
     */
    int sendToAll(const QByteArray& data);

signals:
    /** @brief 新连接添加时发出
     * @param id 新连接的唯一ID
     */
    void connectionAdded(int id);

    /** @brief 连接移除时发出
     * @param id 被移除连接的ID
     */
    void connectionRemoved(int id);

    /** @brief 收到某个连接的数据时发出
     * @param id 来源连接的ID
     * @param data 收到的字节数据
     */
    void dataReceived(int id, const QByteArray& data);

private:
    /** @brief 连接数据到达的统一处理槽 */
    void onDataReceived(const QByteArray& data);

    QMap<int, IConnection*> m_connections;  ///< 连接ID到连接实例的映射
    int m_nextId = 0;                       ///< 下一个分配的连接ID
};

#endif // TCPMULTICONNECTIONMANAGER_H

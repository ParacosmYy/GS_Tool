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
 *   - QTcpSocket: 底层TCP客户端socket
 */

#ifndef TCPMULTICONNECTIONMANAGER_H
#define TCPMULTICONNECTIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QTcpSocket>

/**
 * @brief TCP多连接管理器 - 管理多个独立的TCP连接实例
 *
 * 每个连接通过唯一ID标识，支持添加、移除和批量发送。
 * 内部直接管理QTcpSocket，无需依赖IConnection。
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

    /**
     * @brief 获取连接的主机地址
     * @param id 连接ID
     * @return 主机地址字符串，空字符串表示无效ID
     */
    QString connectionHost(int id) const;

    /**
     * @brief 获取连接的端口号
     * @param id 连接ID
     * @return 端口号，-1表示无效ID
     */
    int connectionPort(int id) const;

signals:
    /** @brief 新连接添加时发出
     * @param id 新连接的唯一ID
     * @param host 目标主机
     * @param port 目标端口
     */
    void connectionAdded(int id, const QString& host, int port);

    /** @brief 连接移除时发出
     * @param id 被移除连接的ID
     */
    void connectionRemoved(int id);

    /** @brief 收到某个连接的数据时发出
     * @param id 来源连接的ID
     * @param data 收到的字节数据
     */
    void dataReceived(int id, const QByteArray& data);

    /** @brief 连接错误时发出
     * @param id 出错的连接ID
     * @param errorMsg 错误描述
     */
    void connectionError(int id, const QString& errorMsg);

private slots:
    /** @brief socket数据到达回调 */
    void onReadyRead();

    /** @brief socket断开回调 */
    void onDisconnected();

    /** @brief socket错误回调 */
    void onError(QAbstractSocket::SocketError error);

private:
    /** @brief 查找socket对应的连接ID */
    int idForSocket(QTcpSocket* socket) const;

    QMap<int, QTcpSocket*> m_connections;  ///< 连接ID到socket的映射
    QMap<int, QString> m_hosts;            ///< 连接ID到主机地址映射
    QMap<int, int> m_ports;                ///< 连接ID到端口号映射
    int m_nextId = 1;                      ///< 下一个分配的连接ID
};

#endif // TCPMULTICONNECTIONMANAGER_H

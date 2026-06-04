/**
 * @file TcpMultiConnectionManager.h
 * @brief TCP多连接管理器 - 管理多个独立的TCP客户端连接
 *
 * 职责:
 *   1. 创建和管理多个TCP客户端连接
 *   2. 提供统一的发送接口(sendToAll)和状态查询
 *   3. 通过信号通知上层连接变化和数据接收
 *   4. 跟踪连接/断开/收发/错误的累计统计
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
    /** @brief 构造函数 */
    explicit TcpMultiConnectionManager(QObject* parent = nullptr);

    /** @brief 析构，关闭并释放所有连接 */
    ~TcpMultiConnectionManager();

    /** @brief 添加一个新的TCP连接 @param host 目标主机地址 @param port 目标端口号 @return 新连接的唯一ID，-1表示失败 */
    int addConnection(const QString& host, int port);

    /** @brief 移除指定ID的连接 @param id 连接唯一ID */
    void removeConnection(int id);

    /** @brief 获取当前连接数量 @return 活跃连接数 */
    int connectionCount() const;

    /** @brief 向所有连接发送数据 @param data 待发送字节数据 @return 成功发送的连接数量 */
    int sendToAll(const QByteArray& data);

    /** @brief 获取连接的主机地址 @param id 连接唯一ID @return 主机地址字符串 */
    QString connectionHost(int id) const;

    /** @brief 获取连接的端口号 @param id 连接唯一ID @return 端口号 */
    int connectionPort(int id) const;

    // ---- 统计接口 ----

    /** @brief 获取累计连接次数(含成功和失败) @return 连接尝试总数 */
    quint64 totalConnections() const { return m_totalConnections; }

    /** @brief 获取累计断开次数 @return 断开总数 */
    quint64 totalDisconnections() const { return m_totalDisconnections; }

    /** @brief 获取累计发送字节数 @return 发送字节总量 */
    quint64 totalBytesSent() const { return m_totalBytesSent; }

    /** @brief 获取累计接收字节数 @return 接收字节总量 */
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }

    /** @brief 获取累计错误次数(含连接错误和socket错误) @return 错误总数 */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 获取累计sendToAll()调用次数 @return 写入调用总数 */
    quint64 totalWrites() const { return m_totalWrites; }

    /** @brief 获取历史同时在线连接数峰值 @return 峰值连接数 */
    quint64 peakConnections() const { return m_peakConnections; }

    /** @brief 获取累计连接失败次数(DNS/超时/拒绝等) @return 连接失败总数 */
    quint64 totalConnectionFailures() const { return m_totalConnectionFailures; }

    /** @brief 重置所有统计计数器(连接/断开/字节/错误/写入/峰值/失败归零) */
    void resetConnectionStatistics();

signals:
    /** @brief 新连接添加时发出 */
    void connectionAdded(int id, const QString& host, int port);
    /** @brief 连接移除时发出 */
    void connectionRemoved(int id);
    /** @brief 收到某个连接的数据时发出 */
    void dataReceived(int id, const QByteArray& data);
    /** @brief 连接错误时发出 */
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

    quint64 m_totalConnections = 0;        ///< 累计连接次数(含成功和失败)
    quint64 m_totalDisconnections = 0;     ///< 累计断开次数
    quint64 m_totalBytesSent = 0;          ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;      ///< 累计接收字节数
    quint64 m_errorCount = 0;              ///< 累计错误次数(含连接错误和socket错误)
    quint64 m_totalWrites = 0;             ///< 累计sendToAll()调用次数
    quint64 m_peakConnections = 0;         ///< 历史同时在线连接数峰值
    quint64 m_totalConnectionFailures = 0; ///< 累计连接失败次数
};

#endif // TCPMULTICONNECTIONMANAGER_H

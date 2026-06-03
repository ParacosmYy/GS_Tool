/**
 * @file TcpServerConnection.h
 * @brief TCP服务器模式连接 - 支持多客户端同时连接的TCP服务端
 *
 * 职责:
 *   1. 在本地端口监听，接受多个远程TCP客户端连接
 *   2. 管理已连接客户端列表，支持广播数据到所有客户端
 *   3. 复用IConnection抽象接口，上层无需关心连接类型
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建TcpServerConnection实例
 *   - MultiConnectionPanel: 提供多连接管理的UI界面
 */

#ifndef TCPSERVERCONNECTION_H
#define TCPSERVERCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

/**
 * @brief TCP服务器模式连接 - 支持多客户端并发连接
 *
 * 服务器监听指定端口，接受多个客户端连接。
 * 每个客户端独立收发数据，支持广播和单播。
 */
class TcpServerConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造TCP服务器连接
     * @param parent 父对象，用于QObject生命周期管理
     */
    explicit TcpServerConnection(QObject* parent = nullptr);

    /** @brief 析构，停止监听并释放资源 */
    ~TcpServerConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- TCP Server特有接口 ----

    /**
     * @brief 开始监听指定地址和端口
     * @param address 监听地址(QHostAddress::Any表示所有接口)
     * @param port 监听端口号
     * @return true=监听成功，false=监听失败(端口被占用等)
     */
    bool listen(const QHostAddress& address, int port);

    /** @brief 停止监听，断开所有客户端 */
    void stopListening();

    /**
     * @brief 获取已连接的客户端列表
     * @return 客户端地址:端口字符串列表
     */
    QStringList connectedClients() const;

    /**
     * @brief 向所有已连接客户端广播数据
     * @param data 待广播的字节数据
     * @return 成功发送的客户端数量
     */
    int broadcastToClients(const QByteArray& data);

    /** @brief 获取历史累计连接客户端总数 */
    quint64 totalClientCount() const;

    /** @brief 获取历史累计断开客户端总数 */
    quint64 totalClientDisconnections() const;

    /** @brief 获取已广播数据包总数 */
    quint64 broadcastCount() const;

    /** @brief 获取累计接收字节数 */
    quint64 totalBytesReceived() const;

    /** @brief 获取累计发送字节数 */
    quint64 totalBytesSent() const;

    /** @brief 获取累计accept错误次数 */
    quint64 totalAcceptErrors() const;

    /** @brief 重置所有统计计数器为零 */
    void resetStatistics();

signals:
    /** @brief 新客户端连接时发出
     * @param clientInfo 客户端地址:端口信息
     */
    void clientConnected(const QString& clientInfo);

    /** @brief 客户端断开时发出
     * @param clientInfo 客户端地址:端口信息
     */
    void clientDisconnected(const QString& clientInfo);

    /** @brief 收到特定客户端数据时发出
     * @param clientInfo 客户端地址:端口信息
     * @param data 收到的字节数据
     */
    void clientData(const QString& clientInfo, const QByteArray& data);

private slots:
    /** @brief 新客户端连接回调 */
    void onNewConnection();

    /** @brief 客户端断开回调 */
    void onClientDisconnected();

    /** @brief 客户端数据到达回调 */
    void onClientReadyRead();

private:
    /** @brief 更新连接状态并发射stateChanged信号 */
    void updateState(ConnectionState newState);

    /** @brief 获取socket对应的客户端标识字符串 */
    static QString clientInfo(QTcpSocket* socket);

    // ---- 配置参数 ----
    QHostAddress m_listenAddress;                    ///< 监听地址
    quint16 m_listenPort = 0;                        ///< 监听端口号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 网络资源 ----
    QTcpServer* m_server = nullptr;                  ///< TCP服务器
    QMap<qintptr, QTcpSocket*> m_clients;            ///< 客户端socket映射(socket描述符→socket)
    bool m_listening = false;                        ///< 是否正在监听

    // ---- 统计计数器 ----
    quint64 m_totalClientCount = 0;           ///< 累计连接客户端总数
    quint64 m_totalClientDisconnections = 0;  ///< 累计断开客户端总数
    quint64 m_broadcastCount = 0;             ///< 广播数据包计数
    quint64 m_totalRxBytes = 0;               ///< 累计接收字节数
    quint64 m_totalTxBytes = 0;               ///< 累计发送字节数
    quint64 m_totalAcceptErrors = 0;          ///< 累计accept错误次数
};

#endif // TCPSERVERCONNECTION_H

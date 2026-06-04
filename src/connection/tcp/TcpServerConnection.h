/**
 * @file TcpServerConnection.h
 * @brief TCP服务器模式连接 - 支持多客户端同时连接的TCP服务端
 *
 * 职责: 本地端口监听/多客户端管理/广播数据/复用IConnection接口
 * 协作: ConnectionFactory(创建) / MultiConnectionPanel(多连接UI)
 */
#ifndef TCPSERVERCONNECTION_H
#define TCPSERVERCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

/** @brief TCP服务器模式连接 - 支持多客户端并发连接 */
class TcpServerConnection : public IConnection {
    Q_OBJECT

public:
    explicit TcpServerConnection(QObject* parent = nullptr); ///< 构造TCP服务器连接
    ~TcpServerConnection() override;                         ///< 析构，停止监听并释放资源
    // ---- IConnection接口 ----
    ConnectionType type() const override;              ///< 返回连接类型(TcpServer)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 开始监听
    void close() override;                             ///< 停止监听并断开所有客户端
    qint64 write(const QByteArray& data) override;     ///< 广播数据到所有客户端
    void configure(const QVariantMap& params) override; ///< 配置TCP服务器参数
    // ---- TCP Server特有接口 ----
    /** @brief 开始监听指定地址和端口 @param address 监听地址 @param port 监听端口号 @return true=监听成功 */
    bool listen(const QHostAddress& address, int port);
    void stopListening();                              ///< 停止监听，断开所有客户端
    QStringList connectedClients() const;              ///< 获取已连接的客户端列表(地址:端口)
    /** @brief 向所有已连接客户端广播数据 @param data 待广播的字节数据 @return 成功发送的客户端数量 */
    int broadcastToClients(const QByteArray& data);
    // ---- 统计接口 ----
    quint64 totalClientCount() const;                  ///< 获取历史累计连接客户端总数
    quint64 totalClientDisconnections() const;         ///< 获取历史累计断开客户端总数
    quint64 broadcastCount() const;                    ///< 获取已广播数据包总数
    quint64 totalBytesReceived() const;                ///< 获取累计接收字节数
    quint64 totalBytesSent() const;                    ///< 获取累计发送字节数
    quint64 totalAcceptErrors() const;                 ///< 获取累计accept错误次数
    quint64 totalListenAttempts() const { return m_totalListenAttempts; } ///< 获取累计监听尝试次数
    quint64 totalWrites() const { return m_totalWrites; } ///< 获取累计write调用次数
    quint64 totalRejectedConnections() const { return m_totalRejectedConnections; } ///< 获取被拒绝连接数
    quint64 peakConnectedClients() const { return m_peakConnectedClients; } ///< 获取同时在线客户端峰值
    quint64 totalErrors() const { return m_totalErrors; } ///< 获取累计客户端socket错误次数
    quint64 totalUnicastSends() const { return m_totalUnicastSends; } ///< 获取累计单播发送次数
    quint64 totalFailedSends() const { return m_totalFailedSends; } ///< 获取累计发送失败次数
    void setMaxClients(int max) { m_maxClients = max; } ///< 设置最大允许同时连接的客户端数
    void resetStatistics();                            ///< 重置所有统计计数器

signals:
    void clientConnected(const QString& clientInfo);    ///< 新客户端连接
    void clientDisconnected(const QString& clientInfo); ///< 客户端断开
    /** @brief 收到特定客户端数据 @param clientInfo 客户端地址:端口 @param data 收到的字节数据 */
    void clientData(const QString& clientInfo, const QByteArray& data);

private slots:
    void onNewConnection();     ///< 新客户端连接回调
    void onClientDisconnected();///< 客户端断开回调
    void onClientReadyRead();   ///< 客户端数据到达回调

private:
    void updateState(ConnectionState newState); ///< 更新连接状态并发射stateChanged信号
    static QString clientInfo(QTcpSocket* socket); ///< 获取socket对应的客户端标识字符串
    // ---- 配置参数 ----
    QHostAddress m_listenAddress;                    ///< 监听地址
    quint16 m_listenPort = 0;                        ///< 监听端口号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态
    // ---- 网络资源 ----
    QTcpServer* m_server = nullptr;                  ///< TCP服务器
    QMap<qintptr, QTcpSocket*> m_clients;            ///< 客户端socket映射
    bool m_listening = false;                        ///< 是否正在监听
    // ---- 统计计数器 ----
    quint64 m_totalClientCount = 0, m_totalClientDisconnections = 0;
    quint64 m_broadcastCount = 0;
    quint64 m_totalRxBytes = 0, m_totalTxBytes = 0;
    quint64 m_totalAcceptErrors = 0, m_totalListenAttempts = 0, m_totalWrites = 0;
    quint64 m_totalRejectedConnections = 0, m_peakConnectedClients = 0;
    quint64 m_totalErrors = 0, m_totalUnicastSends = 0, m_totalFailedSends = 0;
    int m_maxClients = 0;                     ///< 最大允许同时连接的客户端数(0=不限)
};

#endif // TCPSERVERCONNECTION_H

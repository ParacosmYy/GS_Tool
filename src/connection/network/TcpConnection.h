/** @file TcpConnection.h @brief TCP连接实现 - 支持Client/Server双模式。职责: Client模式(主动连接远程TCP服务器) + Server模式(本地监听等待连接)。复用IConnection抽象接口。设计模式: 策略模式。协作: ConnectionFactory/ConnectionController/OtaManager */

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QElapsedTimer>

/** @brief TCP连接实现 - 支持Client/Server双模式。Client模式: 主动连接远程TCP服务器(超时10秒)。Server模式: 本地监听端口。自动TCP KeepAlive，转发bytesWritten信号 */
class TcpConnection : public IConnection {
    Q_OBJECT

public:
    enum Mode { Client, Server }; ///< TCP连接工作模式(Client主动连接/Server本地监听)

    explicit TcpConnection(QObject* parent = nullptr); ///< 构造TCP连接
    ~TcpConnection() override;                         ///< 析构，关闭连接并释放资源
    ConnectionType type() const override;              ///< 返回连接类型(TCP)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 打开TCP连接(Client发起连接/Server开始监听)
    void close() override;                             ///< 关闭TCP连接，释放socket资源
    qint64 write(const QByteArray& data) override;     ///< 发送数据到对端，返回实际写入字节数(-1=失败)
    void configure(const QVariantMap& params) override; ///< 配置TCP参数(mode/host/port)

    // ---- 统计计数器接口 ----
    quint64 totalConnections() const;          ///< 累计连接成功次数
    quint64 totalDisconnections() const;       ///< 累计断开连接次数
    quint64 totalBytesSent() const;            ///< 累计发送字节数
    quint64 totalBytesReceived() const;        ///< 累计接收字节数
    quint64 errorCount() const;                ///< 累计错误次数
    quint64 totalOpenAttempts() const { return m_totalOpenAttempts; } ///< 累计open()调用次数
    quint64 totalWrites() const { return m_totalWrites; }            ///< 累计write()调用次数
    quint64 totalReconnectAttempts() const { return m_totalReconnectAttempts; } ///< 累计重连尝试次数
    quint64 totalReconnects() const { return m_totalReconnects; }   ///< 累计重连成功次数
    quint64 totalDnsLookups() const { return m_totalDnsLookups; }   ///< 累计DNS查询次数
    quint64 totalDnsErrors() const { return m_totalDnsErrors; }     ///< 累计DNS解析失败次数
    quint64 totalConnectionTimeouts() const { return m_totalConnectionTimeouts; } ///< 累计连接超时次数
    quint64 totalKeepAliveProbes() const { return m_totalKeepAliveProbes; } ///< 累计KeepAlive探测次数
    double averageLatencyMs() const;           ///< 连接建立平均延迟(ms)
    qint64 lastLatencyMs() const { return m_lastLatencyMs; }     ///< 最近一次连接延迟(ms)
    qint64 maxLatencyMs() const { return m_maxLatencyMs; }       ///< 最大连接延迟(ms)
    void resetStats();                         ///< 重置所有统计计数器

private slots:
    void onSocketConnected();                  ///< Client模式: 连接成功回调
    void onSocketDisconnected();               ///< 连接断开回调
    void onSocketReadyRead();                  ///< 数据到达回调，转发为dataReceived信号
    void onSocketError(QAbstractSocket::SocketError error); ///< 网络错误回调(翻译为中文描述)
    void onNewConnection();                    ///< Server模式: 新客户端连接回调

private:
    static QString translateNetworkError(QAbstractSocket::SocketError error, const QString& systemError); ///< Qt错误码->中文描述
    void updateState(ConnectionState newState); ///< 更新连接状态并发射stateChanged信号

    // ---- 配置参数 ----
    Mode m_mode = Client;                ///< 连接工作模式
    QString m_host;                      ///< 目标主机地址(Client模式)
    quint16 m_port = 0;                  ///< 目标/监听端口号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态

    // ---- 网络资源 ----
    QTcpSocket* m_socket = nullptr;      ///< Client模式: 与服务器的通信socket
    QTcpServer* m_server = nullptr;      ///< Server模式: 本地监听服务器
    QTcpSocket* m_clientSocket = nullptr; ///< Server模式: 已接受的客户端连接
    QTimer* m_connectTimer = nullptr;    ///< Client模式连接超时定时器(10秒)

    // ---- 延迟追踪 ----
    QElapsedTimer m_connectStartTime;    ///< 连接发起时刻
    qint64 m_lastLatencyMs = 0;          ///< 最近一次连接建立延迟(ms)
    qint64 m_maxLatencyMs = 0;           ///< 最大连接建立延迟(ms)
    quint64 m_latencySampleCount = 0;    ///< 延迟采样次数
    qint64 m_latencySumMs = 0;           ///< 延迟累计总和(ms)
    bool m_isReconnectAttempt = false;   ///< 标记当前连接是否为重连尝试

    // ---- 统计计数器 ----
    quint64 m_totalConnections = 0;      ///< 累计连接成功次数
    quint64 m_totalDisconnections = 0;   ///< 累计断开连接次数
    quint64 m_totalBytesSent = 0;        ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;    ///< 累计接收字节数
    quint64 m_errorCount = 0;            ///< 累计错误次数
    quint64 m_totalOpenAttempts = 0;     ///< 累计open()调用次数
    quint64 m_totalWrites = 0;           ///< 累计write()调用次数
    quint64 m_totalReconnectAttempts = 0;///< 累计重连尝试次数
    quint64 m_totalReconnects = 0;       ///< 累计重连成功次数
    quint64 m_totalDnsLookups = 0;       ///< 累计DNS查询次数
    quint64 m_totalDnsErrors = 0;        ///< 累计DNS解析失败次数
    quint64 m_totalConnectionTimeouts = 0;///< 累计连接超时次数
    quint64 m_totalKeepAliveProbes = 0;  ///< 累计KeepAlive探测次数
};

#endif // TCPCONNECTION_H

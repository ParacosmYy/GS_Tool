/**
 * @file TcpConnection.h
 * @brief TCP连接实现 - 支持Client/Server双模式的TCP网络连接
 *
 * 职责: Client模式(主动连接远程TCP服务器) + Server模式(本地监听等待连接)
 * 复用IConnection抽象接口，上层无需关心连接类型
 *
 * 设计模式: 策略模式 — TcpConnection是IConnection的一个具体策略实现
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建TcpConnection实例
 *   - ConnectionController: 管理连接生命周期、状态分发
 *   - OtaManager: 通过write()发送OTA固件数据
 */

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QElapsedTimer>

/**
 * @brief TCP连接实现 - 支持Client/Server双模式
 *
 * Client模式: 主动连接远程TCP服务器，支持连接超时检测(10秒)
 * Server模式: 本地监听端口，接受第一个客户端连接
 *
 * 网络特性:
 *   - 自动启用TCP KeepAlive，及时检测对端断开
 *   - 转发bytesWritten信号，供上层OTA进度追踪和发送统计
 *   - 写入失败时通过errorOccurred信号报告错误
 */
class TcpConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief TCP连接工作模式 */
    enum Mode {
        Client, ///< 客户端模式: 主动连接远程服务器
        Server  ///< 服务器模式: 本地监听等待连接
    };

    /** @brief 构造TCP连接
     * @param parent 父对象，用于QObject生命周期管理
     */
    explicit TcpConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接并释放资源 */
    ~TcpConnection() override;

    /** @brief 返回连接类型(TCP) */
    ConnectionType type() const override;

    /** @brief 返回连接显示名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开TCP连接(Client模式发起连接，Server模式开始监听)
     * @return true=操作已启动(异步完成)，false=参数错误
     */
    bool open() override;

    /** @brief 关闭TCP连接，释放socket资源 */
    void close() override;

    /** @brief 发送数据到对端
     * @param data 待发送的字节数据
     * @return 实际写入字节数，-1表示连接未就绪或写入失败
     *
     * 写入失败时通过errorOccurred信号报告错误详情
     */
    qint64 write(const QByteArray& data) override;

    /** @brief 配置TCP连接参数
     * @param params 参数映射:
     *   - "mode": int (0=Client, 1=Server)
     *   - "host": QString (目标主机地址，仅Client模式)
     *   - "port": int (目标/监听端口号)
     */
    void configure(const QVariantMap& params) override;

    // ---- 统计计数器接口 ----

    /** @brief 获取累计连接成功次数 @return 连接成功次数 */
    quint64 totalConnections() const;

    /** @brief 获取累计断开连接次数 @return 断开次数 */
    quint64 totalDisconnections() const;

    /** @brief 获取累计发送字节数 @return 发送字节数 */
    quint64 totalBytesSent() const;

    /** @brief 获取累计接收字节数 @return 接收字节数 */
    quint64 totalBytesReceived() const;

    /** @brief 获取累计错误次数 @return 错误次数 */
    quint64 errorCount() const;

    /** @brief 获取累计open()调用次数 @return 连接尝试总次数 */
    quint64 totalOpenAttempts() const { return m_totalOpenAttempts; }

    /** @brief 获取累计write()调用次数 @return 写入调用总次数 */
    quint64 totalWrites() const { return m_totalWrites; }

    /** @brief 获取累计重连尝试次数(Client模式下已连接/连接中时再次调用open) @return 重连尝试次数 */
    quint64 totalReconnectAttempts() const { return m_totalReconnectAttempts; }

    /** @brief 获取累计重连成功次数(重连后onSocketConnected回调触发时计数) @return 重连成功总次数 */
    quint64 totalReconnects() const { return m_totalReconnects; }

    /** @brief 获取累计DNS查询次数(每次connectToHost触发一次) @return DNS查询总次数 */
    quint64 totalDnsLookups() const { return m_totalDnsLookups; }

    /** @brief 获取累计DNS解析失败次数(HostNotFoundError时计数) @return DNS错误总次数 */
    quint64 totalDnsErrors() const { return m_totalDnsErrors; }

    /** @brief 获取累计KeepAlive探测次数(连接成功后设置KeepAlive时计数) @return KeepAlive探测总次数 */
    quint64 totalKeepAliveProbes() const { return m_totalKeepAliveProbes; }

    /** @brief 获取连接建立平均延迟(毫秒，从发起connect到connected回调) @return 平均延迟，无数据时返回0 */
    double averageLatencyMs() const;

    /** @brief 获取最近一次连接建立延迟(毫秒) @return 最近延迟，无数据时返回0 */
    qint64 lastLatencyMs() const { return m_lastLatencyMs; }

    /** @brief 获取最大连接建立延迟(毫秒) @return 最大延迟，无数据时返回0 */
    qint64 maxLatencyMs() const { return m_maxLatencyMs; }

    /** @brief 重置所有统计计数器为零 */
    void resetStats();

private slots:
    /** @brief Client模式: 连接成功回调 */
    void onSocketConnected();

    /** @brief 连接断开回调 */
    void onSocketDisconnected();

    /** @brief 数据到达回调，转发为dataReceived信号 */
    void onSocketReadyRead();

    /** @brief 网络错误回调，翻译为中文描述并转发errorOccurred */
    void onSocketError(QAbstractSocket::SocketError error);

    /** @brief Server模式: 新客户端连接回调 */
    void onNewConnection();

private:
    /** @brief 将Qt网络错误码映射为中文描述
     * @param error Qt套接字错误码
     * @param systemError 系统错误描述字符串
     * @return 用户可读的中文错误描述
     */
    static QString translateNetworkError(QAbstractSocket::SocketError error,
                                         const QString& systemError);

    /** @brief 更新连接状态并发射stateChanged信号 */
    void updateState(ConnectionState newState);

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
    QElapsedTimer m_connectStartTime;    ///< 连接发起时刻，用于计算连接建立延迟
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
    quint64 m_totalKeepAliveProbes = 0;  ///< 累计KeepAlive探测次数
};

#endif // TCPCONNECTION_H

/** @file WebSocketConnection.h @brief WebSocket客户端连接 - 基于QTcpSocket实现RFC 6455帧协议
 * 支持: 文本帧/二进制帧收发, 心跳ping/pong, 消息队列, 复用IConnection接口
 * 协作: ConnectionFactory(创建) / WsConfigPanel(配置UI) */

#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>
#include <QElapsedTimer>
#include <QQueue>

/** @brief WebSocket客户端连接实现 — 基于QTcpSocket手工实现RFC 6455帧协议，内置心跳定时器 */
class WebSocketConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造WebSocket连接 @param parent 父对象 */
    explicit WebSocketConnection(QObject* parent = nullptr);
    /** @brief 析构，关闭连接并释放资源 */
    ~WebSocketConnection() override;

    // ---- IConnection接口 ----
    ConnectionType type() const override;              ///< 返回连接类型(WebSocket)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 打开WebSocket连接
    void close() override;                             ///< 关闭WebSocket连接
    qint64 write(const QByteArray& data) override;     ///< 发送二进制数据，返回实际写入字节数
    void configure(const QVariantMap& params) override; ///< 配置WebSocket参数(url/protocol等)

    // ---- WebSocket特有接口 ----
    /** @brief 连接到ws://或wss://地址 @param url WebSocket URL @return true=连接发起成功 */
    bool connectToUrl(const QString& url);
    /** @brief 发送文本消息 @param message 文本内容 @return 实际发送字节数 */
    qint64 sendTextMessage(const QString& message);
    /** @brief 发送二进制消息 @param data 二进制数据 @return 实际发送字节数 */
    qint64 sendBinaryMessage(const QByteArray& data);
    /** @brief 发送ping帧 @param payload ping载荷 @return true=发送成功 */
    bool ping(const QByteArray& payload = QByteArray());

    // ---- 统计接口 ----
    /** @brief 获取累计WebSocket连接成功次数 */
    quint64 totalConnections() const;
    /** @brief 获取已发送消息总数(文本+二进制) */
    quint64 totalMessagesSent() const;
    /** @brief 获取已接收消息总数(文本+二进制) */
    quint64 totalMessagesReceived() const;
    /** @brief 获取已发送字节总数(帧级别) */
    quint64 totalBytesSent() const;
    /** @brief 获取已接收字节总数(帧级别) */
    quint64 totalBytesReceived() const;
    /** @brief 获取错误计数 */
    quint64 errorCount() const;
    /** @brief 获取已发送帧总数(含所有opcode) */
    quint64 totalFramesSent() const;
    /** @brief 获取已接收帧总数(含所有opcode) */
    quint64 totalFramesReceived() const;
    /** @brief 获取已发送文本帧总数 */
    quint64 totalTextFrames() const;
    /** @brief 获取已接收二进制帧总数 */
    quint64 totalBinaryFrames() const;
    /** @brief 获取已发送ping帧总数 */
    quint64 totalPingFrames() const;
    /** @brief 获取已接收pong帧总数 */
    quint64 totalPongFrames() const;
    /** @brief 获取已接收分片消息总数 */
    quint64 totalFragmentedMessages() const;
    /** @brief 获取累计握手尝试次数 */
    quint64 totalHandshakeAttempts() const { return m_totalHandshakeAttempts; }
    /** @brief 获取累计握手失败次数 */
    quint64 totalHandshakeFailures() const { return m_totalHandshakeFailures; }
    /** @brief 获取累计发送close帧次数 */
    quint64 totalCloseFramesSent() const { return m_totalCloseFramesSent; }
    /** @brief 获取累计接收close帧次数 */
    quint64 totalCloseFramesReceived() const { return m_totalCloseFramesReceived; }
    /** @brief 获取ping/pong交互总次数 */
    quint64 pingPongCount() const;
    /** @brief 获取ping/pong平均延迟(ms) */
    double averageLatencyMs() const;
    /** @brief 获取ping/pong最大延迟(ms) */
    qint64 maxLatencyMs() const;
    /** @brief 获取当前连接运行时长(秒) */
    qint64 connectionUptimeSeconds() const;
    /** @brief 获取当前消息队列大小 */
    int messageQueueSize() const;
    /** @brief 获取消息队列容量上限 */
    int messageQueueLimit() const;
    /** @brief 设置队列容量上限(<=0不限) @param limit 队列上限 */
    void setMessageQueueLimit(int limit);
    /** @brief 获取因队列满而丢弃的消息数 */
    quint64 totalMessagesDropped() const;
    /** @brief 重置所有统计 */
    void resetStats();

signals:
    /** @brief 收到文本消息 @param message 文本内容 */
    void textMessageReceived(const QString& message);
    /** @brief 收到二进制消息 @param data 二进制数据 */
    void binaryMessageReceived(const QByteArray& data);
    /** @brief 收到pong响应 @param payload pong载荷 */
    void pongReceived(const QByteArray& payload);

private slots:
    /** @brief TCP连接成功回调 */
    void onTcpConnected();
    /** @brief TCP断开回调 */
    void onTcpDisconnected();
    /** @brief TCP数据就绪(解析WebSocket帧) */
    void onTcpReadyRead();
    /** @brief 心跳定时器触发 */
    void onPingTimeout();

private:
    /** @brief 更新连接状态 @param newState 新状态 */
    void updateState(ConnectionState newState);
    /** @brief 构建WebSocket帧 @param opcode 帧操作码 @param payload 帧载荷 @return 编码后的帧数据 */
    QByteArray buildFrame(quint8 opcode, const QByteArray& payload) const;
    /** @brief 解析缓冲区中的WebSocket帧 */
    void parseFrames();
    /** @brief 发送HTTP Upgrade握手 */
    void sendHandshake();
    /** @brief 检查握手响应是否完整 @return true=握手成功 */
    bool parseHandshakeResponse();

    // ---- 配置 ----
    QString m_url;                        ///< WebSocket URL(ws://或wss://)
    QString m_protocol;                   ///< 子协议名称
    QString m_host;                       ///< 目标主机地址
    quint16 m_port = 80;                  ///< 目标端口号
    QString m_path;                       ///< URL路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态

    // ---- 网络 ----
    QTcpSocket* m_socket = nullptr;       ///< TCP通信socket
    QTimer* m_pingTimer = nullptr;        ///< 心跳ping定时器
    QByteArray m_buffer;                  ///< 帧接收缓冲区
    QString m_handshakeKey;               ///< 握手随机密钥
    bool m_handshakeDone = false;         ///< 握手是否已完成

    // ---- 延迟 ----
    QElapsedTimer m_pingSendTime;         ///< ping发送时刻
    qint64 m_lastLatencyMs = 0;           ///< 最近一次ping/pong延迟(ms)
    qint64 m_maxLatencyMs = 0;            ///< 最大ping/pong延迟(ms)
    quint64 m_latencySampleCount = 0;     ///< 延迟采样次数
    qint64 m_latencySumMs = 0;            ///< 延迟累计总和(ms)
    QElapsedTimer m_connectionTimer;      ///< 连接运行计时器

    // ---- 消息队列 ----
    struct QueuedMessage { bool isText = false; QByteArray data; }; ///< 队列消息结构
    QQueue<QueuedMessage> m_sendQueue;   ///< 待发送消息队列
    int m_queueLimit = 1000;              ///< 队列容量上限
    quint64 m_totalMessagesDropped = 0;   ///< 因队列满丢弃的消息数

    // ---- 统计 ----
    quint64 m_totalConnections = 0;       ///< 累计连接成功次数
    quint64 m_totalMessagesSent = 0;      ///< 累计发送消息数
    quint64 m_totalMessagesReceived = 0;  ///< 累计接收消息数
    quint64 m_totalBytesSent = 0;         ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;     ///< 累计接收字节数
    quint64 m_errorCount = 0;             ///< 错误计数
    quint64 m_totalFramesSent = 0;        ///< 累计发送帧数
    quint64 m_totalFramesReceived = 0;    ///< 累计接收帧数
    quint64 m_totalTextFrames = 0;        ///< 累计文本帧数
    quint64 m_totalBinaryFrames = 0;      ///< 累计二进制帧数
    quint64 m_totalPingFrames = 0;        ///< 累计ping帧数
    quint64 m_totalPongFrames = 0;        ///< 累计pong帧数
    quint64 m_totalFragmentedMessages = 0; ///< 已接收分片消息(continuation帧)总数
    quint64 m_totalHandshakeAttempts = 0;   ///< 累计HTTP Upgrade握手尝试次数
    quint64 m_totalHandshakeFailures = 0;   ///< 累计握手失败次数(非101响应)
    quint64 m_totalCloseFramesSent = 0;     ///< 累计发送close帧(0x08)次数
    quint64 m_totalCloseFramesReceived = 0; ///< 累计接收close帧(0x08)次数
};

#endif // WEBSOCKETCONNECTION_H

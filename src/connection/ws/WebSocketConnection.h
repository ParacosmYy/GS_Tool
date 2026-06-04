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
    explicit WebSocketConnection(QObject* parent = nullptr);    ///< 构造函数
    ~WebSocketConnection() override;                           ///< 析构，关闭连接

    // ---- IConnection接口 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- WebSocket特有接口 ----
    bool connectToUrl(const QString& url);                     ///< 连接到ws://或wss://地址
    qint64 sendTextMessage(const QString& message);            ///< 发送文本消息
    qint64 sendBinaryMessage(const QByteArray& data);          ///< 发送二进制消息
    bool ping(const QByteArray& payload = QByteArray());       ///< 发送ping帧

    // ---- 统计接口 ----
    quint64 totalConnections() const;                           ///< 累计WebSocket连接成功次数
    quint64 totalMessagesSent() const;                          ///< 已发送消息总数(文本+二进制)
    quint64 totalMessagesReceived() const;                      ///< 已接收消息总数(文本+二进制)
    quint64 totalBytesSent() const;                             ///< 已发送字节总数(帧级别)
    quint64 totalBytesReceived() const;                         ///< 已接收字节总数(帧级别)
    quint64 errorCount() const;                                 ///< 错误计数
    quint64 totalFramesSent() const;                            ///< 已发送帧总数(含所有opcode)
    quint64 totalFramesReceived() const;                        ///< 已接收帧总数(含所有opcode)
    quint64 totalTextFrames() const;                            ///< 已发送文本帧总数
    quint64 totalBinaryFrames() const;                          ///< 已接收二进制帧总数
    quint64 totalPingFrames() const;                            ///< 已发送ping帧总数
    quint64 totalPongFrames() const;                            ///< 已接收pong帧总数
    quint64 totalFragmentedMessages() const;                    ///< 已接收分片消息总数
    quint64 totalHandshakeAttempts() const { return m_totalHandshakeAttempts; } ///< 累计握手尝试次数
    quint64 totalHandshakeFailures() const { return m_totalHandshakeFailures; } ///< 累计握手失败次数
    quint64 totalCloseFramesSent() const { return m_totalCloseFramesSent; }     ///< 累计发送close帧次数
    quint64 totalCloseFramesReceived() const { return m_totalCloseFramesReceived; } ///< 累计接收close帧次数
    quint64 pingPongCount() const;                              ///< ping/pong交互总次数
    double averageLatencyMs() const;                            ///< ping/pong平均延迟(ms)
    qint64 maxLatencyMs() const;                                ///< ping/pong最大延迟(ms)
    qint64 connectionUptimeSeconds() const;                     ///< 当前连接运行时长(秒)
    int messageQueueSize() const;                               ///< 当前消息队列大小
    int messageQueueLimit() const;                              ///< 消息队列容量上限
    void setMessageQueueLimit(int limit);                       ///< 设置队列容量上限(≤0不限)
    quint64 totalMessagesDropped() const;                       ///< 因队列满而丢弃的消息数
    void resetStats();                                          ///< 重置所有统计

signals:
    void textMessageReceived(const QString& message);          ///< 收到文本消息
    void binaryMessageReceived(const QByteArray& data);        ///< 收到二进制消息
    void pongReceived(const QByteArray& payload);              ///< 收到pong响应

private slots:
    void onTcpConnected();                                     ///< TCP连接成功回调
    void onTcpDisconnected();                                  ///< TCP断开回调
    void onTcpReadyRead();                                     ///< TCP数据就绪(解析WebSocket帧)
    void onPingTimeout();                                      ///< 心跳定时器触发

private:
    void updateState(ConnectionState newState);                ///< 更新连接状态
    QByteArray buildFrame(quint8 opcode, const QByteArray& payload) const; ///< 构建WebSocket帧
    void parseFrames();                                        ///< 解析缓冲区中的WebSocket帧
    void sendHandshake();                                      ///< 发送HTTP Upgrade握手
    bool parseHandshakeResponse();                             ///< 检查握手响应是否完整

    // ---- 配置 ----
    QString m_url; QString m_protocol; QString m_host; quint16 m_port = 80; QString m_path;
    ConnectionState m_state = ConnectionState::Disconnected;

    // ---- 网络 ----
    QTcpSocket* m_socket = nullptr; QTimer* m_pingTimer = nullptr;
    QByteArray m_buffer; QString m_handshakeKey; bool m_handshakeDone = false;

    // ---- 延迟 ----
    QElapsedTimer m_pingSendTime; qint64 m_lastLatencyMs = 0; qint64 m_maxLatencyMs = 0;
    quint64 m_latencySampleCount = 0; qint64 m_latencySumMs = 0;
    QElapsedTimer m_connectionTimer;                            ///< 连接运行计时器

    // ---- 消息队列 ----
    struct QueuedMessage { bool isText = false; QByteArray data; };
    QQueue<QueuedMessage> m_sendQueue; int m_queueLimit = 1000; quint64 m_totalMessagesDropped = 0;

    // ---- 统计 ----
    quint64 m_totalConnections = 0; quint64 m_totalMessagesSent = 0; quint64 m_totalMessagesReceived = 0;
    quint64 m_totalBytesSent = 0; quint64 m_totalBytesReceived = 0; quint64 m_errorCount = 0;
    quint64 m_totalFramesSent = 0; quint64 m_totalFramesReceived = 0;
    quint64 m_totalTextFrames = 0; quint64 m_totalBinaryFrames = 0;
    quint64 m_totalPingFrames = 0; quint64 m_totalPongFrames = 0;
    quint64 m_totalFragmentedMessages = 0;   ///< 已接收分片消息(continuation帧)总数
    quint64 m_totalHandshakeAttempts = 0;   ///< 累计HTTP Upgrade握手尝试次数
    quint64 m_totalHandshakeFailures = 0;   ///< 累计握手失败次数(非101响应)
    quint64 m_totalCloseFramesSent = 0;     ///< 累计发送close帧(0x08)次数
    quint64 m_totalCloseFramesReceived = 0; ///< 累计接收close帧(0x08)次数
};

#endif // WEBSOCKETCONNECTION_H

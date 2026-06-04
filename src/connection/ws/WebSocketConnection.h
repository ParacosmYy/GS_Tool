/**
 * @file WebSocketConnection.h
 * @brief WebSocket客户端连接 - 基于QTcpSocket实现WebSocket帧协议
 *
 * 职责:
 *   1. 提供WebSocket客户端连接能力(RFC 6455)
 *   2. 支持文本帧和二进制帧收发
 *   3. 支持心跳检测(ping/pong)
 *   4. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - WsConfigPanel: WebSocket参数配置UI
 */

#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>
#include <QElapsedTimer>
#include <QQueue>

/**
 * @brief WebSocket客户端连接实现
 *
 * 支持ws://和wss://协议，可收发文本和二进制消息。
 * 基于QTcpSocket手工实现RFC 6455 WebSocket帧协议。
 * 内置心跳定时器，定期发送ping帧保持连接活跃。
 */
class WebSocketConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
    explicit WebSocketConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~WebSocketConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- WebSocket特有接口 ----

    /**
     * @brief 连接到指定URL
     * @param url WebSocket地址(如ws://host:port/path)
     * @return true=连接已发起
     */
    bool connectToUrl(const QString& url);

    /**
     * @brief 发送文本消息
     * @param message 文本内容
     * @return 发送字节数
     */
    qint64 sendTextMessage(const QString& message);

    /**
     * @brief 发送二进制消息
     * @param data 二进制数据
     * @return 发送字节数
     */
    qint64 sendBinaryMessage(const QByteArray& data);

    /**
     * @brief 发送ping帧
     * @param payload ping载荷数据
     * @return true=发送成功
     */
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

    /** @brief 获取已发送帧总数(包含所有opcode) @return 帧发送总数 */
    quint64 totalFramesSent() const;

    /** @brief 获取已接收帧总数(包含所有opcode) @return 帧接收总数 */
    quint64 totalFramesReceived() const;

    /** @brief 获取已发送文本帧总数 @return 文本帧发送总数 */
    quint64 totalTextFrames() const;

    /** @brief 获取已接收文本帧总数 @return 文本帧接收总数 */
    quint64 totalBinaryFrames() const;

    /** @brief 获取已发送ping帧总数 @return ping帧发送总数 */
    quint64 totalPingFrames() const;

    /** @brief 获取已接收pong帧总数 @return pong帧接收总数 */
    quint64 totalPongFrames() const;

    /** @brief 获取ping/pong平均延迟(毫秒) @return 平均延迟，无数据时返回0 */
    double averageLatencyMs() const;

    /** @brief 获取ping/pong最大延迟(毫秒) @return 最大延迟，无数据时返回0 */
    qint64 maxLatencyMs() const;

    /** @brief 获取当前连接的运行时长(秒) @return 连接时长，未连接时返回0 */
    qint64 connectionUptimeSeconds() const;

    /** @brief 获取当前消息队列大小 @return 队列中待发送消息数量 */
    int messageQueueSize() const;

    /** @brief 获取消息队列容量上限 @return 队列最大容量 */
    int messageQueueLimit() const;

    /** @brief 设置消息队列容量上限 @param limit 最大容量，小于等于0不限 */
    void setMessageQueueLimit(int limit);

    /** @brief 获取因队列满而丢弃的消息数 @return 丢弃消息总数 */
    quint64 totalMessagesDropped() const;

    /** @brief 重置所有统计数据为零 */
    void resetStats();

signals:
    /** @brief 收到文本消息时发出
     * @param message 文本内容
     */
    void textMessageReceived(const QString& message);

    /** @brief 收到二进制消息时发出
     * @param data 二进制数据
     */
    void binaryMessageReceived(const QByteArray& data);

    /** @brief 收到pong响应时发出
     * @param payload pong载荷数据
     */
    void pongReceived(const QByteArray& payload);

private slots:
    /** @brief TCP连接成功回调 */
    void onTcpConnected();

    /** @brief TCP断开回调 */
    void onTcpDisconnected();

    /** @brief TCP数据就绪回调 — 解析WebSocket帧 */
    void onTcpReadyRead();

    /** @brief 心跳定时器触发 */
    void onPingTimeout();

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    /** @brief 构建WebSocket帧
     * @param opcode 帧操作码(0x01/0x02/0x08/0x09)
     * @param payload 载荷数据
     * @return 帧字节数组
     */
    QByteArray buildFrame(quint8 opcode, const QByteArray& payload) const;

    /** @brief 解析缓冲区中的WebSocket帧 */
    void parseFrames();

    /** @brief 发送HTTP Upgrade握手 */
    void sendHandshake();

    /** @brief 检查握手响应是否完整 */
    bool parseHandshakeResponse();

    // ---- 配置参数 ----
    QString m_url;          ///< WebSocket服务器地址
    QString m_protocol;     ///< 子协议名称
    QString m_host;         ///< 解析后的主机名
    quint16 m_port = 80;    ///< 解析后的端口号
    QString m_path;         ///< 解析后的路径

    ConnectionState m_state = ConnectionState::Disconnected;

    // ---- 网络资源 ----
    QTcpSocket* m_socket = nullptr;     ///< TCP底层socket
    QTimer* m_pingTimer = nullptr;      ///< 心跳定时器
    QByteArray m_buffer;                ///< 接收缓冲区
    QString m_handshakeKey;             ///< 握手Sec-WebSocket-Key
    bool m_handshakeDone = false;       ///< 握手是否完成

    // ---- 延迟追踪 ----
    QElapsedTimer m_pingSendTime;       ///< ping发送时间戳
    qint64 m_lastLatencyMs = 0;         ///< 最近一次ping/pong延迟(ms)
    qint64 m_maxLatencyMs = 0;          ///< 最大ping/pong延迟(ms)
    quint64 m_latencySampleCount = 0;   ///< 延迟采样次数
    qint64 m_latencySumMs = 0;          ///< 延迟累计总和(ms)

    // ---- 连接时长 ----
    QElapsedTimer m_connectionTimer;    ///< 连接建立后的运行计时器

    // ---- 消息队列 ----
    struct QueuedMessage {               ///< 待发送消息结构
        bool isText = false;             ///< true=文本帧, false=二进制帧
        QByteArray data;                 ///< 消息内容(文本用toUtf8()后)
    };
    QQueue<QueuedMessage> m_sendQueue;  ///< 发送队列
    int m_queueLimit = 1000;            ///< 队列容量上限
    quint64 m_totalMessagesDropped = 0; ///< 因队列满而丢弃的消息数

    // ---- 统计计数器 ----
    quint64 m_totalConnections = 0;         ///< 累计WebSocket连接成功次数
    quint64 m_totalMessagesSent = 0;        ///< 已发送消息总数(文本+二进制)
    quint64 m_totalMessagesReceived = 0;    ///< 已接收消息总数(文本+二进制)
    quint64 m_totalBytesSent = 0;           ///< 已发送字节总数(帧级别)
    quint64 m_totalBytesReceived = 0;       ///< 已接收字节总数(帧级别)
    quint64 m_errorCount = 0;               ///< 错误发生次数
    quint64 m_totalFramesSent = 0;          ///< 已发送帧总数(含所有opcode)
    quint64 m_totalFramesReceived = 0;      ///< 已接收帧总数(含所有opcode)
    quint64 m_totalTextFrames = 0;          ///< 已发送文本帧总数
    quint64 m_totalBinaryFrames = 0;        ///< 已发送二进制帧总数
    quint64 m_totalPingFrames = 0;          ///< 已发送ping帧总数
    quint64 m_totalPongFrames = 0;          ///< 已接收pong帧总数
};

#endif // WEBSOCKETCONNECTION_H

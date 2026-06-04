/**
 * @file MqttConnection.h
 * @brief MQTT客户端连接实现 — 基于QTcpSocket实现MQTT v3.1.1线协议
 *
 * 职责: MQTT连接管理、消息发布/订阅、KeepAlive心跳、QoS级别追踪、
 * LWT遗嘱消息配置、消息队列管理，通过IConnection统一接口供上层使用。
 */
#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>
#include <QStringList>
#include <QDateTime>
#include <QQueue>

struct MqttWillConfig {         ///< MQTT遗嘱消息(LWT)配置结构体
    QString topic;              ///< 遗嘱主题
    QByteArray payload;         ///< 遗嘱负载
    int qos = 0;                ///< 遗嘱QoS等级(0/1/2)
    bool retain = false;        ///< 遗嘱保留标志
};

struct MqttPendingMessage {     ///< 待发送消息队列项
    QString topic;              ///< 目标主题
    QByteArray payload;         ///< 消息负载
    int qos = 0;                ///< QoS等级(0/1/2)
};

/** @brief MQTT客户端连接实现，封装MQTT v3.1.1线协议 */
class MqttConnection : public IConnection {
    Q_OBJECT
public:
    explicit MqttConnection(QObject* parent = nullptr); ///< 构造MQTT客户端连接
    ~MqttConnection() override;                         ///< 析构，关闭连接释放资源
    // ---- IConnection接口 ----
    ConnectionType type() const override;                ///< @return 固定返回ConnectionType::Mqtt
    QString name() const override;                       ///< @return "主机:端口"或"未配置"
    ConnectionState state() const override;              ///< @return 连接状态枚举
    bool open() override;                                ///< 发起TCP握手
    void close() override;                               ///< 发送DISCONNECT报文后断开TCP
    qint64 write(const QByteArray& data) override;       ///< QoS0发布
    void configure(const QVariantMap& params) override;  ///< 配置连接参数
    // ---- 自动重连接口 ----
    void setAutoReconnect(bool enabled);               ///< @brief 启用/禁用自动重连
    bool autoReconnect() const;                        ///< @brief 查询自动重连状态
    void setMaxRetries(int max);                       ///< @brief 设置最大重试次数(0=无限)
    int maxRetries() const;                            ///< @brief 获取最大重试次数
    int currentRetryCount() const;                     ///< @brief 获取当前已重试次数
    quint64 totalRetryAttempts() const;                ///< @brief 获取累计重试次数
    // ---- MQTT专用接口 ----
    /** @brief 连接到指定MQTT服务器 @param host 服务器地址 @param port 服务器端口 */
    void connectToHost(const QString& host, int port);
    void disconnectFromHost();                           ///< 断开MQTT连接
    /** @brief 发布消息 @param topic 目标主题 @param payload 消息负载 @param qos QoS等级(0/1/2) @return true=发送成功 */
    bool publish(const QString& topic, const QByteArray& payload, int qos = 0);
    /** @brief 订阅指定主题 @param topic 订阅主题 @param qos QoS等级(0/1/2) @return true=发送成功 */
    bool subscribe(const QString& topic, int qos = 0);
    /** @brief 取消订阅指定主题 @param topic 要取消的主题 */
    void unsubscribe(const QString& topic);
    // ---- LWT遗嘱消息接口 ----
    void setWill(const MqttWillConfig& will);            ///< 配置遗嘱消息
    void clearWill();                                    ///< 清除遗嘱消息配置
    const MqttWillConfig& willConfig() const;            ///< 获取遗嘱消息配置(只读)
    // ---- 消息队列接口 ----
    /** @brief 入队消息(断线时缓存) @param topic 目标主题 @param payload 消息负载 @param qos QoS等级 @return true=入队成功 */
    bool enqueueMessage(const QString& topic, const QByteArray& payload, int qos = 0);
    int queueSize() const;                               ///< 获取待发送消息数
    void setQueueLimit(int maxSize);                     ///< 设置队列最大容量
    int queueLimit() const;                              ///< 获取队列最大容量
    // ---- 统计接口 ----
    quint64 totalPublishes() const;                      ///< @return 累计发布消息数
    quint64 totalReceived() const;                       ///< @return 累计接收消息数
    quint64 totalSubscriptions() const;                  ///< @return 累计订阅次数
    quint64 totalUnsubscriptions() const { return m_totalUnsubscriptions; }  ///< @return 累计退订次数
    quint64 totalMessageReceived() const { return m_totalMessageReceived; }  ///< @return 累计接收PUBLISH消息数
    quint64 totalBytesSent() const;                      ///< @return 累计发送字节数
    quint64 totalBytesReceived() const;                  ///< @return 累计接收字节数
    quint64 errorCount() const;                          ///< @return 累计错误次数
    quint64 connectionAttempts() const;                  ///< @return 累计连接尝试次数
    quint64 qos0Count() const;                           ///< @return QoS0发布计数
    quint64 qos1Count() const;                           ///< @return QoS1发布计数
    quint64 qos2Count() const;                           ///< @return QoS2发布计数
    quint64 keepAliveSent() const;                       ///< @return 累计PINGREQ发送次数
    QDateTime lastConnectTime() const;                   ///< @return 最后连接发起时间
    int subscriptionCount() const;                       ///< 获取当前订阅主题数
    int pendingQueueSize() const;                        ///< 获取队列中的消息数
    void resetStats();                                   ///< 重置所有统计计数器
    quint64 totalSuccessfulReconnects() const;           ///< @brief 累计成功重连次数
    double avgRetryDelayMs() const;                      ///< @brief 平均重试延迟(ms)

signals:
    void messageReceived(const QString& topic, const QByteArray& payload); ///< 收到MQTT消息
    void connected();                                     ///< MQTT连接成功建立
    void disconnected();                                  ///< MQTT连接断开
    /** @brief 消息队列溢出，已丢弃旧消息 @param count 丢弃的消息数 */
    void queueOverflow(int count);
    /** @brief 自动重连状态变更通知 @param retryCount 当前重试次数 @param delayMs 下次重试延迟 */
    void retryScheduled(int retryCount, qint64 delayMs);

private slots:
    void onSocketReadyRead();    ///< TCP socket数据就绪回调，触发MQTT报文解析
    void onSocketConnected();    ///< TCP socket连接成功回调，发送MQTT CONNECT报文
    void onSocketDisconnected(); ///< TCP socket断开回调，更新连接状态
    void onKeepAlive();          ///< KeepAlive定时器回调，发送PINGREQ保活报文
    void onRetryTimeout();       ///< @brief 重连定时器回调，执行指数退避重试

private:
    /** @brief 构建MQTT协议报文 @param packetType 报文类型 @param payload 报文负载 @return 编码后的完整报文 */
    QByteArray buildMqttPacket(quint8 packetType, const QByteArray& payload);
    /** @brief 编码剩余长度字段(MQTT可变长度编码) @param length 长度值 @return 编码后的字节序列 */
    QByteArray encodeRemainingLength(int length);
    void parseIncomingPacket();  ///< 解析TCP接收缓冲区中的MQTT报文
    void sendConnect();          ///< 发送MQTT CONNECT报文
    void handleConnack(const QByteArray& data);  ///< 处理CONNACK响应报文
    /** @brief 处理收到的PUBLISH报文 @param data PUBLISH报文数据 @param flags 报文标志字节 */
    void handlePublish(const QByteArray& data, quint8 flags);
    void handleSuback(const QByteArray& data);   ///< 处理SUBACK响应报文
    QString generateClientId(); ///< 生成唯一客户端ID
    void flushPendingQueue();   ///< 发送队列中缓存的待发消息
    qint64 computeBackoffDelay() const; ///< @brief 计算指数退避延迟(ms)
    void scheduleRetry();               ///< @brief 安排下一次重连尝试
    // 配置参数
    QString m_host;                             ///< MQTT服务器地址
    int m_port = 1883;                          ///< MQTT服务器端口
    QString m_clientId;                         ///< 客户端唯一标识
    QString m_username;                         ///< 认证用户名
    QString m_password;                         ///< 认证密码
    int m_keepAliveInterval = 60;               ///< KeepAlive间隔(秒)
    bool m_cleanSession = true;                 ///< 清除会话标志
    quint16 m_packetId = 0;                     ///< 报文ID自增计数器
    MqttWillConfig m_will;                      ///< 遗嘱消息配置
    QQueue<MqttPendingMessage> m_pendingQueue;  ///< 断线时消息缓存队列
    int m_queueLimit = 100;                     ///< 队列最大容量
    QTcpSocket* m_socket;                       ///< TCP socket
    QTimer* m_keepAlive;                        ///< KeepAlive定时器
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态
    QByteArray m_rxBuffer;                      ///< 接收缓冲区
    int m_expectedLength = -1;                  ///< 期望的报文长度(-1=未解析)
    QStringList m_subscriptions;                ///< 已订阅主题列表
    // 统计计数器
    quint64 m_totalPublishes = 0;               ///< 累计发布消息数
    quint64 m_totalReceived = 0;                ///< 累计接收消息数
    quint64 m_totalSubscriptions = 0;           ///< 累计订阅次数
    quint64 m_totalUnsubscriptions = 0;         ///< 累计退订次数
    quint64 m_totalMessageReceived = 0;         ///< 累计接收PUBLISH消息数
    quint64 m_totalBytesSent = 0;               ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;           ///< 累计接收字节数
    quint64 m_errorCount = 0;                   ///< 累计错误次数
    quint64 m_connectionAttempts = 0;           ///< 累计连接尝试次数
    quint64 m_qos0Count = 0;                    ///< QoS0发布计数
    quint64 m_qos1Count = 0;                    ///< QoS1发布计数
    quint64 m_qos2Count = 0;                    ///< QoS2发布计数
    quint64 m_keepAliveSent = 0;                ///< 累计PINGREQ发送次数
    QDateTime m_lastConnectTime;                ///< 最后连接发起时间
    // 自动重连配置
    QTimer* m_retryTimer;                       ///< 重连定时器(指数退避)
    bool m_autoReconnect = false;               ///< 是否启用自动重连
    int m_maxRetries = 0;                       ///< 最大重试次数(0=无限)
    int m_currentRetryCount = 0;                ///< 当前已重试次数
    qint64 m_currentRetryDelayMs = 1000;        ///< 当前重试延迟(ms)
    static constexpr qint64 kMinRetryDelayMs = 1000;  ///< 最小重试延迟1秒
    static constexpr qint64 kMaxRetryDelayMs = 30000; ///< 最大重试延迟30秒
    quint64 m_totalRetryAttempts = 0;           ///< 累计重试次数
    quint64 m_totalSuccessfulReconnects = 0;    ///< 累计成功重连次数
    qint64 m_totalRetryDelayMs = 0;             ///< 累计重试延迟(ms)
};

#endif // MQTTCONNECTION_H

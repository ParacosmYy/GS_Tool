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

/** @brief MQTT遗嘱消息(LWT)配置结构体 */
struct MqttWillConfig {
    QString topic;          ///< 遗嘱主题
    QByteArray payload;     ///< 遗嘱负载
    int qos = 0;            ///< 遗嘱QoS等级(0/1/2)
    bool retain = false;    ///< 遗嘱保留标志
};

/** @brief 待发送消息队列项 */
struct MqttPendingMessage {
    QString topic;          ///< 目标主题
    QByteArray payload;     ///< 消息负载
    int qos = 0;            ///< QoS等级(0/1/2)
};

/** @brief MQTT客户端连接实现，封装MQTT v3.1.1线协议 */
class MqttConnection : public IConnection {
    Q_OBJECT
public:
    explicit MqttConnection(QObject* parent = nullptr);
    ~MqttConnection() override;

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- MQTT专用接口 ----
    void connectToHost(const QString& host, int port);
    void disconnectFromHost();
    bool publish(const QString& topic, const QByteArray& payload, int qos = 0);
    bool subscribe(const QString& topic, int qos = 0);
    void unsubscribe(const QString& topic);

    // ---- LWT遗嘱消息接口 ----
    /** @brief 配置遗嘱消息 @param will 遗嘱配置结构体 */
    void setWill(const MqttWillConfig& will);
    /** @brief 清除遗嘱消息配置 */
    void clearWill();
    /** @brief 获取当前遗嘱配置 @return 遗嘱配置(只读) */
    const MqttWillConfig& willConfig() const;

    // ---- 消息队列接口 ----
    /** @brief 入队消息(断线时缓存) @param topic 目标主题 @param payload 负载 @param qos QoS @return true=入队成功 */
    bool enqueueMessage(const QString& topic, const QByteArray& payload, int qos = 0);
    /** @brief 获取当前队列大小 @return 待发送消息数 */
    int queueSize() const;
    /** @brief 设置队列最大容量 @param maxSize 最大消息数(默认100) */
    void setQueueLimit(int maxSize);
    /** @brief 获取队列最大容量 @return 最大消息数 */
    int queueLimit() const;

    // ---- 统计接口 ----
    quint64 totalPublishes() const;
    quint64 totalReceived() const;
    quint64 totalSubscriptions() const;
    quint64 totalBytesSent() const;
    quint64 totalBytesReceived() const;
    quint64 errorCount() const;
    quint64 connectionAttempts() const;
    quint64 qos0Count() const;
    quint64 qos1Count() const;
    quint64 qos2Count() const;
    quint64 keepAliveSent() const;
    QDateTime lastConnectTime() const;
    int subscriptionCount() const;
    int pendingQueueSize() const;
    void resetStats();

signals:
    void messageReceived(const QString& topic, const QByteArray& payload);
    void connected();
    void disconnected();
    /** @brief 消息队列溢出信号(丢弃旧消息时发出) @param count 丢弃的消息数 */
    void queueOverflow(int count);

private slots:
    void onSocketReadyRead();
    void onSocketConnected();
    void onSocketDisconnected();
    void onKeepAlive();

private:
    QByteArray buildMqttPacket(quint8 packetType, const QByteArray& payload);
    QByteArray encodeRemainingLength(int length);
    void parseIncomingPacket();
    void sendConnect();
    void handleConnack(const QByteArray& data);
    void handlePublish(const QByteArray& data, quint8 flags);
    void handleSuback(const QByteArray& data);
    QString generateClientId();
    /** @brief 发送队列中缓存的待发消息 */
    void flushPendingQueue();

    // 配置参数
    QString m_host;
    int m_port = 1883;
    QString m_clientId;
    QString m_username;
    QString m_password;
    int m_keepAliveInterval = 60;
    bool m_cleanSession = true;
    quint16 m_packetId = 0;

    // LWT遗嘱消息
    MqttWillConfig m_will;

    // 消息队列
    QQueue<MqttPendingMessage> m_pendingQueue; ///< 断线时消息缓存队列
    int m_queueLimit = 100;                     ///< 队列最大容量

    // 网络资源
    QTcpSocket* m_socket;
    QTimer* m_keepAlive;
    ConnectionState m_state = ConnectionState::Disconnected;
    QByteArray m_rxBuffer;
    int m_expectedLength = -1;
    QStringList m_subscriptions;

    // 统计计数器
    quint64 m_totalPublishes = 0;
    quint64 m_totalReceived = 0;
    quint64 m_totalSubscriptions = 0;
    quint64 m_totalBytesSent = 0;
    quint64 m_totalBytesReceived = 0;
    quint64 m_errorCount = 0;
    quint64 m_connectionAttempts = 0;
    quint64 m_qos0Count = 0;
    quint64 m_qos1Count = 0;
    quint64 m_qos2Count = 0;
    quint64 m_keepAliveSent = 0;
    QDateTime m_lastConnectTime;
};

#endif // MQTTCONNECTION_H

/**
 * @file MqttClientEngine.h
 * @brief MQTT客户端引擎 — 基于QTcpSocket的MQTT v3.1.1协议实现
 *
 * 职责: 管理MQTT连接生命周期、消息发布/订阅、QoS级别处理、
 * KeepAlive心跳、指数退避重连、消息队列和LWT遗嘱消息。
 * 通过信号通知上层组件连接状态变化和收到的消息。
 */

#ifndef MQTTCLIENTENGINE_H
#define MQTTCLIENTENGINE_H

#include "connection/mqtt_client/MqttClientTypes.h"

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QTimer>
#include <QQueue>
#include <QStringList>
#include <QDateTime>

/**
 * @brief MQTT客户端引擎，封装完整的MQTT v3.1.1协议客户端逻辑
 *
 * 支持QoS 0/1/2发布和订阅，自动KeepAlive心跳，断线指数退避重连，
 * 离线消息队列缓存，以及LWT遗嘱消息配置。
 */
class MqttClientEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 构造MQTT客户端引擎 @param parent 父对象 */
    explicit MqttClientEngine(QObject* parent = nullptr);

    /** @brief 析构，发送DISCONNECT报文后关闭连接 */
    ~MqttClientEngine() override;

    // ── 连接管理 ──

    /** @brief 使用指定参数连接到MQTT服务器 @param params 连接参数 */
    void connectToBroker(const MqttConnectionParams& params);

    /** @brief 断开MQTT连接（发送DISCONNECT报文） */
    void disconnectFromBroker();

    /** @brief 查询当前是否已连接 @return true=已连接 */
    bool isConnected() const;

    // ── 发布/订阅 ──

    /**
     * @brief 发布消息到指定主题
     * @param topic 目标主题
     * @param payload 消息负载
     * @param qos QoS等级
     * @param retained 是否保留
     * @return true=发送成功或已入队
     */
    bool publish(const QString& topic, const QByteArray& payload,
                 MqttQos qos = MqttQos::QoS0, bool retained = false);

    /**
     * @brief 订阅指定主题（支持通配符 # 和 +）
     * @param topic 订阅主题过滤器
     * @param qos 最大QoS等级
     * @return true=SUBSCRIBE报文已发送
     */
    bool subscribe(const QString& topic, MqttQos qos = MqttQos::QoS0);

    /** @brief 取消订阅指定主题 @param topic 要取消的主题 */
    void unsubscribe(const QString& topic);

    /** @brief 获取当前活跃订阅列表 @return 主题过滤器列表 */
    QStringList subscriptions() const;

    // ── 遗嘱消息(LWT) ──

    /** @brief 配置遗嘱消息（连接前设置） @param will 遗嘱消息 */
    void setLastWill(const MqttMessage& will);

    /** @brief 清除遗嘱消息配置 */
    void clearLastWill();

    // ── 统计接口 ──

    /** @brief 获取累计发布消息数 @return 发布计数 */
    quint64 totalPublishes() const;
    /** @brief 获取累计接收消息数 @return 接收计数 */
    quint64 totalReceived() const;
    /** @brief 获取累计发送字节数 @return 字节数 */
    quint64 totalBytesSent() const;
    /** @brief 获取累计接收字节数 @return 字节数 */
    quint64 totalBytesReceived() const;
    /** @brief 获取累计错误次数 @return 错误计数 */
    quint64 errorCount() const;
    /** @brief 获取累计连接尝试次数 @return 连接计数 */
    quint64 connectionAttempts() const;
    /** @brief 获取累计重连成功次数 @return 重连计数 */
    quint64 successfulReconnects() const;
    /** @brief 获取累计QoS0发布数 @return QoS0计数 */
    quint64 qos0Publishes() const;
    /** @brief 获取累计QoS1发布数 @return QoS1计数 */
    quint64 qos1Publishes() const;
    /** @brief 获取累计QoS2发布数 @return QoS2计数 */
    quint64 qos2Publishes() const;
    /** @brief 获取累计PINGREQ发送次数 @return 心跳计数 */
    quint64 keepAliveSent() const;
    /** @brief 获取待发送队列大小 @return 队列中消息数 */
    int pendingQueueSize() const;
    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 收到MQTT消息 @param msg 接收到的消息 */
    void messageReceived(const MqttMessage& msg);
    /** @brief 连接成功建立 */
    void connected();
    /** @brief 连接已断开 */
    void disconnected();
    /** @brief 连接错误 @param error 错误描述 */
    void connectionError(const QString& error);
    /** @brief 消息发布确认（QoS1 PUBACK / QoS2 PUBCOMP） @param packetId 报文标识 */
    void publishAcked(quint16 packetId);

private slots:
    void onSocketConnected();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onKeepAlive();
    void onReconnectTimeout();

private:
    void setupSocketSignals();
    bool sendConnectPacket();
    bool sendPublishPacket(const QString& topic, const QByteArray& payload,
                           MqttQos qos, bool retained);
    bool sendSubscribePacket(const QString& topic, MqttQos qos);
    bool sendUnsubscribePacket(const QString& topic);
    bool sendPingReq();
    bool sendDisconnect();
    void handleIncomingPacket();
    void processConnack(const QByteArray& data);
    void processPublish(const QByteArray& data, int header);
    void processSuback(const QByteArray& data);
    void processUnsuback(const QByteArray& data);
    void processPuback(const QByteArray& data);
    void processPubrec(const QByteArray& data);
    void processPubrel(const QByteArray& data);
    void processPubcomp(const QByteArray& data);
    void processPingresp();
    void flushPendingQueue();
    void scheduleReconnect();
    quint16 nextPacketId();
    QByteArray encodeRemainingLength(int length);
    int decodeRemainingLength(const QByteArray& data, int* offset);
    QByteArray buildTopicFilter(const QString& topic);

    QTcpSocket*   m_socket;
    QTimer*       m_keepAliveTimer;
    QTimer*       m_reconnectTimer;

    MqttConnectionParams           m_params;
    MqttMessage                    m_lastWill;
    bool                           m_hasWill;
    bool                           m_connected;
    quint16                        m_nextPacketId;
    int                            m_reconnectAttempts;
    int                            m_maxReconnectAttempts;
    QStringList                    m_subscriptions;
    QQueue<MqttMessage>            m_pendingQueue;
    int                            m_maxQueueSize;
    QByteArray                     m_rxBuffer;

    // 统计
    quint64 m_totalPublishes;
    quint64 m_totalReceived;
    quint64 m_totalBytesSent;
    quint64 m_totalBytesReceived;
    quint64 m_errorCount;
    quint64 m_connectionAttempts;
    quint64 m_successfulReconnects;
    quint64 m_qos0Publishes;
    quint64 m_qos1Publishes;
    quint64 m_qos2Publishes;
    quint64 m_keepAliveSent;
};

#endif // MQTTCLIENTENGINE_H

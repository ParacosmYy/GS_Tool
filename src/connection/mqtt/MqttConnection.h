/**
 * @file MqttConnection.h
 * @brief MQTT客户端连接实现 — 基于QTcpSocket实现MQTT v3.1.1线协议
 *
 * 职责: MQTT连接管理、消息发布/订阅、KeepAlive心跳，
 * 通过IConnection统一接口供上层使用。
 */
#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>
#include <QStringList>

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

    // ---- 统计接口 ----
    quint64 totalPublishes() const;
    quint64 totalSubscriptions() const;
    quint64 totalBytesSent() const;
    quint64 totalBytesReceived() const;
    quint64 errorCount() const;
    void resetStats();
    int subscriptionCount() const;

signals:
    void messageReceived(const QString& topic, const QByteArray& payload);
    void connected();
    void disconnected();

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

    // 配置参数
    QString m_host;
    int m_port = 1883;
    QString m_clientId;
    QString m_username;
    QString m_password;
    int m_keepAliveInterval = 60;
    bool m_cleanSession = true;
    quint16 m_packetId = 0;

    // 网络资源
    QTcpSocket* m_socket;
    QTimer* m_keepAlive;
    ConnectionState m_state = ConnectionState::Disconnected;
    QByteArray m_rxBuffer;
    int m_expectedLength = -1;
    QStringList m_subscriptions;

    // 统计计数器
    quint64 m_totalPublishes = 0;
    quint64 m_totalSubscriptions = 0;
    quint64 m_totalBytesSent = 0;
    quint64 m_totalBytesReceived = 0;
    quint64 m_errorCount = 0;
};

#endif // MQTTCONNECTION_H

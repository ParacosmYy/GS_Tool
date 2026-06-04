/**
 * @file MqttConnectionPublish.cpp
 * @brief MQTT 发布/订阅/退订协议操作、心跳保活、CONNECT报文构建
 *
 * 本文件从 MqttConnectionProtocol.cpp 拆分而来，包含:
 *   - publish():      向指定主题发布消息(QoS0/1/2)
 *   - subscribe():    订阅指定MQTT主题
 *   - unsubscribe():  取消订阅指定主题
 *   - onKeepAlive():  心跳定时器回调，发送PINGREQ保活报文
 *   - sendConnect():  构建并发送MQTT CONNECT报文(含LWT遗嘱)
 *
 * 拆分原因:
 *   MqttConnectionProtocol.cpp 包含报文构建/解析和协议操作，
 *   将发布/订阅/心跳/连接等高层操作独立成文件可降低单文件复杂度。
 */

#include "connection/mqtt/MqttConnection.h"
#include <QRandomGenerator>

// MQTT v3.1.1 报文类型枚举(与MqttConnectionProtocol.cpp共享)
enum {
    MqttPublish_CONNECT = 1,
    MqttPublish_PUBLISH = 3,
    MqttPublish_SUBSCRIBE = 8,
    MqttPublish_UNSUBSCRIBE = 10,
    MqttPublish_PINGREQ = 12
};

// ─── 发布/订阅/退订 ─────────────────────────────────────

/** @brief 向指定主题发布消息 @param topic 主题名 @param payload 消息负载 @param qos 服务质量等级(0/1/2) @return true=发布成功 */
bool MqttConnection::publish(const QString& topic, const QByteArray& payload, int qos)
{
    if (m_state != ConnectionState::Connected || !m_socket) return false;
    QByteArray variableHeader;
    const QByteArray topicUtf8 = topic.toUtf8();
    variableHeader.append(static_cast<char>((topicUtf8.size() >> 8) & 0xFF));
    variableHeader.append(static_cast<char>(topicUtf8.size() & 0xFF));
    variableHeader.append(topicUtf8);
    if (qos > 0) {
        m_packetId = (m_packetId % 65535) + 1;
        variableHeader.append(static_cast<char>((m_packetId >> 8) & 0xFF));
        variableHeader.append(static_cast<char>(m_packetId & 0xFF));
    }
    variableHeader.append(payload);
    QByteArray packet = buildMqttPacket(MqttPublish_PUBLISH, variableHeader);
    if (qos == 1) packet[0] |= 0x02;
    else if (qos == 2) packet[0] |= 0x04;
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) {
        ++m_totalPublishes;
        /* 按QoS级别统计 */
        if (qos == 0) ++m_qos0Count;
        else if (qos == 1) ++m_qos1Count;
        else if (qos == 2) ++m_qos2Count;
        m_totalBytesSent += static_cast<quint64>(written);
        return true;
    }
    ++m_errorCount;
    return false;
}

/** @brief 订阅指定MQTT主题 @param topic 主题过滤器 @param qos 请求的服务质量等级(0/1/2) @return true=订阅报文发送成功 */
bool MqttConnection::subscribe(const QString& topic, int qos)
{
    if (m_state != ConnectionState::Connected || !m_socket) return false;
    QByteArray payload;
    const QByteArray topicUtf8 = topic.toUtf8();
    payload.append(static_cast<char>((topicUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(topicUtf8.size() & 0xFF));
    payload.append(topicUtf8);
    payload.append(static_cast<char>(qos & 0x03));
    m_packetId = (m_packetId % 65535) + 1;
    QByteArray hdr;
    hdr.append(static_cast<char>((m_packetId >> 8) & 0xFF));
    hdr.append(static_cast<char>(m_packetId & 0xFF));
    QByteArray packet = buildMqttPacket(MqttPublish_SUBSCRIBE, hdr + payload);
    packet[0] |= 0x02;
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) {
        ++m_totalSubscriptions;
        m_totalBytesSent += static_cast<quint64>(written);
        if (!m_subscriptions.contains(topic)) m_subscriptions.append(topic);
        return true;
    }
    ++m_errorCount;
    return false;
}

/** @brief 取消订阅指定MQTT主题 @param topic 要取消的主题过滤器 */
void MqttConnection::unsubscribe(const QString& topic)
{
    if (m_state != ConnectionState::Connected || !m_socket) return;
    QByteArray payload;
    const QByteArray topicUtf8 = topic.toUtf8();
    payload.append(static_cast<char>((topicUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(topicUtf8.size() & 0xFF));
    payload.append(topicUtf8);
    m_packetId = (m_packetId % 65535) + 1;
    QByteArray hdr;
    hdr.append(static_cast<char>((m_packetId >> 8) & 0xFF));
    hdr.append(static_cast<char>(m_packetId & 0xFF));
    QByteArray packet = buildMqttPacket(MqttPublish_UNSUBSCRIBE, hdr + payload);
    packet[0] |= 0x02;
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) {
        m_totalBytesSent += static_cast<quint64>(written);
        ++m_totalUnsubscriptions;  ///< 累计退订次数
    }
    m_subscriptions.removeAll(topic);
}

// ─── 心跳保活 ──────────────────────────────────────────────

/** @brief 心跳定时器回调，发送PINGREQ保活报文并统计 */
void MqttConnection::onKeepAlive()
{
    if (m_state == ConnectionState::Connected && m_socket) {
        QByteArray packet = buildMqttPacket(MqttPublish_PINGREQ, {});
        qint64 written = m_socket->write(packet);
        if (written == packet.size()) {
            m_totalBytesSent += static_cast<quint64>(written);
            ++m_keepAliveSent;
        }
    }
}

// ─── CONNECT报文构建 ─────────────────────────────────────

/** @brief 构建并发送MQTT CONNECT报文，包含客户端ID、认证信息和LWT遗嘱 */
void MqttConnection::sendConnect()
{
    if (!m_socket) return;
    QByteArray payload;
    payload.append(static_cast<char>(0));
    payload.append(static_cast<char>(4));
    payload.append("MQTT");
    payload.append(static_cast<char>(4));
    quint8 flags = 0x02; /* Clean Session */
    if (!m_username.isEmpty()) flags |= 0x80;
    if (!m_password.isEmpty()) flags |= 0x40;

    /* LWT遗嘱消息标志位 */
    if (!m_will.topic.isEmpty()) {
        flags |= 0x04; /* Will Flag */
        if (m_will.qos == 1) flags |= 0x08;
        else if (m_will.qos == 2) flags |= 0x10;
        if (m_will.retain) flags |= 0x20;
    }

    payload.append(static_cast<char>(flags));
    payload.append(static_cast<char>((m_keepAliveInterval >> 8) & 0xFF));
    payload.append(static_cast<char>(m_keepAliveInterval & 0xFF));
    const QByteArray clientIdUtf8 = m_clientId.toUtf8();
    payload.append(static_cast<char>((clientIdUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(clientIdUtf8.size() & 0xFF));
    payload.append(clientIdUtf8);

    /* LWT遗嘱消息主题和负载 */
    if (!m_will.topic.isEmpty()) {
        const QByteArray willTopicUtf8 = m_will.topic.toUtf8();
        payload.append(static_cast<char>((willTopicUtf8.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(willTopicUtf8.size() & 0xFF));
        payload.append(willTopicUtf8);
        payload.append(static_cast<char>((m_will.payload.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(m_will.payload.size() & 0xFF));
        payload.append(m_will.payload);
    }

    if (!m_username.isEmpty()) {
        const QByteArray userUtf8 = m_username.toUtf8();
        payload.append(static_cast<char>((userUtf8.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(userUtf8.size() & 0xFF));
        payload.append(userUtf8);
    }
    if (!m_password.isEmpty()) {
        const QByteArray passUtf8 = m_password.toUtf8();
        payload.append(static_cast<char>((passUtf8.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(passUtf8.size() & 0xFF));
        payload.append(passUtf8);
    }
    QByteArray packet = buildMqttPacket(MqttPublish_CONNECT, payload);
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) m_totalBytesSent += static_cast<quint64>(written);
}

/**
 * @file MqttConnectionProtocol.cpp
 * @brief MQTT v3.1.1 线协议报文构建、解析、分发和发布/订阅操作
 *
 * 从 MqttConnection.cpp 拆分而来，包含MQTT报文构建、
 * 剩余长度编码、接收报文解析和各类型报文处理函数，
 * 以及发布/订阅/退订协议操作、CONNECT报文发送和TCP回调槽函数。
 */

#include "connection/mqtt/MqttConnection.h"
#include <QRandomGenerator>

// MQTT v3.1.1 报文类型枚举
enum MqttPacketType {
    CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4,
    SUBSCRIBE = 8, SUBACK = 9, UNSUBSCRIBE = 10,
    PINGREQ = 12, PINGRESP = 13, DISCONNECT = 14
};

/** @brief 构建MQTT协议报文 @param packetType 报文类型 @param payload 负载数据 @return 完整的MQTT报文 */
QByteArray MqttConnection::buildMqttPacket(quint8 packetType, const QByteArray& payload)
{
    QByteArray header;
    header.append(static_cast<char>((packetType << 4) & 0xF0));
    header.append(encodeRemainingLength(payload.size()));
    return header + payload;
}

/** @brief 编码MQTT剩余长度字段(可变长度编码) @param length 剩余长度值 @return 编码后的字节序列 */
QByteArray MqttConnection::encodeRemainingLength(int length)
{
    QByteArray result;
    do {
        quint8 byte = static_cast<quint8>(length % 128);
        length /= 128;
        if (length > 0) byte |= 0x80;
        result.append(static_cast<char>(byte));
    } while (length > 0);
    return result;
}

/** @brief 从接收缓冲区解析完整的MQTT报文并分发到对应处理函数 */
void MqttConnection::parseIncomingPacket()
{
    while (m_rxBuffer.size() >= 2) {
        if (m_expectedLength < 0) {
            int idx = 1;
            quint32 remLen = 0;
            quint32 multiplier = 1;
            while (idx < m_rxBuffer.size()) {
                quint8 byte = static_cast<quint8>(m_rxBuffer.at(idx));
                remLen += (byte & 0x7F) * multiplier;
                multiplier *= 128;
                ++idx;
                if ((byte & 0x80) == 0) break;
                if (idx > 4) { m_rxBuffer.clear(); m_expectedLength = -1; return; }
            }
            if (remLen > 256 * 1024 * 1024) { m_rxBuffer.clear(); m_expectedLength = -1; return; }
            m_expectedLength = static_cast<int>(remLen) + idx;
        }
        if (m_expectedLength < 2 || m_rxBuffer.size() < m_expectedLength) return;
        QByteArray packet = m_rxBuffer.left(m_expectedLength);
        m_rxBuffer.remove(0, m_expectedLength);
        m_expectedLength = -1;
        const quint8 type = (static_cast<quint8>(packet.at(0)) >> 4) & 0x0F;
        const quint8 flags = static_cast<quint8>(packet.at(0)) & 0x0F;
        int headerLen = 1;
        while (headerLen < packet.size()) {
            if ((static_cast<quint8>(packet.at(headerLen)) & 0x80) == 0) { ++headerLen; break; }
            ++headerLen;
        }
        QByteArray data = packet.mid(headerLen);
        switch (type) {
        case CONNACK:   handleConnack(data); break;
        case PUBLISH:   handlePublish(data, flags); break;
        case SUBACK:    handleSuback(data); break;
        case PINGRESP:  break;
        default: break;
        }
    }
}

/** @brief 处理CONNACK响应，根据返回码设置连接状态 @param data CONNACK负载数据 */
void MqttConnection::handleConnack(const QByteArray& data)
{
    if (data.size() < 2) return;
    const quint8 code = static_cast<quint8>(data.at(1));
    if (code == 0) {
        m_state = ConnectionState::Connected;
        emit stateChanged(m_state);
        emit connected();
        m_keepAlive->start();
    } else {
        ++m_errorCount;
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("MQTT连接被拒绝，错误码: %1").arg(code));
    }
}

/** @brief 处理收到的PUBLISH报文，提取主题和负载并回复ACK @param data 可变头部+负载数据 @param flags 报文标志位 */
void MqttConnection::handlePublish(const QByteArray& data, quint8 flags)
{
    if (data.size() < 2) return;
    int topicLen = (static_cast<quint8>(data.at(0)) << 8) | static_cast<quint8>(data.at(1));
    if (data.size() < 2 + topicLen) return;
    QString topic = QString::fromUtf8(data.mid(2, topicLen));
    int offset = 2 + topicLen;
    const int qos = (flags >> 1) & 0x03;
    if (qos > 0 && data.size() >= offset + 2) {
        quint16 recvPacketId = (static_cast<quint8>(data.at(offset)) << 8)
                             | static_cast<quint8>(data.at(offset + 1));
        if (m_socket) {
            if (qos == 1) {
                QByteArray puback;
                puback.append(static_cast<char>((recvPacketId >> 8) & 0xFF));
                puback.append(static_cast<char>(recvPacketId & 0xFF));
                m_socket->write(buildMqttPacket(PUBACK, puback));
            }
        }
        offset += 2;
    }
    QByteArray payload = data.mid(offset);
    ++m_totalBytesReceived;
    ++m_totalReceived;
    emit dataReceived(payload);
    emit messageReceived(topic, payload);
}

/** @brief 处理SUBACK响应，检查每个主题的订阅返回码 @param data SUBACK可变头部+负载数据 */
void MqttConnection::handleSuback(const QByteArray& data)
{
    if (data.size() < 3) {
        ++m_errorCount;
        emit errorOccurred(tr("SUBACK报文长度异常: %1字节").arg(data.size()));
        return;
    }

    const quint16 packetId = (static_cast<quint8>(data.at(0)) << 8)
                           | static_cast<quint8>(data.at(1));

    for (int i = 2; i < data.size(); ++i) {
        const quint8 returnCode = static_cast<quint8>(data.at(i));
        if (returnCode == 0x80) {
            ++m_errorCount;
            emit errorOccurred(
                tr("MQTT订阅被拒绝(PacketID=%1, 第%2个主题)")
                    .arg(packetId)
                    .arg(i - 1));
        }
    }
}

/** @brief 生成随机客户端ID @return "EmbedDebug_XXXXXX" 格式的客户端标识 */
QString MqttConnection::generateClientId()
{
    return QStringLiteral("EmbedDebug_%1").arg(QRandomGenerator::global()->bounded(100000, 999999));
}

// ─── 发布/订阅/退订 ─────────────────────────────────────

/** @brief 向指定主题发布消息 @param topic 主题名 @param payload 消息负载 @param qos 服务质量等级(0/1/2) @return true=发布成功 */
bool MqttConnection::publish(const QString& topic, const QByteArray& payload, int qos)
{
    if (m_state != ConnectionState::Connected) return false;
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
    QByteArray packet = buildMqttPacket(PUBLISH, variableHeader);
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
    if (m_state != ConnectionState::Connected) return false;
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
    QByteArray packet = buildMqttPacket(SUBSCRIBE, hdr + payload);
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
    if (m_state != ConnectionState::Connected) return;
    QByteArray payload;
    const QByteArray topicUtf8 = topic.toUtf8();
    payload.append(static_cast<char>((topicUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(topicUtf8.size() & 0xFF));
    payload.append(topicUtf8);
    m_packetId = (m_packetId % 65535) + 1;
    QByteArray hdr;
    hdr.append(static_cast<char>((m_packetId >> 8) & 0xFF));
    hdr.append(static_cast<char>(m_packetId & 0xFF));
    QByteArray packet = buildMqttPacket(UNSUBSCRIBE, hdr + payload);
    packet[0] |= 0x02;
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) m_totalBytesSent += static_cast<quint64>(written);
    m_subscriptions.removeAll(topic);
}

// ─── 槽函数 ──────────────────────────────────────────────

/** @brief TCP连接建立成功回调，发送MQTT CONNECT报文 */
void MqttConnection::onSocketConnected() { sendConnect(); flushPendingQueue(); }

/** @brief TCP连接断开回调，更新状态并停止心跳 */
void MqttConnection::onSocketDisconnected()
{
    m_keepAlive->stop();
    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
    emit disconnected();
}

/** @brief 底层TCP数据到达回调，追加到接收缓冲区并解析 */
void MqttConnection::onSocketReadyRead()
{
    QByteArray newData = m_socket->readAll();
    m_totalBytesReceived += static_cast<quint64>(newData.size());
    m_rxBuffer.append(newData);
    parseIncomingPacket();
}

/** @brief 心跳定时器回调，发送PINGREQ保活报文并统计 */
void MqttConnection::onKeepAlive()
{
    if (m_state == ConnectionState::Connected) {
        QByteArray packet = buildMqttPacket(PINGREQ, {});
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
    QByteArray packet = buildMqttPacket(CONNECT, payload);
    qint64 written = m_socket->write(packet);
    if (written == packet.size()) m_totalBytesSent += static_cast<quint64>(written);
}

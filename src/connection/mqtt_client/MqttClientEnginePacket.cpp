/**
 * @file MqttClientEnginePacket.cpp
 * @brief MQTT客户端引擎 — 接收报文解析与编解码辅助
 *
 * 处理MQTT v3.1.1所有入站报文(CONNACK/PUBLISH/PUBACK/PUBREC/PUBREL/
 * PUBCOMP/SUBACK/UNSUBACK/PINGRESP)的解析与响应，以及剩余长度编解码和
 * 主题过滤器构建辅助方法。
 */

#include "connection/mqtt_client/MqttClientEngine.h"

/** MQTT固定报文类型 */
enum MqttPacketType {
    CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4,
    PUBREC = 5, PUBREL = 6, PUBCOMP = 7, SUBSCRIBE = 8,
    SUBACK = 9, UNSUBSCRIBE = 10, UNSUBACK = 11, PINGREQ = 12,
    PINGRESP = 13, DISCONNECT = 14
};

// ── 接收报文解析 ────────────────────────────────────────────

/** @brief 处理接收缓冲区中的所有完整报文 */
void MqttClientEngine::handleIncomingPacket()
{
    while (m_rxBuffer.size() >= 2) {
        int offset = 1;
        int remainLen = decodeRemainingLength(m_rxBuffer, &offset);
        if (remainLen < 0) {
            return; /* 数据不完整 */
        }
        int totalLen = offset + remainLen;
        if (m_rxBuffer.size() < totalLen) {
            return; /* 数据不完整 */
        }

        int header = static_cast<quint8>(m_rxBuffer[0]);
        QByteArray data = m_rxBuffer.mid(offset, remainLen);
        m_rxBuffer.remove(0, totalLen);
        m_totalBytesReceived += totalLen;

        int type = (header >> 4) & 0x0F;
        switch (type) {
        case CONNACK:   processConnack(data); break;
        case PUBLISH:   processPublish(data, header); break;
        case PUBACK:    processPuback(data); break;
        case PUBREC:    processPubrec(data); break;
        case PUBREL:    processPubrel(data); break;
        case PUBCOMP:   processPubcomp(data); break;
        case SUBACK:    processSuback(data); break;
        case UNSUBACK:  processUnsuback(data); break;
        case PINGRESP:  processPingresp(); break;
        default: break;
        }
    }
}

/** @brief 处理CONNACK报文 */
void MqttClientEngine::processConnack(const QByteArray& data)
{
    if (data.size() < 2) {
        return;
    }
    quint8 returnCode = static_cast<quint8>(data[1]);
    if (returnCode == 0) {
        m_connected = true;
        m_reconnectAttempts = 0;
        if (m_reconnectAttempts == 0 && m_connectionAttempts > 1) {
            ++m_successfulReconnects;
        }
        int ka = m_params.keepAlive > 0 ? m_params.keepAlive : 60;
        m_keepAliveTimer->start(ka * 1000);
        flushPendingQueue();
        emit connected();
    } else {
        m_connected = false;
        emit connectionError(tr("连接被拒绝，错误码: %1").arg(returnCode));
    }
}

/** @brief 处理收到的PUBLISH报文 */
void MqttClientEngine::processPublish(const QByteArray& data, int header)
{
    if (data.size() < 2) {
        return;
    }
    int pos = 0;
    int topicLen = (static_cast<quint8>(data[pos]) << 8) |
                    static_cast<quint8>(data[pos + 1]);
    pos += 2;
    if (pos + topicLen > data.size()) {
        return;
    }
    QString topic = QString::fromUtf8(data.mid(pos, topicLen));
    pos += topicLen;

    MqttQos qos = static_cast<MqttQos>((header >> 1) & 0x03);
    quint16 pid = 0;
    if (qos != MqttQos::QoS0) {
        if (pos + 2 > data.size()) {
            return;
        }
        pid = (static_cast<quint8>(data[pos]) << 8) |
               static_cast<quint8>(data[pos + 1]);
        pos += 2;
    }

    QByteArray payload = data.mid(pos);
    bool retained = (header & 0x01) != 0;

    ++m_totalReceived;

    /* QoS1 → PUBACK */
    if (qos == MqttQos::QoS1) {
        QByteArray ack;
        ack.append(static_cast<char>(static_cast<int>(PUBACK) << 4));
        ack.append(static_cast<char>(2));
        ack.append(static_cast<char>((pid >> 8) & 0xFF));
        ack.append(static_cast<char>(pid & 0xFF));
        m_socket->write(ack);
    }
    /* QoS2 → PUBREC (简化处理) */
    if (qos == MqttQos::QoS2) {
        QByteArray rec;
        rec.append(static_cast<char>(static_cast<int>(PUBREC) << 4));
        rec.append(static_cast<char>(2));
        rec.append(static_cast<char>((pid >> 8) & 0xFF));
        rec.append(static_cast<char>(pid & 0xFF));
        m_socket->write(rec);
    }

    MqttMessage msg{topic, payload, qos, retained,
                    QDateTime::currentDateTime(), pid};
    emit messageReceived(msg);
}

/** @brief 处理SUBACK报文 */
void MqttClientEngine::processSuback(const QByteArray& /*data*/) { /* 确认已订阅 */ }

/** @brief 处理UNSUBACK报文 */
void MqttClientEngine::processUnsuback(const QByteArray& /*data*/) { /* 确认已退订 */ }

/** @brief 处理PUBACK报文（QoS1发布确认） */
void MqttClientEngine::processPuback(const QByteArray& data)
{
    if (data.size() >= 2) {
        quint16 pid = (static_cast<quint8>(data[0]) << 8) |
                       static_cast<quint8>(data[1]);
        emit publishAcked(pid);
    }
}

/** @brief 处理PUBREC报文（QoS2第一步），回复PUBREL */
void MqttClientEngine::processPubrec(const QByteArray& data)
{
    if (data.size() >= 2) {
        quint16 pid = (static_cast<quint8>(data[0]) << 8) |
                       static_cast<quint8>(data[1]);
        QByteArray rel;
        rel.append(static_cast<char>(static_cast<int>(PUBREL) << 4 | 0x02));
        rel.append(static_cast<char>(2));
        rel.append(static_cast<char>((pid >> 8) & 0xFF));
        rel.append(static_cast<char>(pid & 0xFF));
        m_socket->write(rel);
    }
}

/** @brief 处理PUBREL报文（QoS2第二步），回复PUBCOMP */
void MqttClientEngine::processPubrel(const QByteArray& data)
{
    if (data.size() >= 2) {
        quint16 pid = (static_cast<quint8>(data[0]) << 8) |
                       static_cast<quint8>(data[1]);
        QByteArray comp;
        comp.append(static_cast<char>(static_cast<int>(PUBCOMP) << 4));
        comp.append(static_cast<char>(2));
        comp.append(static_cast<char>((pid >> 8) & 0xFF));
        comp.append(static_cast<char>(pid & 0xFF));
        m_socket->write(comp);
        emit publishAcked(pid);
    }
}

/** @brief 处理PUBCOMP报文（QoS2完成） */
void MqttClientEngine::processPubcomp(const QByteArray& data)
{
    if (data.size() >= 2) {
        quint16 pid = (static_cast<quint8>(data[0]) << 8) |
                       static_cast<quint8>(data[1]);
        emit publishAcked(pid);
    }
}

/** @brief 处理PINGRESP报文 */
void MqttClientEngine::processPingresp() { /* 心跳已确认 */ }

// ── 编解码辅助 ────────────────────────────────────────────────

/** @brief 编码MQTT剩余长度字段 @param length 长度值 @return 编码后的字节 */
QByteArray MqttClientEngine::encodeRemainingLength(int length)
{
    QByteArray result;
    do {
        quint8 byte = static_cast<quint8>(length % 128);
        length /= 128;
        if (length > 0) {
            byte |= 0x80;
        }
        result.append(static_cast<char>(byte));
    } while (length > 0);
    return result;
}

/** @brief 解码MQTT剩余长度字段 @param data 数据 @param offset 起始偏移 @return 长度值（-1=不完整） */
int MqttClientEngine::decodeRemainingLength(const QByteArray& data, int* offset)
{
    int multiplier = 1;
    int value = 0;
    int idx = *offset;
    quint8 byte = 0;
    do {
        if (idx >= data.size()) {
            return -1;
        }
        byte = static_cast<quint8>(data[idx]);
        value += (byte & 0x7F) * multiplier;
        multiplier *= 128;
        ++idx;
    } while ((byte & 0x80) != 0);
    *offset = idx;
    return value;
}

/** @brief 构建主题过滤器字节（未使用，保留扩展） */
QByteArray MqttClientEngine::buildTopicFilter(const QString& topic)
{
    QByteArray tf = topic.toUtf8();
    QByteArray result;
    result.append(static_cast<char>((tf.size() >> 8) & 0xFF));
    result.append(static_cast<char>(tf.size() & 0xFF));
    result.append(tf);
    return result;
}

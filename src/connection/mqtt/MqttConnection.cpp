/**
 * @file MqttConnection.cpp
 * @brief MQTT客户端连接实现 — MQTT v3.1.1线协议
 */

#include "connection/mqtt/MqttConnection.h"
#include <QRandomGenerator>

enum MqttPacketType {
    CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4,
    PUBREC = 5, PUBREL = 6, PUBCOMP = 7, SUBSCRIBE = 8,
    SUBACK = 9, UNSUBSCRIBE = 10, UNSUBACK = 11, PINGREQ = 12,
    PINGRESP = 13, DISCONNECT = 14
};

MqttConnection::MqttConnection(QObject* parent)
    : IConnection(parent)
    , m_socket(new QTcpSocket(this))
    , m_keepAlive(new QTimer(this))
{
    m_keepAlive->setInterval(m_keepAliveInterval * 1000);
    m_clientId = generateClientId();

    connect(m_socket, &QTcpSocket::readyRead,
            this, &MqttConnection::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::connected,
            this, &MqttConnection::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &MqttConnection::onSocketDisconnected);
    connect(m_keepAlive, &QTimer::timeout,
            this, &MqttConnection::onKeepAlive);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError err) {
        Q_UNUSED(err)
        ++m_errorCount;
        m_keepAlive->stop();
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("MQTT连接失败: %1").arg(m_socket->errorString()));
    });
}

MqttConnection::~MqttConnection() { close(); }

ConnectionType MqttConnection::type() const { return ConnectionType::Mqtt; }

QString MqttConnection::name() const
{
    return m_host.isEmpty() ? tr("未配置")
         : QStringLiteral("%1:%2").arg(m_host).arg(m_port);
}

ConnectionState MqttConnection::state() const { return m_state; }

bool MqttConnection::open()
{
    if (m_host.isEmpty()) {
        emit errorOccurred(tr("MQTT服务器地址未配置"));
        return false;
    }
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);
    m_socket->connectToHost(m_host, m_port);
    return true;
}

void MqttConnection::close()
{
    m_keepAlive->stop();
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        if (m_state == ConnectionState::Connected) {
            qint64 w = m_socket->write(buildMqttPacket(DISCONNECT, {}));
            if (w > 0) m_totalBytesSent += static_cast<quint64>(w);
            m_socket->flush();
        }
        m_socket->disconnectFromHost();
    } else if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    if (m_state != ConnectionState::Disconnected) {
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
        emit disconnected();
    }
    m_rxBuffer.clear();
    m_expectedLength = -1;
}

qint64 MqttConnection::write(const QByteArray& data)
{
    return publish("embeddebug/out", data, 0) ? data.size() : -1;
}

void MqttConnection::configure(const QVariantMap& params)
{
    if (params.contains("host"))          m_host = params.value("host").toString();
    if (params.contains("port"))          m_port = params.value("port").toInt();
    if (params.contains("clientId"))      m_clientId = params.value("clientId").toString();
    if (params.contains("username"))      m_username = params.value("username").toString();
    if (params.contains("password"))      m_password = params.value("password").toString();
    if (params.contains("keepAlive")) {
        m_keepAliveInterval = params.value("keepAlive").toInt();
        m_keepAlive->setInterval(m_keepAliveInterval * 1000);
    }
    if (params.contains("cleanSession"))  m_cleanSession = params.value("cleanSession").toBool();
}

void MqttConnection::connectToHost(const QString& host, int port)
{
    m_host = host;
    m_port = port;
    open();
}

void MqttConnection::disconnectFromHost() { close(); }

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
        m_totalBytesSent += static_cast<quint64>(written);
        return true;
    }
    ++m_errorCount;
    return false;
}

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

void MqttConnection::onSocketConnected() { sendConnect(); }

void MqttConnection::onSocketDisconnected()
{
    m_keepAlive->stop();
    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
    emit disconnected();
}

void MqttConnection::onSocketReadyRead()
{
    QByteArray newData = m_socket->readAll();
    m_totalBytesReceived += static_cast<quint64>(newData.size());
    m_rxBuffer.append(newData);
    parseIncomingPacket();
}

void MqttConnection::onKeepAlive()
{
    if (m_state == ConnectionState::Connected) {
        QByteArray packet = buildMqttPacket(PINGREQ, {});
        qint64 written = m_socket->write(packet);
        if (written == packet.size()) m_totalBytesSent += static_cast<quint64>(written);
    }
}

void MqttConnection::sendConnect()
{
    QByteArray payload;
    payload.append(static_cast<char>(0));
    payload.append(static_cast<char>(4));
    payload.append("MQTT");
    payload.append(static_cast<char>(4));
    quint8 flags = 0x02;
    if (!m_username.isEmpty()) flags |= 0x80;
    if (!m_password.isEmpty()) flags |= 0x40;
    payload.append(static_cast<char>(flags));
    payload.append(static_cast<char>((m_keepAliveInterval >> 8) & 0xFF));
    payload.append(static_cast<char>(m_keepAliveInterval & 0xFF));
    const QByteArray clientIdUtf8 = m_clientId.toUtf8();
    payload.append(static_cast<char>((clientIdUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(clientIdUtf8.size() & 0xFF));
    payload.append(clientIdUtf8);
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

QByteArray MqttConnection::buildMqttPacket(quint8 packetType, const QByteArray& payload)
{
    QByteArray header;
    header.append(static_cast<char>((packetType << 4) & 0xF0));
    header.append(encodeRemainingLength(payload.size()));
    return header + payload;
}

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
                qint64 w = m_socket->write(buildMqttPacket(0x04, puback));
                if (w > 0) m_totalBytesSent += static_cast<quint64>(w);
            } else if (qos == 2) {
                QByteArray pubrec;
                pubrec.append(static_cast<char>((recvPacketId >> 8) & 0xFF));
                pubrec.append(static_cast<char>(recvPacketId & 0xFF));
                qint64 w = m_socket->write(buildMqttPacket(0x05, pubrec));
                if (w > 0) m_totalBytesSent += static_cast<quint64>(w);
            }
        }
        offset += 2;
    }
    QByteArray payload = data.mid(offset);
    emit messageReceived(topic, payload);
    emit dataReceived(payload);
}

void MqttConnection::handleSuback(const QByteArray& data) { Q_UNUSED(data) }

QString MqttConnection::generateClientId()
{
    return QStringLiteral("EmbedDebug_%1").arg(QRandomGenerator::global()->bounded(100000, 999999));
}

int MqttConnection::subscriptionCount() const { return m_subscriptions.size(); }

quint64 MqttConnection::totalPublishes() const { return m_totalPublishes; }
quint64 MqttConnection::totalSubscriptions() const { return m_totalSubscriptions; }
quint64 MqttConnection::totalBytesSent() const { return m_totalBytesSent; }
quint64 MqttConnection::totalBytesReceived() const { return m_totalBytesReceived; }
quint64 MqttConnection::errorCount() const { return m_errorCount; }

void MqttConnection::resetStats()
{
    m_totalPublishes = 0;
    m_totalSubscriptions = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}

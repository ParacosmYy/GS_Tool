/**
 * @file MqttConnection.cpp
 * @brief MQTT客户端连接实现 — 连接管理、发布/订阅、统计
 *
 * MQTT v3.1.1线协议报文构建和解析见 MqttConnectionProtocol.cpp。
 */

#include "connection/mqtt/MqttConnection.h"

enum MqttPacketType {
    CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4,
    PUBREC = 5, PUBREL = 6, PUBCOMP = 7, SUBSCRIBE = 8,
    SUBACK = 9, UNSUBSCRIBE = 10, UNSUBACK = 11, PINGREQ = 12,
    PINGRESP = 13, DISCONNECT = 14
};

/** @brief 构造函数，初始化MQTT客户端 @param parent 父对象 */
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

/** @brief 析构函数，关闭连接并释放资源 */
MqttConnection::~MqttConnection() { close(); }

/** @brief 获取连接类型 @return Mqtt类型 */
ConnectionType MqttConnection::type() const { return ConnectionType::Mqtt; }

/** @brief 获取连接显示名称 @return "主机:端口" 或 "未配置" */
QString MqttConnection::name() const
{
    return m_host.isEmpty() ? tr("未配置")
         : QStringLiteral("%1:%2").arg(m_host).arg(m_port);
}

/** @brief 获取当前连接状态 @return 连接状态枚举 */
ConnectionState MqttConnection::state() const { return m_state; }

/** @brief 打开MQTT连接，发起TCP握手 @return true=成功发起连接 */
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

/** @brief 关闭MQTT连接，发送DISCONNECT报文后断开TCP */
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

/** @brief 向默认主题写入数据(QoS0发布) @param data 待发送数据 @return 成功返回数据大小，失败返回-1 */
qint64 MqttConnection::write(const QByteArray& data)
{
    return publish("embeddebug/out", data, 0) ? data.size() : -1;
}

/** @brief 配置MQTT连接参数 @param params 参数映射(支持host/port/clientId/username/password/keepAlive/cleanSession) */
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

/** @brief 连接到指定MQTT服务器 @param host 服务器地址 @param port 端口号 */
void MqttConnection::connectToHost(const QString& host, int port)
{
    m_host = host;
    m_port = port;
    open();
}

/** @brief 断开MQTT连接 */
void MqttConnection::disconnectFromHost() { close(); }

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

/** @brief TCP连接建立成功回调，发送MQTT CONNECT报文 */
void MqttConnection::onSocketConnected() { sendConnect(); }

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

/** @brief 心跳定时器回调，发送PINGREQ保活报文 */
void MqttConnection::onKeepAlive()
{
    if (m_state == ConnectionState::Connected) {
        QByteArray packet = buildMqttPacket(PINGREQ, {});
        qint64 written = m_socket->write(packet);
        if (written == packet.size()) m_totalBytesSent += static_cast<quint64>(written);
    }
}

/** @brief 构建并发送MQTT CONNECT报文，包含客户端ID和认证信息 */
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

// 报文构建/解析/处理见 MqttConnectionProtocol.cpp

/** @brief 获取当前订阅数量 @return 订阅主题数 */
int MqttConnection::subscriptionCount() const { return m_subscriptions.size(); }

/** @brief 获取累计发布消息数 @return 发布计数 */
quint64 MqttConnection::totalPublishes() const { return m_totalPublishes; }
/** @brief 获取累计订阅次数 @return 订阅计数 */
quint64 MqttConnection::totalSubscriptions() const { return m_totalSubscriptions; }
/** @brief 获取累计发送字节数 @return 发送字节数 */
quint64 MqttConnection::totalBytesSent() const { return m_totalBytesSent; }
/** @brief 获取累计接收字节数 @return 接收字节数 */
quint64 MqttConnection::totalBytesReceived() const { return m_totalBytesReceived; }
/** @brief 获取累计错误次数 @return 错误计数 */
quint64 MqttConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计计数器 */
void MqttConnection::resetStats()
{
    m_totalPublishes = 0;
    m_totalSubscriptions = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}

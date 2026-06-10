/**
 * @file MqttClientEngine.cpp
 * @brief MQTT客户端引擎 — 连接管理、发布/订阅、QoS处理、重连逻辑
 *
 * 实现MQTT v3.1.1线协议的核心报文构建与解析，支持QoS0/1/2、
 * KeepAlive心跳、指数退避重连、离线消息队列和LWT遗嘱消息。
 */

#include "connection/mqtt_client/MqttClientEngine.h"

#include <QSslConfiguration>
#include <QHostAddress>

/** MQTT固定报文类型 */
enum MqttPacketType {
    CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4,
    PUBREC = 5, PUBREL = 6, PUBCOMP = 7, SUBSCRIBE = 8,
    SUBACK = 9, UNSUBSCRIBE = 10, UNSUBACK = 11, PINGREQ = 12,
    PINGRESP = 13, DISCONNECT = 14
};

/** @brief 构造函数，初始化引擎和定时器 @param parent 父对象 */
MqttClientEngine::MqttClientEngine(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_keepAliveTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_hasWill(false)
    , m_connected(false)
    , m_nextPacketId(1)
    , m_reconnectAttempts(0)
    , m_maxReconnectAttempts(10)
    , m_maxQueueSize(1000)
    , m_totalPublishes(0)
    , m_totalReceived(0)
    , m_totalBytesSent(0)
    , m_totalBytesReceived(0)
    , m_errorCount(0)
    , m_connectionAttempts(0)
    , m_successfulReconnects(0)
    , m_qos0Publishes(0)
    , m_qos1Publishes(0)
    , m_qos2Publishes(0)
    , m_keepAliveSent(0)
{
    m_keepAliveTimer->setSingleShot(false);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &MqttClientEngine::onReconnectTimeout);
}

/** @brief 析构函数，发送DISCONNECT后关闭连接 */
MqttClientEngine::~MqttClientEngine()
{
    if (m_connected) {
        sendDisconnect();
    }
    if (m_socket) {
        m_socket->abort();
    }
}

// ── 连接管理 ──────────────────────────────────────────────

/** @brief 连接到MQTT服务器 @param params 连接参数 */
void MqttClientEngine::connectToBroker(const MqttConnectionParams& params)
{
    m_params = params;
    m_reconnectAttempts = 0;

    if (m_socket) {
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    if (params.useTls) {
        auto* ssl = new QSslSocket(this);
        QSslConfiguration sslConfig = ssl->sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
        ssl->setSslConfiguration(sslConfig);
        m_socket = ssl;
    } else {
        m_socket = new QTcpSocket(this);
    }

    setupSocketSignals();
    ++m_connectionAttempts;
    m_socket->connectToHost(params.broker, params.port);
}

/** @brief 断开MQTT连接 */
void MqttClientEngine::disconnectFromBroker()
{
    m_reconnectTimer->stop();
    m_keepAliveTimer->stop();
    m_reconnectAttempts = m_maxReconnectAttempts; /* 阻止自动重连 */

    if (m_connected) {
        sendDisconnect();
    }
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

/** @brief 查询连接状态 @return true=已连接 */
bool MqttClientEngine::isConnected() const { return m_connected; }

/** @brief 建立socket信号连接 */
void MqttClientEngine::setupSocketSignals()
{
    connect(m_socket, &QTcpSocket::connected,
            this, &MqttClientEngine::onSocketConnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &MqttClientEngine::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &MqttClientEngine::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError) {
        ++m_errorCount;
        m_keepAliveTimer->stop();
        m_connected = false;
        emit connectionError(tr("连接错误: %1").arg(m_socket->errorString()));
        scheduleReconnect();
    });
}

/** @brief Socket连接成功回调，发送CONNECT报文 */
void MqttClientEngine::onSocketConnected()
{
    sendConnectPacket();
}

/** @brief Socket可读回调，累积接收缓冲并处理报文 */
void MqttClientEngine::onSocketReadyRead()
{
    m_rxBuffer.append(m_socket->readAll());
    handleIncomingPacket();
}

/** @brief Socket断开回调 */
void MqttClientEngine::onSocketDisconnected()
{
    m_connected = false;
    m_keepAliveTimer->stop();
    emit disconnected();
    scheduleReconnect();
}

/** @brief KeepAlive定时器回调，发送PINGREQ */
void MqttClientEngine::onKeepAlive()
{
    if (m_connected) {
        sendPingReq();
    }
}

/** @brief 重连定时器回调 */
void MqttClientEngine::onReconnectTimeout()
{
    if (m_reconnectAttempts < m_maxReconnectAttempts) {
        connectToBroker(m_params);
    }
}

/** @brief 调度重连（指数退避） */
void MqttClientEngine::scheduleReconnect()
{
    if (m_reconnectAttempts >= m_maxReconnectAttempts) {
        return;
    }
    if (m_params.broker.isEmpty()) {
        return;
    }
    ++m_reconnectAttempts;
    int delayMs = qMin(1000 * (1 << (m_reconnectAttempts - 1)), 30000);
    m_reconnectTimer->start(delayMs);
}

// ── CONNECT报文构建 ────────────────────────────────────────

/** @brief 构建并发送CONNECT报文 @return true=发送成功 */
bool MqttClientEngine::sendConnectPacket()
{
    QByteArray packet;
    /* 固定头: CONNECT (0x10) */
    QByteArray variableHeader;
    /* 协议名: "MQTT" */
    variableHeader.append(static_cast<char>(0));
    variableHeader.append(static_cast<char>(4));
    variableHeader.append("MQTT");
    /* 协议级别: 4 = MQTT v3.1.1 */
    variableHeader.append(static_cast<char>(4));
    /* 连接标志 */
    quint8 flags = 0x02; /* Clean Session */
    if (!m_params.username.isEmpty()) {
        flags |= 0x80;
    }
    if (!m_params.password.isEmpty()) {
        flags |= 0x40;
    }
    if (m_hasWill) {
        flags |= 0x04;
        flags |= (static_cast<int>(m_lastWill.qos) & 0x03) << 3;
        if (m_lastWill.retained) {
            flags |= 0x20;
        }
    }
    variableHeader.append(static_cast<char>(flags));
    /* KeepAlive */
    int ka = m_params.keepAlive > 0 ? m_params.keepAlive : 60;
    variableHeader.append(static_cast<char>((ka >> 8) & 0xFF));
    variableHeader.append(static_cast<char>(ka & 0xFF));

    QByteArray payload;
    /* Client ID */
    QByteArray clientIdUtf8 = m_params.clientId.toUtf8();
    payload.append(static_cast<char>((clientIdUtf8.size() >> 8) & 0xFF));
    payload.append(static_cast<char>(clientIdUtf8.size() & 0xFF));
    payload.append(clientIdUtf8);
    /* Will Topic + Payload */
    if (m_hasWill) {
        QByteArray wt = m_lastWill.topic.toUtf8();
        payload.append(static_cast<char>((wt.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(wt.size() & 0xFF));
        payload.append(wt);
        payload.append(static_cast<char>((m_lastWill.payload.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(m_lastWill.payload.size() & 0xFF));
        payload.append(m_lastWill.payload);
    }
    /* Username */
    if (!m_params.username.isEmpty()) {
        QByteArray uname = m_params.username.toUtf8();
        payload.append(static_cast<char>((uname.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(uname.size() & 0xFF));
        payload.append(uname);
    }
    /* Password */
    if (!m_params.password.isEmpty()) {
        QByteArray pwd = m_params.password.toUtf8();
        payload.append(static_cast<char>((pwd.size() >> 8) & 0xFF));
        payload.append(static_cast<char>(pwd.size() & 0xFF));
        payload.append(pwd);
    }

    packet.append(static_cast<char>(0x10));
    packet.append(encodeRemainingLength(variableHeader.size() + payload.size()));
    packet.append(variableHeader);
    packet.append(payload);

    m_totalBytesSent += packet.size();
    return m_socket->write(packet) == packet.size();
}

// ── PUBLISH报文 ────────────────────────────────────────────

/** @brief 发布消息 @param topic 主题 @param payload 负载 @param qos QoS @param retained 保留 @return true=成功 */
bool MqttClientEngine::publish(const QString& topic, const QByteArray& payload,
                                MqttQos qos, bool retained)
{
    if (topic.isEmpty()) {
        return false;
    }
    if (!m_connected) {
        if (m_pendingQueue.size() < m_maxQueueSize) {
            MqttMessage msg{topic, payload, qos, retained, QDateTime::currentDateTime(), 0};
            m_pendingQueue.enqueue(msg);
        }
        return false;
    }
    bool ok = sendPublishPacket(topic, payload, qos, retained);
    if (ok) {
        ++m_totalPublishes;
    }
    return ok;
}

/** @brief 构建并发送PUBLISH报文 @return true=发送成功 */
bool MqttClientEngine::sendPublishPacket(const QString& topic,
                                          const QByteArray& payload,
                                          MqttQos qos, bool retained)
{
    QByteArray packet;
    quint8 header = static_cast<quint8>(PUBLISH) << 4;
    if (retained) {
        header |= 0x01;
    }
    header |= (static_cast<int>(qos) & 0x03) << 1;
    packet.append(static_cast<char>(header));

    QByteArray body;
    QByteArray topicUtf8 = topic.toUtf8();
    body.append(static_cast<char>((topicUtf8.size() >> 8) & 0xFF));
    body.append(static_cast<char>(topicUtf8.size() & 0xFF));
    body.append(topicUtf8);

    quint16 pid = 0;
    if (qos != MqttQos::QoS0) {
        pid = nextPacketId();
        body.append(static_cast<char>((pid >> 8) & 0xFF));
        body.append(static_cast<char>(pid & 0xFF));
    }
    body.append(payload);

    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    m_totalBytesSent += packet.size();
    return m_socket->write(packet) == packet.size();
}

// ── SUBSCRIBE/UNSUBSCRIBE ──────────────────────────────────

/** @brief 订阅主题 @param topic 主题过滤器 @param qos QoS @return true=已发送 */
bool MqttClientEngine::subscribe(const QString& topic, MqttQos qos)
{
    if (topic.isEmpty() || !m_connected) {
        return false;
    }
    bool ok = sendSubscribePacket(topic, qos);
    if (ok && !m_subscriptions.contains(topic)) {
        m_subscriptions.append(topic);
    }
    return ok;
}

/** @brief 构建并发送SUBSCRIBE报文 */
bool MqttClientEngine::sendSubscribePacket(const QString& topic, MqttQos qos)
{
    QByteArray packet;
    packet.append(static_cast<char>(static_cast<int>(SUBSCRIBE) << 4 | 0x02));

    QByteArray body;
    quint16 pid = nextPacketId();
    body.append(static_cast<char>((pid >> 8) & 0xFF));
    body.append(static_cast<char>(pid & 0xFF));

    QByteArray tf = topic.toUtf8();
    body.append(static_cast<char>((tf.size() >> 8) & 0xFF));
    body.append(static_cast<char>(tf.size() & 0xFF));
    body.append(tf);
    body.append(static_cast<char>(static_cast<int>(qos)));

    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    m_totalBytesSent += packet.size();
    return m_socket->write(packet) == packet.size();
}

/** @brief 取消订阅 @param topic 要取消的主题 */
void MqttClientEngine::unsubscribe(const QString& topic)
{
    if (topic.isEmpty() || !m_connected) {
        return;
    }
    sendUnsubscribePacket(topic);
    m_subscriptions.removeAll(topic);
}

/** @brief 构建并发送UNSUBSCRIBE报文 */
bool MqttClientEngine::sendUnsubscribePacket(const QString& topic)
{
    QByteArray packet;
    packet.append(static_cast<char>(static_cast<int>(UNSUBSCRIBE) << 4 | 0x02));

    QByteArray body;
    quint16 pid = nextPacketId();
    body.append(static_cast<char>((pid >> 8) & 0xFF));
    body.append(static_cast<char>(pid & 0xFF));
    QByteArray tf = topic.toUtf8();
    body.append(static_cast<char>((tf.size() >> 8) & 0xFF));
    body.append(static_cast<char>(tf.size() & 0xFF));
    body.append(tf);

    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    m_totalBytesSent += packet.size();
    return m_socket->write(packet) == packet.size();
}

/** @brief 获取当前订阅列表 @return 主题列表 */
QStringList MqttClientEngine::subscriptions() const { return m_subscriptions; }

// ── PINGREQ / DISCONNECT ───────────────────────────────────

/** @brief 发送PINGREQ心跳报文 @return true=发送成功 */
bool MqttClientEngine::sendPingReq()
{
    QByteArray packet;
    packet.append(static_cast<char>(static_cast<int>(PINGREQ) << 4));
    packet.append(static_cast<char>(0));
    m_totalBytesSent += packet.size();
    return m_socket->write(packet) == packet.size();
}

/** @brief 发送DISCONNECT报文 @return true=发送成功 */
bool MqttClientEngine::sendDisconnect()
{
    QByteArray packet;
    packet.append(static_cast<char>(static_cast<int>(DISCONNECT) << 4));
    packet.append(static_cast<char>(0));
    m_totalBytesSent += 2;
    m_socket->write(packet);
    m_socket->flush();
    return true;
}

// ── 遗嘱消息(LWT) ──────────────────────────────────────────

/** @brief 配置遗嘱消息 @param will 遗嘱消息内容 */
void MqttClientEngine::setLastWill(const MqttMessage& will)
{
    m_lastWill = will;
    m_hasWill = true;
}

/** @brief 清除遗嘱消息 */
void MqttClientEngine::clearLastWill() { m_hasWill = false; }

// ── 辅助方法 ────────────────────────────────────────────────

/** @brief 刷出离线消息队列 */
void MqttClientEngine::flushPendingQueue()
{
    while (!m_pendingQueue.isEmpty()) {
        MqttMessage msg = m_pendingQueue.dequeue();
        sendPublishPacket(msg.topic, msg.payload, msg.qos, msg.retained);
        ++m_totalPublishes;
    }
}

/** @brief 生成下一个报文标识符 @return 1~65535 */
quint16 MqttClientEngine::nextPacketId()
{
    quint16 id = m_nextPacketId++;
    if (m_nextPacketId == 0) {
        m_nextPacketId = 1;
    }
    return id;
}

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

/** @brief 构建主题过滤器字节（保留扩展） */
QByteArray MqttClientEngine::buildTopicFilter(const QString& topic)
{
    QByteArray tf = topic.toUtf8();
    QByteArray result;
    result.append(static_cast<char>((tf.size() >> 8) & 0xFF));
    result.append(static_cast<char>(tf.size() & 0xFF));
    result.append(tf);
    return result;
}

    return id;
}


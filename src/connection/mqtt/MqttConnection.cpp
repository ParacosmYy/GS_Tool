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

/** @brief 构造函数，初始化MQTT客户端(含重连定时器) @param parent 父对象 */
MqttConnection::MqttConnection(QObject* parent)
    : IConnection(parent)
    , m_socket(new QTcpSocket(this))
    , m_keepAlive(new QTimer(this))
    , m_retryTimer(new QTimer(this))
{
    m_keepAlive->setInterval(m_keepAliveInterval * 1000);
    m_retryTimer->setSingleShot(true); /* 单次触发，手动重启实现指数退避 */
    m_clientId = generateClientId();

    connect(m_socket, &QTcpSocket::readyRead,
            this, &MqttConnection::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::connected,
            this, &MqttConnection::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &MqttConnection::onSocketDisconnected);
    connect(m_keepAlive, &QTimer::timeout,
            this, &MqttConnection::onKeepAlive);
    connect(m_retryTimer, &QTimer::timeout,
            this, &MqttConnection::onRetryTimeout);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError err) {
        Q_UNUSED(err)
        ++m_errorCount;
        m_keepAlive->stop();
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("MQTT连接失败: %1").arg(m_socket->errorString()));
        /* 触发自动重连(如果启用) */
        scheduleRetry();
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
         : tr("%1:%2").arg(m_host).arg(m_port);
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
    ++m_connectionAttempts;
    m_lastConnectTime = QDateTime::currentDateTime();
    m_socket->connectToHost(m_host, m_port);
    return true;
}

/** @brief 关闭MQTT连接，发送DISCONNECT报文后断开TCP，同时停止重连定时器 */
void MqttConnection::close()
{
    m_keepAlive->stop();
    m_retryTimer->stop();         /* 停止可能正在等待的重连 */
    m_currentRetryCount = 0;      /* 重置重试计数，close()是主动断开 */
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

/** @brief 启用/禁用自动重连 @param enabled true=启用 */
void MqttConnection::setAutoReconnect(bool enabled)
{
    m_autoReconnect = enabled;
    if (!enabled) {
        m_retryTimer->stop();
        m_currentRetryCount = 0;
    }
}

/** @brief 查询自动重连状态 @return true=已启用 */
bool MqttConnection::autoReconnect() const { return m_autoReconnect; }

/** @brief 设置最大重试次数 @param max 最大次数，0=无限重试 */
void MqttConnection::setMaxRetries(int max) { m_maxRetries = qMax(0, max); }

/** @brief 获取最大重试次数 @return 最大重试次数 */
int MqttConnection::maxRetries() const { return m_maxRetries; }

/** @brief 获取当前已重试次数 @return 重试计数 */
int MqttConnection::currentRetryCount() const { return m_currentRetryCount; }

/** @brief 获取累计重试次数 @return 累计重试计数 */
quint64 MqttConnection::totalRetryAttempts() const { return m_totalRetryAttempts; }

/** @brief 获取累计成功重连次数 @return 成功重连计数 */
quint64 MqttConnection::totalSuccessfulReconnects() const { return m_totalSuccessfulReconnects; }

/** @brief 获取平均重试延迟 @return 平均延迟(ms) */
double MqttConnection::avgRetryDelayMs() const
{
    return m_totalRetryAttempts > 0
        ? static_cast<double>(m_totalRetryDelayMs) / static_cast<double>(m_totalRetryAttempts)
        : 0.0;
}

/** @brief 计算指数退避延迟(1s→2s→4s→8s→16s→30s→30s...) @return 延迟时间(ms) */
qint64 MqttConnection::computeBackoffDelay() const
{
    /* 指数增长: delay = min(kMinRetryDelayMs * 2^retryCount, kMaxRetryDelayMs) */
    qint64 delay = kMinRetryDelayMs;
    for (int i = 0; i < m_currentRetryCount; ++i) {
        delay *= 2;
        if (delay >= kMaxRetryDelayMs) {
            delay = kMaxRetryDelayMs;
            break;
        }
    }
    return delay;
}

/** @brief 安排下一次重连尝试(指数退避) */
void MqttConnection::scheduleRetry()
{
    if (!m_autoReconnect || m_host.isEmpty()) {
        return;
    }
    /* 检查是否超过最大重试次数 */
    if (m_maxRetries > 0 && m_currentRetryCount >= m_maxRetries) {
        emit errorOccurred(tr("MQTT重连失败: 已达到最大重试次数(%1)").arg(m_maxRetries));
        return;
    }
    ++m_currentRetryCount;
    m_currentRetryDelayMs = computeBackoffDelay();
    m_totalRetryDelayMs += m_currentRetryDelayMs;
    ++m_totalRetryAttempts;
    m_retryTimer->start(static_cast<int>(m_currentRetryDelayMs));
    emit retryScheduled(m_currentRetryCount, m_currentRetryDelayMs);
}

/** @brief 重连定时器回调，执行指数退避重连 */
void MqttConnection::onRetryTimeout()
{
    if (!m_autoReconnect || m_host.isEmpty()) {
        return;
    }
    open();
}

// publish/subscribe/unsubscribe/onSocket*/onKeepAlive/sendConnect 见 MqttConnectionProtocol.cpp
// 队列/LWT方法见 MqttConnectionQueue.cpp
// 统计getter/resetStats见 MqttConnectionStats.cpp

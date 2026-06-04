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

// publish/subscribe/unsubscribe/onSocket*/onKeepAlive/sendConnect 见 MqttConnectionProtocol.cpp
// 队列/LWT方法见 MqttConnectionQueue.cpp
// 统计getter/resetStats见 MqttConnectionStats.cpp

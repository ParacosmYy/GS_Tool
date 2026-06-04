/**
 * @file WebSocketConnection.cpp
 * @brief WebSocket客户端连接实现 — 基于RFC 6455帧协议
 */

#include "connection/ws/WebSocketConnection.h"

#include <QRandomGenerator>
#include <QSslSocket>
#include <QUrl>

/** @brief 构造WebSocket连接，初始化心跳定时器和延迟追踪 @param parent 父对象 */
WebSocketConnection::WebSocketConnection(QObject* parent)
    : IConnection(parent)
    , m_pingTimer(new QTimer(this))
{
    m_pingTimer->setInterval(30000);
    connect(m_pingTimer, &QTimer::timeout,
            this, &WebSocketConnection::onPingTimeout);
}

/** @brief 析构函数，关闭连接并释放资源 */
WebSocketConnection::~WebSocketConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::WebSocket */
ConnectionType WebSocketConnection::type() const
{
    return ConnectionType::WebSocket;
}

/** @brief 获取连接显示名称 @return 连接URL或默认字符串 */
QString WebSocketConnection::name() const
{
    if (m_state == ConnectionState::Connected && !m_url.isEmpty()) {
        return m_url;
    }
    return tr("WebSocket (未连接)");
}

/** @brief 获取当前连接状态 @return 连接状态枚举 */
ConnectionState WebSocketConnection::state() const
{
    return m_state;
}

/** @brief 打开连接(使用当前URL发起WebSocket连接) @return true=连接已发起 */
bool WebSocketConnection::open()
{
    return connectToUrl(m_url);
}

/** @brief 关闭WebSocket连接(发送close帧后关闭TCP，清空消息队列) */
void WebSocketConnection::close()
{
    if (m_pingTimer) {
        m_pingTimer->stop();
    }
    if (m_socket && m_handshakeDone) {
        // 发送close帧
        m_socket->write(buildFrame(0x08, QByteArray()));
        m_socket->waitForBytesWritten(500);
    }
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_handshakeDone = false;
    m_buffer.clear();
    m_sendQueue.clear();
    m_connectionTimer.invalidate();
    updateState(ConnectionState::Disconnected);
}

/** @brief 发送二进制数据(委托给sendBinaryMessage) @param data 待发送数据 @return 发送字节数 */
qint64 WebSocketConnection::write(const QByteArray& data)
{
    return sendBinaryMessage(data);
}

/** @brief 配置WebSocket参数(URL和子协议) @param params 配置键值对 */
void WebSocketConnection::configure(const QVariantMap& params)
{
    if (params.contains("url")) {
        m_url = params["url"].toString();
    }
    if (params.contains("protocol")) {
        m_protocol = params["protocol"].toString();
    }
}

/** @brief 连接到指定URL(解析URL并发起TCP+HTTP升级) @param url WebSocket地址 @return true=连接已发起 */
bool WebSocketConnection::connectToUrl(const QString& url)
{
    // 无论当前状态，先清理旧连接
    if (m_socket) {
        close();
    }

    m_url = url;
    QUrl wsUrl(url);
    if (!wsUrl.isValid()) {
        emit errorOccurred(tr("无效的WebSocket URL"));
        return false;
    }

    m_host = wsUrl.host();
    m_port = static_cast<quint16>(wsUrl.port(wsUrl.scheme() == "wss" ? 443 : 80));
    m_path = wsUrl.path().isEmpty() ? QStringLiteral("/") : wsUrl.path();
    if (!wsUrl.query().isEmpty()) {
        m_path += QStringLiteral("?") + wsUrl.query();
    }

    // 生成随机Sec-WebSocket-Key
    QByteArray randomBytes(16, 0);
    for (int i = 0; i < 16; ++i) {
        randomBytes[i] = static_cast<char>(QRandomGenerator::global()->generate());
    }
    m_handshakeKey = randomBytes.toBase64();

    // 创建TCP socket
    m_socket = new QTcpSocket(this);
    m_handshakeDone = false;
    m_buffer.clear();
    m_sendQueue.clear();
    m_connectionTimer.invalidate();

    connect(m_socket, &QTcpSocket::connected,
            this, &WebSocketConnection::onTcpConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &WebSocketConnection::onTcpDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &WebSocketConnection::onTcpReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError err) {
        Q_UNUSED(err)
        ++m_errorCount;
        emit errorOccurred(m_socket->errorString());
        updateState(ConnectionState::Error);
    });

    updateState(ConnectionState::Connecting);
    m_socket->connectToHost(m_host, m_port);
    return true;
}

// 帧构建、解析、消息发送和统计见 WebSocketFrame.cpp

/** @brief TCP连接成功回调，发送HTTP Upgrade握手 */
void WebSocketConnection::onTcpConnected()
{
    sendHandshake();
}

/** @brief TCP断开回调，停止心跳并更新连接状态 */
void WebSocketConnection::onTcpDisconnected()
{
    m_pingTimer->stop();
    m_handshakeDone = false;
    updateState(ConnectionState::Disconnected);
}

/** @brief TCP数据就绪回调，先完成握手再解析WebSocket帧 */
void WebSocketConnection::onTcpReadyRead()
{
    if (!m_socket) { return; }
    m_buffer.append(m_socket->readAll());

    if (!m_handshakeDone) {
        if (!parseHandshakeResponse()) { return; }
        m_connectionTimer.start();  // 连接成功，启动运行计时器
        updateState(ConnectionState::Connected);
        m_pingTimer->start();
    }

    parseFrames();
}

/** @brief 心跳定时器触发，发送ping帧保持连接活跃并记录发送时间戳 */
void WebSocketConnection::onPingTimeout()
{
    m_pingSendTime.start();  // 记录ping发送时刻，用于延迟计算
    ping();
}

/** @brief 更新连接状态并发射stateChanged信号 @param newState 新的连接状态 */
void WebSocketConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/** @brief 发送HTTP Upgrade握手请求 */
void WebSocketConnection::sendHandshake()
{
    QString req = QString("GET %1 HTTP/1.1\r\n"
                          "Host: %2\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: %3\r\n"
                          "Sec-WebSocket-Version: 13\r\n")
                      .arg(m_path, m_host, m_handshakeKey);
    if (!m_protocol.isEmpty()) {
        req += QString("Sec-WebSocket-Protocol: %1\r\n").arg(m_protocol);
    }
    req += "\r\n";
    m_socket->write(req.toUtf8());
    m_socket->waitForBytesWritten(3000);
}

/** @brief 检查握手响应是否完整并解析HTTP 101状态码 @return true=握手成功 */
bool WebSocketConnection::parseHandshakeResponse()
{
    if (!m_buffer.contains("\r\n\r\n")) {
        return false;
    }
    int headerEnd = m_buffer.indexOf("\r\n\r\n");
    QString header = QString::fromUtf8(m_buffer.left(headerEnd));

    if (!header.contains("101")) {
        ++m_errorCount;
        emit errorOccurred(tr("WebSocket握手失败: %1").arg(header.left(64)));
        updateState(ConnectionState::Error);
        return false;
    }

    m_buffer.remove(0, headerEnd + 4);
    m_handshakeDone = true;
    ++m_totalConnections;  // WebSocket握手成功计为一次连接
    return true;
}

// 帧构建、解析、消息发送和统计见 WebSocketFrame.cpp

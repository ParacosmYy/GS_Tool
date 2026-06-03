/**
 * @file WebSocketConnection.cpp
 * @brief WebSocket客户端连接实现 — 基于RFC 6455帧协议
 */

#include "connection/ws/WebSocketConnection.h"

#include <QRandomGenerator>
#include <QSslSocket>
#include <QUrl>

/** @brief 构造WebSocket连接，初始化心跳定时器 @param parent 父对象 */
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

/** @brief 关闭WebSocket连接(发送close帧后关闭TCP) */
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

/** @brief 构建WebSocket帧(RFC 6455) @param opcode 操作码(0x01/0x02/0x08/0x09/0x0A) @param payload 载荷数据 @return 完整帧字节数组 */
QByteArray WebSocketConnection::buildFrame(quint8 opcode,
                                            const QByteArray& payload) const
{
    QByteArray frame;
    quint8 byte1 = 0x80 | opcode; // FIN=1 + opcode
    frame.append(static_cast<char>(byte1));

    int len = payload.size();
    if (len <= 125) {
        frame.append(static_cast<char>(0x80 | len)); // MASK=1 + length
    } else if (len <= 65535) {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((len >> 8) & 0xFF));
        frame.append(static_cast<char>(len & 0xFF));
    } else {
        frame.append(static_cast<char>(0x80 | 127));
        quint64 l = static_cast<quint64>(len);
        for (int i = 56; i >= 0; i -= 8) {
            frame.append(static_cast<char>((l >> i) & 0xFF));
        }
    }

    // 生成4字节mask key
    QByteArray maskKey(4, 0);
    for (int i = 0; i < 4; ++i) {
        maskKey[i] = static_cast<char>(QRandomGenerator::global()->generate());
    }
    frame.append(maskKey);

    // 掩码处理payload
    QByteArray masked = payload;
    for (int i = 0; i < masked.size(); ++i) {
        masked[i] = masked[i] ^ maskKey[i % 4];
    }
    frame.append(masked);
    return frame;
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

/** @brief 解析接收缓冲区中的WebSocket帧，按opcode分发处理 */
void WebSocketConnection::parseFrames()
{
    while (m_buffer.size() >= 2) {
        quint8 byte1 = static_cast<quint8>(m_buffer[0]);
        quint8 byte2 = static_cast<quint8>(m_buffer[1]);
        int opcode = byte1 & 0x0F;
        bool masked = (byte2 & 0x80) != 0;
        quint64 payloadLen = byte2 & 0x7F;

        int headerSize = 2;
        if (payloadLen == 126) {
            if (m_buffer.size() < 4) { break; }
            payloadLen = (static_cast<quint8>(m_buffer[2]) << 8)
                         | static_cast<quint8>(m_buffer[3]);
            headerSize = 4;
        } else if (payloadLen == 127) {
            if (m_buffer.size() < 10) { break; }
            payloadLen = 0;
            for (int i = 0; i < 8; ++i) {
                payloadLen = (payloadLen << 8)
                             | static_cast<quint8>(m_buffer[2 + i]);
            }
            headerSize = 10;
        }

        int maskSize = masked ? 4 : 0;
        // 防御超大帧导致int溢出
        if (payloadLen > static_cast<quint64>(INT_MAX) - headerSize - maskSize) {
            m_buffer.clear();
            return;
        }
        int totalFrameSize = headerSize + maskSize
                             + static_cast<int>(payloadLen);
        if (m_buffer.size() < totalFrameSize) { break; }

        // 提取payload
        QByteArray payload = m_buffer.mid(headerSize + maskSize,
                                          static_cast<int>(payloadLen));

        // 解除掩码
        if (masked) {
            QByteArray maskKey = m_buffer.mid(headerSize, 4);
            for (int i = 0; i < payload.size(); ++i) {
                payload[i] = payload[i] ^ maskKey[i % 4];
            }
        }

        m_buffer.remove(0, totalFrameSize);

        // 按opcode分发 — 统计消息接收和字节数
        switch (opcode) {
        case 0x01: // 文本帧
            ++m_totalMessagesReceived;
            m_totalBytesReceived += static_cast<quint64>(payload.size());
            emit textMessageReceived(QString::fromUtf8(payload));
            emit dataReceived(payload);
            break;
        case 0x02: // 二进制帧
            ++m_totalMessagesReceived;
            m_totalBytesReceived += static_cast<quint64>(payload.size());
            emit binaryMessageReceived(payload);
            emit dataReceived(payload);
            break;
        case 0x08: // close帧
            close();
            return;
        case 0x09: // ping → 回复pong
            if (m_socket) {
                m_socket->write(buildFrame(0x0A, payload));
            }
            break;
        case 0x0A: // pong
            emit pongReceived(payload);
            break;
        default:
            break;
        }
    }
}

/** @brief 发送文本消息(构建0x01文本帧并发送) @param message 文本内容 @return 发送字节数 */
qint64 WebSocketConnection::sendTextMessage(const QString& message)
{
    if (!m_socket || !m_handshakeDone) { return -1; }
    QByteArray frame = buildFrame(0x01, message.toUtf8());
    qint64 written = m_socket->write(frame);
    if (written > 0) {
        ++m_totalMessagesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    }
    return written;
}

/** @brief 发送二进制消息(构建0x02二进制帧并发送) @param data 二进制数据 @return 发送字节数 */
qint64 WebSocketConnection::sendBinaryMessage(const QByteArray& data)
{
    if (!m_socket || !m_handshakeDone) { return -1; }
    QByteArray frame = buildFrame(0x02, data);
    qint64 written = m_socket->write(frame);
    if (written > 0) {
        ++m_totalMessagesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    }
    return written;
}

/** @brief 发送ping帧(心跳检测) @param payload ping载荷数据 @return true=发送成功 */
bool WebSocketConnection::ping(const QByteArray& payload)
{
    if (!m_socket || !m_handshakeDone) { return false; }
    QByteArray frame = buildFrame(0x09, payload);
    return m_socket->write(frame) == frame.size();
}

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
        updateState(ConnectionState::Connected);
        m_pingTimer->start();
    }

    parseFrames();
}

/** @brief 心跳定时器触发，发送ping帧保持连接活跃 */
void WebSocketConnection::onPingTimeout()
{
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

// ---- 统计接口实现 ----

/** @brief 获取累计WebSocket连接成功次数 */
quint64 WebSocketConnection::totalConnections() const { return m_totalConnections; }

/** @brief 获取已发送消息总数(文本+二进制) */
quint64 WebSocketConnection::totalMessagesSent() const { return m_totalMessagesSent; }

/** @brief 获取已接收消息总数(文本+二进制) */
quint64 WebSocketConnection::totalMessagesReceived() const { return m_totalMessagesReceived; }

/** @brief 获取已发送字节总数(帧级别) */
quint64 WebSocketConnection::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取已接收字节总数(帧级别) */
quint64 WebSocketConnection::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取错误计数 */
quint64 WebSocketConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计数据为零 */
void WebSocketConnection::resetStats()
{
    m_totalConnections = 0;
    m_totalMessagesSent = 0;
    m_totalMessagesReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}

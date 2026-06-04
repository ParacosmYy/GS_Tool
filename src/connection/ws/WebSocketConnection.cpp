/**
 * @file WebSocketConnection.cpp
 * @brief WebSocket客户端连接实现 — 基于RFC 6455帧协议
 */

#include "connection/ws/WebSocketConnection.h"

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
        ++m_totalCloseFramesSent;  ///< 累计发送close帧次数
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

// 帧构建、解析、消息发送和统计见 WebSocketFrame.cpp
// connectToUrl/TCP回调/握手/状态更新见 WebSocketConnectionHandshake.cpp

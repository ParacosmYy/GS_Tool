/**
 * @file WebSocketFrame.cpp
 * @brief WebSocket帧构建、解析和消息发送
 *
 * 从 WebSocketConnection.cpp 拆分而来，包含RFC 6455帧构建、
 * 帧解析、文本/二进制消息发送、ping/pong和统计接口。
 */

#include "connection/ws/WebSocketConnection.h"

#include <QRandomGenerator>

// ============================================================================
// 帧构建(RFC 6455)
// ============================================================================

/** @brief 构建WebSocket帧(RFC 6455) @param opcode 操作码(0x01/0x02/0x08/0x09/0x0A) @param payload 载荷数据 @return 完整帧字节数组 */
QByteArray WebSocketConnection::buildFrame(quint8 opcode,
                                            const QByteArray& payload) const
{
    QByteArray frame;
    quint8 byte1 = 0x80 | opcode; // FIN=1 + opcode
    frame.append(static_cast<char>(byte1));

    int len = payload.size();
    if (len <= 125) {
        frame.append(static_cast<char>(0x80 | len));
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

    QByteArray maskKey(4, 0);
    for (int i = 0; i < 4; ++i) {
        maskKey[i] = static_cast<char>(QRandomGenerator::global()->generate());
    }
    frame.append(maskKey);

    QByteArray masked = payload;
    for (int i = 0; i < masked.size(); ++i) {
        masked[i] = masked[i] ^ maskKey[i % 4];
    }
    frame.append(masked);
    return frame;
}

// ============================================================================
// 帧解析
// ============================================================================

/** @brief 解析接收缓冲区中的WebSocket帧，按opcode分发处理并累计帧级统计 */
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
        if (payloadLen > static_cast<quint64>(INT_MAX) - headerSize - maskSize) {
            m_buffer.clear();
            return;
        }
        int totalFrameSize = headerSize + maskSize
                             + static_cast<int>(payloadLen);
        if (m_buffer.size() < totalFrameSize) { break; }

        QByteArray payload = m_buffer.mid(headerSize + maskSize,
                                          static_cast<int>(payloadLen));

        if (masked) {
            QByteArray maskKey = m_buffer.mid(headerSize, 4);
            for (int i = 0; i < payload.size(); ++i) {
                payload[i] = payload[i] ^ maskKey[i % 4];
            }
        }

        m_buffer.remove(0, totalFrameSize);
        ++m_totalFramesReceived;  // 累计接收帧总数

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
        case 0x09: // ping → pong
            if (m_socket) {
                m_socket->write(buildFrame(0x0A, payload));
            }
            break;
        case 0x0A: { // pong — 计算延迟
            ++m_totalPongFrames;
            if (m_pingSendTime.isValid()) {
                qint64 latency = m_pingSendTime.elapsed();
                m_lastLatencyMs = latency;
                if (latency > m_maxLatencyMs) {
                    m_maxLatencyMs = latency;
                }
                m_latencySumMs += latency;
                ++m_latencySampleCount;
                m_pingSendTime.invalidate();  // 重置，避免重复计算
            }
            emit pongReceived(payload);
            break;
        }
        default:
            break;
        }
    }
}

// ============================================================================
// 消息发送
// ============================================================================

/** @brief 发送文本消息(构建0x01文本帧并发送) @param message 文本内容 @return 发送字节数 */
qint64 WebSocketConnection::sendTextMessage(const QString& message)
{
    if (!m_socket || !m_handshakeDone) { return -1; }
    QByteArray frame = buildFrame(0x01, message.toUtf8());
    qint64 written = m_socket->write(frame);
    if (written > 0) {
        ++m_totalMessagesSent;
        ++m_totalFramesSent;
        ++m_totalTextFrames;
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
        ++m_totalFramesSent;
        ++m_totalBinaryFrames;
        m_totalBytesSent += static_cast<quint64>(written);
    }
    return written;
}

/** @brief 发送ping帧(心跳检测)，累计ping帧计数 @param payload ping载荷数据 @return true=发送成功 */
bool WebSocketConnection::ping(const QByteArray& payload)
{
    if (!m_socket || !m_handshakeDone) { return false; }
    QByteArray frame = buildFrame(0x09, payload);
    qint64 written = m_socket->write(frame);
    if (written == frame.size()) {
        ++m_totalPingFrames;
        ++m_totalFramesSent;
        return true;
    }
    return false;
}

// ============================================================================
// 统计接口
// ============================================================================

quint64 WebSocketConnection::totalConnections() const { return m_totalConnections; }
quint64 WebSocketConnection::totalMessagesSent() const { return m_totalMessagesSent; }
quint64 WebSocketConnection::totalMessagesReceived() const { return m_totalMessagesReceived; }
quint64 WebSocketConnection::totalBytesSent() const { return m_totalBytesSent; }
quint64 WebSocketConnection::totalBytesReceived() const { return m_totalBytesReceived; }
quint64 WebSocketConnection::errorCount() const { return m_errorCount; }
quint64 WebSocketConnection::totalFramesSent() const { return m_totalFramesSent; }
quint64 WebSocketConnection::totalFramesReceived() const { return m_totalFramesReceived; }
quint64 WebSocketConnection::totalTextFrames() const { return m_totalTextFrames; }
quint64 WebSocketConnection::totalBinaryFrames() const { return m_totalBinaryFrames; }
quint64 WebSocketConnection::totalPingFrames() const { return m_totalPingFrames; }
quint64 WebSocketConnection::totalPongFrames() const { return m_totalPongFrames; }

/** @brief 获取ping/pong交互总次数 @return ping帧发送数+pong帧接收数 */
quint64 WebSocketConnection::pingPongCount() const { return m_totalPingFrames + m_totalPongFrames; }

/** @brief 获取ping/pong平均延迟(毫秒) @return 平均延迟，无采样数据时返回0 */
double WebSocketConnection::averageLatencyMs() const
{
    if (m_latencySampleCount == 0) { return 0.0; }
    return static_cast<double>(m_latencySumMs) / static_cast<double>(m_latencySampleCount);
}

/** @brief 获取ping/pong最大延迟(毫秒) @return 最大延迟，无采样数据时返回0 */
qint64 WebSocketConnection::maxLatencyMs() const { return m_maxLatencyMs; }

/** @brief 获取当前连接运行时长(秒) @return 连接时长，未连接返回0 */
qint64 WebSocketConnection::connectionUptimeSeconds() const
{
    if (!m_connectionTimer.isValid()) { return 0; }
    return m_connectionTimer.elapsed() / 1000;
}

/** @brief 获取当前消息队列大小 @return 队列中待发送消息数量 */
int WebSocketConnection::messageQueueSize() const { return m_sendQueue.size(); }

/** @brief 获取消息队列容量上限 @return 队列最大容量 */
int WebSocketConnection::messageQueueLimit() const { return m_queueLimit; }

/** @brief 设置消息队列容量上限 @param limit 最大容量，小于等于0表示不限 */
void WebSocketConnection::setMessageQueueLimit(int limit) { m_queueLimit = limit; }

/** @brief 获取因队列满而丢弃的消息数 @return 丢弃消息总数 */
quint64 WebSocketConnection::totalMessagesDropped() const { return m_totalMessagesDropped; }

/** @brief 重置所有统计数据(含帧统计、延迟追踪和队列计数)为零 */
void WebSocketConnection::resetStats()
{
    m_totalConnections = 0;
    m_totalMessagesSent = 0;
    m_totalMessagesReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalFramesSent = 0;
    m_totalFramesReceived = 0;
    m_totalTextFrames = 0;
    m_totalBinaryFrames = 0;
    m_totalPingFrames = 0;
    m_totalPongFrames = 0;
    m_lastLatencyMs = 0;
    m_maxLatencyMs = 0;
    m_latencySampleCount = 0;
    m_latencySumMs = 0;
    m_totalMessagesDropped = 0;
}

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
        case 0x0A: // pong
            emit pongReceived(payload);
            break;
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

// ============================================================================
// 统计接口
// ============================================================================

quint64 WebSocketConnection::totalConnections() const { return m_totalConnections; }
quint64 WebSocketConnection::totalMessagesSent() const { return m_totalMessagesSent; }
quint64 WebSocketConnection::totalMessagesReceived() const { return m_totalMessagesReceived; }
quint64 WebSocketConnection::totalBytesSent() const { return m_totalBytesSent; }
quint64 WebSocketConnection::totalBytesReceived() const { return m_totalBytesReceived; }
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

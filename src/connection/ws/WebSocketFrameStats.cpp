/**
 * @file WebSocketFrameStats.cpp
 * @brief WebSocket连接统计查询和重置方法实现
 *
 * 从 WebSocketFrame.cpp 拆分而来，包含所有统计getter、
 * 延迟计算、队列配置和resetStats方法。
 */

#include "connection/ws/WebSocketConnection.h"

/** @brief 获取累计WebSocket连接成功次数 @return 连接成功总数 */
quint64 WebSocketConnection::totalConnections() const { return m_totalConnections; }
/** @brief 获取累计发送消息数 @return 发送消息总数 */
quint64 WebSocketConnection::totalMessagesSent() const { return m_totalMessagesSent; }
/** @brief 获取累计接收消息数 @return 接收消息总数 */
quint64 WebSocketConnection::totalMessagesReceived() const { return m_totalMessagesReceived; }
/** @brief 获取累计发送字节数 @return 发送字节总量 */
quint64 WebSocketConnection::totalBytesSent() const { return m_totalBytesSent; }
/** @brief 获取累计接收字节数 @return 接收字节总量 */
quint64 WebSocketConnection::totalBytesReceived() const { return m_totalBytesReceived; }
/** @brief 获取累计错误次数 @return 错误总数 */
quint64 WebSocketConnection::errorCount() const { return m_errorCount; }
/** @brief 获取累计发送帧数 @return 帧发送总数 */
quint64 WebSocketConnection::totalFramesSent() const { return m_totalFramesSent; }
/** @brief 获取累计接收帧数 @return 帧接收总数 */
quint64 WebSocketConnection::totalFramesReceived() const { return m_totalFramesReceived; }
/** @brief 获取累计发送文本帧数 @return 文本帧发送总数 */
quint64 WebSocketConnection::totalTextFrames() const { return m_totalTextFrames; }
/** @brief 获取累计发送二进制帧数 @return 二进制帧发送总数 */
quint64 WebSocketConnection::totalBinaryFrames() const { return m_totalBinaryFrames; }
/** @brief 获取累计发送ping帧数 @return ping帧发送总数 */
quint64 WebSocketConnection::totalPingFrames() const { return m_totalPingFrames; }
/** @brief 获取累计接收pong帧数 @return pong帧接收总数 */
quint64 WebSocketConnection::totalPongFrames() const { return m_totalPongFrames; }
/** @brief 获取累计分片消息数(continuation帧) @return 分片消息总数 */
quint64 WebSocketConnection::totalFragmentedMessages() const { return m_totalFragmentedMessages; }

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

/** @brief 重置所有统计数据(含帧统计、延迟追踪、队列计数和分片消息)为零 */
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
    m_totalFragmentedMessages = 0;
    m_lastLatencyMs = 0;
    m_maxLatencyMs = 0;
    m_latencySampleCount = 0;
    m_latencySumMs = 0;
    m_totalMessagesDropped = 0;
}

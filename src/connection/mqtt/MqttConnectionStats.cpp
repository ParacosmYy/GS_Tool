/**
 * @file MqttConnectionStats.cpp
 * @brief MQTT连接 - 统计查询与重置接口实现
 *
 * 从 MqttConnection.cpp 拆分而来，包含所有统计getter和resetStats方法。
 */

#include "connection/mqtt/MqttConnection.h"

/** @brief 获取累计发布消息数 @return 发布计数 */
quint64 MqttConnection::totalPublishes() const { return m_totalPublishes; }
/** @brief 获取累计接收消息数 @return 接收计数 */
quint64 MqttConnection::totalReceived() const { return m_totalReceived; }
/** @brief 获取累计订阅次数 @return 订阅计数 */
quint64 MqttConnection::totalSubscriptions() const { return m_totalSubscriptions; }
/** @brief 获取累计发送字节数 @return 发送字节数 */
quint64 MqttConnection::totalBytesSent() const { return m_totalBytesSent; }
/** @brief 获取累计接收字节数 @return 接收字节数 */
quint64 MqttConnection::totalBytesReceived() const { return m_totalBytesReceived; }
/** @brief 获取累计错误次数 @return 错误计数 */
quint64 MqttConnection::errorCount() const { return m_errorCount; }
/** @brief 获取累计连接尝试次数 @return 连接尝试计数 */
quint64 MqttConnection::connectionAttempts() const { return m_connectionAttempts; }
/** @brief 获取QoS0发布消息数 @return QoS0计数 */
quint64 MqttConnection::qos0Count() const { return m_qos0Count; }
/** @brief 获取QoS1发布消息数 @return QoS1计数 */
quint64 MqttConnection::qos1Count() const { return m_qos1Count; }
/** @brief 获取QoS2发布消息数 @return QoS2计数 */
quint64 MqttConnection::qos2Count() const { return m_qos2Count; }
/** @brief 获取累计PINGREQ发送次数 @return 心跳计数 */
quint64 MqttConnection::keepAliveSent() const { return m_keepAliveSent; }
/** @brief 获取最后一次连接发起时间 @return 时间戳 */
QDateTime MqttConnection::lastConnectTime() const { return m_lastConnectTime; }
/** @brief 获取待发送队列大小 @return 队列中的消息数 */
int MqttConnection::pendingQueueSize() const { return m_pendingQueue.size(); }

/** @brief 重置所有统计计数器(含重连统计) */
void MqttConnection::resetStats()
{
    m_totalPublishes = 0;
    m_totalReceived = 0;
    m_totalSubscriptions = 0;
    m_totalUnsubscriptions = 0;
    m_totalMessageReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_connectionAttempts = 0;
    m_qos0Count = 0;
    m_qos1Count = 0;
    m_qos2Count = 0;
    m_keepAliveSent = 0;
    m_lastConnectTime = QDateTime();
    m_totalRetryAttempts = 0;
    m_totalSuccessfulReconnects = 0;
    m_totalRetryDelayMs = 0;
}

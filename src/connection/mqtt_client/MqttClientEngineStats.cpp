/**
 * @file MqttClientEngineStats.cpp
 * @brief MQTT客户端引擎 — 统计查询与重置接口实现
 *
 * 从 MqttClientEngine.cpp 拆分而来，包含所有统计 getter 和 resetStats 方法。
 */

#include "connection/mqtt_client/MqttClientEngine.h"

/** @brief 获取累计发布消息数 @return 发布计数 */
quint64 MqttClientEngine::totalPublishes() const { return m_totalPublishes; }
/** @brief 获取累计接收消息数 @return 接收计数 */
quint64 MqttClientEngine::totalReceived() const { return m_totalReceived; }
/** @brief 获取累计发送字节数 @return 字节数 */
quint64 MqttClientEngine::totalBytesSent() const { return m_totalBytesSent; }
/** @brief 获取累计接收字节数 @return 字节数 */
quint64 MqttClientEngine::totalBytesReceived() const { return m_totalBytesReceived; }
/** @brief 获取累计错误次数 @return 错误计数 */
quint64 MqttClientEngine::errorCount() const { return m_errorCount; }
/** @brief 获取累计连接尝试次数 @return 连接计数 */
quint64 MqttClientEngine::connectionAttempts() const { return m_connectionAttempts; }
/** @brief 获取累计重连成功次数 @return 重连计数 */
quint64 MqttClientEngine::successfulReconnects() const { return m_successfulReconnects; }

/** @brief 重置所有统计计数器 */
void MqttClientEngine::resetStats()
{
    m_totalPublishes = 0;
    m_totalReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_connectionAttempts = 0;
    m_successfulReconnects = 0;
}

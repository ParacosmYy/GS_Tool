/**
 * @file MqttConnectionQueue.cpp
 * @brief MQTT消息队列与遗嘱消息管理
 *
 * 从 MqttConnection.cpp 拆分而来，集中管理离线消息队列
 * (入队/溢出/刷新)和遗嘱消息(LWT)的配置/清除/查询。
 *
 * @see MqttConnection.cpp — 连接管理、发布/订阅、统计
 * @see MqttConnectionProtocol.cpp — MQTT线协议报文构建和解析
 */

#include "connection/mqtt/MqttConnection.h"

// ============================================================
// LWT遗嘱消息接口
// ============================================================

/** @brief 配置遗嘱消息，下次连接时生效 @param will 遗嘱配置 */
void MqttConnection::setWill(const MqttWillConfig& will)
{
    m_will = will;
}

/** @brief 清除遗嘱消息配置 */
void MqttConnection::clearWill()
{
    m_will = MqttWillConfig();
}

/** @brief 获取当前遗嘱配置 @return 只读遗嘱配置引用 */
const MqttWillConfig& MqttConnection::willConfig() const
{
    return m_will;
}

// ============================================================
// 消息队列接口
// ============================================================

/** @brief 入队消息(断线时缓存，连接后自动发送) @param topic 目标主题 @param payload 负载 @param qos QoS等级 @return true=入队成功 */
bool MqttConnection::enqueueMessage(const QString& topic, const QByteArray& payload, int qos)
{
    if (topic.isEmpty()) return false;

    /* 队列溢出时丢弃最旧的消息 */
    if (m_pendingQueue.size() >= m_queueLimit) {
        int dropped = 0;
        while (m_pendingQueue.size() >= m_queueLimit) {
            m_pendingQueue.dequeue();
            ++dropped;
        }
        emit queueOverflow(dropped);
    }

    MqttPendingMessage msg;
    msg.topic = topic;
    msg.payload = payload;
    msg.qos = qos;
    m_pendingQueue.enqueue(msg);
    return true;
}

/** @brief 获取当前队列大小 @return 待发送消息数 */
int MqttConnection::queueSize() const { return m_pendingQueue.size(); }

/** @brief 设置队列最大容量 @param maxSize 最大消息数 */
void MqttConnection::setQueueLimit(int maxSize)
{
    m_queueLimit = (maxSize > 0) ? maxSize : 100;
}

/** @brief 获取队列最大容量 @return 最大消息数 */
int MqttConnection::queueLimit() const { return m_queueLimit; }

/** @brief 发送队列中缓存的所有待发消息 */
void MqttConnection::flushPendingQueue()
{
    while (!m_pendingQueue.isEmpty() && m_state == ConnectionState::Connected) {
        const auto& msg = m_pendingQueue.head();
        if (publish(msg.topic, msg.payload, msg.qos)) {
            m_pendingQueue.dequeue();
        } else {
            break;
        }
    }
}

/** @brief 获取当前订阅数量 @return 订阅主题数 */
int MqttConnection::subscriptionCount() const { return m_subscriptions.size(); }

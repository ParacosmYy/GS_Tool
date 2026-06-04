/**
 * @file MqttTopicModelRoute.cpp
 * @brief MQTT主题模型 — 主题查询、索引查找和消息路由实现
 *
 * 从 MqttTopicModel.cpp 拆分而来，包含:
 *   - hasTopic/topicCount/topicQos/topics: 主题查询接口
 *   - findTopicIndex: 主题路径到模型索引映射
 *   - routeMessage: 消息到订阅主题的路由匹配
 *
 * 主题增删改(CRUD)方法见 MqttTopicModel.cpp。
 * QAbstractItemModel接口见 MqttTopicModelIndex.cpp。
 * 辅助方法和统计见 MqttTopicModelHelpers.cpp。
 */

#include "connection/mqtt/MqttTopicModel.h"

// ============================================================
// 主题查询接口
// ============================================================

/** @brief 检查主题是否已存在
 *  @param topic 目标主题路径
 *  @return true=主题已存在
 */
bool MqttTopicModel::hasTopic(const QString& topic) const
{
    return m_topics.contains(topic);
}

/** @brief 获取当前主题数量
 *  @return 扁平列表中的主题总数
 */
int MqttTopicModel::topicCount() const
{
    return m_topics.size();
}

/** @brief 获取指定主题的QoS等级
 *  @param topic 目标主题路径
 *  @return QoS等级(0/1/2)，不存在返回-1
 */
int MqttTopicModel::topicQos(const QString& topic) const
{
    if (!m_topics.contains(topic)) return -1;
    TopicNode* node = findLeafNode(m_rootNode, topic);
    return node ? node->qos : -1;
}

/** @brief 获取所有已添加的主题列表
 *  @return 主题路径字符串列表
 */
QStringList MqttTopicModel::topics() const
{
    return m_topics;
}

/** @brief 根据主题路径查找其模型索引
 *  @param topic 完整主题路径
 *  @return 对应第0列的QModelIndex，未找到返回无效索引
 */
QModelIndex MqttTopicModel::findTopicIndex(const QString& topic) const
{
    if (topic.isEmpty()) return {};

    TopicNode* leaf = findLeafNode(m_rootNode, topic);
    if (!leaf || !leaf->parent) return {};

    /* 从叶节点向上回溯构建索引链 */
    int row = leaf->parent->children.indexOf(leaf);
    if (row < 0) return {};

    return createIndex(row, 0, leaf);
}

// ============================================================
// 消息路由接口
// ============================================================

/** @brief 将收到的消息路由到匹配的主题节点，更新统计计数
 *  @param topic 消息主题(精确匹配)
 *  @param payload 消息负载(用于未来扩展，如消息计数)
 *  @return true=匹配到至少一个主题
 */
bool MqttTopicModel::routeMessage(const QString& topic, const QByteArray& payload)
{
    Q_UNUSED(payload)
    if (topic.isEmpty()) return false;

    /* 精确匹配: 查找是否已有该主题 */
    bool matched = false;
    if (m_topics.contains(topic)) {
        matched = true;
    }

    /* 通配符匹配: 检查已有订阅是否匹配该消息主题 */
    const QStringList topicParts = topic.split('/', Qt::SkipEmptyParts);
    for (const auto& sub : m_topics) {
        if (topicMatchesSubscription(topicParts, sub.split('/', Qt::SkipEmptyParts))) {
            matched = true;
            break;
        }
    }

    if (matched) ++m_totalMessagesRouted;
    return matched;
}

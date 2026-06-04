/**
 * @file MqttTopicModel.cpp
 * @brief MQTT主题树模型实现 — 构造/析构 + 主题增删改(CRUD)操作
 *
 * 包含主题管理核心方法: addTopic/removeTopic/updateTopic/clearTopics。
 * QAbstractItemModel接口见 MqttTopicModelIndex.cpp。
 * 主题查询/索引查找/消息路由见 MqttTopicModelRoute.cpp。
 * 辅助方法和统计见 MqttTopicModelHelpers.cpp。
 */

#include "connection/mqtt/MqttTopicModel.h"

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造函数，初始化根节点 @param parent 父对象指针 */
MqttTopicModel::MqttTopicModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new TopicNode)
{
    m_rootNode->name = QStringLiteral("<root>");
}

/** @brief 析构函数，释放整棵主题树 */
MqttTopicModel::~MqttTopicModel()
{
    delete m_rootNode;
}

// ============================================================

// QAbstractItemModel 完整接口实现见 MqttTopicModelIndex.cpp

// 主题管理接口
// ============================================================

/** @brief 添加MQTT主题到树模型，按'/'分割构建层级节点
 *  @param topic 完整主题路径(如 "sensor/temperature/room1")
 *  @param qos 服务质量等级(0/1/2)
 */
void MqttTopicModel::addTopic(const QString& topic, int qos)
{
    if (topic.isEmpty()) return;

    /* 去重检查 */
    if (m_topics.contains(topic)) {
        ++m_totalDuplicateSkips;
        return;
    }

    beginResetModel();
    m_topics.append(topic);

    /* 按'/'分割路径，逐级构建树 */
    const QStringList parts = topic.split('/', Qt::SkipEmptyParts);
    TopicNode* current = m_rootNode;
    QString path;

    for (int i = 0; i < parts.size(); ++i) {
        path = (path.isEmpty()) ? parts[i] : path + "/" + parts[i];
        current = findOrCreateChild(current, parts[i]);
        current->fullPath = path;
        /* 最后一级节点设置QoS */
        if (i == parts.size() - 1) {
            current->qos = qos;
        }
    }
    endResetModel();
    ++m_totalTopicsAdded;
    emit topicAdded(topic, qos);
}

/** @brief 从树模型中移除指定主题并重建树结构
 *  @param topic 待移除的完整主题路径
 */
void MqttTopicModel::removeTopic(const QString& topic)
{
    if (!m_topics.contains(topic)) return;

    beginResetModel();

    /* 保存所有剩余主题的QoS映射 */
    QMap<QString, int> qosMap;
    for (const auto& t : m_topics) {
        if (t == topic) continue;
        TopicNode* node = findLeafNode(m_rootNode, t);
        if (node) {
            qosMap[t] = node->qos;
        }
    }

    m_topics.removeAll(topic);

    /* 重建整棵树 */
    delete m_rootNode;
    m_rootNode = new TopicNode;
    m_rootNode->name = QStringLiteral("<root>");

    /* 重新添加剩余主题，保留QoS */
    QStringList saved = m_topics;
    m_topics.clear();
    for (const auto& t : saved) {
        addTopic(t, qosMap.value(t, 0));
    }
    endResetModel();
    ++m_totalTopicsRemoved;
    emit topicRemoved(topic);
}

/** @brief 更新已有主题的QoS等级
 *  @param topic 目标主题路径
 *  @param qos 新的QoS等级(0/1/2)
 *  @return true=更新成功，false=主题不存在或QoS非法
 */
bool MqttTopicModel::updateTopic(const QString& topic, int qos)
{
    if (qos < 0 || qos > 2) return false;
    if (!m_topics.contains(topic)) return false;

    /* 在树中查找叶节点 */
    TopicNode* node = findLeafNode(m_rootNode, topic);
    if (!node) return false;

    const int oldQos = node->qos;
    if (oldQos == qos) return false;

    node->qos = qos;
    ++m_totalQosUpdates;

    /* 通知视图更新该节点 */
    QModelIndex idx = findTopicIndex(topic);
    if (idx.isValid()) {
        QModelIndex qosIdx = index(idx.row(), 1, parent(idx));
        emit dataChanged(qosIdx, qosIdx, {Qt::DisplayRole, Qt::EditRole});
    }

    emit topicQosChanged(topic, oldQos, qos);
    return true;
}

/** @brief 清除所有主题，重置模型到空状态 */
void MqttTopicModel::clearTopics()
{
    if (m_topics.isEmpty()) return;

    beginResetModel();
    m_topics.clear();

    delete m_rootNode;
    m_rootNode = new TopicNode;
    m_rootNode->name = QStringLiteral("<root>");

    endResetModel();
    emit topicsCleared();
}

// 主题查询/索引查找/消息路由见 MqttTopicModelRoute.cpp

// 辅助方法+通配符匹配+统计接口见 MqttTopicModelHelpers.cpp

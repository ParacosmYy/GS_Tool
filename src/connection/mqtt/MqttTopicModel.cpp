/**
 * @file MqttTopicModel.cpp
 * @brief MQTT主题树模型实现 — 完整QAbstractItemModel接口+主题增删改查+统计追踪
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

// 辅助方法+通配符匹配+统计接口见 MqttTopicModelHelpers.cpp

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

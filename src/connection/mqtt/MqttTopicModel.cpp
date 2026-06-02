/**
 * @file MqttTopicModel.cpp
 * @brief MQTT主题树模型实现 — 层级树节点管理
 */

#include "connection/mqtt/MqttTopicModel.h"

MqttTopicModel::MqttTopicModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new TopicNode)
{
    m_rootNode->name = "<root>";
}

MqttTopicModel::~MqttTopicModel()
{
    delete m_rootNode;
}

QVariant MqttTopicModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    if (role != Qt::DisplayRole) return {};

    auto* node = nodeFromIndex(index);
    if (!node) return {};

    if (index.column() == 0) {
        return node->name;
    } else if (index.column() == 1) {
        return node->qos;
    }
    return {};
}

QVariant MqttTopicModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    if (section == 0) return tr("主题");
    if (section == 1) return tr("QoS");
    return {};
}

int MqttTopicModel::rowCount(const QModelIndex& parent) const
{
    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    return parentNode ? parentNode->children.size() : 0;
}

int MqttTopicModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 2;  // 主题、QoS
}

QModelIndex MqttTopicModel::index(int row, int column,
                                   const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return {};

    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode || row >= parentNode->children.size()) return {};

    return createIndex(row, column, parentNode->children.at(row));
}

QModelIndex MqttTopicModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return {};

    auto* childNode = nodeFromIndex(child);
    if (!childNode || !childNode->parent) return {};

    auto* parentNode = childNode->parent;
    if (parentNode == m_rootNode) return {};

    /* 在祖父节点中查找父节点的行号 */
    auto* grandParent = parentNode->parent;
    if (!grandParent) return {};

    int parentRow = grandParent->children.indexOf(parentNode);
    if (parentRow < 0) parentRow = 0;
    return createIndex(parentRow, 0, parentNode);
}

void MqttTopicModel::addTopic(const QString& topic, int qos)
{
    if (topic.isEmpty() || m_topics.contains(topic)) return;

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
}

void MqttTopicModel::removeTopic(const QString& topic)
{
    if (!m_topics.contains(topic)) return;

    beginResetModel();
    m_topics.removeAll(topic);

    /* 保存所有主题的QoS映射 */
    QMap<QString, int> qosMap;
    for (const auto& t : m_topics) {
        /* 从旧树中查找QoS（遍历叶节点） */
        TopicNode* node = findLeafNode(m_rootNode, t);
        if (node) {
            qosMap[t] = node->qos;
        }
    }

    /* 查找并删除叶节点(简化实现：重建整棵树) */
    delete m_rootNode;
    m_rootNode = new TopicNode;
    m_rootNode->name = "<root>";

    /* 重新添加剩余主题，保留QoS */
    QStringList saved = m_topics;
    m_topics.clear();
    for (const auto& t : saved) {
        addTopic(t, qosMap.value(t, 0));
    }
    endResetModel();
}

TopicNode* MqttTopicModel::findLeafNode(TopicNode* root, const QString& fullPath) const
{
    if (!root) return nullptr;

    /* 叶节点匹配完整路径 */
    if (root->fullPath == fullPath) return root;

    /* 递归搜索子节点 */
    for (auto* child : root->children) {
        TopicNode* found = findLeafNode(child, fullPath);
        if (found) return found;
    }
    return nullptr;
}

QStringList MqttTopicModel::topics() const
{
    return m_topics;
}

TopicNode* MqttTopicModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<TopicNode*>(index.internalPointer())
        : m_rootNode;
}

TopicNode* MqttTopicModel::findOrCreateChild(TopicNode* parentNode,
                                              const QString& name)
{
    /* 查找已有子节点 */
    for (auto* child : parentNode->children) {
        if (child->name == name) return child;
    }

    /* 创建新子节点 */
    auto* newNode = new TopicNode;
    newNode->name = name;
    newNode->parent = parentNode;
    parentNode->children.append(newNode);
    return newNode;
}

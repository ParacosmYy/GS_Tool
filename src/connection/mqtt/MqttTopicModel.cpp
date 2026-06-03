/**
 * @file MqttTopicModel.cpp
 * @brief MQTT主题树模型实现 — 层级树节点管理
 */

#include "connection/mqtt/MqttTopicModel.h"

/** @brief 构造函数，初始化根节点 @param parent 父对象指针 */
MqttTopicModel::MqttTopicModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new TopicNode)
{
    m_rootNode->name = "<root>";
}

/** @brief 析构函数，释放整棵主题树 */
MqttTopicModel::~MqttTopicModel()
{
    delete m_rootNode;
}

/** @brief 获取指定索引处的显示数据 @param index 模型索引 @param role 显示角色 @return 主题名称或QoS值 */
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

/** @brief 获取表头数据 @param section 列号 @param orientation 方向 @param role 显示角色 @return 列标题文本 */
QVariant MqttTopicModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    if (section == 0) return tr("主题");
    if (section == 1) return tr("QoS");
    return {};
}

/** @brief 获取指定父节点下的行数 @param parent 父模型索引 @return 子节点数量 */
int MqttTopicModel::rowCount(const QModelIndex& parent) const
{
    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    return parentNode ? parentNode->children.size() : 0;
}

/** @brief 获取列数(固定2列: 主题、QoS) @param parent 父模型索引(未使用) @return 固定返回2 */
int MqttTopicModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 2;  // 主题、QoS
}

/** @brief 根据行列创建子节点模型索引 @param row 行号 @param column 列号 @param parent 父模型索引 @return 子节点索引 */
QModelIndex MqttTopicModel::index(int row, int column,
                                   const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return {};

    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode || row >= parentNode->children.size()) return {};

    return createIndex(row, column, parentNode->children.at(row));
}

/** @brief 获取子节点的父索引 @param child 子节点模型索引 @return 父节点索引 */
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

/** @brief 添加MQTT主题到树模型，按'/'分割构建层级节点 @param topic 完整主题路径 @param qos 服务质量等级 */
void MqttTopicModel::addTopic(const QString& topic, int qos)
{
    if (topic.isEmpty() || m_topics.contains(topic)) {
        if (m_topics.contains(topic)) {
            ++m_totalDuplicateSkips;
        }
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
}

/** @brief 从树模型中移除指定主题并重建树结构 @param topic 待移除的完整主题路径 */
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
    ++m_totalTopicsRemoved;
}

/** @brief 递归查找指定完整路径的叶节点 @param root 搜索起始根节点 @param fullPath 目标完整路径 @return 匹配的叶节点指针，未找到返回nullptr */
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

/** @brief 获取所有已添加的主题列表 @return 主题路径字符串列表 */
QStringList MqttTopicModel::topics() const
{
    return m_topics;
}

/** @brief 从模型索引获取对应的树节点指针 @param index 模型索引 @return 对应的TopicNode指针 */
TopicNode* MqttTopicModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<TopicNode*>(index.internalPointer())
        : m_rootNode;
}

/** @brief 在父节点下查找或创建指定名称的子节点 @param parentNode 父节点指针 @param name 子节点名称 @return 已有或新创建的子节点指针 */
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

// ---- 统计接口 ----

/** @brief 获取已添加主题的总数 @return 累计添加次数 */
quint64 MqttTopicModel::totalTopicsAdded() const
{
    return m_totalTopicsAdded;
}

/** @brief 获取已移除主题的总数 @return 累计移除次数 */
quint64 MqttTopicModel::totalTopicsRemoved() const
{
    return m_totalTopicsRemoved;
}

/** @brief 获取跳过重复主题的总次数 @return 累计去重跳过次数 */
quint64 MqttTopicModel::totalDuplicateSkips() const
{
    return m_totalDuplicateSkips;
}

/** @brief 重置所有主题统计计数器 */
void MqttTopicModel::resetTopicStatistics()
{
    m_totalTopicsAdded = 0;
    m_totalTopicsRemoved = 0;
    m_totalDuplicateSkips = 0;
}

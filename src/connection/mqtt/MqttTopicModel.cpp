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
// QAbstractItemModel 完整接口实现
// ============================================================

/** @brief 获取指定索引处的显示/工具提示数据
 *  @param index 模型索引
 *  @param role 显示角色(DisplayRole/ToolTipRole/EditRole)
 *  @return 主题名称、QoS值或工具提示文本
 */
QVariant MqttTopicModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};

    auto* node = nodeFromIndex(index);
    if (!node) return {};

    /* 第0列: 主题名称 */
    if (index.column() == 0) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return node->name;
        }
        if (role == Qt::ToolTipRole) {
            /* 工具提示显示完整路径 */
            return node->fullPath.isEmpty() ? node->name : node->fullPath;
        }
    }

    /* 第1列: QoS等级 */
    if (index.column() == 1) {
        if (role == Qt::DisplayRole) {
            return QString::number(node->qos);
        }
        if (role == Qt::EditRole) {
            return node->qos;
        }
        if (role == Qt::ToolTipRole) {
            return tr("QoS %1").arg(node->qos);
        }
    }

    return {};
}

/** @brief 设置指定索引的数据(仅支持QoS列编辑)
 *  @param index 目标模型索引
 *  @param value 新的QoS值
 *  @param role 编辑角色
 *  @return true=设置成功并发射dataChanged
 */
bool MqttTopicModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) return false;
    if (index.column() != 1) return false; /* 仅允许编辑QoS列 */

    auto* node = nodeFromIndex(index);
    if (!node || node == m_rootNode) return false;

    const int newQos = value.toInt();
    if (newQos < 0 || newQos > 2) return false; /* QoS只允许0/1/2 */
    if (node->qos == newQos) return false;       /* 值未变化 */

    const int oldQos = node->qos;
    node->qos = newQos;
    ++m_totalQosUpdates;

    emit dataChanged(index, index, {role});
    emit topicQosChanged(node->fullPath, oldQos, newQos);
    return true;
}

/** @brief 获取表头数据
 *  @param section 列号
 *  @param orientation 方向
 *  @param role 显示角色
 *  @return 列标题文本
 */
QVariant MqttTopicModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    if (section == 0) return tr("主题");
    if (section == 1) return tr("QoS");
    return {};
}

/** @brief 获取指定父节点下的行数
 *  @param parent 父模型索引
 *  @return 子节点数量
 */
int MqttTopicModel::rowCount(const QModelIndex& parent) const
{
    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    return parentNode ? parentNode->children.size() : 0;
}

/** @brief 获取列数(固定2列: 主题、QoS)
 *  @param parent 父模型索引(未使用)
 *  @return 固定返回2
 */
int MqttTopicModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 2;
}

/** @brief 根据行列创建子节点模型索引
 *  @param row 行号
 *  @param column 列号
 *  @param parent 父模型索引
 *  @return 子节点索引
 */
QModelIndex MqttTopicModel::index(int row, int column,
                                   const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return {};

    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode || row >= parentNode->children.size()) return {};

    return createIndex(row, column, parentNode->children.at(row));
}

/** @brief 获取子节点的父索引
 *  @param child 子节点模型索引
 *  @return 父节点索引，根节点的子返回无效索引
 */
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

/** @brief 获取项标志(可选择、QoS列可编辑、启用)
 *  @param index 模型索引
 *  @return 项标志位组合
 */
Qt::ItemFlags MqttTopicModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    /* QoS列(第1列)允许编辑 */
    if (index.column() == 1) {
        defaultFlags |= Qt::ItemIsEditable;
    }

    return defaultFlags;
}

/** @brief 判断父节点是否有子项(优化QTreeView展开性能)
 *  @param parent 父模型索引
 *  @return true=有子节点
 */
bool MqttTopicModel::hasChildren(const QModelIndex& parent) const
{
    auto* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    return parentNode && !parentNode->children.isEmpty();
}

// ============================================================
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

// ============================================================
// 内部辅助方法
// ============================================================

/** @brief 从模型索引获取对应的树节点指针
 *  @param index 模型索引
 *  @return 对应的TopicNode指针
 */
TopicNode* MqttTopicModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<TopicNode*>(index.internalPointer())
        : m_rootNode;
}

/** @brief 在父节点下查找或创建指定名称的子节点
 *  @param parentNode 父节点指针
 *  @param name 子节点名称
 *  @return 已有或新创建的子节点指针
 */
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

/** @brief 递归查找指定完整路径的叶节点
 *  @param root 搜索起始根节点
 *  @param fullPath 目标完整路径
 *  @return 匹配的叶节点指针，未找到返回nullptr
 */
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

/** @brief 递归统计以指定节点为根的子树节点总数
 *  @param node 起始节点
 *  @return 该节点及其所有后代的总数(包含自身)
 */
int MqttTopicModel::countNodes(const TopicNode* node) const
{
    if (!node) return 0;
    int count = 1; /* 包含自身 */
    for (const auto* child : node->children) {
        count += countNodes(child);
    }
    return count;
}

// ============================================================
// 统计接口
// ============================================================

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

/** @brief 获取QoS更新总次数 @return 累计更新次数 */
quint64 MqttTopicModel::totalQosUpdates() const
{
    return m_totalQosUpdates;
}

/** @brief 获取树中所有节点总数(含非叶节点)
 *  @return 节点总数(不含虚拟根节点)
 */
int MqttTopicModel::totalNodeCount() const
{
    return countNodes(m_rootNode) - 1; /* 减去虚拟根节点 */
}

/** @brief 重置所有主题统计计数器 */
void MqttTopicModel::resetTopicStatistics()
{
    m_totalTopicsAdded = 0;
    m_totalTopicsRemoved = 0;
    m_totalDuplicateSkips = 0;
    m_totalQosUpdates = 0;
}

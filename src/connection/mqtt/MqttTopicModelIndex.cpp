/**
 * @file MqttTopicModelIndex.cpp
 * @brief MQTT主题树模型 — QAbstractItemModel完整接口实现(data/setData/headerData/rowCount/columnCount/index/parent/flags/hasChildren)
 */

#include "connection/mqtt/MqttTopicModel.h"

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

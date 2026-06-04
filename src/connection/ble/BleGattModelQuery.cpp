/**
 * @file BleGattModelQuery.cpp
 * @brief GATT树模型 — 查询/查找/统计方法实现
 *
 * 从 BleGattModel.cpp 拆分而来，包含:
 * - UUID查找与值更新 (findNodeByUuid, updateValueByUuid)
 * - 数据访问 (nodeFromIndex, serviceCount, serviceNames, totalNodeCount)
 * - 统计计数管理 (resetGattStatistics)
 * - 私有辅助 (findNodeRecursive, countNodes, indexForNode)
 */

#include "connection/ble/BleGattModel.h"

// ============================================================
// 数据查询接口
// ============================================================

/** @brief 获取当前已加载的GATT服务数量 @return 根节点下的直接子节点数 */
int BleGattModel::serviceCount() const
{
    return m_rootNode->children.size();
}

/** @brief 从模型索引获取内部GattNode指针 @param index 模型索引 @return GattNode指针 */
GattNode* BleGattModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }
    return static_cast<GattNode*>(index.internalPointer());
}

/** @brief 根据UUID查找节点，搜索整棵GATT树 @param uuid 目标UUID字符串 @return 匹配的节点指针或nullptr */
GattNode* BleGattModel::findNodeByUuid(const QString& uuid) const
{
    if (uuid.isEmpty()) {
        return nullptr;
    }
    return findNodeRecursive(m_rootNode, uuid);
}

/** @brief 根据UUID更新节点值并通知视图刷新 @param uuid 目标节点UUID @param newValue 新的值字符串 @return 更新成功返回true */
bool BleGattModel::updateValueByUuid(const QString& uuid, const QString& newValue)
{
    GattNode* node = findNodeByUuid(uuid);
    if (!node) {
        ++m_errorCount;
        return false;
    }

    node->value = newValue;
    ++m_totalWrites;

    // 查找该节点的模型索引并发射dataChanged信号
    const QModelIndex idx = indexForNode(node);
    if (idx.isValid()) {
        emit dataChanged(idx, idx.sibling(idx.row(), ColCount - 1),
                         {Qt::DisplayRole, Qt::EditRole});
    }
    return true;
}

/** @brief 获取所有服务名称列表(仅第一层服务节点) @return 服务名称QStringList */
QStringList BleGattModel::serviceNames() const
{
    QStringList names;
    names.reserve(m_rootNode->children.size());
    for (const GattNode* svc : m_rootNode->children) {
        names.append(svc->name);
    }
    return names;
}

/** @brief 计算所有节点总数(服务+特征+描述符) @return 节点总数 */
int BleGattModel::totalNodeCount() const
{
    return countNodes(m_rootNode);
}

/** @brief 重置所有GATT操作统计计数器 */
void BleGattModel::resetGattStatistics()
{
    m_totalServicesDiscovered = 0;
    m_totalCharacteristicsRead = 0;
    m_totalWrites = 0;
    m_errorCount = 0;
}

// ============================================================
// 私有辅助方法
// ============================================================

/** @brief 深度优先递归查找UUID匹配的节点 @param node 当前搜索起点 @param uuid 目标UUID @return 匹配节点指针或nullptr */
GattNode* BleGattModel::findNodeRecursive(GattNode* node, const QString& uuid) const
{
    if (!node) {
        return nullptr;
    }

    // 当前节点匹配则直接返回
    if (node->uuid == uuid) {
        return node;
    }

    // 递归搜索子节点
    for (GattNode* child : node->children) {
        GattNode* found = findNodeRecursive(child, uuid);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

/** @brief 递归统计节点及其所有子孙节点总数 @param node 起始节点 @return 节点总数(不含自身) */
int BleGattModel::countNodes(const GattNode* node) const
{
    if (!node) {
        return 0;
    }
    int count = 0;
    for (const GattNode* child : node->children) {
        count += 1 + countNodes(child);
    }
    return count;
}

/** @brief 根据GattNode指针反向查找其模型索引 @param node 目标节点指针 @return 对应的QModelIndex(指向第0列) */
QModelIndex BleGattModel::indexForNode(GattNode* node) const
{
    if (!node || node == m_rootNode) {
        return {};
    }

    // 根节点的直接子节点(服务层)
    if (node->parent == m_rootNode) {
        const int row = m_rootNode->children.indexOf(node);
        if (row >= 0) {
            return createIndex(row, 0, node);
        }
        return {};
    }

    // 递归向上查找: 先获取父索引，再在父的children中找当前节点的行号
    const QModelIndex parentIdx = indexForNode(node->parent);
    if (!parentIdx.isValid()) {
        return {};
    }

    GattNode* parentNode = node->parent;
    const int row = parentNode->children.indexOf(node);
    if (row < 0) {
        return {};
    }

    return createIndex(row, 0, node);
}

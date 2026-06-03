/**
 * @file BleGattModel.cpp
 * @brief GATT服务/特征树模型实现
 *
 * 实现完整的树形模型: GattNode内部数据结构管理，
 * index/parent/row/column/data/headerData全部实现。
 */

#include "connection/ble/BleGattModel.h"

/** @brief 构造GATT树模型，创建根节点 @param parent 父QObject指针 */
BleGattModel::BleGattModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new GattNode())
{
    m_rootNode->name = QStringLiteral("GATT Root");
}

/** @brief 析构模型，释放根节点及其所有子节点 */
BleGattModel::~BleGattModel()
{
    delete m_rootNode;
}

/** @brief 获取指定索引处的数据 @param index 模型索引 @param role 显示角色 @return 对应列的数据 */
QVariant BleGattModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole) {
        return {};
    }

    const GattNode* node = static_cast<GattNode*>(index.internalPointer());
    if (!node) {
        return {};
    }

    switch (index.column()) {
    case ColName:       return node->name;
    case ColUuid:       return node->uuid;
    case ColProperties: return node->properties;
    case ColValue:      return node->value;
    default:            return {};
    }
}

/** @brief 获取表头数据 @param section 列号 @param orientation 方向 @param role 显示角色 @return 列标题文本 */
QVariant BleGattModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }
    switch (section) {
    case ColName:       return tr("名称");
    case ColUuid:       return tr("UUID");
    case ColProperties: return tr("属性");
    case ColValue:      return tr("值");
    default:            return {};
    }
}

/** @brief 获取指定父节点下的子行数 @param parent 父模型索引 @return 子节点数量 */
int BleGattModel::rowCount(const QModelIndex& parent) const
{
    const GattNode* parentNode = parent.isValid()
        ? static_cast<GattNode*>(parent.internalPointer())
        : m_rootNode;
    return parentNode ? parentNode->children.size() : 0;
}

/** @brief 获取列数(固定ColCount列) @param parent 父模型索引 @return 列数 */
int BleGattModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColCount;
}

/** @brief 创建子节点索引 @param row 行号 @param column 列号 @param parent 父索引 @return 子节点的模型索引 */
QModelIndex BleGattModel::index(int row, int column,
                                const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    const GattNode* parentNode = parent.isValid()
        ? static_cast<GattNode*>(parent.internalPointer())
        : m_rootNode;

    if (!parentNode || row < 0 || row >= parentNode->children.size()) {
        return {};
    }

    GattNode* childNode = parentNode->children.at(row);
    return createIndex(row, column, childNode);
}

/** @brief 获取指定子节点索引的父索引 @param child 子节点模型索引 @return 父节点的模型索引 */
QModelIndex BleGattModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return {};
    }

    const GattNode* childNode = static_cast<GattNode*>(child.internalPointer());
    if (!childNode || childNode->parent == m_rootNode || !childNode->parent) {
        return {};
    }

    // 在祖父节点中查找父节点的行号
    GattNode* parentNode = childNode->parent;
    GattNode* grandParent = parentNode->parent;
    if (!grandParent) {
        return {};
    }

    const int row = grandParent->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

/** @brief 设置GATT服务数据，重建整个树模型 @param services 服务列表(QVariantList格式) */
void BleGattModel::setServices(const QVariantList& services)
{
    beginResetModel();

    // 清除旧数据
    qDeleteAll(m_rootNode->children);
    m_rootNode->children.clear();

    // 从QVariantList构建新树
    for (const QVariant& svcVar : services) {
        const QVariantMap svcMap = svcVar.toMap();
        buildNode(svcMap, m_rootNode);
        ++m_totalServicesDiscovered;
    }

    endResetModel();
}

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

/** @brief 递归构建GATT节点树 @param itemMap 当前节点的属性映射 @param parentNode 父节点指针 */
void BleGattModel::buildNode(const QVariantMap& itemMap, GattNode* parentNode)
{
    auto* node = new GattNode();
    node->name = itemMap.value("name").toString();
    node->uuid = itemMap.value("uuid").toString();
    node->properties = itemMap.value("properties").toString();
    node->value = itemMap.value("value").toString();
    node->parent = parentNode;
    parentNode->children.append(node);

    // 递归构建子节点
    const QVariantList children = itemMap.value("characteristics").toList();
    if (children.isEmpty()) {
        // 也检查 "descriptors" 字段
        const QVariantList descs = itemMap.value("descriptors").toList();
        for (const QVariant& descVar : descs) {
            buildNode(descVar.toMap(), node);
        }
    } else {
        for (const QVariant& childVar : children) {
            buildNode(childVar.toMap(), node);
            ++m_totalCharacteristicsRead;
        }
    }
}

/** @brief 重置所有GATT操作统计计数器 */
void BleGattModel::resetGattStatistics()
{
    m_totalServicesDiscovered = 0;
    m_totalCharacteristicsRead = 0;
    m_totalWrites = 0;
    m_errorCount = 0;
}

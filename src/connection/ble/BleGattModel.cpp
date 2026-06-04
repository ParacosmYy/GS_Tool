/**
 * @file BleGattModel.cpp
 * @brief GATT服务/特征树模型 — QAbstractItemModel接口 + 数据操作实现
 *
 * 实现QAbstractItemModel核心接口(data/setData/headerData/rowCount/columnCount/
 * index/parent/flags)及模型数据操作(setServices/clear/buildNode)。
 * 查询/查找/统计方法见 BleGattModelQuery.cpp。
 */

#include "connection/ble/BleGattModel.h"

/** @brief 构造GATT树模型，创建虚拟根节点 @param parent 父QObject指针 */
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

// ============================================================
// QAbstractItemModel 核心接口实现
// ============================================================

/** @brief 获取指定索引处的显示数据 @param index 模型索引 @param role 显示角色 @return 对应列的数据 */
QVariant BleGattModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const GattNode* node = static_cast<GattNode*>(index.internalPointer());
    if (!node) {
        return {};
    }

    // 显示角色和工具提示角色返回文本内容
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
        case ColName:       return node->name;
        case ColUuid:       return node->uuid;
        case ColProperties: return node->properties;
        case ColValue:      return node->value;
        default:            return {};
        }
    }

    // 前景颜色角色: 值列有内容时用蓝色高亮
    if (role == Qt::ForegroundRole && index.column() == ColValue) {
        if (!node->value.isEmpty()) {
            return QColor(Qt::darkCyan);
        }
    }

    // 文本对齐角色: UUID和属性列居中
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColUuid || index.column() == ColProperties) {
            return static_cast<int>(Qt::AlignCenter);
        }
    }

    return {};
}

/** @brief 设置指定索引的数据(支持编辑值列) @param index 模型索引 @param value 新值 @param role 角色 @return 设置成功返回true */
bool BleGattModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // 只支持编辑角色和值列
    if (!index.isValid() || role != Qt::EditRole || index.column() != ColValue) {
        return false;
    }

    auto* node = static_cast<GattNode*>(index.internalPointer());
    if (!node) {
        return false;
    }

    node->value = value.toString();
    ++m_totalWrites;
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
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

/** @brief 获取项标志 — 选中、启用，值列可编辑 @param index 模型索引 @return 项标志位组合 */
Qt::ItemFlags BleGattModel::flags(const QModelIndex& index) const
{
    // 无效索引返回默认标志
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // 基础标志: 可选中、可启用
    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // 值列支持编辑
    if (index.column() == ColValue) {
        defaultFlags |= Qt::ItemIsEditable;
    }

    return defaultFlags;
}

// ============================================================
// 数据操作接口
// ============================================================

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

/** @brief 清除所有GATT服务数据，重置模型 */
void BleGattModel::clear()
{
    beginResetModel();
    qDeleteAll(m_rootNode->children);
    m_rootNode->children.clear();
    endResetModel();
}

// ============================================================
// 私有辅助方法
// ============================================================

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

    // 递归构建子节点: 优先检查 "characteristics" 字段
    const QVariantList children = itemMap.value("characteristics").toList();
    if (!children.isEmpty()) {
        for (const QVariant& childVar : children) {
            buildNode(childVar.toMap(), node);
            ++m_totalCharacteristicsRead;
        }
    }

    // 也检查 "descriptors" 字段(特征下的描述符)
    const QVariantList descs = itemMap.value("descriptors").toList();
    for (const QVariant& descVar : descs) {
        buildNode(descVar.toMap(), node);
    }
}

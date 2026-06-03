/**
 * @file BleGattModel.cpp
 * @brief GATT服务/特征树模型实现
 *
 * 实现完整的树形模型: GattNode内部数据结构管理，
 * index/parent/row/column/data/headerData全部实现。
 */

#include "connection/ble/BleGattModel.h"

BleGattModel::BleGattModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new GattNode())
{
    m_rootNode->name = QStringLiteral("GATT Root");
}

BleGattModel::~BleGattModel()
{
    delete m_rootNode;
}

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

int BleGattModel::rowCount(const QModelIndex& parent) const
{
    const GattNode* parentNode = parent.isValid()
        ? static_cast<GattNode*>(parent.internalPointer())
        : m_rootNode;
    return parentNode ? parentNode->children.size() : 0;
}

int BleGattModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColCount;
}

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

int BleGattModel::serviceCount() const
{
    return m_rootNode->children.size();
}

GattNode* BleGattModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }
    return static_cast<GattNode*>(index.internalPointer());
}

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

void BleGattModel::resetGattStatistics()
{
    m_totalServicesDiscovered = 0;
    m_totalCharacteristicsRead = 0;
    m_totalWrites = 0;
    m_errorCount = 0;
}

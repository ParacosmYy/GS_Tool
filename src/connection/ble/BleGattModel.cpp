/**
 * @file BleGattModel.cpp
 * @brief GATT服务/特征树模型 — 构造/析构/数据操作实现
 *
 * 包含构造/析构及模型数据操作(setServices/clear/buildNode)。
 * QAbstractItemModel核心接口见 BleGattModelIndex.cpp。
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

// QAbstractItemModel 核心接口见 BleGattModelIndex.cpp

// ============================================================
// 数据操作接口
// ============================================================

/** @brief 设置GATT服务数据，重建整个树模型 @param services 服务列表(QVariantList格式) */
void BleGattModel::setServices(const QVariantList& services)
{
    beginResetModel();
    ++m_totalModelResets;

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
    ++m_totalModelResets;
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
